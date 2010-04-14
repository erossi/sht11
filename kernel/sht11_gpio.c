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

#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include "sht11_gpio.h"

/* clock delay in ms */
#define SHT11_SCK_DELAY 1

#define VALUE_HIGH 1
#define VALUE_LOW 0

#define SHT11_CMD_STATUS_REG_W 6
#define SHT11_CMD_STATUS_REG_R 7
#define SHT11_CMD_MEASURE_TEMP 3
#define SHT11_CMD_MEASURE_HUMI 5
#define SHT11_CMD_RESET        15

#define SHT11_C1 -4.0           /* for 12 Bit */
#define SHT11_C2  0.0405        /* for 12 Bit */
#define SHT11_C3 -0.0000028     /* for 12 Bit */
#define SHT11_T1  0.01          /* for 14 Bit @ 5V */
#define SHT11_T2  0.00008       /* for 14 Bit @ 5V */

static struct sht11_t *sht11;
static int sht11_sck = 0;
static int sht11_data = 0;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Enrico Rossi");
module_param(sht11_data, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(sht11_data, "GPIO pin connected to sht11 DATA line");
module_param(sht11_sck, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(sht11_sck, "GPIO pin connected to sht11 SCK line");

static void set_sck_out(void)
{
	gpio_direction_output(sht11_sck, 0); 
}

static void set_sck_in(void)
{
	gpio_direction_input(sht11_sck);
}

static void set_sck_high(void)
{
	gpio_set_value(sht11_sck, VALUE_HIGH);
}

static void set_sck_low(void)
{
	gpio_set_value(sht11_sck, VALUE_LOW);
}

static void set_data_out(void)
{
	gpio_direction_output(sht11_data, 0); 
}

static void set_data_in(void)
{
	gpio_direction_input(sht11_data);
}

static void set_data_high(void)
{
	/* release the data pin, pullup do the rest */
	gpio_set_value(sht11_data, VALUE_HIGH);
	set_data_in();
}

static void set_data_low(void)
{
	set_data_out();
}

static uint8_t read_data_pin(void)
{
	return(gpio_get_value(sht11_data));
}

static void sck_delay(void)
{
	msleep(SHT11_SCK_DELAY);
}

static uint8_t wait_until_data_is_ready(void)
{
	/* And if nothing came back this code hangs here
	   loop_until_bit_is_set(SHT11_PIN, SHT11_DATA);
	   loop_until_bit_is_clear(SHT11_PIN, SHT11_DATA);
	 */
	while (!read_data_pin());
	while (read_data_pin());
	return(0);
}

/*
   sensirion has implemented the CRC the wrong way round. We
   need to swap everything.
   bit-swap a byte (bit7->bit0, bit6->bit1 ...)
   code provided by Guido Socher http://www.tuxgraphics.org/
 */
static uint8_t bitswapbyte(uint8_t byte)
{
	uint8_t i=8;
	uint8_t result=0;
	while(i) {
		result=(result<<1);

		if (1 & byte) {
			result=result | 1;
		}

		i--;
		byte=(byte>>1);
	}

	return(result);
}

/* Code from avr-libc http://www.nongnu.org/avr-libc/ */
static uint8_t _crc_ibutton_update(uint8_t crc, uint8_t data)
{
	uint8_t i;

	crc = crc ^ data;
	for (i = 0; i < 8; i++)
	{
		if (crc & 0x01)
			crc = (crc >> 1) ^ 0x8C;
		else
			crc >>= 1;
	}

	return crc;
}

static uint8_t sht11_crc8(uint8_t crc, uint8_t data)
{
	return(_crc_ibutton_update(crc, bitswapbyte(data)));
}

/* Common part with avr version */

static void send_byte(uint8_t byte)
{
	uint8_t i;

	i = 8;

	while (i) {
		--i;

		if (byte & (1<<i))
			set_data_high();
		else
			set_data_low();

		sck_delay();
		set_sck_high();
		sck_delay();
		set_sck_low();
	}
}

static uint8_t read_byte(void)
{
	uint8_t i, result;

	result = 0;
	i = 8;

	while (i) {
		--i;
		sck_delay();
		set_sck_high();

		if (read_data_pin())
			result |= (1<<i);

		sck_delay();
		set_sck_low();

	}

	return (result);
}

static void send_ack(void)
{
	/* Send ack */
	set_data_low();
	sck_delay();
	set_sck_high();
	sck_delay();
	set_sck_low();
	set_data_in();
}

static uint8_t read_ack(void)
{
	uint8_t ack;

	/* read ack after command */
	set_data_in();
	sck_delay();
	set_sck_high();
	ack = read_data_pin();
	sck_delay();
	set_sck_low();

	return (ack);
}

/* terminate a session without sending an ack */
static void terminate_no_ack(void)
{
	set_data_high();
	sck_delay();
	set_sck_high();
	sck_delay();
	set_sck_low();
	set_data_in();
}

static void send_start_command(void)
{
	/* DATA:   _____           ________
	   DATA:         |_______|
	   SCK :       ___       ___
	   SCK :  ___|     |___|     |______
	 */

	set_data_high();
	/*   set_data_out (); */
	sck_delay();
	set_sck_high();
	sck_delay();
	set_data_low();
	sck_delay();
	set_sck_low();
	sck_delay();
	set_sck_high();
	sck_delay();
	set_data_high();
	sck_delay();
	set_sck_low();
	sck_delay();
}

static void sht11_read_status_reg(void)
{
	uint8_t ack;

	sht11->cmd = SHT11_CMD_STATUS_REG_R;
	sht11->status_reg_crc8 = 0;
	sht11->status_reg_crc8c = 0;
	send_start_command();
	send_byte(sht11->cmd);
	ack = read_ack();
	sht11->status_reg_crc8c = sht11_crc8(sht11->status_reg_crc8c, sht11->cmd);

	if (!ack) {
		sht11->status_reg = read_byte();
		sht11->status_reg_crc8c = sht11_crc8(sht11->status_reg_crc8c, sht11->status_reg);
		send_ack();
		sht11->status_reg_crc8 = read_byte();
		terminate_no_ack();
	}
}

/* Disable Interrupt to avoid possible clk problem. */
static void send_cmd(void)
{
	uint8_t ack, byte;

	/* safety 000xxxxx */
	sht11->cmd &= 31;
	sht11->result = 0;
	sht11->crc8 = 0;
	sht11->crc8c = 0;

	send_start_command();
	send_byte(sht11->cmd);
	ack = read_ack();
	sht11->crc8c = sht11_crc8(sht11->crc8c, sht11->cmd);

	if (!ack) {
		/* And if nothing came back this code hangs here */
		wait_until_data_is_ready();

		/* inizio la lettura dal MSB del primo byte */
		byte = read_byte();
		sht11->result = byte << 8;
		sht11->crc8c = sht11_crc8(sht11->crc8c, byte);

		/* Send ack */
		send_ack();

		/* inizio la lettura dal MSB del secondo byte */
		byte = read_byte();
		sht11->result |= byte;
		sht11->crc8c = sht11_crc8(sht11->crc8c, byte);

		send_ack();

		/* inizio la lettura del CRC-8 */
		sht11->crc8 = read_byte();
		terminate_no_ack();
	}
}

static void sht11_read_temperature_raw(void)
{
	sht11->cmd = SHT11_CMD_MEASURE_TEMP;
	send_cmd();
	sht11->raw_temperature = sht11->result;
	sht11->raw_temperature_crc8 = sht11->crc8;
	sht11->raw_temperature_crc8c = sht11->crc8c;
}

static void sht11_read_humidity_raw(void)
{
	sht11->cmd = SHT11_CMD_MEASURE_HUMI;
	send_cmd();
	sht11->raw_humidity = sht11->result;
	sht11->raw_humidity_crc8 = sht11->crc8;
	sht11->raw_humidity_crc8c = sht11->crc8c;
}

/*

   Kernel only part

 */

static ssize_t show_temperature(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	sht11_read_temperature_raw();

	return sprintf(buf, "%d\n", sht11->raw_temperature);
}

static ssize_t show_humidity(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	sht11_read_humidity_raw();

	return sprintf(buf, "%d\n", sht11->raw_humidity);
}

static struct kobj_attribute temperature_attribute =
	__ATTR(sht11->raw_temperature, 0444, show_temperature, NULL);

static struct kobj_attribute humidity_attribute =
	__ATTR(sht11->raw_humidity, 0444, show_humidity, NULL);

/*
 * Create a group of attributes so that we can create and destory them all
 * at once.
 */
static struct attribute *attrs[] = {
	&temperature_attribute.attr,
	&humidity_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *sht11_kobj;

/* Export sck and data pins, todo 1 = export, 0 = unexport */
static int export_pins(int todo)
{
	int err = 0;

	if (todo) {
		err = gpio_request(sht11_data, "sht11_data");
		err |= gpio_request(sht11_sck, "sht11_sck");
	} else {
		gpio_free(sht11_data);
		gpio_free(sht11_sck);
	}

	return(err);
}

static int __init sht11_init(void)
{
	int err;
	int retval;

	/* check if something is passed */
	err = sht11_sck && sht11_data;

	if (!err) {
		printk(KERN_INFO "You must specify gpio's pins (sck and data)\n");
		return(-1);
	}

	/* Are these gpios valid? */
	if (!gpio_is_valid(sht11_sck) || !gpio_is_valid(sht11_data)) {
		printk(KERN_INFO "Error, unavailable gpio\n");
		return(-1);
	}

	sht11 = kmalloc(sizeof(struct sht11_t), GFP_KERNEL);
	err = export_pins(1);

	if (err) {
		printk(KERN_INFO "Unable to export gpio's pin\n");
		return(err);
	}

	/*
	 * Create a simple kobject with the name of "kobject_example",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	sht11_kobj = kobject_create_and_add("kobject_sht11", kernel_kobj);

	if (!sht11_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(sht11_kobj, &attr_group);

	if (retval)
		kobject_put(sht11_kobj);

	set_sck_out();
	set_sck_low();
	sck_delay();
	sck_delay();
	sck_delay();
	sck_delay();
	sht11_read_status_reg();

	printk(KERN_INFO "Sht11 sr: %d, crc8: %d, crc8c: %d\n", sht11->status_reg, sht11->status_reg_crc8, sht11->status_reg_crc8c);

	return retval;
}

static void __exit sht11_exit(void)
{
	set_data_high();
	set_sck_high();
	set_data_in();
	set_sck_in();
	export_pins(0);
	kfree(sht11);
	kobject_put(sht11_kobj);
	printk(KERN_INFO "Sht11 unloaded\n");
}

module_init(sht11_init);
module_exit(sht11_exit);

