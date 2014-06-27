/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2013 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
//#include <wrc.h>
//#include <syscon.h>
#include "sys/types.h"
#include <shell.h>
#include <linux/jiffies.h>
#include "hw/tics.h"
#include "board-wrc.h"


#define timer_get_tics() jiffies 

static unsigned long usleep_lpj; /* loops per jiffy */


static inline void __delay(unsigned long count)
{
	while (count > 0){
		count--;
		asm("");
	}
}


//static unsigned long verify_lpj(unsigned long lpj)
unsigned long verify_lpj(unsigned long lpj)
{
	unsigned long j, k, l;

	/* wait for the beginning of a tick */
	//pp_printf("wait for the beginning of a tick\n");
	j = timer_get_tics();
//	j = timer_get_tics();
	//while (timer_get_tics() < j)
		//; //pp_printf("imer_get_tics() %lu j %lu\n", timer_get_tics() , j);

	__delay(lpj);

	/* did it expire? */
	//l=timer_get_tics();
	j =  timer_get_tics() - j;
	if (0)
		pp_printf("check %7i: j %li\n", lpj, j);
		
	return j;
}

void usleep_init(void)
{
	unsigned long lpj = 1, test_lpj=0;
	unsigned long  step = 9;

	/* Increase until we get over it 
	while (verify_lpj(lpj) == 0) {
		lpj += step;
		step *= 2;
	}
	/* Ok, now we are over; half again and restart 
	lpj /= 2; step /= 4;

	/* So, *this* jpj is lower, and with two steps we are higher 
	while (step) {
		test_lpj = lpj + step;
		if (verify_lpj(test_lpj) == 0)
			lpj = test_lpj;
		step /= 2;
	}*/
	
	while(1){
		test_lpj = verify_lpj(lpj);
		if ((test_lpj*10/TICS_PER_uSECOND) >= step)
			break;
		lpj++;
	}
	
	usleep_lpj = lpj-1;
	pp_printf("Loops per jiffy: %lu\n", usleep_lpj);
}

/* lpj is around 20800 on the spec: the above calculation overflows at 200ms */
int usleep(useconds_t usec)
{
	/* Sleep 10ms each time, so we support 20x faster cards */
	const unsigned long step = 10 * 1000;
	//const unsigned long usec_per_jiffy = 1000 * 1000 / TICS_PER_SECOND;
	const unsigned long count_per_step = usleep_lpj * step;

	while (usec > step)  {
		__delay(count_per_step);
		usec -= step;
		//pp_printf ("usec = %i, step = %i\n", usec, step);
	}
	
	//pp_printf ("usec = %i, step = %i\n", usec, step);
	__delay(usec * usleep_lpj);
	return 0;
}
