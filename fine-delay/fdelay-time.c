/*
 * Time-related functions
 *
 * Copyright (C) 2012 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2 as published by the Free Software Foundation or, at your
 * option, any later version.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

//#include <linux/zio.h>
//#include <linux/zio-user.h>
#define FDELAY_INTERNAL
#include "fdelay-lib.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
int fd_time_get(struct fd_dev *fd, struct fd_time *t, struct timespec *ts);
extern struct fd_dev fd; 

static char *names[] = {
	"utc-h",
	"utc-l",
	"coarse"
};

int fdelay_set_time(struct fdelay_board *userb, struct fdelay_time *t)
{
	//__define_board(b, userb);
	uint32_t attrs[ARRAY_SIZE(names)];
	//struct fd_time tm;
	int i;

	/*attrs[0] = t->utc >> 32;
	attrs[1] = t->utc;
	attrs[2] = t->coarse;*/


	if (fd_time_set(&fd, &t, NULL)<0)
		return -1;
	
		
	return 0;
}

int fdelay_get_time(struct fdelay_board *userb, struct fdelay_time *t)
{
	__define_board(b, userb);
	uint32_t attrs[ARRAY_SIZE(names)];
	int i;


	for (i = 0; i < ARRAY_SIZE(names); i++)
		if (fdelay_sysfs_get(b, names[i], attrs + i) < 0)
			return -1;
	t->utc = (long long)attrs[0] << 32;
	t->utc += attrs[1];
	t->coarse = attrs[2];
	return 0;
}

/*int fdelay_set_host_time(struct fdelay_board *userb)
{
	__define_board(b, userb);
	return __fdelay_command(b, FD_CMD_HOST_TIME);
}*/

