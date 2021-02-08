#include "main.h"
#include "audio.h"

#ifndef _menuSystem_
#define _menuSystem_

bool comingthrough = false;
bool submenu = false;
bool getOUT = false;
bool firstChange = false;
bool secondChange = false;
bool encoderDisable = false;

void setBoundaries(int newMin, int newMax);
void printStatus();
void showModeLCD();

portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE synch2 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE synch3 = portMUX_INITIALIZER_UNLOCKED;

volatile unsigned long LastMicros;
void IRAM_ATTR isr() {
  portENTER_CRITICAL(&synch);
  if ((micros() - LastMicros) >= 25 * 1000UL) {
    if (digitalRead(button.encoder) == LOW  && !button.encoderPressed && !encoderDisable && Connected) {
      button.encoderPressed = true;
    }
    LastMicros = micros();
  }
  portEXIT_CRITICAL(&synch);
}

void IRAM_ATTR isr2() {
  portENTER_CRITICAL(&synch2);
  static volatile uint32_t lastMillis = 0;
  if ((micros() - lastMillis) >= 25 * 1000UL && !button.pressed && !encoderDisable && Connected) {
    if (digitalRead(button.S1) == LOW) {
      lastMillis = micros();
      button.pressed = true;
      //  Serial.println("Interrupt 1 CALLED!");
    }
  }
  portEXIT_CRITICAL(&synch2);
}
void IRAM_ATTR isr3() {
  portENTER_CRITICAL(&synch3);
  static volatile uint32_t lastMillis = 0 ;
  if ((micros() - lastMillis) >= 25 * 1000UL &&  !button.pressed2 && !encoderDisable && Connected) {
    if (digitalRead(button.S2) == LOW) {
      lastMillis = micros();
      button.pressed2 = true;
      // Serial.println("Interrupt 2 CALLED!");
    }
  }
  portEXIT_CRITICAL(&synch3);
}


void menuSetup() {
  encoderDisable = true;
  Serial.println("Beginning menu Setup");
  Wire.begin(19, 18);
  lcd.init(); //lcd initiëren
  lcd.backlight(); //verlichting lcd aanzetten
  lcd.clear();
  lcd.print("Hamster wakker");
  lcd.setCursor(0, 1);
  lcd.print("maken.");
  delay(2000);

  printStatus();

  pinMode(button.S1, INPUT_PULLUP);
  pinMode(button.S2, INPUT_PULLUP);
  pinMode(button.encoder, INPUT_PULLUP);
  attachInterrupt(button.S1, isr2, CHANGE);
  attachInterrupt(button.S2, isr3, CHANGE);
  attachInterrupt(button.encoder, isr, CHANGE);

  encoderQueue = xQueueCreate(1, sizeof(int16_t));
  buttonQueue = xQueueCreate(1, sizeof(int16_t));
  dataQueue = xQueueCreate(1, sizeof(int16_t));



  lcd.clear();
  lcd.print("Hamster is");
  lcd.setCursor(0, 1);
  lcd.print("wakker.");
  delay(2000);
  Serial.println("menu Setup completed");
}
void printStatus() {
//  Serial.println("print status");
  if (!Connected) {
    lcd.clear();
    lcd.print("Zoeken naar");
    lcd.setCursor(0, 1);
    lcd.print("het Scherm.");
  } else {
    lcd.clear();
    lcd.print("Het Scherm");
    lcd.setCursor(0, 1);
    lcd.print("is gevonden!");
    delay(3000);
  }
}

void showModeLCD() {
  switch (parameters.Modus) {
    case 0:
      lcd.setCursor(15, 1);
      lcd.print("U");
      break;
    case 1:
      lcd.setCursor(15, 1);
      lcd.print("M");
      break;
    case 2:
      lcd.setCursor(15, 1);
      lcd.print("D");
      break;
  }
}

bool checkButtons() {
  bool Access = false;
  if (button.pressed) {
    numPattern--;
    delay(10);
    Serial.println(numPattern);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Vorige animatie");
    lcd.setCursor(0, 1);
    lcd.print("Animatie nr: ");
    lcd.print(numPattern);
    button.pressed = false;
    delay(990);
    Access = true;
  } else  if (button.pressed2) {
    numPattern++;
    delay(10);
    Serial.println(numPattern);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Vlgnde animatie");
    lcd.setCursor(0, 1);
    lcd.print("Animatie nr: ");
    lcd.print(numPattern);
    button.pressed2 = false;
    delay(990);
    Access = true;

  }
  return Access;
}

void encoderButtonClick() {
  if (button.encoderPressed && !encoderDisable && !comingthrough) {
    button.encoderPressed = false;
    if (strcmp(settings[parameters.currentScreen], "Instellingen")) {
      if (strcmp(settings[parameters.currentScreen], "Terug")) { // strcmp() returns 0 if match
        if (ChangeSettings) {
          lcd.noBlink();
          secondChange = true;
          firstChange = false;
          ChangeSettings = false;
        } else {
          lcd.blink();
          firstChange = true;
          secondChange   = false;
          ChangeSettings = true;
        }
      } else if (submenu)
        getOUT = true;
    } else if (!submenu)
      comingthrough = true;
  }
}



void menuTask(void *pvParameters) {
  int oldMode;
  bool firstEntery = true;
  delay(1000);
  for (;;) {
   if (Connected){
    if (firstEntery) {
      //  firstEntery = false;
      Serial.println("main menu reactivated changing things");
      ChangeSettings = false;
      encoderDisable = false;
      setBoundaries(mainMenuRange[0], mainMenuRange[1]);
      Serial.println(encoder1->setValue(mainMenuRange[0]));
      parameters.currentScreen = mainMenuRange[0];
    }
   EVERY_N_SECONDS(2){ int val = uxTaskGetStackHighWaterMark(NULL); Serial.print("stack left for main menu:"); Serial.println(val);}//print the unused asigned memory, this must be between 50 and 300. lower the better.
    encoderButtonClick();
    if (parameters.encoderValue != parameters.oldCurrentScreen && !submenu || firstEntery || firstChange || secondChange || checkButtons()) {
      parameters.oldCurrentScreen = parameters.encoderValue;
      if (firstEntery) firstEntery = false;
      lcd.clear();
      lcd.print(settings[parameters.currentScreen]);
      showModeLCD();
      if (!strcmp(settings[parameters.currentScreen], "Kies Modus:")) { // strcmp() returns 0 if match
        if (ChangeSettings) {if (firstChange && ChangeSettings && !secondChange) {setBoundaries(0, numOfKeuzes);parameters.encoderValue = parameters.Modus;encoder1->setValue(parameters.Modus);firstChange = false;} parameters.Modus = parameters.encoderValue;
        } else {setBoundaries(mainMenuRange[0], mainMenuRange[1]);if (secondChange && !ChangeSettings && !firstChange) {encoder1->setValue(parameters.currentScreen);secondChange = false;}}
        lcd.setCursor(0, 1);
        lcd.print(Keuzes[parameters.Modus]);

      }else if (!strcmp(settings[parameters.currentScreen], "Instellingen")) { // strcmp() returns 0 if match
        encoderButtonClick();
      }
    }
    if ((parameters.Modus != oldMode && !ChangeSettings) || comingthrough) {
      oldMode = parameters.Modus;
      comingthrough = false;
      switch (parameters.Modus) {
        case 0:
          encoder1->setValue(mainMenuRange[0]);
          parameters.currentScreen = mainMenuRange[0];
          break;
        case 1: // Suspend main menu task and resumes music menu task
          vTaskResume(mMenuTaskHandler);
          vTaskSuspend(menuTaskHandler);
          delay(2);
          Serial.println("");
          Serial.println("im back from music");
          firstEntery = true;
          ChangeSettings = false;
          encoderDisable = true;

          break;
        case 2: // Suspend main menu task and resumes demo menu task
          vTaskResume(dMenuTaskHandler);
          vTaskSuspend(menuTaskHandler);
          delay(2);
          Serial.println("");
          Serial.println("im back from demo");
          firstEntery = true;
          ChangeSettings = false;
          encoderDisable = true;
          break;
      }
    }
  }
 }
  vTaskDelete(nullptr);
}

void dMenuTask(void *pvParameters) {
  bool firstEntery = true;
  vTaskSuspend(dMenuTaskHandler);
  for (;;) {

    if (firstEntery) {
      Serial.println("");
      Serial.println("demo settings menu activated changing things");
      submenu = true;
      ChangeSettings = false;
      encoderDisable = false;
      setBoundaries(demoMenuRange[0], demoMenuRange[1]);
      Serial.println( encoder1->setValue(demoMenuRange[0]));
      parameters.currentScreen = demoMenuRange[0];
    }
EVERY_N_SECONDS(2){ int val = uxTaskGetStackHighWaterMark(NULL); Serial.print("stack left for Demo menu:"); Serial.println(val);}//print the unused asigned memory, this must be between 50 and 300. how lower the better.
    encoderButtonClick();
    if ((parameters.encoderValue != parameters.oldCurrentScreen && submenu) || firstEntery || getOUT || firstChange || secondChange || checkButtons()) {
      if (firstEntery) firstEntery = false;
      parameters.oldCurrentScreen = parameters.encoderValue;
      lcd.clear();
      lcd.print(settings[parameters.currentScreen]);
      showModeLCD();
      if (!strcmp(settings[parameters.currentScreen], "Felheid:")) { // strcmp() returns 0 if match
        if (ChangeSettings) {if (firstChange && ChangeSettings && !secondChange) {parameters.encoderValue = parameters.felheidP;setBoundaries(0, 100);encoder1->setValue(parameters.felheidP);firstChange = false;} parameters.felheidP = parameters.encoderValue;
        } else {setBoundaries(demoMenuRange[0], demoMenuRange[1]);if (secondChange && !ChangeSettings && !firstChange) {encoder1->setValue(parameters.currentScreen);secondChange = false;}}
        lcd.setCursor(0, 1);
        lcd.print(parameters.felheidP);
        lcd.print("%");

      } else if (!strcmp(settings[parameters.currentScreen], "Terug") && getOUT) { // strcmp() returns 0A if match
        encoderDisable = true;
        ChangeSettings = false;
        getOUT = false;
        submenu = false;
        vTaskResume(menuTaskHandler);
        // taskYIELD();
        Serial.println("stop demo menu");
        vTaskSuspend(dMenuTaskHandler);
        firstEntery = true;
      }
    }
  }
  vTaskDelete(nullptr);
}


void mMenuTask(void *pvParameters) {
  bool firstEntery = true;
  vTaskSuspend(mMenuTaskHandler);
  for (;;) {
    if (firstEntery) {
      Serial.println("");
      Serial.println("music settings menu activated changing things");
      ChangeSettings = false;
      encoderDisable = false;
      submenu = true;
      Serial.println( encoder1->setValue(musicMenuRange[0]));
      parameters.currentScreen = musicMenuRange[0];
      setBoundaries(musicMenuRange[0], musicMenuRange[1]);
    }
    EVERY_N_SECONDS(2){ int val = uxTaskGetStackHighWaterMark(NULL); Serial.print("stack left for music menu:"); Serial.println(val);}//print the unused asigned memory, this must be between 50 and 300. how lower the better.
    encoderButtonClick();
    if ((parameters.encoderValue != parameters.oldCurrentScreen && submenu) || firstEntery || getOUT || firstChange || secondChange || checkButtons()) {
      parameters.oldCurrentScreen = parameters.encoderValue;
      if (firstEntery) firstEntery = false;

      lcd.clear();
      lcd.print(settings[parameters.currentScreen]);
      showModeLCD();
      if (!strcmp(settings[parameters.currentScreen], "Felheid:")) { // strcmp() returns 0 if match
        if (ChangeSettings) {if (firstChange && ChangeSettings && !secondChange) {parameters.encoderValue = parameters.felheidP;setBoundaries(0, 100);encoder1->setValue(parameters.felheidP);firstChange = false;} parameters.felheidP = parameters.encoderValue;
        } else {setBoundaries(musicMenuRange[0], musicMenuRange[1]);if (secondChange && !ChangeSettings && !firstChange) {encoder1->setValue(parameters.currentScreen);secondChange = false;}}
        lcd.setCursor(0, 1);
        lcd.print(parameters.felheidP);
        lcd.print("%");

      } else if (!strcmp(settings[parameters.currentScreen], "Gvlgheid audio:")) { // strcmp() returns 0 if match
        if (ChangeSettings) {if (firstChange && ChangeSettings && !secondChange) {parameters.encoderValue = parameters.gvlAudio;setBoundaries(0, 100);encoder1->setValue(parameters.gvlAudio);firstChange = false;} parameters.gvlAudio = parameters.encoderValue;
        } else {setBoundaries(musicMenuRange[0], musicMenuRange[1]);if (secondChange && !ChangeSettings && !firstChange) {encoder1->setValue(parameters.currentScreen);secondChange = false;}}
        lcd.setCursor(0, 1);
        lcd.print(parameters.gvlAudio);
        lcd.print("%");

      } else if (!strcmp(settings[parameters.currentScreen], "Reactie op:")) { // strcmp() returns 0 if match
        if (ChangeSettings) {if (firstChange && ChangeSettings && !secondChange) {parameters.encoderValue = parameters.reactie;setBoundaries(0, numOfTonen);encoder1->setValue(parameters.reactie);firstChange = false;} parameters.reactie = parameters.encoderValue;
        } else {setBoundaries(musicMenuRange[0], musicMenuRange[1]);if (secondChange && !ChangeSettings && !firstChange) {encoder1->setValue(parameters.currentScreen);secondChange = false;}}
        lcd.setCursor(0, 1);
        lcd.print(tonen[parameters.reactie]);

      } else if (!strcmp(settings[parameters.currentScreen], "Terug") && getOUT) { // strcmp() returns 0A if match
        encoderDisable = true;
        getOUT = false;
        ChangeSettings = false;
        submenu = false;
        vTaskResume(menuTaskHandler);
        // taskYIELD();
        Serial.println("stop music menu");
        vTaskSuspend(mMenuTaskHandler);
        firstEntery = true;
      }
    }
  }
  vTaskDelete(nullptr);
}

int oldMin;
int oldMax;
void setBoundaries(int newMin, int newMax) {
  if (newMin != oldMin || newMax != oldMax) {
    oldMin = newMin;
    oldMax = newMax;
    Serial.print("Setting new boundaries: ");
    Serial.print("New min: ");
    Serial.print(newMin);
    Serial.print("  New max: ");
    Serial.println(newMax);
    if (!encoder1->newSettings(newMin, newMax)) { // when change has failed, check if newMin is lower than maxVal and newCurrent is between newMin and newMax.
      Serial.println(F("Change Failed. Check if newMin is lower than maxVal and maxVal higher than newMin. newCurrent values must be between these two values. Aborting."));
      Serial.println("");
      Serial.println("");
      Serial.println("");
      ESP.restart();
      // vTaskDelete(nullptr);
    }
    Serial.println(F("Boundary Changed Successfully"));
  } // Serial.println(F("Boundary No Need To Be Changed"));
}

void ESP_ISR callBack(NewEncoder &enc) {
  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
  static int16_t lastReading = 0;

  int16_t currentReading = enc;
  if (currentReading != lastReading && !encoderDisable) {
    lastReading = currentReading;
    xQueueSendToBackFromISR(encoderQueue, &currentReading, &pxHigherPriorityTaskWoken);
    if (pxHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
  }
}

void handleEncoder(void *pvParameters) {
  int16_t encoderReading;

  if (!encoder1->begin()) {
    Serial.println(F("Encoder Failed to Start. Check pin assignments and available interrupts. Aborting."));
    vTaskDelete(nullptr);
    ESP.restart();
  }
  Serial.print(F("Encoder Successfully Started at value = "));
  Serial.println((int16_t)*encoder1);
  encoder1->attachCallback(callBack);

  for (;;) {
    //EVERY_N_SECONDS(2){ int val = uxTaskGetStackHighWaterMark(NULL); Serial.print("stack left for encoder:"); Serial.println(val);}//print the unused asigned memory, this must be between 50 and 300. how lower the better.
    xQueueReceive(encoderQueue, &encoderReading, portMAX_DELAY);
    parameters.encoderValue = encoderReading;
    if (!ChangeSettings) {
      parameters.currentScreen = parameters.encoderValue;
    }
    //  xQueueSend(dataQueue, &encoderReading, pdTRUE);
    // Serial.print("Encoder: ");
    // Serial.println(encoderReading);
  }
  vTaskDelete(nullptr);
}



#endif
