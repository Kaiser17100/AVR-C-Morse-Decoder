MCU     = atmega328p
F_CPU   = 16000000UL
BAUD    = 57600
PORT    = /dev/ttyUSB0
CC      = avr-gcc
OBJCOPY = avr-objcopy
SIZE    = avr-size
AVRDUDE = avrdude

CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=gnu99
AFLAGS  = -mmcu=$(MCU) -x assembler-with-cpp -DF_CPU=$(F_CPU) -Wall

all:	compile

compile:	sketch.c
	$(CC) $(CFLAGS) -o sketchc.elf $<
	$(OBJCOPY) -O ihex sketchc.elf sketchc.hex
	$(SIZE) --format=avr --mcu=$(MCU) sketchc.elf

upload:	sketchc
	$(AVRDUDE) -c arduino -p m328p -P $(PORT) -b $(BAUD) -U flash:w:sketchc.hex


clean:
	rm -f *.elf *.hex
