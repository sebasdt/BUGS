#include "audio.h"
#include "main.h"
#include "menuHelper.h"
#include "sendDataHelper.h"
#include "taskHelper.h"

void setup() {
  //start serial comunication
  // Useful to see if the ESP32 crashes during setup
  Serial.begin(115200);
  delay(1000);//wait for a bit
  Serial.println("Beginning Setup");

  //setup and start the menu system.
  menuSetup();

  //We need to set up a WIFI connection between the Esp32s using the espNOW protocol.
  setupEspNOW();

  //We need to set up the audio chip used for FFT and some other varables.
  audioSetup();
  
  //print the state of espnow To the lcd screen.
  printStatus();

  do
  {
    if (slaveCount < 1)
    {
      printStatus();//print the state of espnow To the lcd screen.
      scanForSlaves();
      manageSlaves();
      delay(5000);
      Serial.println("Searching for screen");
    }
  } while (slaveCount < 1);

  Serial.println("Setup completed");


  // We need to set up independent tasks that could procces and send audio data.
  taskSetup();
}
int a = 0;
void loop() {
  if (slaveCount < 1)
  {
    delay(5000);
    printStatus();//print the state of espnow To the lcd screen.
    scanForSlaves();
    manageSlaves();
    Serial.println("Searching for screen");
  }else{ //delay(2000);Serial.print("stack left MALLOC_CAP_INTERNAL:"); Serial.println(heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    switch (a) {
        case 0:
          printSpectrum(false);// print the freq values coming from the fft chip
          break;
        case 1:
          printSpectrum(true);//print the freq values in the "system"
          break;
        case 2:
          printBeatBins();
          break;
        default:
          break;
      }
    }
}
