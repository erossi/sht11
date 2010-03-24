/* This file is part of sht11
 * Copyright (C) 2010 Enrico Rossi
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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sht11.h"
#include "sht11_gpio.h"

struct sht11_t *sht11;

int main(void)
{
	sht11_init();
	sht11_read_all();
	printf("Raw temperature: %u \n", sht11->raw_temperature);
	printf("Raw temperature crc8: %u \n", sht11->raw_temperature_crc8);
	printf("Raw humidity: %u \n", sht11->raw_humidity);
	printf("Raw humidity crc8: %u \n", sht11->raw_humidity_crc8);
	printf("Temperature: %f \n", sht11->temperature);
	printf("Humidity linear: %f \n", sht11->humidity_linear);
	printf("Humidity compensated: %f \n", sht11->humidity_compensated);
	printf("Dewpoint: %f \n", sht11->dewpoint);
	sht11_end();
	return(0);
}
