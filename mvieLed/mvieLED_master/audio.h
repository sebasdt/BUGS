#include "main.h"
#include "sendDataHelper.h"
#ifndef _audio_
#define _audio_


void audioSetup() {
  // TODO: add way for input to be chosen either by button or over WiFi somehow.
  // Input can only be changed when Teensy is turned on / rebooted.
  delay(2000);

  // AUDIO STUFF
  Audio.Init(); // start the audio analyzer
  ////
  mixer = xQueueCreate ( 1, sizeof(mixAmount) );
  beatLow = xQueueCreate ( 1, sizeof(beatDetected[lowBeatBin]) );
  beatMid = xQueueCreate ( 1, sizeof(beatDetected[midBeatBin]) );
  beatHigh = xQueueCreate ( 1, sizeof(beatDetected[highBeatBin]) );

}

// Clears the running mean and st dev. Used mostly between songs when silence is detected.
void clearStats() {
  for (int i = 0; i < numFFTBins; i++) {
    freq[i].clear();
    beatPeak[i] = 0;
    binScore[i] = 0;
  }
  lowBeatBin = 2;
  midBeatBin = 3;
  highBeatBin = 5;

}

void detectSilence() {
  uint8_t silenceCounter = 0;
  // If FFT bin data is very low, consider it as silence in the bin
  // reset the statistics because it's likely the song ended
  for (int i = 0; i < numFFTBins; i++) {
    if (spectrumValue[i] < 100) { // 10 is an arbitrary number but it means it is basically silent. Some audio sources have noise in higher FFT bins.
      silenceCounter++;
    }
    else {
      break; // stop counting silent FFT bins
    }
  }
  // If the song is 5 minutes long, reset stats.
  if (songDuration > 300000) {
    Serial.println("reset Stats");
    songDuration = 0;
    clearStats();
  }

  // If we've counted over 11 bins that were silent, then clear stats, set silence = true, and set songDuration = 0.
  if (silenceCounter > 3) {
    // Serial.println("its silent here");
    clearStats();
    silence = true;
    songDuration = 0;
  }
  else { // else (if silence isn't detected), set silence = false, silenceDuration = 0
    silence = false;
    silenceDuration = 0;
  }
}

void fillStats() {
  // Save old FFT data in spectrumValueOld
  for (int i = 0; i < numFFTBins; i++)
    spectrumValueOld[i] = spectrumValue[i];
  Audio.ReadFreq(FreqVal);
  //Serial.println(mult);

  spectrumValue[0] = mult * FreqVal[0];// * 1000; // multiply by 1000 to make them integers
  spectrumValue[1] = mult * FreqVal[1];// * 1000;
  spectrumValue[2] = mult * FreqVal[2];// * 1000;
  spectrumValue[3] = mult * FreqVal[3];// * 1000;
  spectrumValue[4] = mult * FreqVal[4];// * 1000;
  spectrumValue[5] = mult * FreqVal[5];// * 1000;
  spectrumValue[6] = mult * FreqVal[6];// * 1000;


  // reset volume variable
  volume = 0;
  // Update the average and standard deviation of each FFT bin value
  for (int i = 0; i < numFFTBins; i++) {
    freq[i].add(spectrumValue[i]);
    average[i] = freq[i].average();
    stDev[i] = freq[i].pop_stdev();
    volume = volume + spectrumValue[i]; // add all the values to find the volume
  }

  // See if there is silence
  detectSilence();
}

// Measure the time between beats
void beatTiming(int i) {
  if (beatDetected[i] == 2) { // if a new beat has been detected
    if (beatTimer[i] < 200) // but it is less than 200ms from the last one
      beatDetected[i] = 1; // cancel the beat
    else { // if it's more than 200ms from the last one
      beatInterval[i] = beatTimer[i];  // A real beat has been detected. Save the time since last beat.
      // TODO: could use absolute value below (Wasn't working for unknown reason)
      if ((beatIntervalOld[i] - beatInterval[i]) < 30 && (beatIntervalOld[i] - beatInterval[i]) > -30) // If the time between the new interval and old interval is less than 30 ms
        constantBeatCounter[i]++;

      beatTimer[i] = 0; // reset beat timer
      beatIntervalOld[i] = beatInterval[i]; // save new beat interval as old beat interval
    }
  }
  if (constantBeatCounter[i] > 0 && beatTimer[i] > beatInterval[i] + 50) constantBeatCounter[i] = 0; // clears beat counter when more time than the beat has passed

  // These print statements will print the constant beat counter of each bin
  /* if (i == numFFTBins - 1) Serial.println(constantBeatCounter[i]);
    else {
    Serial.print(constantBeatCounter[i]);
     Serial.print("\t");
    }*/

  // NOTE: remember this beatTiming function is run within a for loop in musicAnalytics()
  if ( i < 3) { // for the 4 lowest frequency FFT bins
    if (constantBeatCounter[i] > maxConstBeat) { // if this FFT bin's constantBeatCounter is greater than the current highest constant beat detected (within 4 lowest bins)...
      maxConstBeat = constantBeatCounter[i]; // the new maxConstBeat is this bin's constantBeatCounter
      constBeatBin = i; // This bin currently has the maxConstBeat
    }
  }
  if (maxConstBeat > 10) // If we've reached over 10 beats with a constant time interval...
    constBeat = true; // Go in to a constBeat pattern ( used in normalOperations() )
  else
    constBeat = false;
}

void beatDetection(int i) {
  // 0 = no beat detected
  // 1 = old beat hasn't dropped / reset yet
  // 2 = new beat detected
  // Peak detection is performed here. FFT value must be greater than the average+(2.3*st.dev) and greater than a threshold of 100, which is pretty quiet
  if (spectrumValue[i] > average[i] + 2.3 * stDev[i] && spectrumValue[i] > 200) {
    if (beatDetected[i] == 2) // If it's already 2, then the beat has already been detected...
      beatDetected[i] = 1; // so drop the value to 1.
    if (beatDetected[i] == 0) // If 0, then this is a new beat...
      beatDetected[i] = 2; // so make the value 2.
    // if it's == 1, it stays 1. no code is needed.
  }
  // This is where 1's get reset to 0. This prevents multiple beats being triggered from 1 beat, or when they aren't well defined
  else { // if beat is not detected...
    if (beatDetected[i] == 1) { // and it's value is 1...
      if (spectrumValue[i] <= average[i]) // and it has dropped below the running average of that FFT bin...
        beatDetected[i] = 0; // reset to 0
    }
  }
}

// scoreBins figures out which FFT bins have the best beats to visualize.
void scoreBins(int i) {
  // If there's a constant beat above 5 counts, increase score by 2
  if (constantBeatCounter[i] > 5)
    binScore[i] += 2;

  // If there's a beat detected, increase score by 1
  if (beatDetected[i] == 2)
    binScore[i] += 1;

  // If an FFT bin's score is high and it doesn't have a constant beat, lower that score
  if (binScore[i] > 300 && constantBeatCounter[i] < 2)
    binScore[i] -= 2;

  ///   These print statements will print the score of each bin
  /*if (i == numFFTBins - 1)
     Serial.println(binScores[i]);
    else {
     Serial.print(binScores[i]);
      Serial.print("\t");
    }*/
}

void musicAnalytics() {
  // Write FFT data to spectrumValue[ ] array
  fillStats();

  maxConstBeat = 0; // reset this before it is set within beatTiming().
  // All the deep analytics happen from the functions in this for loop
  for (int i = 0; i < numFFTBins; i++) {

    beatDetection(i);
    beatTiming(i);
    //rintDetectedBeats(i); // diagnostic function, uncomment to see if beats are being detected.
    scoreBins(i);

    // Fill Fade Values
    if (spectrumValue[i] > average[i])
      fadeVals[i] += (spectrumValue[i] - average[i]) / 2;
    else
      //fadeVals[i] -= (average[i] - spectrumValue[i]) / 2;
      fadeVals[i] -= 5;

    fadeVals[i] = constrain(fadeVals[i], 0, 255);
  }

  // This for loop checks the lowest 4 bins, mid 4 bins, high 4 bins. (Some bins are not counted b/c they're in between. Might need fixing)

  // Find the beats in the low, mid, and high ranges
  if (binScore[0] > binScore[lowBeatBin]) lowBeatBin = 0;
  if (binScore[1] > binScore[lowBeatBin]) lowBeatBin = 1;
  if (binScore[2] > binScore[lowBeatBin]) lowBeatBin = 2;

  if (binScore[3] > binScore[midBeatBin]) midBeatBin = 3;
  if (binScore[4] > binScore[midBeatBin]) midBeatBin = 4;

  if (binScore[5] > binScore[highBeatBin])highBeatBin = 5;
  if (binScore[6] > binScore[highBeatBin])highBeatBin = 6;

  // Find highest averages in the low, mid, and high ranges
  if (average[0] > average[lowBeatBin]) lowAveBin = 0;
  if (average[1] > average[lowBeatBin]) lowAveBin = 1;
  if (average[2] > average[lowBeatBin]) lowAveBin = 2;

  if (average[3] > average[midAveBin]) midAveBin = 3;
  if (average[4] > average[midAveBin]) midAveBin = 4;

  if (average[5] > average[highAveBin])highAveBin = 5;
  if (average[6] > average[highAveBin])highAveBin = 6;

  // This local variable is what moves the mixAmount variable
  static int mixAmountInfluencer;

  // If a beat is detected or there's currently a constant beat over 8 counts, increase mixAmount
  if (beatDetected[lowBeatBin] == 2 || constantBeatCounter[lowBeatBin] > 8)
    mixAmountInfluencer += 5;

  // Otherwise, it is constantly decreasing
  EVERY_N_MILLIS(150)
  mixAmountInfluencer -= 1;

  mixAmountInfluencer = constrain(mixAmountInfluencer, -10, 10);
  mixAmount = constrain(mixAmount + mixAmountInfluencer, 0, 255);
  //Serial.print(mixAmountInfluencer);
  //Serial.print("\t");
  //Serial.println(mixAmount);



  if (slaveCount > 0 && !ChangeSettings && parameters.Modus == music)
  {
    switch (parameters.reactie) {
      case 0:
        if (beatDetected[lowBeatBin] != oldbeat) {
          oldbeat = beatDetected[lowBeatBin];
          //  Serial.println("Lift of!");
          xQueueSendToFront( beatLow , ( void * ) &beatDetected[lowBeatBin], 0 );
             Serial.println(beatDetected[lowBeatBin]);
        }
        break;
      case 1:
        if (beatDetected[midBeatBin] != oldbeat) {
          oldbeat = beatDetected[midBeatBin];
          //  Serial.println("Lift of!");
          xQueueSendToFront( beatMid , ( void * ) &beatDetected[midBeatBin], 0 );
           Serial.println(beatDetected[midBeatBin]);
        }
        break;
      case 2:
        if (beatDetected[highBeatBin] != oldbeat) {
          oldbeat = beatDetected[highBeatBin];
          //  Serial.println("Lift of!");
          xQueueSendToFront( beatHigh , ( void * ) &beatDetected[highBeatBin], 0 );
           Serial.println(beatDetected[highBeatBin]);
        }
        break;
    }
  }
}

void printBeatBins() {
  //   Prints all the beat bins
  Serial.print(lowBeatBin);
  Serial.print("\t");
  Serial.print(binScore[lowBeatBin]);
  Serial.print("\t");
  Serial.print(midBeatBin);
  Serial.print("\t");
  Serial.print(binScore[midBeatBin]);
  Serial.print("\t");
  Serial.print(highBeatBin);
  Serial.print("\t");
  Serial.println(binScore[highBeatBin]);
}

void printNumber(float n) {
  if (n >= 0.004) {
    Serial.print(n, 3);
    Serial.print(" ");
  }
  else {
    //  Serial.print("   -  "); // don't print "0.00"
    Serial.print("   0  "); // print 0 for excel purposes
  }
}

void printSpectrum(bool Mode) {
  Audio.ReadFreq(FreqVal);//return 7 value of 7 bands pass filiter
  //Frequency(Hz):63  160  400  1K  2.5K  6.25K  16K
  //FreqVal[]:      0    1    2    3    4    5    6
  for (int i = 0; i < 7; i++)
  {
    if (Mode) {
      if (spectrumValue[i] >= 10) Serial.print(max((spectrumValue[i]),  0)); //Transimit the DC value of the seven bands

      if (i < 6)  Serial.print(",");
      else Serial.println();
      Serial.print("\t");
    } else {
      if (spectrumValue[i] >= 10)  Serial.print(max((FreqVal[i]), 0)); //Transimit the DC value of the seven bands
      if (i < 6)  Serial.print(",");
      else Serial.println();
    }
    delay(2);
  }
}

// Used for diagnostics / watching whether a song is having beats detected
void printDetectedBeats(int i) {
  // These print statements will print the beat status of each bin
  if (i == numFFTBins - 1) Serial.println(beatDetected[i]);
  else {
    Serial.print(beatDetected[i]);
    Serial.print("\t");
  }
}

void helpAudio ( void *pvParameters )
{
  for (;;)
  {
    if (slaveCount > 0)
    {
      EVERY_N_SECONDS(2){ int val = uxTaskGetStackHighWaterMark(NULL); Serial.print("stack left for audio read:"); Serial.println(val);}//print the unused asigned memory, this must be between 50 and 300. lower the better.
      musicAnalytics();
     /* switch (parameters.DebugGeluid) {
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
      }*/
    }
  }
  vTaskDelete( NULL );
}

void audioRead( void *pvParameters )
{
  delay(2000);
  xTaskCreatePinnedToCore( helpAudio  , "Help audio", 1200, NULL, 1, NULL, 1 ); //assigned to core 1
  delay(2000);
  int beats;
  for (;;)
  {
   EVERY_N_SECONDS(2){ int val = uxTaskGetStackHighWaterMark(NULL); Serial.print("stack left for audio read:"); Serial.println(val);}//print the unused asigned memory, this must be between 50 and 300. lower the better.
    if (slaveCount > 0 && !ChangeSettings && parameters.Modus == music) {
      switch (parameters.reactie) {
        case 0:
          //  Serial.println("low beat wait");
          if (xQueueReceive( beatLow, &beats,  portMAX_DELAY) == pdTRUE)
          {
            // Serial.println("low beat send: ");
            Serial.println(beats);
            sendRequest(34, beats);

          } break;
        case 14:
          if (xQueueReceive( beatMid, &beats,  portMAX_DELAY) == pdTRUE)
          {
            // Serial.print("mid beat send: ");
            Serial.println(beats);
            sendRequest(35, beats);
          } break;
        case 2:
          if (xQueueReceive( beatHigh, &beats,  portMAX_DELAY) == pdTRUE)
          {
            // Serial.println("high beat send: ");
            Serial.println(beats);
            sendRequest(36, beats);
          } break;
      }
    }
  }
  vTaskDelete( NULL );
}


#endif
