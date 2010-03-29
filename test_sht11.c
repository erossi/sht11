/* This file is part of sht11
 * Copyright (C) 2005-2010 Enrico Rossi
 *
 * Sht11 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Sht11 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "uart.h"
#include "sht11.h"

int main(void)
{
  char *string;
  char *line;
  struct sht11_t dataset;

  /*
     PORTB pin 0 and 1 IN, connect to switch 0 and 1.
     Connect port b pin 2 and 3 to led 0 and 1.
     When push switch 0, a blink on led 0 signal sht11 data sent 
     to rs232 port.
   */

  DDRB = 12;
  PORTB = 12;
  string = malloc (20);
  line = malloc (74);

  sht11_init(&dataset);
  uart_init();

  strcpy_P (line, PSTR("\n\nStatus register: "));
  string = utoa (dataset.status_reg, string, 10);
  strcat (line, string);
  uart_printstr (line);
  uart_putchar ('\n');

  strcpy_P (line, PSTR("Status register crc8, crc8c: "));
  string = utoa (dataset.status_reg_crc8, string, 10);
  strcat (line, string);
  strcat_P (line, PSTR(" ,"));
  string = utoa (dataset.status_reg_crc8c, string, 10);
  strcat (line, string);
  uart_printstr (line);
  uart_putchar ('\n');

  if (dataset.status_reg_crc8 && (dataset.status_reg_crc8 == dataset.status_reg_crc8c)) {
	  strcpy_P (line, PSTR("Device detected OK."));
	  uart_printstr (line);
	  uart_putchar ('\n');
  } else {
	  strcpy_P (line, PSTR("Device not detected!"));
	  uart_printstr (line);
	  uart_putchar ('\n');
  }

  for (;;)
  {
    /* wait until a button is pressed */
    loop_until_bit_is_clear (PINB, 0);

    /* flash led connected to PORTB */
    PORTB = 0;
    _delay_ms (100);
    PORTB = 12;

    sht11_read_all (&dataset);

    strcpy_P (line, PSTR("Raw temperature: "));
    string = ultoa (dataset.raw_temperature, string, 10);
    strcat (line, string);
    uart_printstr (line);
    uart_putchar ('\n');

    strcpy_P (line, PSTR("Raw temperature crc8, crc8c: "));
    string = utoa (dataset.raw_temperature_crc8, string, 10);
    strcat (line, string);
    strcat_P (line, PSTR(" ,"));
    string = utoa (dataset.raw_temperature_crc8c, string, 10);
    strcat (line, string);
    uart_printstr (line);
    uart_putchar ('\n');

    strcpy_P (line, PSTR("Raw humidity: "));
    string = ultoa (dataset.raw_humidity, string, 10);
    strcat (line, string);
    uart_printstr (line);
    uart_putchar ('\n');

    strcpy_P (line, PSTR("Raw humidity crc8, crc8c: "));
    string = utoa (dataset.raw_humidity_crc8, string, 10);
    strcat (line, string);
    strcat_P (line, PSTR(" ,"));
    string = utoa (dataset.raw_humidity_crc8c, string, 10);
    strcat (line, string);
    uart_printstr (line);
    uart_putchar ('\n');

    strcpy_P (line, PSTR("Temperature: "));
    string = dtostrf ((double)dataset.temperature, 6, 3, string);
    strcat (line, string);
    uart_printstr (line);
    uart_putchar ('\n');

    strcpy_P (line, PSTR("Humidity linear: "));
    string = dtostrf ((double)dataset.humidity_linear, 6, 3, string);
    strcat (line, string);
    uart_printstr (line);
    uart_putchar ('\n');

    strcpy_P (line, PSTR("Humidity compensated: "));
    string = dtostrf ((double)dataset.humidity_compensated, 6, 3, string);
    strcat (line, string);
    uart_printstr (line);
    uart_putchar ('\n');

    strcpy_P (line, PSTR("Dewpoint: "));
    string = dtostrf ((double)dataset.dewpoint, 6, 3, string);
    strcat (line, string);
    uart_printstr (line);
    uart_putchar ('\n');

    uart_putchar ('\n');
    _delay_ms (100);
  }

  free (line);
  free (string);
}
