MCU = atmega8
INC = -I/usr/avr/include/
LIBS = m
OPTLEV = s
export CFLAGS = $(INC) -Wall -Wstrict-prototypes -pedantic -mmcu=$(MCU) -O$(OPTLEV)
export LFLAGS = -l$(LIBS)
export CC = avr-gcc

OBJCOPY = avr-objcopy -j .text -j .data -O ihex
OBJDUMP = avr-objdump
SIZE = avr-size
DUDE = avrdude -c stk500v1 -p $(MCU) -P /dev/ttyUSB0 -e -U flash:w:test_sht11.hex
REMOVE = rm -f
objects = uart.o sht11.o

.PHONY: clean indent
.SILENT: help
.SUFFIXES: .c, .o

all: $(objects)
	$(CC) $(CFLAGS) -o test_sht11.elf test_sht11.c $(objects) $(LFLAGS)
	$(OBJCOPY) test_sht11.elf test_sht11.hex

program:
	$(DUDE)

clean:
	$(REMOVE) *.elf *.hex $(objects)

indent:
	indent *.c *.h

