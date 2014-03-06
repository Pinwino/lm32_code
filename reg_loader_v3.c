#include <stdio.h>
#include <inttypes.h>

#include <stdarg.h>

#include <wrc.h>
#include <w1.h>
#include "uart.h"
#include "eeprom.h"
#include "hw/fd_channel_regs.h"
#include "hw/fd_main_regs.h"
#include <pp-printf.h>
#define mprintf pp_printf
#define vprintf pp_vprintf
#define sprintf pp_sprintf

extern unsigned char *BASE_UART;
extern unsigned char *BASE_FINE_DELAY;

 
int main(void)
{
	const unsigned int main_registers[] = {FD_REG_TM_SECH, FD_REG_TM_SECL, FD_REG_TM_CYCLES};
	
	const unsigned int port_base_adress[] = {0x300, 0x400}; 
	const unsigned int port_registers[] = {FD_REG_DCR, FD_REG_U_STARTH, FD_REG_U_STARTL, FD_REG_C_START, FD_REG_F_START, FD_REG_U_ENDH, FD_REG_U_ENDL, FD_REG_C_END, FD_REG_F_END, FD_REG_U_DELTA, FD_REG_C_DELTA, FD_REG_F_DELTA, FD_REG_RCR};

	const unsigned int pps_vals[]	= {0x0, 0x0, 0x0, 0xbebc21, 0x760, 0x0, 0x0, 0xbebc27, 0xb60, 0x1, 0x0, 0x000, 0x10000};
	const unsigned int XMhz_vals[]	= {0x0, 0x0, 0x0, 0xbebc1b, 0x34a, 0x0, 0x0, 0xbebc21, 0x74a, 0x0, 0x6, 0x400, 0x10000};
	const unsigned int crtl []		= {0x81, 0x92, 0x83, 0x87};
	
	int i, j, *dir, *ind, sel, ch;
	sdb_find_devices();
	uart_init_sw();
	uart_init_hw();

	for (i=0; i<10000000; i++)
		__asm__("nop\n");

	mprintf("\n\n**********************************************************\n");
	mprintf("LM32 UART: starting up...\n\n");
	mprintf("Uart base adress %08X\n", BASE_UART);
	mprintf("Finde Dalay base adress %08X\n\n", BASE_FINE_DELAY);
	
	dir=BASE_FINE_DELAY + FD_REG_RSTR;
	*dir=0xdead0003;
}

