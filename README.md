This device follows the beat of the music it hears through its microphone by lighting an LED in rithm with the music. This can serve as a base for light effects that change with music or perhaps a dancing doll.

The device first of all pefroms an FFT on the inconing signal and does peak detection on an FFT bin corresponding to a frequency between roughly 300Hz and 380Hz. This corresponds to a snare drum beat. The peak threshold decays in time so multiple beats can be detected and for each the LED is lit. Once a beat is established the device applys a window around it, in time domain, so that eventual changes in rythm in the song don't cause multiple triggerings. The window constantly adjusts to the current beat so that slides in rythm can be followed.

Hardware
===========

The hardware is a Teensy 3.1, which is running an ARM Cortex M4 processor, and its companion audio board that sports a SGTL5000 chip that takes care to handle the analog signal input as well as provides a software controllable microphone pre-amp.

![Proto](documentation/proto.png)
