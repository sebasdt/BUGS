#include "Arduino.h"
#include "NewEncoder.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //adress, width en hight of the lcd screen.


#ifndef ESP32
#error ESP32 Only
#endif

void handleEncoder(void *pvParameters);
void ESP_ISR callBack(NewEncoder &enc);
QueueHandle_t encoderQueue;

void ESP_ISR callBack(NewEncoder &enc);


#define pinA 34
#define pinB 25
#define encoderButton 23
// This example uses Pins 25 & 26 for Encoder. Specify correct pins for your ESP32 / Encoder setup. See README for meaning of constructor arguments.
// Use FULL_PULSE for encoders that produce one complete quadrature pulse per detnet, such as: https://www.adafruit.com/product/377
// Use HALF_PULSE for endoders that produce one complete quadrature pulse for every two detents, such as: https://www.adafruit.com/product/377
NewEncoder *encoder1 = new NewEncoder(pinA, pinB, 0, 10, 0, FULL_PULSE);


QueueHandle_t buttonQueue;
QueueHandle_t dataQueue;


TaskHandle_t mMenuTaskHandler;
TaskHandle_t dMenuTaskHandler;
TaskHandle_t menuTaskHandler;
void setBoundaries(int newMin, int newMax);

/*
    setValue()
    getValue()
    upClick()
   downClick()
*/

const int numOfSettingsBox = 7  ;
String settings[numOfSettingsBox] = {{"Choose Mode:"}, {"Brightness:"}, {"Back"}, {"Brightness:"}, {"Sensitivity audio:"},  {"Reacts on:"},  {"Back"},
};

const int numOfkeuzes  = 3  ;
String Keuzes[numOfkeuzes] = {{"Off"}, {"Music"}, {"Demo"}
};

const int numOfTonen  = 3  ;
String tonen[numOfTonen] = {{"Low tones"}, {"Middle tones"}, {"High tones"},
};


typedef struct {
  uint8_t Modus;    //parameter for what mode the led matrix should be in (OFF, MUSIC or DEMO, or 0, 1, 2)

  int16_t encoderValue;     //parameter for the current encoder value (valu min and max is adusted between menu screens)
  uint8_t oldValue;         //parameter for the old encoder value
  uint8_t currentScreen;    //parameter for the value of the current screen that is shown
  uint8_t oldCurrentScreen; //parameter for the old currentscreen value
} Parameters;
Parameters parameters = { 0, 0, 0, 0, 0};

bool clickButton = false;
bool ChangeSettings = false;
portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;

volatile bool buttonPressed = false;
volatile unsigned long LastMicros;
void IRAM_ATTR isr() {
  portENTER_CRITICAL(&synch);
  //Serial.println("interupt called");
 if ((micros() - LastMicros) >= 25 * 1000UL) {
    // LastMicros = micros();
    if (digitalRead(encoderButton) == LOW) {
      buttonPressed = true;
      //   xQueueSendFromISR(buttonQueue, &buttonPressed, NULL);
    }
    LastMicros = micros();
  }
  portEXIT_CRITICAL(&synch);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Beginning menu Setup");
  Wire.begin(19, 18);
  lcd.init(); //lcd initiëren
  lcd.backlight(); //verlichting lcd aanzetten
  lcd.clear();
  lcd.print("WAKE UP YOU FOOL");
  delay(3000);

  // printStatus();

  pinMode(encoderButton, INPUT_PULLUP);
  attachInterrupt(encoderButton, isr, CHANGE);

  encoderQueue = xQueueCreate(1, sizeof(int16_t));


  buttonQueue = xQueueCreate ( 1, sizeof(int16_t) );
  dataQueue = xQueueCreate ( 1, sizeof(int16_t) );


  xTaskCreatePinnedToCore(handleEncoder, "Handle Encoder", 800, NULL, 3, NULL, 1);

  xTaskCreatePinnedToCore(menuTask, "menu Task", 10000, NULL, 2, &menuTaskHandler, 1);
  xTaskCreatePinnedToCore(mMenuTask, "music Menu Task", 10000, NULL, 1, &mMenuTaskHandler, 1);
  xTaskCreatePinnedToCore(dMenuTask, "demo Menu Task", 10000, NULL, 1, &dMenuTaskHandler, 1);
}


bool encoderButtonClick() {
  // bool button;
  // xQueueReceive(encoderQueue, &button, portMAX_DELAY);
  if (buttonPressed) {
    buttonPressed = false;
    if (!settings[parameters.currentScreen].equals("Terug")) {
      if (ChangeSettings) {
        lcd.noBlink();
        ChangeSettings = false;
      } else {
        lcd.blink();
        ChangeSettings = true;
      }

    }
    // return back;
  }
  return ChangeSettings;
}


void loop() {

  vTaskDelete(nullptr);
}
int oldMode;
bool submenu = false;
bool Access = true;
void menuTask(void *pvParameters) {

  for (;;) {
    // vTaskSuspend(dMenuTaskHandler);
    // vTaskSuspend(mMenuTaskHandler);
    //  encoderButtonClick();

    if (parameters.encoderValue != parameters.oldCurrentScreen && encoderButtonClick() && !submenu || Access) {
      Serial.println("im back");
      Access = false;// after setup update the lcd once after that it should only update when ChangeSettings is true
      if (ChangeSettings) setBoundaries(0, 2);
      parameters.oldCurrentScreen = parameters.encoderValue;
      parameters.Modus = parameters.encoderValue;
      lcd.clear();

      lcd.print("Choose Mode:");
      delay(10);
      lcd.setCursor(0, 1);
      lcd.print(Keuzes[parameters.Modus]);
      encoderButtonClick();
    }

    if (parameters.Modus != oldMode && parameters.Modus != 0 && !encoderButtonClick()) { //
      oldMode = parameters.Modus;
      switch (parameters.Modus) {
        case 0:

          break;
        case 1://stop all menu tasks exept for music settings menu
          vTaskResume(mMenuTaskHandler);
          vTaskSuspend(menuTaskHandler);
        //  setBoundaries(0, 2);
          break;
        case 2://stop all menu tasks exept for demo settings menu
          vTaskResume(dMenuTaskHandler);
          vTaskSuspend(menuTaskHandler);
       //  setBoundaries(0, 2);
          break;
      }
      //vTaskSuspend(menuTaskHandler);
    }

  }
  vTaskDelete(nullptr);
}



void dMenuTask(void *pvParameters) {
  vTaskSuspend(dMenuTaskHandler);
  for (;;) {
    submenu = true;
    ChangeSettings = false;
    Serial.println("demo settings menu activated");

    delay(1000);
    lcd.clear();
    lcd.print("Demo");
    setBoundaries(1, 2);
    delay(2000);
    
    submenu = false;
    Access = true;
    setBoundaries(0, 2);
    vTaskResume(menuTaskHandler);
    taskYIELD();
    
    Serial.println("oh no i need to stop");
    vTaskSuspend(dMenuTaskHandler);
  }
  vTaskDelete(nullptr);
}

void mMenuTask(void *pvParameters) {
  vTaskSuspend(mMenuTaskHandler);
  for (;;) {
    submenu = true;
    ChangeSettings = false;
    Serial.println(" music settings menu activated");
    
    delay(1000);
    lcd.clear();
    lcd.print("Music");
    setBoundaries(3, 6);
    delay(2000);
    
    submenu = false;
    Access = true;
    setBoundaries(0, 2);
    vTaskResume(menuTaskHandler);
    taskYIELD();
    
    Serial.println("oh no");
    vTaskSuspend(mMenuTaskHandler);

  }
  vTaskDelete(nullptr);
}






int oldMin;
int oldMax;
void setBoundaries(int newMin, int newMax) {
  if (newMin != oldMin || newMax != oldMax) {
    oldMin = newMin;
    oldMax = newMax;
    Serial.print("Setting new boundaries: "); Serial.print("New min: "); Serial.print(newMin); Serial.print("  New max: "); Serial.println(newMax);
    if (!encoder1->newSettings(newMin,  newMax)) { //when change has failed, check if newMin is lower than maxVal and newCurrent is between newMin and newMax.
      Serial.println(F("Change Failed. Check if newMin is lower than maxVal and maxVal higher than newMin. newCurrent values must be between these two values. Aborting."));
      Serial.println("");
      Serial.println("");
      Serial.println("");
      ESP.restart();
      // vTaskDelete(nullptr);
    }
    Serial.println(F("Boundary Changed Successfully"));
  }
}

void ESP_ISR callBack(NewEncoder & enc) {
  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
  static int16_t lastReading = 0;

  int16_t currentReading = enc;
  if (currentReading != lastReading) {
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
  }
  Serial.print(F("Encoder Successfully Started at value = "));
  Serial.println((int16_t) *encoder1);
  encoder1->attachCallback(callBack);

  for (;;) {
    xQueueReceive(encoderQueue, &encoderReading, portMAX_DELAY);
    parameters.encoderValue = encoderReading;
    if (!ChangeSettings) {
      parameters.currentScreen = parameters.encoderValue;
    }
    //  xQueueSend(dataQueue, &encoderReading, pdTRUE);
    //Serial.print("Encoder: ");
    Serial.println(encoderReading);
  }
  vTaskDelete(nullptr);
}
