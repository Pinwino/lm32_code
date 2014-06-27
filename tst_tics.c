#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdarg.h>

#include <wrc.h>
#include <w1.h>
#include "uart.h"
#include "eeprom.h"
#include "fine-delay.h"
#include "hw/fd_channel_regs.h"
#include "hw/fd_main_regs.h"
#include "syscon.h"
#include <pp-printf.h>
#include "errno.h"
#include "shell.h"
#include "lib/ipv4.h"
#include "hw/irq_timer.h"
#include "irq.h"
#include "irq_ctrl.h"


#define mprintf pp_printf
#define vprintf pp_vprintf
#define sprintf pp_sprintf
#define HZ 100
#define udelay usleep
#define msleep timer_delay_ms


struct fd_dev fd;
//struct fmc_device fmc_loc;
struct irq_timer itmr;

extern unsigned char * BASE_TIMER;

extern uint32_t _endram;
extern uint32_t _fstack;
uint32_t irq_count = 0;
#define ENDRAM_MAGIC 0xbadc0ffe


void _irq_entry()
{
	uint32_t *dir;
	mprintf("\n************** WITH INTERRUPTS **************\n");
	mprintf("Interrupt num %u\n", irq_count);
	irq_pop();
	irq_count++;
	clear_irq();
	
	//fd->temp_timer.function(fd->temp_timer.data)
}

static void check_stack(void)
{
	while (_endram != ENDRAM_MAGIC) {
		mprintf("Stack overflow!\n");
		timer_delay_ms(1000);
	}
}


 uint64_t div_u64_rem(uint64_t n, uint32_t base, uint32_t *remainder){
	uint64_t rem = n;
	uint64_t b = base;
	uint64_t res, d = 1;
	uint32_t high = rem >> 32;
	
	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (uint64_t) high << 32;
		rem -= (uint64_t) (high*base) << 32;
	}
	
	while ((int64_t)b > 0 && b < rem) {
		b = b+b;
		d = d+d;
	}
	
	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		
		b >>= 1;
		d >>= 1;
	} while (d);
	
	*remainder = rem;
		
	return res;
}

static void ui_update()
{
	shell_interactive();
}
 
 void arm_checker (struct irq_timer *itmr){
	 if (irq_timer_check_armed (itmr))
		mprintf("armed\n");
	else
		mprintf("disarmed\n");
}
 
int main(void)
{
	const unsigned int main_registers[] = {FD_REG_TM_SECH, FD_REG_TM_SECL, FD_REG_TM_CYCLES};
	
	const unsigned int port_base_adress[] = {0x300, 0x400}; 
	const unsigned int port_registers[] = {FD_REG_DCR, FD_REG_U_STARTH, FD_REG_U_STARTL, FD_REG_C_START, FD_REG_F_START, FD_REG_U_ENDH, FD_REG_U_ENDL, FD_REG_C_END, FD_REG_F_END, FD_REG_U_DELTA, FD_REG_C_DELTA, FD_REG_F_DELTA, FD_REG_RCR};

	const unsigned int pps_vals[]	= {0x0, 0x0, 0x0, 0xbebc21, 0x760, 0x0, 0x0, 0xbebc27, 0xb60, 0x1, 0x0, 0x000, 0x10000};
	const unsigned int XMhz_vals[]	= {0x0, 0x0, 0x0, 0xbebc1b, 0x34a, 0x0, 0x0, 0xbebc21, 0x74a, 0x0, 0x6, 0x400, 0x10000};
	const unsigned int crtl []		= {0x81, 0x92, 0x83, 0x87};
	
	int wrc_ui_mode = UI_SHELL_MODE;

	uint32_t *dir;
	int i, j, *ind, sel, ch, eep;
	size_t size=0;
	
	_endram = ENDRAM_MAGIC;
	
	sdb_find_devices();
	uart_init_sw();
	uart_init_hw();
	
	shell_init();
	
	enable_irq();
	
	fd.fd_regs_base = BASE_FINE_DELAY;
	/*fd.fd_owregs_base= fd.fd_regs_base + 0x500;		/* regs_base + 0x500 */
	
	itmr.timer_addr_base = BASE_TIMER;
	itmr.timer_id_num = 0x0;
	itmr.timer_dead_line = 2.5*0xee6b280;
	//itmr.msi_msg = "A";
	itmr.msi_addr = 0x1010;
	itmr.arm = timer_arm;
	itmr.timer_mode = diff_time_periodic;
	itmr.cascade = cascade_disable;
	
	mprintf("\n\n**********************************************************\n");
	mprintf("WR-Siggen: starting up...\n");
	mprintf("\tUart base adress %08X\n", BASE_UART);
	mprintf("\tTIMER base adress %08X\n", itmr.timer_addr_base);
	mprintf("\tIRQ_CTRL base adress %08X\n", BASE_IRQ_CTRL);
	mprintf("\tFinde Dalay base adress %08X\n\n", fd.fd_regs_base);
	//mprintf("FD starting initilization...\n");
	/*mprintf("\tTimer base adress %08X\n", BASE_TIMER);
	mprintf("\tTics base adress %08X\n", BASE_TICS);*/
	//struct fmc_device fmc_loc;
	//mprintf("\t%X\n", fmc_readl(&fmc_loc, BASE_TICS));
	//mprintf("\t%X\n", irq_timer_readl(&fmc_loc, SRC_STAT));
	
		
	//dir = TIMER_SEL + ARM_CLR;
	//*dir = 0x1;
	//irq_timer_writel(&itmr, 0x1, ARM_CLR);
	irq_timer_arm(&itmr, timer_disarm);
	arm_checker(&itmr);
	
	
	/************ CONFIGURE TIMER ************/
	//selec timer
	/*dir= TIMER_SEL + BASE_TIMER;
	mprintf("TIMER_SEL = %08x\n", *dir);
	*dir=0x0;
	mprintf("TIMER_SEL = %08x\n", *dir);*/
	irq_timer_writel(&itmr, 0x0, TIMER_SEL);
	
	/*//dead line
	dir= TM_TIME_HI + BASE_TIMER;
	mprintf("TM_TIME_HI = %08x\n", *dir);
	*dir= 0x0;
	mprintf("TM_TIME_HI = %08x\n", *dir);
	dir= TM_TIME_LO + BASE_TIMER;
	mprintf("TM_TIME_LO = %08x\n", *dir);
	*dir= 2.5*0xee6b280;
	mprintf("TM_TIME_LO = %08x\n", *dir);*/
	dir= TM_TIME_HI + BASE_TIMER;
	mprintf("TM_TIME_HI = %08x\n", *dir);
	dir= TM_TIME_LO + BASE_TIMER;
	mprintf("TM_TIME_LO = %08x\n", *dir);
	
	irq_timer_set_time(&itmr, itmr.timer_dead_line);
	
	dir= TM_TIME_HI + BASE_TIMER;
	mprintf("TM_TIME_HI = %08x\n", *dir);
	dir= TM_TIME_LO + BASE_TIMER;
	mprintf("TM_TIME_LO = %08x\n", *dir);
	
	//MSI msg
	//dir = TM_MSG + BASE_TIMER;
	//mprintf("TM_MSG = %08x\n", *dir);
	/*dir="a";
	mprintf("TM_MSG = %08x\n", *dir);
	//MSI addr
	dir = TM_DST_ADR + BASE_TIMER;
	mprintf("TM_DST_ADR = %08x\n", *dir);
	*dir = 0x0;
	mprintf("TM_DST_ADR = %08x\n", *dir);	
	/************ *************** ************/
	/************ GENERAL CONFIG. ************/
	//select normal start
	dir = BASE_TIMER + CSC_STAT; //check 
	mprintf("CSC_STAT = %08x\n", *dir);
	//dir = BASE_TIMER + CSC_CLR; //set
	//*dir=0x1;
	irq_timer_sel_cascade(&itmr, cascade_disable);
	dir = BASE_TIMER + CSC_STAT; //check start
	mprintf("CSC_STAT = %08x\n", *dir);
	
	//select diff mode
	dir = BASE_TIMER + SRC_STAT; //check
	mprintf("SRC_STAT = %08x\n", *dir);
	dir = BASE_TIMER + D_MODE_STAT; //check
	mprintf("D_MODE_STAT = %08x\n", *dir);
	//dir = BASE_TIMER + SRC_SET; //set
	//*dir = 0x1;

	irq_timer_time_mode(&itmr, diff_time_periodic);

	dir = BASE_TIMER + SRC_STAT; //check
	mprintf("SRC_STAT = %08x\n", *dir);
	//select periodic
	dir = BASE_TIMER + D_MODE_STAT; //check
	mprintf("D_MODE_STAT = %08x\n", *dir);
	//dir = BASE_TIMER + D_MODE_SET; //set
	//*dir = 0x1;
	/*dir = BASE_TIMER + D_MODE_STAT; //check
	mprintf("D_MODE_STAT = %08x\n", *dir);	
	
	//arm timer
	dir = BASE_TIMER + ARM_STAT;
	mprintf("ARM_STAT = %08x\n", *dir);	//check armed timers
	dir = BASE_TIMER + ARM_SET;
	*dir=0x1;
	dir = BASE_TIMER + ARM_STAT;
	mprintf("ARM_STAT = %08x\n", *dir);	//check armed timers
	//reset counters
	/************ *************** ************/
	
	irq_timer_arm(&itmr, timer_arm);
	arm_checker(&itmr);
	
	check_stack();

	while(1){
		ui_update();
		mprintf("jiffies = %08x\n", jiffies);
	}
/* 
 * TM_TIME_HI = 00000000
 * TM_TIME_HI = 00000000
 * TM_TIME_LO = 2540be40
 * TM_TIME_LO = 2540be40
 * CSC_STAT = 00000000
 * CSC_STAT = 00000000
 * SRC_STAT = 00000001
 * SRC_STAT = 00000001
 * D_MODE_STAT = 00000001
 * D_MODE_STAT = 00000001
 * 
 */
		
}
