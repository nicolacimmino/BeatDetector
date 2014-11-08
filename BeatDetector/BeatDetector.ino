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
AudioAnalyzeToneDetect   toneDetector; 
AudioConnection          patchCord1(i2s_in, 0, toneDetector, 0);
AudioControlSGTL5000     sgtl5000;

#define LED_KATODE_PIN A7
#define LED_ANODE_PIN A6

// Threshold of the detector, scale is 0 to 1
#define NOISE_THRESHOLD 0.1

// Beat length comparator aperture in %/100
#define BEAT_COMPARATOR_APERTURE 0.05

// Max interval between beats, in mS before
// the comparator resets the beat interval measurement.
#define MAX_BEAT_INTERVAL 2000

// Min interval between bits. This prevents
// retriggering on the same beat.
#define MIN_BEAT_INTERVAL 100

// Frequency detector center frequency.
#define DETECTOR_FREQUENCY 300

// Delay in mS between two consecutive samples of the 
// spectrum frequency content.
#define SAMPLE_DELAY 50

// Decay rate of the last beat interval register
// expressed as a %/100.
// Note: this is the decay in SAMPLE_DELAY mS
#define LAST_BEAT_INTERVAL_DECAY_RATE 0.05

// Decay rate of the peak detector expressed as linear value.
// Note: this is the decay in SAMPLE_DELAY mS
#define PEAK_DETECTOR_DECAY_RATE 0.01

// Current peak detected.
float peakDetectorMax=NOISE_THRESHOLD;

// Timer value of the last valid beat detected.
long lastBeatTime=0;

// Last detected interval between beats. This allows
// to track the current beat and avoid retriggering
// when there are rythm alterations in the beat.
float lastBeatInterval=0;


void setup()
{
  AudioMemory(12);
  sgtl5000.enable();
  sgtl5000.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000.lineInLevel(15);  
  
  toneDetector.frequency(DETECTOR_FREQUENCY);
  
  // For testing we have for now a LED between LED_KATODE_PIN and LED_ANODE_PIN
  pinMode(LED_KATODE_PIN, OUTPUT);
  pinMode(LED_ANODE_PIN, OUTPUT);
  analogWrite(LED_KATODE_PIN, 0);
  analogWrite(LED_ANODE_PIN, 0);
}

void loop()
{
  while(!toneDetector.available()){ delay(10); }
  
  // Get the current energy from the detector.
  float currentEnergy=toneDetector.read();
  
  if(currentEnergy>peakDetectorMax)
  {
        // We have a new peak.
        peakDetectorMax=currentEnergy;
   
        // Estimate time elapsed since last beat.
        float beatInterval=millis()-lastBeatTime;
        
        // Apply a window so that if this beat time is below BEAT_COMPARATOR_APERTURE from the last beat time
        // we skip this beat. We still accept it if this is the first one or it's been too long since the last
        // beat (e.g. track change).
        if(lastBeatInterval==0 || beatInterval>MAX_BEAT_INTERVAL || beatInterval>lastBeatInterval*(1.0-BEAT_COMPARATOR_APERTURE))
        {
          lastBeatTime=millis();
        
          lastBeatInterval=beatInterval;
          analogWrite(LED_ANODE_PIN, HIGH);
          Serial.println(lastBeatInterval);
          delay(MIN_BEAT_INTERVAL);
          analogWrite(LED_ANODE_PIN, LOW);
        }
  }
  else
  {
    // No beat detected, we need to wait before we sample the spectrum again. 
    delay(SAMPLE_DELAY);
    
    // Let the last beat interval decay so we allow for slow beat change
    // without skipping beats.
    lastBeatInterval=lastBeatInterval*(1.0-LAST_BEAT_INTERVAL_DECAY_RATE);
   
    // Let the detected peak decay up to the noise threshold so we can
    // adjust ourselves to volume changes.
    if(peakDetectorMax>NOISE_THRESHOLD)
    {
      peakDetectorMax-=PEAK_DETECTOR_DECAY_RATE;
    } 
  }
  
}


