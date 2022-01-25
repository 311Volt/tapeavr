# tapeavr

A very crude program that converts .hex files to a .wav file containing SPI signals that can be amplified and filtered with a relatively simple circuit to program an AVR microcontroller.

Originally made for a paper about microcontrollers at Gdansk University of Technology.

Currently only officially supports the ATmega8A.

# usage
```bash 
tapeavr filename.hex
```
The result will be saved in output.wav. 
(someone remind me to add civilized command line options)

# building the program
 - build libsndfile https://github.com/libsndfile/libsndfile/releases/
 - compile src/*.cpp, linking against libsndfile


# building the circuit
## the signal
The output file's left channel is MOSI and the right channel is CLK. Since we want to be resistant to DC blocking filters in audio equipment, both signals have a low duty cycle. Negative sample values are not used. This way, neither a bipolar supply nor a virtual ground is needed.

## the regenerator
To turn noisy audio signals into clean digital ones, Schmitt triggers may be used. Assuming a peak voltage of 700 mV at the PC's audio output at max volume, we'll regard anything over 200 mV as logical high. 

An example schematic of a non-inverting Schmitt trigger with 100 mV - 200 mV thresholds:

![schematic](https://github.com/311Volt/tapeavr/blob/master/regenerator/schematic.png?raw=true)

The second opamp may be replaced with a digital NOT gate. The input is an audio signal, while the output is a serial pin of the MCU.

A minimal version of a programmer consists of two of these (they may be made with a single LM324) and a button/switch hooked up to the RESET pin of the microcontroller. With that, the MCU can be programmed by playing the audio file while holding RESET low.

## checking communication
It is highly recommended to implement some way of checking communication. Per the datasheet, the AVR should echo back 0x53 while the programmer is sending the third byte of the Programming Enable instruction. This may be done with two daisy-chained 74595s with their shift and latch clocks tied to **inverted** CLK (output of the left opamp in the schematic) with 8 indicator LEDs out of the first register's Q7 and the second register's Q0-Q6.

The generated audio file will have a 1500 ms delay after the Programming Enable command to give the human operator time to react if the MCU does not echo the 0x53 properly.

#
Copyright (C) Jan Trusiłło 2022  
Gdańsk University of Technology