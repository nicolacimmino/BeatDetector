// BeatDetect implements a detector for music beat.
//
//  Copyright (C) 2014 Nicola Cimmino
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see http://www.gnu.org/licenses/.
//
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

AudioInputI2S            i2s_in;
AudioAnalyzeFFT256       fft;
AudioConnection          patchCord1(i2s_in, 0, fft, 0);
AudioPlaySdWav           playWav1;
AudioOutputI2S           i2s_out;
AudioConnection          patchCord2(playWav1, 0, i2s_out, 0);
AudioConnection          patchCord3(playWav1, 1, i2s_out, 1);
AudioControlSGTL5000     sgtl5000;

// This is the amount of peaks detected, in frequency domain, from
// the original melody sample.
#define KEY_LEN 200

// This is the amount of times the user should repeat the original
// whistled melody.
#define VERIFICATION_COUNT 2

// Peaks in frequency domain from the original melody.
int key[KEY_LEN];

// Estimated during the verificatin phase keeps into account
//  the ability of the user to repeat the melody consistently.
int expectedError = 0;

// How main bins a detected peak can be off from the expected
// and still be considered valid.
#define DETECT_ERROR_HYSTERESIS 5

void setup()
{
  AudioMemory(12);
  sgtl5000.enable();
  sgtl5000.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000.lineInLevel(15);  
  sgtl5000.volume(0.5);
  
  // For testing we have for now a LED between A7 and A6
  pinMode(A7, OUTPUT);
  pinMode(A6, OUTPUT);
  analogWrite(A7, 0);
  analogWrite(A6, 0);

}

float maxValue=0.1;
long lastBeatTime=0;
float beatInterval=0;
float lastBeatInterval=0;

void loop()
{
  while(!fft.available())
  {
    delay(10); 
  };
  
  float currentEnergy=fft.read(4);
  if(currentEnergy>maxValue)
  {
        maxValue=currentEnergy;
   
        beatInterval=millis()-lastBeatTime;
        lastBeatTime=millis();
        
        if(lastBeatInterval==0 || lastBeatInterval>2000 || beatInterval>lastBeatInterval*0.8)
        {
          lastBeatInterval=beatInterval;
          analogWrite(A6, HIGH);
        }
          delay(100);
          analogWrite(A6, LOW);
       
  }
  else
  {
    analogWrite(A6, LOW); 
    beatInterval=beatInterval*0.95;
   
    delay(50);
    if(maxValue>0.1)
    {
      maxValue-=0.01;
    } 
  }
  
}


