
#include "audio.h"
#include "main.h"
#include "menuHelper.h"
#include "sendDataHelper.h"
 
void taskSetup() {
//  xTaskCreatePinnedToCore( helpAudio  , "Help audio", 1200, NULL, 1, NULL, 1 ); //assigned to core 1
  xTaskCreatePinnedToCore( audioRead  , "Audio Read", 1200, NULL, 1, NULL, 1 ); //assigned to core 1
  xTaskCreatePinnedToCore( helpSendValues  , "help Send Values", 1300, NULL, 1, NULL, 1 ); //assigned to core 1
  xTaskCreatePinnedToCore(handleEncoder, "Handle Encoder", 800, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(menuTask,  "menu Task", 1200, NULL, 2, &menuTaskHandler, 1);
  xTaskCreatePinnedToCore(dMenuTask, "demo Menu Task", 1300, NULL, 2, &dMenuTaskHandler, 1);
  xTaskCreatePinnedToCore(mMenuTask, "music Menu Task", 1300, NULL, 2, &mMenuTaskHandler, 1);
}
