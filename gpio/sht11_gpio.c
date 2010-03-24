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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#ifdef HAVE_DEFAULT
#include "default.h"
#endif

#include "sht11.h"
#include "sht11_io.h"
#include "sht11_gpio.h"

extern struct sht11_t *sht11;

/* Export sck and data pins from the gpio,
   todo 1 = export, 0 = unexport */
int export_pins(int todo)
{
	FILE *f;
	char *s;

	s = malloc(80);

	if (todo)
		strcpy(s, "/sys/class/gpio/export");
	else
		strcpy(s, "/sys/class/gpio/unexport");

	if ((f = fopen(s, "ab")) == NULL) {
		printf("Cannot open file: %s\n", s);
		free(s);
		return(1);
	}

	rewind(f);
	fwrite(DATA_GPIO_PIN, sizeof(char), strlen(DATA_GPIO_PIN), f);
	rewind(f);
	fwrite(SCK_GPIO_PIN, sizeof(char), strlen(SCK_GPIO_PIN), f);
	fclose(f);
	free(s);
	return(0);
}

int open_files(void)
{
	int err;

	sht11->data_ddr = fopen(sht11->gpio_data_ddr_path, "rb+");
	sht11->data_value = fopen(sht11->gpio_data_value_path, "rb+");
	sht11->sck_ddr = fopen(sht11->gpio_sck_ddr_path, "rb+");
	sht11->sck_value = fopen(sht11->gpio_sck_value_path, "rb+");
	err = sht11->data_ddr == NULL;
	err = err || (sht11->data_value == NULL);
	err = err || (sht11->sck_ddr == NULL);
	err = err || (sht11->sck_value == NULL);

	if (err)
		printf("Cannot open files \n");

	return(err);
}

/* set pin's direction or value (data or sck) */
void set_pin(FILE *f, const char *ddr)
{
	rewind(f);
	fwrite(ddr, sizeof(char), sizeof(ddr), f);
}

uint8_t read_pin(FILE *f, char *value)
{
	rewind(f);
	fread(value, sizeof(char), 1, f);

	if (strcmp(value, "0"))
		return(1);
	else
		return(0);
}

void set_sck_out(void)
{
	set_pin(sht11->sck_ddr, DDR_OUT);
}

void set_sck_in(void)
{
	set_pin(sht11->sck_ddr, DDR_IN);
}

void set_sck_high(void)
{
	set_pin(sht11->sck_value, VALUE_HIGH);
}

void set_sck_low(void)
{
	set_pin(sht11->sck_value, VALUE_LOW);
}

void set_data_out(void)
{
	set_pin(sht11->data_ddr, DDR_OUT);
}

void set_data_in(void)
{
	set_pin(sht11->data_ddr, DDR_IN);
}

void set_data_high(void)
{
	/* release the data pin, pullup do the rest */
	set_pin(sht11->data_value, VALUE_HIGH);
	set_data_in();
}

void set_data_low(void)
{
	set_pin(sht11->data_value, VALUE_LOW);
	set_data_out();
}

uint8_t read_data_pin(void)
{
	return(read_pin(sht11->data_value, sht11->gpio_data_value));
}

void sck_delay(void)
{
	usleep(10000);
}

uint8_t wait_until_data_is_ready(void)
{
	/* And if nothing came back this code hangs here
	   loop_until_bit_is_set(SHT11_PIN, SHT11_DATA);
	   loop_until_bit_is_clear(SHT11_PIN, SHT11_DATA);
	 */
	while (!read_data_pin());
	while (read_data_pin());
	return(0);
}

int sht11_io_init(void)
{
	int err;

	sht11 = malloc(sizeof(struct sht11_t));
	/* 80 chars for something like /sys/class/gpio/gpio11/value */
	sht11->gpio_data_ddr_path = malloc(80);
	sht11->gpio_data_value_path = malloc(80);
	sht11->gpio_data_value = malloc(2);
	sht11->gpio_sck_ddr_path = malloc(80);
	sht11->gpio_sck_value_path = malloc(80);

	/* load data direction path into the struct */
	strcpy(sht11->gpio_data_ddr_path, "/sys/class/gpio/gpio");
	strcat(sht11->gpio_data_ddr_path, DATA_GPIO_PIN);
	strcat(sht11->gpio_data_ddr_path, "/direction");
	/* load data value path into the struct */
	strcpy(sht11->gpio_data_value_path, "/sys/class/gpio/gpio");
	strcat(sht11->gpio_data_value_path, DATA_GPIO_PIN);
	strcat(sht11->gpio_data_value_path, "/value");
	/* load sck direction path into the struct */
	strcpy(sht11->gpio_sck_ddr_path, "/sys/class/gpio/gpio");
	strcat(sht11->gpio_sck_ddr_path, SCK_GPIO_PIN);
	strcat(sht11->gpio_sck_ddr_path, "/direction");
	/* load sck value path into the struct */
	strcpy(sht11->gpio_sck_value_path, "/sys/class/gpio/gpio");
	strcat(sht11->gpio_sck_value_path, SCK_GPIO_PIN);
	strcat(sht11->gpio_sck_value_path, "/value");

	err = export_pins(1);
	err = err || open_files();

	if (err)
		return(1);
	else {
		set_sck_out();
		set_sck_low();
		sck_delay();
		return(0);
	}
}

void sht11_io_end(void)
{
	fclose(sht11->data_ddr);
	fclose(sht11->data_value);
	fclose(sht11->sck_ddr);
	fclose(sht11->sck_value);
	export_pins(0);
	free(sht11->gpio_sck_value_path);
	free(sht11->gpio_sck_ddr_path);
	free(sht11->gpio_data_value);
	free(sht11->gpio_data_value_path);
	free(sht11->gpio_data_ddr_path);
	free(sht11);
}

