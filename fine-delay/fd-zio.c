/*
 * ZIO interface for the fine-delay driver
 *
 * Copyright (C) 2012 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2 as published by the Free Software Foundation or, at your
 * option, any later version.
 */

#include <linux/kernel.h>
//#include <linux/module.h>
#include <linux/moduleparam.h>
//#include <linux/init.h>
//#include <linux/timer.h>
#include <linux/jiffies.h>
//#include <linux/bitops.h>
#include <linux/io.h>

//#include <linux/zio.h>
//#include <linux/zio-buffer.h>
//#include <linux/zio-trigger.h>

#include <fmc.h>

#include "fine-delay.h"
#include "hw/fd_main_regs.h"
#include "hw/fd_channel_regs.h"

#define NSEC_PER_SEC 1000*1000*1000

#define _RW_ (S_IRUGO | S_IWUGO) /* I want 80-col lines so this lazy thing */
extern struct fd_dev fd;

static int fd_use_raw_tdc;

/* The user may want to use raw TDC registers for faster input */
//module_param_named(raw_tdc, fd_use_raw_tdc, int, 0444);

/* The sample size. Mandatory, device-wide 
ZIO_ATTR_DEFINE_STD(ZIO_DEV, fd_zattr_dev_std) = {
	ZIO_ATTR(zdev, ZIO_ATTR_NBITS, S_IRUGO, 0, 32), /* 32 bits. Really? *
};

/* Extended attributes for the device *
static struct zio_attribute fd_zattr_dev[] = {
	ZIO_ATTR_EXT("version", S_IRUGO,	FD_ATTR_DEV_VERSION,
		      FDELAY_VERSION),
	ZIO_ATTR_EXT("utc-h", _RW_,		FD_ATTR_DEV_UTC_H, 0),
	ZIO_ATTR_EXT("utc-l", _RW_,		FD_ATTR_DEV_UTC_L, 0),
	ZIO_ATTR_EXT("coarse", _RW_,		FD_ATTR_DEV_COARSE, 0),
	ZIO_ATTR_EXT("command", S_IWUGO,	FD_ATTR_DEV_COMMAND, 0),
	ZIO_ATTR_EXT("temperature", _RW_,	FD_ATTR_DEV_TEMP, 0),
};

/* Extended attributes for the TDC (== input) cset *
static struct zio_attribute fd_zattr_input[] = {
	ZIO_ATTR_EXT("utc-h", S_IRUGO,		FD_ATTR_TDC_UTC_H, 0),
	ZIO_ATTR_EXT("utc-l", S_IRUGO,		FD_ATTR_TDC_UTC_L, 0),
	ZIO_ATTR_EXT("coarse", S_IRUGO,	FD_ATTR_TDC_COARSE, 0),
	ZIO_ATTR_EXT("frac", S_IRUGO,		FD_ATTR_TDC_FRAC, 0),
	ZIO_ATTR_EXT("seq", S_IRUGO,		FD_ATTR_TDC_SEQ, 0),
	ZIO_ATTR_EXT("chan", S_IRUGO,		FD_ATTR_TDC_CHAN, 0),
	ZIO_ATTR_EXT("flags", _RW_,		FD_ATTR_TDC_FLAGS, 0),
	ZIO_ATTR_EXT("offset", _RW_,		FD_ATTR_TDC_OFFSET, 0),
	ZIO_ATTR_EXT("user-offset", _RW_,	FD_ATTR_TDC_USER_OFF, 0),
};

/* Extended attributes for the output csets (most not-read-nor-write mode) *
static struct zio_attribute fd_zattr_output[] = {
	ZIO_ATTR_EXT("mode", S_IRUGO,		FD_ATTR_OUT_MODE, 0),
	ZIO_ATTR_EXT("rep", S_IRUGO,		FD_ATTR_OUT_REP, 0),
	ZIO_ATTR_EXT("start-h", S_IRUGO,	FD_ATTR_OUT_START_H, 0),
	ZIO_ATTR_EXT("start-l", S_IRUGO,	FD_ATTR_OUT_START_L, 0),
	ZIO_ATTR_EXT("start-coarse", S_IRUGO,	FD_ATTR_OUT_START_COARSE, 0),
	ZIO_ATTR_EXT("start-fine", S_IRUGO,	FD_ATTR_OUT_START_FINE, 0),
	ZIO_ATTR_EXT("end-h", S_IRUGO,		FD_ATTR_OUT_END_H, 0),
	ZIO_ATTR_EXT("end-l", S_IRUGO,		FD_ATTR_OUT_END_L, 0),
	ZIO_ATTR_EXT("end-coarse", S_IRUGO,	FD_ATTR_OUT_END_COARSE, 0),
	ZIO_ATTR_EXT("end-fine", S_IRUGO,	FD_ATTR_OUT_END_FINE, 0),
	ZIO_ATTR_EXT("delta-l", S_IRUGO,	FD_ATTR_OUT_DELTA_L, 0),
	ZIO_ATTR_EXT("delta-coarse", S_IRUGO,	FD_ATTR_OUT_DELTA_COARSE, 0),
	ZIO_ATTR_EXT("delta-fine", S_IRUGO,	FD_ATTR_OUT_DELTA_FINE, 0),
	ZIO_ATTR_EXT("delay-offset", _RW_,	FD_ATTR_OUT_DELAY_OFF, 0),
	ZIO_ATTR_EXT("user-offset", _RW_,	FD_ATTR_OUT_USER_OFF, 0),
};


/* This identifies if our "struct device" is device, input, output *
enum fd_devtype {
	FD_TYPE_WHOLEDEV,
	FD_TYPE_INPUT,
	FD_TYPE_OUTPUT,
};

/*static enum fd_devtype __fd_get_type(struct device *dev)
{
	struct zio_obj_head *head = to_zio_head(dev);
	struct zio_cset *cset;

	if (head->zobj_type == ZIO_DEV)
		return FD_TYPE_WHOLEDEV;
	cset = to_zio_cset(dev);
	if (cset->index == 0)
		return FD_TYPE_INPUT;
	return FD_TYPE_OUTPUT;
}*/

/* TDC input attributes: only the user offset is special */
int fd_zio_info_tdc(struct fd_dev *fd, enum fd_zattr_in_idx option,
			     uint32_t *usr_val)
{
	//printk("--*%s\n", __func__);
	//struct zio_cset *cset;
	//struct fd_dev *fd;

	//cset = to_zio_cset(dev);
	//fd = cset->zdev->priv_d;
	//printk("%i\n", option);

	if (option == FD_ATTR_TDC_USER_OFF) {
		//printk("FD_ATTR_TDC_USER_OFF\n");
		*usr_val = fd->tdc_user_offset;
		return 0;
	}
	if (option == FD_ATTR_TDC_FLAGS) {
		//printk("FD_ATTR_TDC_FLAGS\n");
		*usr_val = fd->tdc_flags;
		return 0;
	}
	/*
	 * Following code is about TDC values, for the last TDC event.
	 * For efficiency reasons at read_fifo() time, we store an
	 * array of integers instead of filling attributes, so here
	 * pick the values from our array.*
	 */
	//usr_val = fd->tdc_attrs[FD_CSET_INDEX(zattr->id)];

	return 0;
}

/* output channel: only the two offsets */
//static int fd_zio_info_output(struct device *dev, struct zio_attribute *zattr, uint32_t *usr_val)
int fd_zio_info_output(struct fd_dev *fd, int ch, enum fd_zattr_in_idx option, uint32_t *usr_val)
{
	//printk("--*%s\n", __func__);
	//struct zio_cset *cset;
	//struct fd_dev *fd;
	//int ch;

	/*cset = to_zio_cset(dev);
	ch = cset->index - 1;
	fd = cset->zdev->priv_d;*/

	if (option == FD_ATTR_OUT_DELAY_OFF) {
		*usr_val = fd->calib.zero_offset[ch];
		return 0;
	}
	if (option == FD_ATTR_OUT_USER_OFF) {
		*usr_val = fd->ch_user_offset[ch];
		return 0;
	}
	/* Reading the mode tells the current mode and whether it triggered or not */
	if (option == FD_ATTR_OUT_MODE) {
		uint32_t dcr = fd_ch_readl(fd, ch, FD_REG_DCR);
		if(! (dcr & FD_DCR_ENABLE))
		    *usr_val = FD_OUT_MODE_DISABLED;
		else if(dcr & FD_DCR_MODE)
		    *usr_val = FD_OUT_MODE_PULSE;
		else
		    *usr_val = FD_OUT_MODE_DELAY;

		if(dcr & FD_DCR_PG_TRIG)
			*usr_val |= 0x80;
		return 0;
	}

	/* readout of output config delays */
	if (option == FD_ATTR_OUT_START_H) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_U_STARTH);
		return 0;
	}
	if (option == FD_ATTR_OUT_START_L) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_U_STARTL);
		return 0;
	}
	if (option == FD_ATTR_OUT_START_COARSE) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_C_START);
		return 0;
	}
	if (option == FD_ATTR_OUT_START_FINE) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_F_START);
		return 0;
	}

	if (option == FD_ATTR_OUT_END_H) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_U_ENDH);
		return 0;
	}
	if (option == FD_ATTR_OUT_END_L) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_U_ENDL);
		return 0;
	}
	if (option == FD_ATTR_OUT_END_COARSE) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_C_END);
		return 0;
	}
	if (option == FD_ATTR_OUT_END_FINE) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_F_END);
		return 0;
	}

	if (option == FD_ATTR_OUT_DELTA_L) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_U_DELTA);
		return 0;
	}
	if (option == FD_ATTR_OUT_DELTA_COARSE) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_C_DELTA);
		return 0;
	}
	if (option == FD_ATTR_OUT_DELTA_FINE) {
		*usr_val = fd_ch_readl(fd, ch, FD_REG_F_DELTA);
		return 0;
	}
	if (option == FD_ATTR_OUT_REP) {
		uint32_t rcr = fd_ch_readl(fd, ch, FD_REG_RCR);
		if(rcr & FD_RCR_CONT)
		    	*usr_val = 0xffffffff;
		else
	            *usr_val = FD_RCR_REP_CNT_R(rcr) + 1;
		return 0;
	}

	return 0;
}

int fd_wr_mode(struct fd_dev *fd, int on)
{
	//printk("--*%s\n", __func__);
	unsigned long flags;
	uint32_t tcr;

	//printk("&fd->flags = 0x%08x, fd->flags = 0x%08x\n", &fd->flags, fd->flags);
	//spin_lock_irqsave(&fd->lock, flags);
	tcr = fd_readl(fd, FD_REG_TCR);

	if (on) {
		fd_writel(fd, FD_TCR_WR_ENABLE, FD_REG_TCR);
		set_bit(FD_FLAG_WR_MODE, fd->flags);
	} else {
		fd_writel(fd, 0, FD_REG_TCR);
		clear_bit(FD_FLAG_WR_MODE, fd->flags);
		/* not white-rabbit: write default to DAC for VCXO */
		fd_spi_xfer(fd, FD_CS_DAC, 24,
			    fd->calib.vcxo_default_tune & 0xffff, NULL);
	}
	
	//printk("&fd->flags = 0x%08x, fd->flags = 0x%08x\n", &fd->flags, fd->flags);
	//mprintf("tcr = %08x and FD_TCR_WR_PRESENT = %08x = %08x\n", tcr, FD_TCR_WR_PRESENT, !(tcr & FD_TCR_WR_PRESENT));
	//mprintf("tcr = %08x and FD_TCR_WR_LINK = %08x = %08x\n", tcr, FD_TCR_WR_LINK, !(tcr & FD_TCR_WR_LINK));
	//spin_unlock_irqrestore(&fd->lock, flags);
	
	if(! (tcr & FD_TCR_WR_PRESENT)){
		//mprintf("EOPNOTSUPP\n");
		return -EOPNOTSUPP;
	}
	else if( ! (tcr & FD_TCR_WR_LINK)){
		//mprintf("FD_TCR_WR_LINK\n");
		return -ENOLINK;
	}
	else
		return 0;
}

int fd_wr_query(struct fd_dev *fd)
{
	////printk("--*%s\n", __func__);
	int ena = test_bit(FD_FLAG_WR_MODE, fd->flags);

	if (!ena)
		return -ENODEV;
	if (! (fd_readl(fd, FD_REG_TCR) & FD_TCR_WR_LINK))
		return -ENOLINK;
	if (fd_readl(fd, FD_REG_TCR) & FD_TCR_WR_LOCKED)
		return 0;
	return -EAGAIN;
}


/* Overall and device-wide attributes: only get_time is special *
static int fd_zio_info_get(struct device *dev, struct zio_attribute *zattr,
			   uint32_t *usr_val)
{
	//printk("--*%s\n", __func__);
	struct fd_time t;
	struct zio_device *zdev;
	struct fd_dev *fd;
	struct zio_attribute *attr;

	if (__fd_get_type(dev) == FD_TYPE_INPUT)
		return fd_zio_info_tdc(dev, zattr, usr_val);
	if (__fd_get_type(dev) == FD_TYPE_OUTPUT)
		return fd_zio_info_output(dev, zattr, usr_val);

	/* reading temperature *
	zdev = to_zio_dev(dev);
	attr = zdev->zattr_set.ext_zattr;
	fd = zdev->priv_d;

	if (zattr->id == FD_ATTR_DEV_TEMP) {
		if (fd->temp_ready)
		{
			attr[FD_ATTR_DEV_TEMP].value = fd->temp;
			return 0;
		} else
			return -EAGAIN;
	}
	/* following is whole-dev *
	if (zattr->id != FD_ATTR_DEV_UTC_H)
		return 0;
	/* reading utc-h calls an atomic get-time *
	fd_time_get(fd, &t, NULL);
	attr[FD_ATTR_DEV_UTC_H].value = t.utc >> 32;
	attr[FD_ATTR_DEV_UTC_L].value = t.utc & 0xffffffff;
	attr[FD_ATTR_DEV_COARSE].value = t.coarse;
	return 0;
}

/* TDC input attributes: the flags */

/*int fd_zio_conf_tdc(struct device *dev, struct zio_attribute *zattr,
			    uint32_t  usr_val)*/
int fd_zio_conf_tdc(struct fd_dev *fd, enum fd_zattr_in_idx option,
			     uint32_t usr_val)	
{
	//printk("--*%s\n", __func__);
	//struct zio_cset *cset;
	//struct fd_dev *fd;
	uint32_t reg;
	int change;

	//printk("%i\n", option);
	//cset = to_zio_cset(dev);
	//fd = cset->zdev->priv_d;

	switch (option) {
	case FD_ATTR_TDC_OFFSET:
		////printk("FD_ATTR_TDC_OFFSET\n");
		fd->calib.tdc_zero_offset = usr_val;
		goto out;

	case FD_ATTR_TDC_USER_OFF:
		////printk("FD_ATTR_TDC_USER_OFF\n");
		fd->tdc_user_offset = usr_val;
		goto out;

	case FD_ATTR_TDC_FLAGS:
		////printk("FD_ATTR_TDC_FLAGS\n");
		break; /* code below */
	default:
		goto out;
	}

	/* This code is only about FD_ATTR_TDC_FLAGS */
	change = fd->tdc_flags ^ usr_val; /* old xor new */
	////printk("%i ^ %i = %i\n", fd->tdc_flags, usr_val, change);

	/* No need to lock, as configuration is serialized by zio-core */
	if (change & FD_TDCF_DISABLE_INPUT) {
		reg = fd_readl(fd, FD_REG_GCR);
		////printk("%08x\n", change & FD_TDCF_DISABLE_INPUT);
		if (usr_val & FD_TDCF_DISABLE_INPUT)
			reg &= ~FD_GCR_INPUT_EN;
		else
			reg |= FD_GCR_INPUT_EN;
		fd_writel(fd, reg, FD_REG_GCR);
	}

	if (change & FD_TDCF_DISABLE_TSTAMP) {
		////printk("%08x\n", change & FD_TDCF_DISABLE_TSTAMP);
		reg = fd_readl(fd, FD_REG_TSBCR);
		if (usr_val & FD_TDCF_DISABLE_TSTAMP)
			reg &= ~FD_TSBCR_ENABLE;
		else
			reg |= FD_TSBCR_ENABLE;
		fd_writel(fd, reg, FD_REG_TSBCR);
	}

	if (change & FD_TDCF_TERM_50) {
		////printk("%08x\n", change & FD_TDCF_TERM_50);
		if (usr_val & FD_TDCF_TERM_50){
			////mprintf("fd_gpio_set(fd, FD_GPIO_TERM_EN)\n");
			fd_gpio_set(fd, FD_GPIO_TERM_EN);
		}
		else{
			////mprintf("fd_gpio_clr(fd, FD_GPIO_TERM_EN)\n");
			fd_gpio_clr(fd, FD_GPIO_TERM_EN);
		}
	}
out:
	/* We need to store in the local array too (see info_tdc() above) */
	fd->tdc_flags = usr_val;

	return 0;
}

/* only the two offsets */
/*static int fd_zio_conf_output(struct device *dev, struct zio_attribute *zattr,
			      uint32_t  usr_val)
{
	//printk("--*%s\n", __func__);
	struct zio_cset *cset;
	struct fd_dev *fd;
	int ch;

	cset = to_zio_cset(dev);
	fd = cset->zdev->priv_d;
	ch = cset->index - 1;

	if (zattr->id == FD_ATTR_OUT_DELAY_OFF) {
		fd->calib.zero_offset[ch] = usr_val;
		return 0;
	}
	if (zattr->id == FD_ATTR_OUT_USER_OFF) {
		fd->ch_user_offset[ch] = usr_val;
		return 0;
	}
	return 0;
}

/* conf_set dispatcher and  and device-wide attributes */
/*static int fd_zio_conf_set(struct device *dev, struct zio_attribute *zattr, uint32_t  usr_val)*/
int fd_zio_conf_set(struct fd_dev *fd, enum fd_zattr_in_idx option, uint32_t *attr, uint32_t  usr_val)
{
	////printk("--*%s\n", __func__);
	struct fd_time t;
	//struct zio_device *zdev;
	//struct fd_dev *fd;
	//struct zio_attribute *attr;

	/*if (__fd_get_type(dev) == FD_TYPE_INPUT)
		return fd_zio_conf_tdc(dev, zattr, usr_val);
	if (__fd_get_type(dev) == FD_TYPE_OUTPUT)
		return fd_zio_conf_output(dev, zattr, usr_val);*/

	/* Remains: wholedev *
	zdev = to_zio_dev(dev);
	attr = zdev->zattr_set.ext_zattr;
	fd = zdev->priv_d;*/

	////mprintf("Recived %i\n", option);
	if (option == FD_ATTR_DEV_UTC_H) {
		////mprintf("FD_ATTR_DEV_UTC_H %i\n", FD_ATTR_DEV_UTC_H);
		/* no changing of the time when WR is on */
		if (test_bit(FD_FLAG_WR_MODE, fd->flags))
			return -EAGAIN;

		/* writing utc-h calls an atomic set-time */
		t.utc = (uint64_t)attr[FD_ATTR_DEV_UTC_H] << 32;
		t.utc |= attr[FD_ATTR_DEV_UTC_L];
		t.coarse = attr[FD_ATTR_DEV_COARSE];
		fd_time_set(fd, &t, NULL);
		return 0;
	}

	/* Not command, nothing to do *
	if (zattr->id != FD_ATTR_DEV_COMMAND)
		return 0;*/

	switch(usr_val) {
	case FD_CMD_WR_ENABLE:
		////mprintf("FD_CMD_WR_ENABLE\n");
		return fd_wr_mode(fd, 1);
	case FD_CMD_WR_DISABLE:
		return fd_wr_mode(fd, 0);
	case FD_CMD_WR_QUERY:
		return fd_wr_query(fd);
	/*case FD_CMD_DUMP_MCP:
		return fd_dump_mcp(fd);
	case FD_CMD_PURGE_FIFO:
		fd_writel(fd, FD_TSBCR_PURGE | FD_TSBCR_RST_SEQ
			  | FD_TSBCR_CHAN_MASK_W(1) | FD_TSBCR_ENABLE,
			  FD_REG_TSBCR);
		return 0;*/
	default:
		return -EINVAL;
	}
}

/*
 * We are over with attributes, now there's real I/O (part is in fd-irq.c)
 */

/* We need to change the time in attribute tuples, so here it is */
enum attrs {__UTC_H, __UTC_L, __COARSE, __FRAC}; /* the order of our attrs */

static void fd_attr_sub(uint32_t *a, uint32_t pico)
{
	////printk("--*%s\n", __func__);
	uint32_t coarse, frac;

	fd_split_pico(pico, &coarse, &frac);
	if (a[__FRAC] >= frac) {
		a[__FRAC] -= frac;
	} else {
		a[__FRAC] += 4096;
		a[__FRAC] -= frac;
		coarse++;
	}
	if (a[__COARSE] >= coarse) {
		a[__COARSE] -= coarse;
	} else {
		a[__COARSE] += 125*1000*1000;
		a[__COARSE] -= coarse;
		if ((a[__UTC_L] != 0)) {
			a[__UTC_L]--;
		} else {
			a[__UTC_L] = ~0;
			a[__UTC_H]--;
		}
	}
}

static void fd_attr_add(uint32_t *a, uint32_t pico)
{
	////printk("--*%s\n", __func__);
	uint32_t coarse, frac;

	fd_split_pico(pico, &coarse, &frac);
	a[__FRAC] += frac;
	if (a[__FRAC] >= 4096) {
		a[__FRAC] -= 4096;
		coarse++;
	}
	a[__COARSE] += coarse;
	if (a[__COARSE] >= 125*1000*1000) {
		a[__COARSE] -= 125*1000*1000;
		a[__UTC_L]++;
		if ((a[__UTC_L] == 0))
			a[__UTC_H]++;
	}
}

void fd_apply_offset(uint32_t *a, int32_t off_pico)
{
	//printk("--*%s\n", __func__);
	if (off_pico) {
		if (off_pico > 0)
			fd_attr_add(a, off_pico);
		else
			fd_attr_sub(a, -off_pico);
	}
}

/* Internal output engine */
int __fd_zio_output(struct fd_dev *fd, int index1_4, uint32_t *attrs)
{
	////printk("--*%s\n", __func__);
	struct timespec delta, width, delay;
	int ch = index1_4;
	int mode = attrs[FD_ATTR_OUT_MODE];
	int rep = attrs[FD_ATTR_OUT_REP];
	int dcr = 0;
	
	////mprintf("ch = %i\n", ch);

	////printk("mode == FD_OUT_MODE_DELAY = %0x, mode == FD_OUT_MODE_DISABLED = %x\n", FD_OUT_MODE_DELAY, FD_OUT_MODE_DISABLED);
	if (mode == FD_OUT_MODE_DELAY || mode == FD_OUT_MODE_DISABLED) {
		if(rep < 0 || rep > 16) /* delay mode allows trains of 1 to 16 pulses. */
			return 0;

		/* check delay lower limits. FIXME: raise an alarm */
		delay.tv_sec = attrs[FD_ATTR_OUT_START_L];
		delay.tv_nsec = attrs[FD_ATTR_OUT_START_COARSE] * 8;
		if (delay.tv_sec == 0 && delay.tv_nsec < 600)
			return 0;

		fd_apply_offset(attrs + FD_ATTR_OUT_START_H,
			    fd->calib.tdc_zero_offset);
		fd_apply_offset(attrs + FD_ATTR_OUT_END_H,
			    fd->calib.tdc_zero_offset);
	}

	fd_apply_offset(attrs + FD_ATTR_OUT_START_H,
			    fd->calib.zero_offset[ch]);

	printk("FD_ATTR_OUT_START_H = %08x, fd->calib.zero_offset[%i] = %08x\n", FD_ATTR_OUT_START_H, ch, fd->calib.zero_offset[ch]);

	fd_apply_offset(attrs + FD_ATTR_OUT_START_H,
			  fd->ch_user_offset[ch]);
			  
	printk("FD_ATTR_OUT_START_H = %08x, fd->ch_user_offset[%i] = %08x\n", FD_ATTR_OUT_START_H, ch, fd->ch_user_offset[ch]);

	fd_apply_offset(attrs + FD_ATTR_OUT_END_H,
			    fd->calib.zero_offset[ch]);

	////printk("FD_ATTR_OUT_END_H = %08x, fd->calib.zero_offset[%i] = %08x\n", FD_ATTR_OUT_END_H, ch, fd->calib.zero_offset[ch]);

	fd_apply_offset(attrs + FD_ATTR_OUT_END_H,
			  fd->ch_user_offset[ch]);

	////printk("FD_ATTR_OUT_END_H = %08x, fd->ch_user_offset[%i] = %08x\n", FD_ATTR_OUT_END_H, ch, fd->ch_user_offset[ch]);

	fd_ch_writel(fd, ch, fd->ch[ch].frr_cur,  FD_REG_FRR);
	
	////printk("ch = %08x, fd->ch[ch].frr_cur = %08x, FD_REG_FRR = %08x\n", ch, fd->ch[ch].frr_cur, FD_REG_FRR);

	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_START_H],      FD_REG_U_STARTH);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_REG_U_STARTH, attrs[FD_ATTR_OUT_START_H]);
	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_START_L],      FD_REG_U_STARTL);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_ATTR_OUT_START_L, attrs[FD_ATTR_OUT_START_L]);
	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_START_COARSE], FD_REG_C_START);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_ATTR_OUT_START_COARSE, attrs[FD_ATTR_OUT_START_COARSE]);
	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_START_FINE],   FD_REG_F_START);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_ATTR_OUT_START_FINE, attrs[FD_ATTR_OUT_START_FINE]);

	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_END_H],      FD_REG_U_ENDH);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_ATTR_OUT_END_H, attrs[FD_ATTR_OUT_END_H]);
	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_END_L],      FD_REG_U_ENDL);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_ATTR_OUT_END_L, attrs[FD_ATTR_OUT_END_L]);
	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_END_COARSE], FD_REG_C_END);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_ATTR_OUT_END_COARSE, attrs[FD_ATTR_OUT_END_COARSE]);
	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_END_FINE],   FD_REG_F_END);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_ATTR_OUT_END_FINE, attrs[FD_ATTR_OUT_END_FINE]);

	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_DELTA_L],      FD_REG_U_DELTA);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_ATTR_OUT_START_L, attrs[FD_ATTR_OUT_DELTA_L]);
	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_DELTA_COARSE], FD_REG_C_DELTA);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_ATTR_OUT_DELTA_COARSE, attrs[FD_ATTR_OUT_DELTA_COARSE]);
	fd_ch_writel(fd, ch, attrs[FD_ATTR_OUT_DELTA_FINE],   FD_REG_F_DELTA);
	////printk("ch = %08x, attrs[%08x] = %08x\n", ch, FD_ATTR_OUT_DELTA_FINE, attrs[FD_ATTR_OUT_START_L]);
	
	////printk("mode == FD_OUT_MODE_DELAY = %0x, mode == FD_OUT_MODE_DISABLED = %x\n", FD_OUT_MODE_DELAY, FD_OUT_MODE_DISABLED);
	if (mode == FD_OUT_MODE_DELAY || mode == FD_OUT_MODE_DISABLED) {
		dcr = 0;
		fd_ch_writel(fd, ch, FD_RCR_REP_CNT_W(rep - 1)
			     | (rep < 0 ? FD_RCR_CONT : 0), FD_REG_RCR);
	} else {
		dcr = FD_DCR_MODE;
		fd_ch_writel(fd, ch, FD_RCR_REP_CNT_W(rep < 0 ? 0 : rep - 1)
			    | (rep < 0 ? FD_RCR_CONT : 0), FD_REG_RCR);
	}

	/*
	 * For narrowly spaced pulses, we don't have enough time to reload
	 * the tap number into the corresponding SY89295.
	 * Therefore, the width/spacing resolution is limited to 4 ns.
	 * We put the threshold at 200ns, i.e. when coarse == 25.
	 *
	 * Trivially it would be
	 *    if((delta_ps - width_ps) < 200000 || (width_ps < 200000))
	 *               dcr |= FD_DCR_NO_FINE;
	 *
	 * Most likely the calculation below fails with negatives, but
	 * with negative spacing we get no pulses, and fine is irrelevant
	 */
	////printk("NSEC_PER_SEC = %i\n", (unsigned int) NSEC_PER_SEC);
	delta.tv_sec = attrs[FD_ATTR_OUT_DELTA_L];
	delta.tv_nsec = attrs[FD_ATTR_OUT_DELTA_COARSE] * 8;
	width.tv_sec = ((uint64_t)(attrs[FD_ATTR_OUT_END_H]) << 32
			| attrs[FD_ATTR_OUT_END_L])
		- ((uint64_t)(attrs[FD_ATTR_OUT_START_H]) << 32
		   | attrs[FD_ATTR_OUT_START_L]);
	if (attrs[FD_ATTR_OUT_END_COARSE] > attrs[FD_ATTR_OUT_START_COARSE]) {
		////printk("1st if\n");
		width.tv_nsec = 8 * attrs[FD_ATTR_OUT_END_COARSE]
			- 8 * attrs[FD_ATTR_OUT_START_COARSE];
	} else {
		////printk("1st else\n");
		width.tv_sec--;
		width.tv_nsec = NSEC_PER_SEC 
			- 8 * attrs[FD_ATTR_OUT_START_COARSE]
			+ 8 * attrs[FD_ATTR_OUT_END_COARSE];		
		////printk("NSEC_PER_SEC-8*%08x+8*%08x=%08x\n", attrs[FD_ATTR_OUT_START_COARSE], attrs[FD_ATTR_OUT_END_COARSE], width.tv_nsec);
	}
	////printk("*-*-*--> width=%llu:%llu\n", width.tv_sec, width.tv_nsec);
	////printk("*-*-*--> delta=%llu:%llu\n", delta.tv_sec, delta.tv_nsec);
	/* delta = delta - width (i.e.: delta is the low-signal width */
	delta.tv_sec -= width.tv_sec;
	if (delta.tv_nsec > width.tv_nsec) {
		delta.tv_nsec -= width.tv_nsec;
		////printk("*-*-*- 2nd if -> delta=%llu:%llu\n", delta.tv_sec, delta.tv_nsec);
	} else {
		////printk("2nd else\n");
		delta.tv_sec--;
		delta.tv_nsec = NSEC_PER_SEC - width.tv_nsec + delta.tv_nsec;
		////printk("*-*-*-2nd else -> delta=%llu:%llu\n", delta.tv_sec, delta.tv_nsec);
	}
	/* finally check */
	if (width.tv_sec == 0 && width.tv_nsec < 200)
		dcr |= FD_DCR_NO_FINE;
	if (delta.tv_sec == 0 && delta.tv_nsec < 200)
		dcr |= FD_DCR_NO_FINE;
		
	////printk("*-*-*--> width=%llu:%llu\n", width.tv_sec, width.tv_nsec);
	////printk("*-*-*--> delta=%llu:%llu\n", delta.tv_sec, delta.tv_nsec);
	fd_ch_writel(fd, ch, dcr, FD_REG_DCR);
	////printk("ch = %08x, dcr = %08x, FD_REG_DCR = %08x\n", ch, dcr, FD_REG_DCR);
	fd_ch_writel(fd, ch, dcr | FD_DCR_UPDATE, FD_REG_DCR);
	////printk("ch = %08x, dcr | FD_DCR_UPDATE = %08x, FD_REG_DCR = %08x\n", ch, dcr | FD_DCR_UPDATE, FD_REG_DCR);
	
	////printk("mode = %08x\n");
	 
	if (mode == FD_OUT_MODE_DELAY) {
	    fd_ch_writel(fd, ch, dcr | FD_DCR_ENABLE, FD_REG_DCR);
	   // //printk("FD_OUT_MODE_DELAY\n");
	   // //printk("ch = %08x, dcr = %08x, dcr | FD_DCR_ENABLE = %08x, FD_REG_DCR = %08x\n", ch, dcr, dcr | FD_DCR_ENABLE, FD_REG_DCR);
	} else if (mode == FD_OUT_MODE_PULSE) {
		fd_ch_writel(fd, ch, dcr | FD_DCR_ENABLE | FD_DCR_PG_ARM,  FD_REG_DCR);
		////printk("FD_OUT_MODE_PULSE\n");
		////printk("ch = %08x, dcr = %08x, dcr | FD_DCR_ENABLE | FD_DCR_PG_ARM = %08x, FD_REG_DCR = %08x\n", ch, dcr, dcr | FD_DCR_ENABLE | FD_DCR_PG_ARM, FD_REG_DCR);
	}
	return 0;
}

/* This is called on user write */
int fd_zio_output(struct fd_dev *fd, int channel, uint32_t *a)
{
	//printk("--*%s\n", __func__);
	int i;
	//struct fd_dev *fd;
	//struct zio_control *ctrl;

	//fd = cset->zdev->priv_d;
	//ctrl = zio_get_ctrl(cset->chan->active_block);

	if (1) {
		dev_info(&fd->fmc->dev,
			 "%s: attrs for chanell %i: ", __func__, channel);
		/*for (i = FD_ATTR_DEV__LAST; i < FD_ATTR_OUT__LAST; i++)
			//printk("%08x%c", a[i],
			       i == FD_ATTR_OUT__LAST -1 ? '\n' : ' ');*/
	}
	return __fd_zio_output(fd, channel, a);
}

/*
 * The input method may return immediately, because input is
 * asynchronous. The data_done callback is invoked when the block is
 * full.
 */
/*static int fd_zio_input(struct zio_cset *cset)
{
	//printk("--*%s\n", __func__);
	struct fd_dev *fd;
	fd = cset->zdev->priv_d;

	/* Configure the device for input *
	if (!test_bit(FD_FLAG_DO_INPUT, &fd->flags)) {
		fd_writel(fd, FD_TSBCR_PURGE | FD_TSBCR_RST_SEQ, FD_REG_TSBCR);
		fd_writel(fd, FD_TSBCR_CHAN_MASK_W(1) | FD_TSBCR_ENABLE,
			  FD_REG_TSBCR);
		set_bit(FD_FLAG_DO_INPUT, &fd->flags);
	}
	/* Ready for input. If there's already something, return it now *
	if (fd_read_sw_fifo(fd, cset->chan) == 0) {
		return 0; /* don't call data_done, let the caller do it *
	}
	/* Mark the active block is valid, and return EAGAIN *
	set_bit(FD_FLAG_INPUT_READY, &fd->flags);
	return -EAGAIN;
}

/*
 * The probe function receives a new zio_device, which is different from
 * what we allocated (that one is the "hardwre" device) but has the
 * same private data. So we make the link and return success.
 */
/*static int fd_zio_probe(struct zio_device *zdev)
{
	//printk("--*%s\n", __func__);
	struct fd_dev *fd;

	/* link the new device from the fd structure *
	fd = zdev->priv_d;
	fd->zdev = zdev;

	fd->tdc_attrs[FD_CSET_INDEX(FD_ATTR_TDC_OFFSET)] = \
		fd->calib.tdc_zero_offset;

	/* We don't have csets at this point, so don't do anything more *
	return 0;
}

/* Our sysfs operations to access internal settings */
/*static const struct zio_sysfs_operations fd_zio_s_op = {
	.conf_set = fd_zio_conf_set,
	.info_get = fd_zio_info_get,
};*/

/* We have 5 csets, since each output triggers separately */
/*static struct zio_cset fd_cset[] = {
	{
		ZIO_SET_OBJ_NAME("fd-input"),
		.raw_io =	fd_zio_input,
		.n_chan =	1,
		.ssize =	0,
		.flags =	ZIO_DIR_INPUT | ZIO_CSET_TYPE_TIME
				| ZIO_CSET_SELF_TIMED,
		.zattr_set = {
			.ext_zattr = fd_zattr_input,
			.n_ext_attr = ARRAY_SIZE(fd_zattr_input),
		},
	},
	{
		ZIO_SET_OBJ_NAME("fd-ch1"),
		.raw_io =	fd_zio_output,
		.n_chan =	1,
		.ssize =	0,
		.flags =	ZIO_DIR_OUTPUT | ZIO_CSET_TYPE_TIME,
		.zattr_set = {
			.ext_zattr = fd_zattr_output,
			.n_ext_attr = ARRAY_SIZE(fd_zattr_output),
		},
	},
	{
		ZIO_SET_OBJ_NAME("fd-ch2"),
		.raw_io =	fd_zio_output,
		.n_chan =	1,
		.ssize =	0,
		.flags =	ZIO_DIR_OUTPUT | ZIO_CSET_TYPE_TIME,
		.zattr_set = {
			.ext_zattr = fd_zattr_output,
			.n_ext_attr = ARRAY_SIZE(fd_zattr_output),
		},
	},
	{
		ZIO_SET_OBJ_NAME("fd-ch3"),
		.raw_io =	fd_zio_output,
		.n_chan =	1,
		.ssize =	0,
		.flags =	ZIO_DIR_OUTPUT | ZIO_CSET_TYPE_TIME,
		.zattr_set = {
			.ext_zattr = fd_zattr_output,
			.n_ext_attr = ARRAY_SIZE(fd_zattr_output),
		},
	},
	{
		ZIO_SET_OBJ_NAME("fd-ch4"),
		.raw_io =	fd_zio_output,
		.n_chan =	1,
		.ssize =	0,
		.flags =	ZIO_DIR_OUTPUT | ZIO_CSET_TYPE_TIME,
		.zattr_set = {
			.ext_zattr = fd_zattr_output,
			.n_ext_attr = ARRAY_SIZE(fd_zattr_output),
		},
	},
};

static struct zio_device fd_tmpl = {
	.owner =		THIS_MODULE,
	.preferred_trigger =	"user",
	.s_op =			&fd_zio_s_op,
	.cset =			fd_cset,
	.n_cset =		ARRAY_SIZE(fd_cset),
	.zattr_set = {
		.std_zattr= fd_zattr_dev_std,
		.ext_zattr= fd_zattr_dev,
		.n_ext_attr = ARRAY_SIZE(fd_zattr_dev),
	},
};

static const struct zio_device_id fd_table[] = {
	{"fd", &fd_tmpl},
	{},
};

static struct zio_driver fd_zdrv = {
	.driver = {
		.name = "fd",
		.owner = THIS_MODULE,
	},
	.id_table = fd_table,
	.probe = fd_zio_probe,
};


/* Register and unregister are used to set up the template driver */
/*int fd_zio_register(void)
{
	//printk("--*%s\n", __func__);
	int err;

	if (fd_use_raw_tdc) {
		/* Hack: change the input channel to return raw registers *
		fd_cset[0].ssize = sizeof(struct fd_time);
		fd_cset[0].flags = ZIO_DIR_INPUT | ZIO_CSET_TYPE_RAW;
	}

	err = zio_register_driver(&fd_zdrv);
	if (err)
		return err;

	return 0;
}

void fd_zio_unregister(void)
{
	zio_unregister_driver(&fd_zdrv);
	/* FIXME *
}


/* preinitializes the outputs to some meaningful register values */
/*static void __fd_init_outputs(struct fd_dev *fd)
{
	//printk("--*%s\n", __func__);
	uint32_t attrs [ FD_ATTR_OUT__LAST]; 
	int i;
	
	attrs [ FD_ATTR_OUT_MODE ] = FD_OUT_MODE_DISABLED;
	attrs [ FD_ATTR_OUT_REP ] = 1;
	attrs [ FD_ATTR_OUT_START_H ] = 0;
	attrs [ FD_ATTR_OUT_START_L ] = 0;
	attrs [ FD_ATTR_OUT_START_COARSE ] = 75; /* 600 ns delay *
	attrs [ FD_ATTR_OUT_START_FINE ] = 0;
	attrs [ FD_ATTR_OUT_END_H ] = 0;
	attrs [ FD_ATTR_OUT_END_L ] = 0;
	attrs [ FD_ATTR_OUT_END_COARSE ] = 75 + 31; /* 250 ns width *
	attrs [ FD_ATTR_OUT_END_FINE ] = 1024;
	attrs [ FD_ATTR_OUT_DELTA_L ] = 0;
	attrs [ FD_ATTR_OUT_DELTA_COARSE ] = 125; /* 1us ns period *
	attrs [ FD_ATTR_OUT_DELTA_FINE ] = 0;

	for (i = 1; i <= 4; i++)
	    __fd_zio_output(fd, i, attrs);
}*/

/* Init and exit are called for each FD card we have */
/*int fd_zio_init(struct fd_dev *fd)
{
	//printk("--*%s\n", __func__);
	int err = 0;
	int dev_id;

	fd->hwzdev = zio_allocate_device();
	if (IS_ERR(fd->hwzdev))
		return PTR_ERR(fd->hwzdev);

	/* Mandatory fields *
	fd->hwzdev->owner = THIS_MODULE;
	fd->hwzdev->priv_d = fd;

	dev_id = fd->fmc->device_id;

	err = zio_register_device(fd->hwzdev, "fd", dev_id);
	if (err) {
		zio_free_device(fd->hwzdev);
		return err;
	}

	__fd_init_outputs(fd);

	return 0;
}

void fd_zio_exit(struct fd_dev *fd)
{
	//printk("--*%s\n", __func__);
	zio_unregister_device(fd->hwzdev);
	zio_free_device(fd->hwzdev);
}*/
