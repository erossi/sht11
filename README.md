# sht11 sensor with atmel atmega8.

__WARN: unmaintained, obsolete.__

This project illustrate how to make sht11 sensor routines work with atmel atmega8.

__This page is not complete and detailed description are missing so use at your own risk. You must understand what you are doing or you will damage your equipment.__

There are many comments in the source code and it should be easily readable so look at the code, please!

#### What you need

 * Temperature/Humidity sensor Sensirion Sht1x/7x (I use the sht11).
 * MCU Atmel AtMega 8 (use other MCU like atmega16 should not be a problem).
 * Development Board Atmel Stk500.
 * a linux with avr-libc, avrdude, git and make utilities installed.
 * a little electronic knowledge just not to burn out your equipment.

Schematics

_insert schematic image here_

_insert assembled picture here_

#### Download

First download the latest version of the software, then compile it and install the .hex file into your MCU.

` git clone http://tecnobrain.com/git/sht11.git `

edit Makefile and change eventually the MCU type or the serial port used to connect to stk500.

Compile with

```
make
make program
```

or if you have different MCU, programmer or settings, program the MCU with the test_sht11.hex file in your enviroment.

Connect the SHT11-data pin to PC1 and SHT11-sck pin to PC0, look inside the code (file sht11_avr_io.h) to find out or change these values.

On the stk500 connect portB pin 0 and 1 to switch 0 and 1, pin 2 and 3 to leds 2 and 3.

Connect the stk500 to PC using rs-232 spare, use your prefered terminal client, use 9600 bps 8N1 no hardware or software control.

When evarything is ok turn on the board and hit switch 0, read the sht11 values on the terminal.
