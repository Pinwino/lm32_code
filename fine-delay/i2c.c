/*
 * I2C access (on-board EEPROM)
 *
 * Copyright (C) 2012 CERN (www.cern.ch)
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2 as published by the Free Software Foundation or, at your
 * option, any later version.
 */

//#include <linux/io.h>
//#include <linux/time.h>
//#include <linux/slab.h>
//#include <linux/delay.h>
//#include <linux/random.h>
#include "fine-delay.h"
#include "hw/fd_main_regs.h"
#define ndelay usleep
#define printk mprintf

static void set_sda(struct fd_dev *fd, int val)
{
	uint32_t reg;

	reg = fd_readl(fd, FD_REG_I2CR) & ~FD_I2CR_SDA_OUT;
	if (val)
		reg |= FD_I2CR_SDA_OUT;
	//mprintf("ADR 0x%08x VAL 0x%08x\n", FD_REG_I2CR, reg);
	fd_writel(fd, reg, FD_REG_I2CR);
	//ndelay(2000);
	ndelay(2);
}

static void set_scl(struct fd_dev *fd, int val)
{
	uint32_t reg;

	reg = fd_readl(fd, FD_REG_I2CR) & ~FD_I2CR_SCL_OUT;
	if (val)
		reg |= FD_I2CR_SCL_OUT;
		//mprintf("ADR 0x%08x VAL 0x%08x\n", FD_REG_I2CR, reg);
	fd_writel(fd, reg, FD_REG_I2CR);
	//ndelay(2000);
	ndelay(2);
}

static int get_sda(struct fd_dev *fd)
{
	return fd_readl(fd, FD_REG_I2CR) & FD_I2CR_SDA_IN ? 1 : 0;
};

static void mi2c_start(struct fd_dev *fd)
{
	set_sda(fd, 0);
	set_scl(fd, 0);
}

static void mi2c_stop(struct fd_dev *fd)
{
	set_sda(fd, 0);
	set_scl(fd, 1);
	set_sda(fd, 1);
}

int mi2c_put_byte(struct fd_dev *fd, int data)
{
	int i;
	int ack;

	for (i = 0; i < 8; i++, data<<=1) {
		set_sda(fd, data & 0x80);
		set_scl(fd, 1);
		set_scl(fd, 0);
	}

	set_sda(fd, 1);
	set_scl(fd, 1);

	ack = get_sda(fd);

	set_scl(fd, 0);
	set_sda(fd, 0);

	return ack ? -EIO : 0; /* ack low == success */
}

int mi2c_get_byte(struct fd_dev *fd, unsigned char *data, int sendack)
{
	int i;
	int indata = 0;

	/* assert: scl is low */
	set_scl(fd, 0);
	set_sda(fd, 1);
	for (i = 0; i < 8; i++) {
		set_scl(fd, 1);
		indata <<= 1;
		if (get_sda(fd))
			indata |= 0x01;
		set_scl(fd, 0);
	}

	set_sda(fd, (sendack ? 0 : 1));
	set_scl(fd, 1);
	set_scl(fd, 0);
	set_sda(fd, 0);

	*data= indata;
	return 0;
}

void mi2c_init(struct fd_dev *fd)
{
	set_scl(fd, 1);
	set_sda(fd, 1);
}

void mi2c_scan(struct fd_dev *fd)
{
	int i;
	for(i = 0; i < 256; i += 2) {
		mi2c_start(fd);
		if(!mi2c_put_byte(fd, i)){
			mprintf("Found i2c device at 0x%x\n", i >> 1);
			/*dev_info(&fd->fmc->dev,
				 "%s: Found i2c device at 0x%x\n",
			         KBUILD_MODNAME, i >> 1);*/
		}
		mi2c_stop(fd);
	}
}

/* FIXME: this is very inefficient: read several bytes in a row instead */
int fd_eeprom_read(struct fd_dev *fd, int i2c_addr, uint32_t offset,
		void *buf, size_t size)
{
	int i;
	uint8_t *buf8 = buf;
	uint32_t aux=0;
	unsigned char c;

	/*for(i = 0; i < size; i++) {
		mi2c_start(fd);
		if(mi2c_put_byte(fd, i2c_addr << 1) < 0) {
			mprintf("0x%x\n", i2c_addr);
			mi2c_stop(fd);
			return -EIO;
		}

		mi2c_put_byte(fd, (offset >> 8) & 0xff);
		mi2c_put_byte(fd, offset & 0xff);
		offset++;
		mi2c_stop(fd);
		mi2c_start(fd);
		mi2c_put_byte(fd, (i2c_addr << 1) | 1);
		mi2c_get_byte(fd, &c, 0);
		*buf8++ = c;
		mi2c_stop(fd);
	}*/
	
		/* Read it all in a single loop: hardware allows it */
	mi2c_start(fd);
	if(mi2c_put_byte(fd, i2c_addr << 1) < 0) {
		mi2c_stop(fd);
		return -EIO;
	}
	aux = (offset >> 8) & 0xff;
	printk("\t(offset >> 8) & 0xff = %lu\n", aux);
	mi2c_put_byte(fd, aux);
	aux = offset & 0xff;
	printk("\toffset & 0xff = %lu\n", aux);
	mi2c_put_byte(fd, aux);
	mi2c_stop(fd);
	mi2c_start(fd);
	aux=((i2c_addr << 1) | 1);
	printk("\t(fmc->eeprom_addr << 1) | 1 = 0x%x\n", aux);
	mi2c_put_byte(fd, aux);
	while (size--) {
		mi2c_get_byte(fd, &c, size != 0);
		*buf8++ = c;
		//printk("read 0x%08x, %4i to go\n", c, size);
	}
	mi2c_stop(fd);
	return size;
}

int fd_eeprom_write(struct fd_dev *fd, int i2c_addr, uint32_t offset,
		 void *buf, size_t size)
{
	int i, busy;
	uint8_t *buf8 = buf;

	for(i = 0; i < size; i++) {
		mi2c_start(fd);

		if(mi2c_put_byte(fd, i2c_addr << 1) < 0) {
			mi2c_stop(fd);
			return -1;
		}
		mi2c_put_byte(fd, (offset >> 8) & 0xff);
		mi2c_put_byte(fd, offset & 0xff);
		mi2c_put_byte(fd, *buf8++);
		offset++;
		mi2c_stop(fd);

		do { /* wait until the chip becomes ready */
			mi2c_start(fd);
			busy = mi2c_put_byte(fd, i2c_addr << 1);
			mi2c_stop(fd);
		} while(busy);
	}
	return size;
}

/*int fd_eeprom_write(struct fd_dev *fd, int i2c_addr, uint32_t offset,
		 void *buf, size_t size)
{
	int ret = size;
	uint8_t *buf8 = buf;
	uint32_t aux;
	unsigned char c;
	
	printk("****> spec_eeprom_read <****\n");
	printk("\tfmc->eeprom_addr = 0x%x\n", fmc->eeprom_addr);
	printk("\toffset = %lu\n", offset);
	printk("\tsize = %d\n", size);

	/* Read it all in a single loop: hardware allows it 
	mi2c_start(fmc);
	if(mi2c_put_byte(fmc, fmc->eeprom_addr << 1) < 0) {
		mi2c_stop(fmc);
		return -EIO;
	}
	aux = (offset >> 8) & 0xff;
	printk("\t(offset >> 8) & 0xff = %lu\n", aux);
	mi2c_put_byte(fmc, aux);
	aux = offset & 0xff;
	printk("\toffset & 0xff = %lu\n", aux);
	mi2c_put_byte(fmc, aux);
	mi2c_stop(fmc);
	mi2c_start(fmc);
	aux=((fmc->eeprom_addr << 1) | 1);
	printk("\t(fmc->eeprom_addr << 1) | 1 = 0x%x\n", aux);
	mi2c_put_byte(fmc, aux);
	while (size--) {
		mi2c_get_byte(fmc, &c, size != 0);
		*buf8++ = c;
		//printk("read 0x%08x, %4i to go\n", c, size);
	}
	mi2c_stop(fmc);
	return ret;
}*/


int fd_i2c_init(struct fd_dev *fd)
{
	mi2c_scan(fd);
	return 0;
}

void fd_i2c_exit(struct fd_dev *fd)
{
	/* nothing to do */
}

