#include "Arduino.h"
#include "main.h"

#ifndef _sendData_
#define _sendData_


//void printStruct();
void sendRequest(int req, int val);
void sendValues() ;

void sendMVMode(int req) {
  currentMode = req;
  sendAnimation = currentMode - 4;
  Serial.print("send Animation :");
  Serial.println(sendAnimation);
  strcpy(sendMode.b, "<");
  strcpy(sendMode.Mode, "m");
  sendMode.Value = sendAnimation;
  strcpy(sendMode.e, ">");
  sendData();
}


void sendAmbMode(int req) {
  currentMode = req;
  sendAnimation = currentMode - 16;
  Serial.print("send Animation :");
  Serial.println(sendAnimation);
  strcpy(sendMode.b, "<");
  strcpy(sendMode.Mode, "a");
  sendMode.Value = sendAnimation;
  strcpy(sendMode.e, ">");
  sendData();
}


void helpSendValues ( void *pvParameters ){for (;;){if (slaveCount > 0)sendValues();}vTaskDelete( NULL );}

bool screenOn;
void sendValues()  {
  if (parameters.Modus != old.Modus && !ChangeSettings) {
    old.Modus = parameters.Modus;
    Serial.println("Mode changed, sending new mode");
    switch (parameters.Modus) {
      case 0:
        if (screenOn) screenOn = false;
        sendRequest(0, NULL); //send the off command
        break;
      case 1://set the LED matrix to music mode
        if (!screenOn) {
          sendRequest(1, NULL);
          screenOn = true;
        }
        delay(5);
        sendRequest(33, NULL);
        break;
      case 2:
        if (!screenOn) {
          sendRequest(1, NULL);
          screenOn = true;
        }
        delay(5);
        sendRequest(32, NULL);
        break;
    }
  }

  if (numPattern != old.numPattern) {
    //Serial.println(numPattern);
    old.numPattern = numPattern;
    if (parameters.Modus == music) {
      if (numPattern > endOfMV) numPattern = beginOfMV;
      if (numPattern < beginOfMV) numPattern = endOfMV;
      // Serial.println(numPattern);
      sendRequest(numPattern, NULL);
    } else {
      if (numPattern > endOfAmb) numPattern = beginOfAmb;
      if (numPattern < beginOfAmb) numPattern = endOfAmb;

      // Serial.println(numPattern);
      sendRequest(numPattern, NULL);
    }
  }



  if (parameters.felheidP != old.Brightness && !ChangeSettings) {
    parameters.Felheid = map(parameters.felheidP, 0, 100, 0, 255);
    old.Brightness = parameters.felheidP;
    currentBrightness = parameters.Felheid;
    Serial.println("Brightness value changed, sending new value");
    sendRequest(30, NULL); //send the current brightness value
  }
  if (parameters.inputHue != old.InputHue && !ChangeSettings) {
    old.InputHue = parameters.inputHue;
    inputHue = parameters.inputHue;
    Serial.println("Hue value changed, sending new value");
    sendRequest(2, NULL);
  }
  if (parameters.inputSat != old.InputSat && !ChangeSettings) {
    old.InputSat = parameters.inputSat;
    inputSat = parameters.inputSat;
    Serial.println("Saturation value changed, sending new value");
    sendRequest(3, NULL);
  }
  if (parameters.inputVal != old.InputVal && !ChangeSettings) {
    old.InputVal = parameters.inputVal;
    inputVal = parameters.inputVal;
    Serial.println("Hue Brightness value changed, sending new value");
    sendRequest(4, NULL);
  }
  if (parameters.gvlAudio != old.gvlAudio && !ChangeSettings) {
    mult = map(parameters.gvlAudio, 0, 100, 0, 300);
    old.gvlAudio = parameters.gvlAudio;
  }

  if (mixAmount != oldMixAmount && parameters.Modus != 0 && !ChangeSettings ) {
    oldMixAmount = mixAmount;
    sendRequest(29, NULL);
  }

}



void sendRequest(int req, int val) {
  Serial.println("sending: ");
  switch (req) {
    case 0:
      Serial.println("OFF command");
      currentMode = req;//off command
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "f");
      sendMode.Value = 000;
      strcpy(sendMode.e, ">");
      //  printStruct();
      sendData();
      break;
    case 1: // on command
      Serial.println("ON command");
      currentMode = req;
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "o");
      sendMode.Value = 000;
      strcpy(sendMode.e, ">");
      //  printStruct();
      sendData();
      break;
    case 2: //send solidcolor and so a hue value
      Serial.println("Hue Value");
      currentMode = req;
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "s");
      sendMode.Value = inputHue;
      strcpy(sendMode.e, ">");
      //  printStruct();
      sendData();
      break;
    case 3:// Send the Saturation
      Serial.println("Saturation Value");
      currentMode = req;
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "t");
      sendMode.Value = inputSat;
      strcpy(sendMode.e, ">");
      //  printStruct();
      sendData();
      break;
    case 4: // Send the value of the set Brightness for hue
      Serial.println("Brightness hue Value");
      currentMode = req;
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "b");
      sendMode.Value = inputVal;
      strcpy(sendMode.e, ">");
      //  printStruct();
      sendData();
      break;
    case 5: //music parttern 1
      currentMode = req;
      sendMVMode(req);
      break;
    case 6://music parttern 2
      currentMode = req;
      sendMVMode(req);
      break;
    case 7://music parttern 3
      currentMode = req;
      sendMVMode(req);
      break;
    case 8://music parttern 4
      currentMode = req;
      sendMVMode(req);
      break;
    case 9://music parttern 5
      currentMode = req;
      sendMVMode(req);
      break;
    case 10://music parttern 6
      currentMode = req;
      sendMVMode(req);
      break;
    case 11://music parttern 7
      currentMode = req;
      sendMVMode(req);
      break;
    case 12://music parttern 8
      currentMode = req;
      sendMVMode(req);
      break;
    case 13://music parttern 9
      currentMode = req;
      sendMVMode(req);
      break;
    case 14://music parttern 10
      currentMode = req;
      sendMVMode(req);
      break;
    case 15://music parttern 11
      currentMode = req;
      sendMVMode(req);
      break;

    case 17://demo parttern 1
      currentMode = req;
      sendAmbMode(req);
      break;
    case 18://demo parttern 2
      currentMode = req;
      sendAmbMode(req);
      break;
    case 19://demo parttern 3
      currentMode = req;
      sendAmbMode(req);
      break;
    case 20://demo parttern 4
      currentMode = req;
      sendAmbMode(req);
      break;
    case 21://demo parttern 5
      currentMode = req;
      sendAmbMode(req);
      break;
    case 22://demo parttern 6
      currentMode = req;
      sendAmbMode(req);
      break;
    case 23://demo parttern 7
      currentMode = req;
      sendAmbMode(req);
      break;
    case 24://demo parttern 8
      currentMode = req;
      sendAmbMode(req);
      break;
    case 25://demo parttern 9
      currentMode = req;
      sendAmbMode(req);
      break;
    case 26://demo parttern 10
      currentMode = req;
      sendAmbMode(req);
      break;
    case 27://demo parttern 11
      currentMode = req;
      sendAmbMode(req);
      break;

    case 29: //send the mixAmount
      Serial.println("send the mixAmount ");
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "g");
      sendMode.Value = mixAmount;
      strcpy(sendMode.e, ">");
      sendData();
      break;

    case 30:  // Send the to the mothership brightnes
      Serial.println("mothership Brightness");
      //currentMode = req;
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "z");
      sendMode.Value = currentBrightness;
      strcpy(sendMode.e, ">");
      sendData();
      break;
    case 31:// case 98 makes the patterns automatically switching
      Serial.println("Brightness Value");
      currentMode = req;
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "x");
      sendMode.Value = 000;
      strcpy(sendMode.e, ">");
      sendData();
      break;
    case 32: // send and set the led matrix to Demo mode
      Serial.println("Set matrix to Demo mode");
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "d");
      sendMode.Value = 000;
      strcpy(sendMode.e, ">");
      sendData();
      break;
    case 33: //send and set the led matrix to Music mode
      Serial.println("Set matrix to Music mode");
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "c");
      sendMode.Value = 000;
      strcpy(sendMode.e, ">");
      sendData();
      break;

    case 34: //send and set the led matrix to Music mode
      Serial.println("send audio data: LOW Freq");
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "L");
      sendMode.Value = val;
      strcpy(sendMode.e, ">");
      sendData();
      break;
    case 35: //send and set the led matrix to Music mode
      Serial.println("send audio data: MID Freq");
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "M");
      sendMode.Value = val;
      strcpy(sendMode.e, ">");
      sendData();
      break;
    case 36: //send and set the led matrix to Music mode
      Serial.println("send audio data: HIGH Freq");
      strcpy(sendMode.b, "<");
      strcpy(sendMode.Mode, "H");
      sendMode.Value = val;
      strcpy(sendMode.e, ">");
      sendData();
      break;
  }
}


void setupEspNOW() {
  Serial.println("Beginning EspNOW Setup");

  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);

  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: ");  Serial.println(WiFi.macAddress());

  // Init ESPNow with a fallback logic
  initESPNow();

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(onDataSent);

  scanForSlaves();
  manageSlaves();
  Serial.println("EspNOW Setup completed");
}


void initESPNow()
{
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK)
  {
    Serial.println("ESPNow Init Success");
  }
  else
  {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}
// Scan for slaves in AP mode
void scanForSlaves()
{
  int8_t scanResults = WiFi.scanNetworks();
  //printStatus();
  //reset slaves
  memset(slaves, 0, sizeof(slaves));
  slaveCount = 0;

  Serial.println("");

  if (scanResults == 0)
  {
    Serial.println("No WiFi devices in AP Mode found");
  }
  else
  {
    Serial.print("Found ");
    Serial.print(scanResults);
    Serial.println(" devices ");

    for (int i = 0; i < scanResults; i++)
    {
      // Print SSID and RSSI for each device found
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      if (PRINTSCANRESULTS)
      {
        Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
      }

      delay(10);

      // Check if the current device starts with `Slave`
      if (SSID.indexOf("Screen") == 0)
      {
        // SSID of interest
        Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");

        // Get BSSID => Mac Address of the Slave
        int mac[6];

        if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
        {
          for (int j = 0; j < 6; j++)
          {
            slaves[slaveCount].peer_addr[j] = (uint8_t)mac[j];
          }
        }

        slaves[slaveCount].channel = CHANNEL; // pick a channel
        slaves[slaveCount].encrypt = 0; // no encryption
        slaveCount++;
      }
    }
  }

  if (slaveCount > 0)
  {
    Serial.print(slaveCount); Serial.println(" Screen(s) found, processing..");
  }
  else
  {
    Serial.println("No Screen Found, trying again.");
  }

  // clean up ram
  WiFi.scanDelete();
}

// Check if the slave is already paired with the master.
// If not, pair the slave with master
void manageSlaves()
{
  //printStatus();
  if (slaveCount > 0)
  {
    for (int i = 0; i < slaveCount; i++)
    {
      esp_now_peer_info_t *peer = &slaves[i];
      const uint8_t *peer_addr = slaves[i].peer_addr;
      Serial.print("Processing: ");

      for (int j = 0; j < 6; j++)
      {
        Serial.print((uint8_t) slaves[i].peer_addr[j], HEX);
        if (j != 5)
        {
          Serial.print(":");
        }
      }
      Serial.print(" Status: ");
      // check if the peer exists
      bool exists = esp_now_is_peer_exist(peer_addr);
      if (exists)
      {
        Connected = true;
        // Slave already paired.
        Serial.println("Already Paired");
      }
      else
      {
        // Slave not paired, attempt pair
        esp_err_t addStatus = esp_now_add_peer(peer);
        if (addStatus == ESP_OK)
        {
          // Pair success
          Connected = true;
          Serial.println("Pair success");
        }
        else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT)
        {
          // How did we get so far!!
          Serial.println("ESPNOW Not Init");
        }
        else if (addStatus == ESP_ERR_ESPNOW_ARG)
        {
          Serial.println("Add Peer - Invalid Argument");
        }
        else if (addStatus == ESP_ERR_ESPNOW_FULL)
        {
          Serial.println("Peer list full");
        }
        else if (addStatus == ESP_ERR_ESPNOW_NO_MEM)
        {
          Serial.println("Out of memory");
        }
        else if (addStatus == ESP_ERR_ESPNOW_EXIST)
        {
          Serial.println("Peer Exists");
        }
        else
        {
          Serial.println("Not sure what happened");
        }
        delay(1000);
      }
    }
  }
  else
  {
    // No slave found to process
    Connected = false;
    Serial.println("No Screen found to process");
  }
}

void printStruct()
{
  // Serial.println("printing values");
  // Serial.println(sizeof(sendModestruct));
  //Serial.print("char 1: ");
  Serial.print(sendMode.b);
  //  Serial.print("char 2: ");
  Serial.print(sendMode.Mode);
  // Serial.print("int: ");
  Serial.print(sendMode.Value);
  // Serial.print("char 3: ");
  Serial.println(sendMode.e);
  Serial.println("");
  // Serial.println("done printing values");
}


bool clearToSend = false;
// send data
void sendData() {
 // if (!clearToSend) {
   // clearToSend = false;
    for (int i = 0; i < slaveCount; i++) {
      const uint8_t *peer_addr = slaves[i].peer_addr;
      /* if (i == 0) {Serial.println("Sending ");printStruct();   }*/// print only for first slave

      esp_err_t result = esp_now_send(peer_addr, (uint8_t *) &sendMode, sizeof(sendMode));
      Serial.print("Send Status: ");
      if (result == ESP_OK) {
        Serial.println("Success");
      } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW not Init.");
      } else if (result == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
      } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
        Serial.println("Internal Error");
      } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("ESP_ERR_ESPNOW_NO_MEM");
      } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
        Serial.println("Peer not found.");
      } else {
        Serial.println("Not sure what happened");
      }
      // delay(10);
    }
  //} else {
 //   Serial.println("not ready to send");
 // }
}



// callback when data is sent from Master to Slave
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  char macStr[18];
  // sendTime = millis();
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
//  if (status != ESP_NOW_SEND_SUCCESS) {
 ////   clearToSend = true;
 /// } else clearToSend = false; *
  //  Serial.print("Last Packet Sent to: ");
  // Serial.println(macStr);
  // Serial.print("Last Packet Send Status: ");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
#endif
