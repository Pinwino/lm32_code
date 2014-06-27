/*
 * PLL access (AD9516) for fine-delay card 
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


#include "fine-delay.h"
#include "hw/pll_config.h" /* the table to be written */
#include <syscon.h>
#include <errno.h>
#include <pp-printf.h>

//#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define jiffies timer_get_tics
#define HZ 100
#define vcxo_default_tune  41711 // FAAAAKEEEEEEEEEEEEEEEEE

static int pll_writel(struct fd_dev *fd, int val, int reg)
{
	//mprintf("From writel -> FD base adress 0x%08x\n", fd->fd_regs_base);
	return fd_spi_xfer(fd, FD_CS_PLL, 24, (reg << 8) | val, NULL);
}

static int pll_readl(struct fd_dev *fd, int reg)
{
	uint32_t ret;
	int err;
	//mprintf("From readl -> FD base adress 0x%08x\n", fd->fd_regs_base);

	err = fd_spi_xfer(fd, FD_CS_PLL, 24, (reg << 8) | (1 << 23), &ret);
	if (err < 0)
		return err;
	return ret & 0xff;
}

int fd_pll_init(struct fd_dev *fd)
{
	int i;
	unsigned long j;
	const struct ad9516_reg *r;

	mprintf("Starting PLL....\n");
	//mprintf("From init -> FD base adress 0x%08x\n", fd->fd_regs_base);
	mprintf("1st reg\n");
	if (pll_writel(fd, 0x99, 0x000) < 0)
		goto out;
	mprintf("2nd reg\n");
	if (pll_writel(fd, 0x01, 0x232) < 0)
		goto out;
	mprintf("1st pll_readl\n");
	i = pll_readl(fd, 0x003);
	if (i < 0)
		goto out;
	if (i != 0xc3) {
		mprintf("Error in PLL communication\n");
		mprintf("   (got 0x%x, expected 0xc3)\n", i);
		return -EIO;
	}
	mprintf("Write the magic config\n");
	/* Write the magic config */
	for (i = 0, r = __9516_regs; i < ARRAY_SIZE(__9516_regs); i++, r++) {
		if (pll_writel(fd, r->val, r->reg) < 0) {
			mprintf("Error in configuring PLL (step %i)\n", i);
			return -EIO;
		}
	}

	if (pll_writel(fd, 0x01, 0x232) < 0)
		goto out;

	/* Wait for it to lock */
	j = jiffies + HZ / 2;
	while (jiffies < j) {
		mprintf("Wait for it to lock...\n");
		i = pll_readl(fd, 0x1f);
		if (i < 0)
			return -EIO;
		if (i & 1)
			break;
		usleep(1000);//msleep(1);
	}
	if (!(i & 1))
		return -ETIMEDOUT;

	/*
	 * Synchronize the phase of all clock outputs
	 * (this is critical for the accuracy!)
	 */

	if (pll_writel(fd, 0x01, 0x230) < 0)
		goto out;
	if (pll_writel(fd, 0x01, 0x232) < 0)
		goto out;
	if (pll_writel(fd, 0x00, 0x230) < 0)
		goto out;
	if (pll_writel(fd, 0x01, 0x232) < 0)
		goto out;

	return 0;

out:
	mprintf("Error in SPI communication\n");
	return -EIO;
}
