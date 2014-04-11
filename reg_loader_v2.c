#include <stdio.h>
#include <inttypes.h>

#include <stdarg.h>

#include <wrc.h>
#include <w1.h>
#include "uart.h"
#include "eeprom.h"
//#include "fine-delay.h"
#include "hw/fd_channel_regs.h"
#include "hw/fd_main_regs.h"
//#include <syscon.h>
#include <pp-printf.h>
#define mprintf pp_printf
#define vprintf pp_vprintf
#define sprintf pp_sprintf

extern unsigned char *BASE_UART;
struct fd_dev *fd;
extern unsigned char *BASE_FINE_DELAY;


/*int fd_gpio_defaults(struct fd_dev *fd)
{
	fd_gpio_dir(fd, FD_GPIO_TRIG_INTERNAL, FD_GPIO_OUT);
	fd_gpio_set(fd, FD_GPIO_TRIG_INTERNAL);

	fd_gpio_set(fd, FD_GPIO_OUTPUT_MASK);
	fd_gpio_dir(fd, FD_GPIO_OUTPUT_MASK, FD_GPIO_OUT);

	fd_gpio_dir(fd, FD_GPIO_TERM_EN, FD_GPIO_OUT);
	fd_gpio_clr(fd, FD_GPIO_TERM_EN);
	return 0;
}*/

 
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
//	mprintf("Uart base adress %08X\n", BASE_UART);
//	mprintf("Finde Dalay base adress %08X\n\n", BASE_FINE_DELAY);
	
//	dir=BASE_FINE_DELAY + FD_REG_RSTR;
//	*dir=0xdead0003;
	
//	mprintf("Dir %08X val %08X\n", dir, *dir);
//	mprintf("Dir %zu\n",timer_get_tics());
	
// 	for(i=0; i<3; i++){
//		dir = BASE_FINE_DELAY + port_registers[4] + port_base_adress[1];
//		*dir += i;
		//j=fd_readl( (port_registers[4] + port_base_adress[1]) )+i;
//		//fd_writel(j, (port_registers[4] + port_base_adress[1]));
//		//mprintf("Dir %08X val %08X\n", dir, *dir);
//	}
	
	//dir=BASE_FINE_DELAY + FD_REG_RSTR;
	//*dir=0xdead0001;
	//mprintf("FD Core Reset -> Dir %08X val %08X\n", dir, *dir);

//	mprintf("\n--------> Init spi\n");
	//fd_spi_init();
	
//	mprintf("\n--------> Init gpio\n");
//	fd_gpio_init(fd);
//
//	mprintf("\n--------> Init pll\n");
	//fd_pll_init();	
	
	//mprintf("\n--------> Init gpio-default\n");
//	fd_gpio_defaults(fd);
	
//	dir=BASE_FINE_DELAY + FD_REG_RSTR;
//	*dir=0xdead0001;
//	*dir=0xdead0003;
	
	//set all output enable stages 
	//for (ch = 1; ch <= FD_CH_NUMBER; ch++)
		//fd_gpio_set(fd, ch);

	mprintf("Starting port programming\n");
		
	ind=pps_vals;	
	for (j=0; j<2; j++){
		
		mprintf("\tChannel base adress %08X\n", port_base_adress[j]);
		for (i=0x0; i<(sizeof(port_registers)/sizeof(port_registers[0])); i++)
		{			
				dir = BASE_FINE_DELAY + port_base_adress[j] + port_registers[i];
				*dir = *(ind+i);
				
				mprintf("\t\tIter %d Dir %08X val %08X\n", i, dir, *dir);
		}
		
		ind=XMhz_vals;
	}
		

	for (j=0; j<2; j++)
	{
		mprintf("\tUpdating base %08X\n", port_base_adress[j]);	
		for(i=0; i<sizeof(crtl)/sizeof(crtl[0]); i++)
		{
			dir = BASE_FINE_DELAY + port_base_adress[j];
			*dir = crtl[i];
			mprintf("\t\tIter %d Dir %08X val %08X\n", i, dir, *dir);
		}
	}
	
	
	mprintf("\tUpdating time registers\n");
	for (i=0x0; i<(sizeof(main_registers)/sizeof(main_registers[0])); i++){
		dir=BASE_FINE_DELAY + main_registers[i];
		*dir=0x0;
		mprintf("\t\tIter %d Dir %08X val %08X\n", i, dir, *dir);
	}
	
	mprintf("\tUpdating control-time register\n");
	dir=BASE_FINE_DELAY + FD_REG_TCR;
	*dir=0x89;
	mprintf("\t\tIter %d Dir %08X val %08X\n", i, dir, *dir);*/
}
