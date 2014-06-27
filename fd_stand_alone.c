#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdarg.h>

//#include <wrc.h>
#include <w1.h>
#include "uart.h"
#include "eeprom.h"
#include "fine-delay.h"
#include "hw/fd_channel_regs.h"
#include "hw/fd_channel_regs_struct.h"
#include "hw/fd_main_regs.h"
#include <pp-printf.h>
#include "errno.h"
#include "shell.h"
#include "lib/ipv4.h"
#include "irq.h"
#include "linux/jiffies.h"
#include "linux/delay.h"
#include <irq_ctrl.h>

#define mprintf pp_printf
#define vprintf pp_vprintf
#define sprintf pp_sprintf


struct fd_dev fd;
struct fmc_device fmc_loc;

//extern unsigned long  usleep(useconds_t usec);
extern void usleep_init(void);
extern unsigned long verify_lpj(unsigned long lpj);
extern unsigned char *BASE_TIMER;
//extern unsigned char *BASE_TICS;
extern unsigned char *BASE_IRQ_CTRL;	
extern uint32_t _endram;
extern uint32_t _fstack;
int init_iterator=0;
#define ENDRAM_MAGIC 0xbadc0ffe

//uint32_t irq_count=0;

void _irq_entry()
{
	uint32_t *dir;
	mprintf("\n************** WITH INTERRUPTS **************\n");
	//mprintf("Interrupt num %u\n", irq_count);
	irq_ctrl_pop();
	//irq_count++;
	
	fd.temp_timer.function(fd.temp_timer.data);
	clear_irq();
}

/*void module_param_lm32(int name){

	mprintf("name = %i , d_type = %i\n", name, d_type);

}*/

static void check_stack(void)
{
	while (_endram != ENDRAM_MAGIC) {
		mprintf("Stack overflow!\n");
		init_iterator=-ENOMEM;
		//msleep(1000);
	}
}

/* The reset function (by Tomasz) */
static void fd_do_reset(struct fd_dev *fd, int hw_reset)
{
	uint32_t val, adr;
	if (hw_reset) {
		val= FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_CORE_MASK;
		adr=FD_REG_RSTR;
		//mprintf("\t\tVAL %08X ADR %08X\n", val, adr);
		fd_writel(fd, val, adr);
		udelay(10000);
		val= FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_CORE_MASK | FD_RSTR_RST_FMC_MASK;
		adr=FD_REG_RSTR;
		//mprintf("\t\tVAL %08X ADR %08X\n", val, adr);
		fd_writel(fd, val, adr);
		/* TPS3307 supervisor needs time to de-assert master reset */
		//msleep(600);
		return;
	}
	
	val = FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_FMC_MASK;
	adr = FD_REG_RSTR;
	//mprintf("\t\tVAL %08X ADR %08X\n", val, adr);
	fd_writel(fd, val,  adr);
	udelay(1000);
	val = FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_FMC_MASK | FD_RSTR_RST_CORE_MASK;
	adr = FD_REG_RSTR;
	//mprintf("\t\tVAL %08X ADR %08X\n", val, adr);
	fd_writel(fd, val, adr);
	udelay(1000);
}

/* *************************************************** */

/* ************** RESET FUNCTION FOR FD ************** */
int fd_reset_again(struct fd_dev *fd)
{
	unsigned long j;

	/* Reset the FD core once we have proper reference/TDC clocks */
	fd_do_reset(fd, 0 /* not hw */);

	j = jiffies + 2 * HZ;
	while (time_before(jiffies, j)) {
		if (fd_readl(fd, FD_REG_GCR) & FD_GCR_DDR_LOCKED)
			break;
		msleep(10);
	}
	if (time_after_eq(jiffies, j)) 
		dev_err(&fd->fmc->dev,
			"%s: timeout waiting for GCR lock bit\n", __func__);

	fd_do_reset(fd, 0 /* not hw */);
	return 0;
}
/* *************************************************** */

int fd_gpio_defaults(struct fd_dev *fd)
{
	fd_gpio_dir(fd, FD_GPIO_TRIG_INTERNAL, FD_GPIO_OUT);
	fd_gpio_set(fd, FD_GPIO_TRIG_INTERNAL);

	fd_gpio_set(fd, FD_GPIO_OUTPUT_MASK);
	fd_gpio_dir(fd, FD_GPIO_OUTPUT_MASK, FD_GPIO_OUT);

	fd_gpio_dir(fd, FD_GPIO_TERM_EN, FD_GPIO_OUT);
	fd_gpio_clr(fd, FD_GPIO_TERM_EN);
	return 0;
}
 
void kernel_dev(int subsys, const char *fmt, ...)
{	
	va_list ap;

	if (subsys == 0)
		mprintf("Error: ");
	else if (subsys == 1)
		mprintf("Warning: ");
	else if (subsys == 2)
		mprintf("Info: ");

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

int main(void)
{
	int wrc_ui_mode = UI_SHELL_MODE;

		
	int i, j, *dir, *ind, sel, ch, eep;
	uint64_t pos=10;
	uint64_t neg=2;
	uint32_t neg2=2;
	uint32_t rem;
	uint64_t aux;
	
	uint64_t dividend = 10;
	uint64_t divisor = 5;
	uint64_t *remainder;
	size_t size=0;
	
	_endram = ENDRAM_MAGIC;
	
	sdb_find_devices();
	uart_init_sw();
	uart_init_hw();
		
	
	usleep_init();
	
	shell_init();
	
	
	//enable_irq();
	
	fd.fd_regs_base = BASE_FINE_DELAY;
	fd.fd_owregs_base= fd.fd_regs_base + 0x500;	
	fd.temp_timer.itmr.timer_addr_base = BASE_TIMER;
	fd.temp_timer.itmr.cascade = cascade_disable;
	fd.temp_timer.itmr.time_source = diff_time_periodic;
	fd.temp_timer.itmr.timer_id_num = 0x0;
	fd.temp_timer.itmr.timer_mode = 0x0;
	
	/*fd.temp_timer.itmr.timer_dead_line = 2.5*0xee6b280;
	
	
	
	irq_timer_set_time(&fd.temp_timer.itmr, fd.temp_timer.itmr.timer_dead_line);
	irq_timer_sel_cascade(&fd.temp_timer.itmr, cascade_disable);
	irq_timer_time_mode(&fd.temp_timer.itmr, diff_time_periodic);*/	
	while(1){
		check_stack();
		switch(init_iterator){
			case 0:
				mprintf("\n\n**********************************************************\n");
				mprintf("WR-Dbg: starting up...\n\n");
				
				fd_i2c_init(&fd);		
				// Hack 
				fd.fmc = &fmc_loc; 
				fd.fmc->eeprom_len = SPEC_I2C_EEPROM_SIZE;
				//mprintf("LEN = %d\n", fd.fmc->eeprom_len);
				fd.fmc->eeprom = malloc((size_t) (fd.fmc->eeprom_len));
				fd_eeprom_read(&fd, 0x50, 0, fd.fmc->eeprom, (size_t) (fd.fmc->eeprom_len));
				fd_handle_eeprom_calibration(&fd);
			
				free(fd.fmc->eeprom);
				init_iterator++;
			break;
			
			case 1:
				//mprintf("\tFD Core Reset\n");
				fd_do_reset(&fd, 1);
				if (fd_spi_init(&fd)<0)
					return 0;
				usleep(1000*1000);
				init_iterator++;
			break;
			
			case 2:
				//mprintf("\tInit GPIO\n");
				if (fd_gpio_init(&fd)<0){
				//mprintf("\tInit error\n");
					return 0;
				}
				init_iterator++;
				//init_iterator=9;
			break;
			
			case 3:
				fd_gpio_defaults(&fd);
				//mprintf("\tInit PLL\n");
				eep=fd_pll_init(&fd);
				
				if (eep<0){
					mprintf("error= %i\n", eep);
					return 0;
				}
				
				init_iterator++;
			break;
			
			case 4:
				//mprintf("\tInit OneWire\n");
				if (fd_onewire_init (&fd)<0)
					return 0;
				init_iterator++;
			break;

			case 5:
				//mprintf("\tReset Again\n");
				fd_reset_again(&fd);
				//mprintf("\tInit ACAM\n");
				fd_acam_init(&fd);
				init_iterator++;
			break;

			case 6:
				//mprintf("\tInit TIME\n");
				fd_time_init(&fd);
				init_iterator++;
			break;

			case 7:
				//mprintf("\tSet all output enable stages...\n");
				for (ch = 1; ch <= FD_CH_NUMBER; ch++){
				//mprintf("\t\tEnable channel %d\n", ch);
					fd_gpio_set(&fd, FD_GPIO_OUTPUT_EN(ch));
				}
				init_iterator++;
				mprintf("\n*-*-*-*- Node initialized -*-*-*-*\n");
			break;
			
			case 8:
				shell_interactive();
			break;
			
			default:
				if (init_iterator != -ENOMEM)
					mprintf("\t\tIntiliazatión error\n");
			return -1;
		}	
	}
	
	mprintf("The end#\n");
}
