/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2011,2012 CERN (www.cern.ch)
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * Author: Grzegorz Daniluk <grzegorz.daniluk@cern.ch>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#include <stdio.h>
#include <inttypes.h>

#include <stdarg.h>

#include <wrc.h>
//#include <w1.h>
#include "syscon.h"
#include "uart.h"
//#include "endpoint.h"
//#include "minic.h"
#include "pps_gen.h"
//#include "ptpd_netif.h"
#include "i2c.h"
#include "eeprom.h"
//#include "softpll_ng.h"
#include "onewire.h"
#include "pps_gen.h"
#include "shell.h"
//#include "lib/ipv4.h"
//#include "rxts_calibrator.h"

//#include "wrc_ptp.h"

void _irq_entry()
{}

int wrc_ui_mode = UI_SHELL_MODE;
//int wrc_ui_refperiod = TICS_PER_SECOND; /* 1 sec */
//int wrc_phase_tracking = 1;

//int wrc_man_phase = 0;

static void ui_update()
{
	shell_interactive();
}

extern uint32_t _endram;
extern uint32_t _fstack;
#define ENDRAM_MAGIC 0xbadc0ffe

static void check_stack(void)
{
	while (_endram != ENDRAM_MAGIC) {
		mprintf("Stack overflow!\n");
		timer_delay_ms(1000);
	}
}

#ifdef CONFIG_CHECK_RESET

static void check_reset(void)
{
	extern void _reset_handler(void); /* user to reset again m */
	/* static variables to preserve stack (for dumping it) */
	static uint32_t *p, *save;

	/* _endram is set to ENDRAM_MAGIC after calling this function */
	if (_endram != ENDRAM_MAGIC)
		return;

	/* Before calling anything, find the beginning of the stack */
	p = &_endram + 1;
	while (!*p)
		p++;
	p = (void *)((unsigned long)p & ~0xf); /* align 

	/* Copy it to the beginning of the stack, then reset pointers */
	save = &_endram;
	while (p <= &_fstack)
		*save++ = *p++;
	p -= (save - &_endram);
	save = &_endram;

	/* Ok, now init the devices so we can printf and delay */
	sdb_find_devices();
	uart_init_sw();
	uart_init_hw();

	pp_printf("\nWarning: the CPU was reset\nStack trace:\n");
	while (p < &_fstack) {
		pp_printf("%08x: %08x %08x %08x %08x\n",
			  (int)p, save[0], save[1], save[2], save[3]);
		p += 4;
		save += 4;
	}
	pp_printf("Rebooting in 1 second\n\n\n");
	timer_delay_ms(1000);

	/* Zero the stack and start over (so we dump correctly next time) */
	for (p = &_endram; p < &_fstack; p++)
		*p = 0;
	_endram = 0;
	_reset_handler();
}

# else /* no CONFIG_CHECK_RESET */

static void check_reset(void) {}

#endif


int main(void)
{
	//check_reset();
	wrc_ui_mode = UI_SHELL_MODE;
	_endram = ENDRAM_MAGIC;
	
	sdb_find_devices();
	uart_init_sw();
	uart_init_hw();

	shell_init();


	for (;;) {
		ui_update();
		check_stack();
	}
}
