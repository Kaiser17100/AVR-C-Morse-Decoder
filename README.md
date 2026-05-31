# AVR C Morse Code Decoder
## Description
An embedded AVR application written in __AVR C__ that decodes incoming electrical signals as __morse code__ and displays the decoded alphanumeric characters on a 16x2 Character LCD over the I2C (TWI) protocol using a PCF8574 adapter.
The Decoding process relies on a **Binary Tree** lookup structure and hardware timer intrupts
## Features
- __Real Time Decoding:__ The embedded system accurately distinguishes a dash (-) and a dot (.)
- __Binary Tree Lookup:__ Highly optimized character lookup using a flat array modeled as a binary tree.
- __I2C/TWI Protocol:__ Custom implementation for controlling a PCF8574-based LCD backpack without external libraries.
- __Smart Delays:__  Automatically transmits/prints a character after a 500 ms idle window.
  > Auto-clears the screen if no input is detected for 10 seconds.
## Used Hardware
- __Microcontroller:__ ATmega328p
- __Display:__ 16x2 LCD Character Display with a PCF8574 I2C Backpack
- __Buzzer:__ Standart buzzer on Wokwi to alert the user of an incoming message
- __Button:__ To simulate incoming singal on Wokwi
## How to Compile
### Prerequisites
Ensure you have the AVR toolchain installed _(avr-gcc, avr-libc, and avrdude)_.
### Compilation and Flashing
 1) Adjust your microcontroller model and clock frequency inside the MakeFile.
 2) Run the __make__ bash command on the directory which contains both the MakeFile and the sketch.c file.
    > If it gives _avrdude: stk500_getsync() attempt 1 of 10: not in sync_ error change the baud rate to 115200
 4) Run the __make upload__ bash command on the directory which contains both the MakeFile and the sketch.c file.
 5) __Optional:__ Run the __make clean__ bash command to clear out the temporary build files _(.elf and .hex)_ 
## License
This project is open-source and free to use for educational and hobbyist purposes. Feel free to modify and expand it!
