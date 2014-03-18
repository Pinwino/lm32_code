/*
 * SPI access to fine-delay internals
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

#include <inttypes.h>
#include "fine-delay.h"
#include "hw/fd_main_regs.h"
#include <syscon.h>
#include <errno.h>
#include <pp-printf.h>
#define printk pp_printf

//#define jiffies (unsigned long) timer_get_tics()
#define jiffies (unsigned long) usleep(1000);
#define HZ 100
#define vcxo_default_tune  41711 // FAAAAKEEEEEEEEEEEEEEEEE -> Default calibration

int fd_spi_xfer(struct fd_dev *fd, int ss, int num_bits,
		uint32_t in, uint32_t *out)
{
	uint32_t scr = 0, r;
	unsigned long j =  jiffies + HZ;

	scr = FD_SCR_DATA_W(in)| FD_SCR_CPOL;
	if(ss == FD_CS_PLL)
		scr |= FD_SCR_SEL_PLL;
	else if(ss == FD_CS_GPIO)
		scr |= FD_SCR_SEL_GPIO;
	
	fd_writel(fd, scr, FD_REG_SCR);
	mprintf("\t\tADR %08X VAL %08X\n", FD_REG_SCR, scr);
	fd_writel(fd, scr | FD_SCR_START, FD_REG_SCR);
	mprintf("\t\tADR %08X VAL %08X\n", FD_REG_SCR, scr | FD_SCR_START);
	//mprintf("\t\t-PREP -> j=%ld jiffies=%ld\n", j, jiffies);
	//mprintf("\t\ttimer_get_tics = %ld", timer_get_tics());
	while (!(fd_readl(fd, FD_REG_SCR) & FD_SCR_READY)){
		//mprintf("Reading (tic %ld) ...\n", jiffies);
		//mprintf("timer_get_tics = %ld\n", timer_get_tics());
		//mprintf("\t\tPOST -> j=%ld jiffies=%ld\n", j, timer_get_tics());
		mprintf("\t\tPOST\n");
		usleep(1000*1000);
		//if (jiffies > j)
			break;
		}
	if (!(fd_readl(fd, FD_REG_SCR) & FD_SCR_READY)){
		mprintf("Faliure on read\n");
		return -EIO;
		}
	scr = fd_readl(fd, FD_REG_SCR);
	r = FD_SCR_DATA_R(scr);
	if(out) *out=r;
	usleep(100);//udelay(100); /* FIXME: check */
	return 0;
}


int fd_spi_init(struct fd_dev *fd)
{
	/* write default to DAC for VCXO */
	printk("\t\tIn 0x%08X\n", vcxo_default_tune & 0xffff);
	fd_spi_xfer(fd, FD_CS_DAC, 24, vcxo_default_tune & 0xffff,
		    NULL);
	return 0;
}


