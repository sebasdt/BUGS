//!!!!!!!!! keep away from all parameters in Main.h, please. these parameters below don't need to change look in Instelling.h for the settings!!!!!!!!!!!!!!!
#include "Instellingen.h"
#ifndef ESP32
#error ESP32 Only
#endif

#ifndef _main_
#define _main_


//  Includes go here

// Standard ESP32 WiFi
#include <WiFi.h>

//esp32 peer to peer cominucation
#include <esp_now.h>

//Display I2C cominucation for LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //adress, width en hight of the lcd screen.

//FastLED for some sertain timers.
#include <FastLED.h>

//arduino fft
#include <arduinoFFT.h>

//keeps the track of time
#include <elapsedMillis.h>

//for pulling the data from the msgeq7.
#include "AudioAnalyzer.h"

// Statistic.h library is used for music analytics
#include <Statistic.h>

//used for the readings from the rotery encoder
#include "NewEncoder.h"


// !!!!!!!!!keep away from all parameters in Main.h, please. these parameters below don't need to change look in Instelling.h for the settings!!!!!!!!!!!!!
typedef struct {
  const uint8_t S1;
  const uint8_t S2;
  const uint8_t encoder;
  volatile bool pressed;
  volatile bool pressed2;
  volatile bool encoderPressed;
} Button;
Button button = {PinS1, PinS2, encoderButton, false, false, false};

typedef struct {
  uint8_t gvlAudio; // parameter for the audio sensitivity (0-100)
  uint8_t Felheid;  //parameter for the brightness of the led matrix (0-255)
  uint8_t felheidP; // parameter for the shown percentage on the lcd  (0-100)
  uint8_t reactie;  // parameter for on which audio freq the led matrix should react to. (LOW, MID or HIGH range or 0, 1, 2)
  uint8_t Modus; // parameter for what mode the led matrix should be in (OFF, MUSIC, DEMO or 0, 1, 2)

  int16_t encoderValue; // parameter for the current encoder value (valu min and max is adusted between menu screens)
  uint8_t oldValue;      // parameter for the old encoder value
  uint8_t currentScreen; // parameter for the value of the current screen that is shown
  uint8_t oldCurrentScreen; // parameter for the old currentscreen value

  uint8_t inputHue; //parameter for the hue value (0 -255)
  uint8_t inputSat; //parameter for the saturation value  (0 -255)
  uint8_t inputSatP; //not used yet
  uint8_t inputVal; //parameter for the brightness value of the color (0 -255)
  uint8_t inputValP; //not used yet
  uint8_t DebugGeluid;
} Parameters;
Parameters parameters ={50, 255, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0};


bool Connected = false;
bool ChangeSettings = false;


const int mainMenuRange[] = {0, 1};  //set the number to where Main menu begins and ends 
const int musicMenuRange[] = {2, 5}; //set the number to where Music menu begins and ends 
const int demoMenuRange[] = {6, 7};  //set the number to where Demo menu begins and ends 

const char *settings[] = {"Kies Modus:", "Instellingen", "Reactie op:", "Gvlgheid audio:", "Felheid:", "Terug", "Felheid:", "Terug"};
const int numOfSettings = sizeof(settings) / sizeof(char *) - 1;

const char *Keuzes[] = {"Uit", "Muziek", "Demo"};
const int numOfKeuzes = sizeof(Keuzes) / sizeof(char *) - 1;
//int off = 0; //Set this number to where in the array UIT is placed.
int music =1; //Set this number to where in the array MUZIEK is placed.
int demo =2; //Set this number to where in the array DEMO is placed.


const char *tonen[] = {"Lage Tonen", "Middel Tonen", "Hoge Tonen"};
const int numOfTonen = sizeof(tonen) / sizeof(char *) - 1;


//define on which pins the FFT Chip is placed
Analyzer Audio = Analyzer(StrobePin, ResetPin, AnalogReadPin); //Strobe pin ->26  RST pin ->27 Analog Pin ->35



void handleEncoder(void *pvParameters);
void ESP_ISR callBack(NewEncoder &enc);

//  See README for meaning of constructor arguments. Use
// FULL_PULSE for encoders that produce one complete quadrature pulse per
// detnet, such as: https://www.adafruit.com/product/377 Use HALF_PULSE for
// endoders that produce one complete quadrature pulse for every two detents,
// such as: https://www.adafruit.com/product/377
NewEncoder *encoder1 = new NewEncoder(EncoderPinA, EncoderPinB, 0, 1, 0, FULL_PULSE);



QueueHandle_t encoderQueue;
QueueHandle_t buttonQueue;
QueueHandle_t dataQueue;

TaskHandle_t mMenuTaskHandler;
TaskHandle_t dMenuTaskHandler;
TaskHandle_t menuTaskHandler;



#define CHANNEL 3
#define NUM_SLAVES 3                   // ESP-Now can handle a maximum of 20 slaves
#define PRINTSCANRESULTS 0

int slaveCount = 0;                     // Keeps count of no. of slaves with the defined prefix
esp_now_peer_info_t slaves[NUM_SLAVES]; // Stores the information of each of the slave that is added as a peer

void initESPNow();
void manageSlaves();
void scanForSlaves();
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void sendData(); //to send the mode data



// Stores LED state
uint8_t currentBrightness;
int currentMode;
int currentPattern;
int inputAudioMultiplier;

int inputHue = 0;
int inputSat = 255;
int inputVal = 255;
int inputMasterBrightness;
int sendAnimation;
int numPattern = 5;





// Structure to send data
// Must match the receiver structure
typedef struct sendModestruct {
  char b[4];
  char Mode[4];
  int Value;
  char e[4];
} sendModestruct;

// Create a struct_message called sendMode
sendModestruct sendMode;

typedef struct {
  int Modus;
  int Brightness;
  int InputHue;
  int InputSat;
  int InputVal;
  int gvlAudio;
  int numPattern;
}  OldValueStruct;
//
// Create a struct_message called old
OldValueStruct old = {0, 51, 0, 0, 0,0,5}; //put some values in the int

// "mult" is the multiplier that can be adjusted in the menu system.
double mult = parameters.gvlAudio / 100;




// AUDIO DETECTION VARIABLES
// Number of FFT bins we are populating
// Raw FFT data
void printDetectedBeats(int i);
int32_t  spectrumValue[numFFTBins];
int32_t  spectrumValueOld[numFFTBins];
// Running averages in the FFT bins
double average[numFFTBins];
// Running standard deviations in the FFT bins
double stDev[numFFTBins];
// TODO: implement total volume data.
int volume, volumeAverage;
Statistic vol;
// fadeValues are used to control how fast LEDs fade that are colored by the FFT data directly.
// The higher the FFT data (the louder the sound is), the higher the fadeValue (the faster the light fades).
int fadeVals[numFFTBins];
// freq is an array of values that we can apply the 'Statistic' library to
Statistic freq[numFFTBins];
// beatDetected monitors whether a beat has been detected. 2 = new beat detected. 1 = old beat hasn't gone down yet. 0 = no beat detected
uint8_t beatDetected[numFFTBins] = {};
// BeatTimer measures the amount of time between detected beats
elapsedMillis beatTimer[numFFTBins] = {};
// BeatIntervals are where beatTimer data is stored
int beatInterval[numFFTBins];
int beatIntervalOld[numFFTBins];
// constantBeatCounter keeps track of how many beats have occurred with a constant time between them
int constantBeatCounter[numFFTBins];
// TODO: utilize the peaks of the beats to have more accurate peak detection and utilize for beat-dependent patterns
int beatPeak[numFFTBins];
int beatPeakOld[numFFTBins];
// The higher the activity in the FFT bin, the higher the score
int binScore[numFFTBins];
// Which FFT bins in the low frequencies, mid frequencies, and high frequencies have the the strongest beats within the song.
int lowBeatBin, midBeatBin, highBeatBin;
// Which FFT bins in the low frequencies, mid frequencies, and high frequencies have the highest averages
int lowAveBin, midAveBin, highAveBin;
// How long has silence been detected
elapsedMillis silenceDuration;
// How long has song been going on for. Useful for DJ sets that don't have silence between songs. This keeps the song data clearing every 5 mins
elapsedMillis songDuration;
bool silence = false;
// Constant beat detected variables
bool constBeat = false;
int maxConstBeat = 0;
int32_t  constBeatBin;
// mixAmount bounces between 0 and 255 depending on whether a strong beat is detected. 255 = beat detcted, 0 = no beat detected
int mixAmount;
//keep track of what the old mixAmount was.
int oldMixAmount;
//keep the last beat
int oldbeat;
////

////
QueueHandle_t mixer;
QueueHandle_t beatLow;
QueueHandle_t beatMid;
QueueHandle_t beatHigh;

#endif
