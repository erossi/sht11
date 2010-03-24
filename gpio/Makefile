LIBS = m
OPTLEV = 2
CFLAGS = -g -Wall -Wstrict-prototypes -pedantic -O$(OPTLEV)
LFLAGS = -l$(LIBS)
CC = gcc

REMOVE = rm -f

gpio_objects = sht11_gpio.o
objects = $(gpio_objects) sht11.o

.PHONY: clean
.SILENT: help
.SUFFIXES: .c, .o

all: $(objects)
	$(CC) $(CFLAGS) -o sht11 main.c $(objects) $(LFLAGS)

test_gpio: $(gpio_objects)
	$(CC) $(CFLAGS) -o test_gpio test_gpio.c $(gpio_objects) $(LFLAGS)

clean:
	$(REMOVE) $(objects) main.o sht11

