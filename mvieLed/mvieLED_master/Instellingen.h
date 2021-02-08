//Here below is what could be changed without breaking the code. 
//if 
#ifndef _Settings_
#define _Settings_

//defining the pins used for the rotery encoder.
#define EncoderPinA 34
#define EncoderPinB 25


//defining the pins used for the buttons.
#define PinS1 21
#define PinS2 22
#define encoderButton 23

//defining the pins used for the MSGEQ7 FFT chip.
#define StrobePin 26
#define ResetPin 27
#define AnalogReadPin 35
const int numFFTBins = 7;
int FreqVal[7];

//defining how many animations there're for ambiaent and music visualisation
const int numOfAmbAnimation = 11;
const int beginOfAmb =5;
const int endOfAmb =beginOfAmb +numOfAmbAnimation;

const int numOfMVAnimation = 11;
const int beginOfMV =15;
const int endOfMV =beginOfMV+numOfMVAnimation;













#endif
