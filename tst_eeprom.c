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
		msleep(1000);
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
	/*const unsigned int main_registers[] = {FD_REG_TM_SECH, FD_REG_TM_SECL, FD_REG_TM_CYCLES};
	
	const unsigned int port_base_adress[] = {0x300, 0x400}; 
	const unsigned int port_registers[] = {FD_REG_DCR, FD_REG_U_STARTH, FD_REG_U_STARTL, FD_REG_C_START, FD_REG_F_START, FD_REG_U_ENDH, FD_REG_U_ENDL, FD_REG_C_END, FD_REG_F_END, FD_REG_U_DELTA, FD_REG_C_DELTA, FD_REG_F_DELTA, FD_REG_RCR};

	const unsigned int pps_vals[]	= {0x0, 0x0, 0x0, 0xbebc21, 0x760, 0x0, 0x0, 0xbebc27, 0xb60, 0x1, 0x0, 0x000, 0x10000};
	const unsigned int XMhz_vals[]	= {0x0, 0x0, 0x0, 0xbebc1b, 0x34a, 0x0, 0x0, 0xbebc21, 0x74a, 0x0, 0x6, 0x400, 0x10000};
	const unsigned int crtl []		= {0x81, 0x92, 0x83, 0x87};
	
	const unsigned int vec[] = {1 , 512, 1024, 2048, 1024*1000};*/
	
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
	
	/*aux = mul_u64(pos, neg);
	if (aux == 0)
		printf("No hace na!\n");
	printf("%llu x ",  pos << 32);
	printf("%llu ",  neg << 32);
	printf("= %llu\n",  aux << 32);
	printf("= %llu\n",  aux >> 32);
	
	printf("Div\n");
	aux = div_u64_rem(pos, neg2, rem);
	if (aux == 0)
		printf("No hace na!\n");
	printf("%llu / ",  pos << 32);
	printf("%llu ",  neg2);
	printf("= %llu\n",  aux << 32);
	printf("= %llu\n",  aux >> 32);*/
	
	
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
	
	
	mprintf("\n\n**********************************************************\n");
	mprintf("WR-Dbg: starting up...\n");
	/*pos=div64_u64_rem(dividend, divisor, &remainder);
	pp_printf("res %llu\n", pos<<32);
	pp_printf("res %llu\n", pos>>32);*/
	
	/*i=0;
	for (pos=1;pos <= 1000*1000*100; pos*=1000){
		pp_printf("iter %i\n", i++);
		pp_printf("lu %lu\n", pos);
		pp_printf("llu %llu\n", pos);
		pp_printf("li %li\n", pos);
		pp_printf("lli %lli\n\n", pos);
	}*/

	//mprintf("\tUart base adress %08X\n", BASE_IRQ_CTRL);
	/*mprintf("\tUart base adress %08X\n", BASE_UART);
	/*mprintf("\tTimer base adress %08X\n", BASE_TIMER);
	/*mprintf("\tTics base adress %08X\n", BASE_TICS);
	//mprintf("\tFinde Dalay base adress %08X\n\n", fd.fd_regs_base);*/
	/*mprintf("FD starting initilization...\n");

	//verify_lpj(26800);*/
		
	fd_i2c_init(&fd);		
	// Hack 
	fd.fmc = &fmc_loc; 
	fd.fmc->eeprom_len = SPEC_I2C_EEPROM_SIZE;
	mprintf("LEN = %d\n", fd.fmc->eeprom_len);
	fd.fmc->eeprom = malloc((size_t) (fd.fmc->eeprom_len));
	fd_eeprom_read(&fd, 0x50, 0, fd.fmc->eeprom, (size_t) (fd.fmc->eeprom_len));
	fd_handle_eeprom_calibration(&fd);
	
	free(fd.fmc->eeprom);
	
	//mprintf("\tFD Core Reset\n");
	fd_do_reset(&fd, 1);
	mprintf("\tFD Core Reset Done\n\n");
	check_stack();
	mprintf("\tInit SPI\n");
		if (fd_spi_init(&fd)<0)
			return 0;
	mprintf("\tSPI initialized\n\n");
	check_stack();
	usleep(1000*1000);
	
	mprintf("\tInit GPIO\n");
		if (fd_gpio_init(&fd)<0)
			return 0;
	mprintf("\tGPIO initialized\n\n");
	check_stack();
	mprintf("\tInit gpio-default\n");
		fd_gpio_defaults(&fd);
	mprintf("\tgpio-default initialized\n\n");
	check_stack();
	//usleep(1000*1000);
	
	mprintf("\tInit PLL\n");
		eep=fd_pll_init(&fd);
		
		if (eep<0){
			mprintf("error= %i\n", eep);
			return 0;
		}
	mprintf("\tPLL initialized\n\n");
	check_stack();
	mprintf("\tInit OneWire\n");
		if (fd_onewire_init (&fd)<0)
			return 0;
	mprintf("\tOneWire initialized\n\n");
	check_stack();
	mprintf("\tReset Again\n");
		fd_reset_again(&fd);
	mprintf("\tFD Reset Again Done\n\n");
	check_stack();
	mprintf("\tInit ACAM\n");
		fd_acam_init(&fd);
	mprintf("\tACAM initialized\n");
	check_stack();
	mprintf("\tInit TIME\n");
		fd_time_init(&fd);
	mprintf("\tTIME initialized\n");
	check_stack();
	mprintf("\tSet all output enable stages...\n");
		for (ch = 1; ch <= FD_CH_NUMBER; ch++){
			mprintf("\t\tEnable channel %d\n", ch);
			fd_gpio_set(&fd, FD_GPIO_OUTPUT_EN(ch));
		}
	//mprintf("\tSet all output enable stages\n\n");
	//check_stack();
	//mprintf("Starting port programming\n");
	/*ind=pps_vals;	
	for (j=0; j<ARRAY_SIZE(port_base_adress); j++){
		//mprintf("\tChannel base adress %08X\n", port_base_adress[j]);
		for (i=0x0; i<(sizeof(port_registers)/sizeof(port_registers[0])); i++)
		{			
				dir = BASE_FINE_DELAY + port_base_adress[j] + port_registers[i];
				*dir = *(ind+i);
				
				//mprintf("\t\tIter %d Dir %08X val %08X\n", i, dir, *dir);
		}
		
		ind=XMhz_vals;
	}
		

	for (j=0; j<ARRAY_SIZE(port_base_adress); j++)
	{
		//mprintf("\tUpdating base %08X\n", port_base_adress[j]);	
		for(i=0; i<sizeof(crtl)/sizeof(crtl[0]); i++)
		{
			dir = BASE_FINE_DELAY + port_base_adress[j];
			*dir = crtl[i];
			//mprintf("\t\tIter %d Dir %08X val %08X\n", i, dir, *dir);
		}
	}
	
	
	//mprintf("\tUpdating time registers\n");
	for (i=0x0; i<(sizeof(main_registers)/sizeof(main_registers[0])); i++){
		dir=BASE_FINE_DELAY + main_registers[i];
		*dir=0x0;
		//mprintf("\t\tIter %d Dir %08X val %08X\n", i, dir, *dir);
	}*/
	
	//mprintf("\tUpdating control-time register\n");
	/*dir=BASE_FINE_DELAY + FD_REG_TCR;
	*dir=0x89;*/
	//mprintf("\t\tIter %d Dir %08X val %08X\n", i, dir, *dir);
	/*mprintf("****    ****\n");
	

	/* hotfix: the ZIO has a bug blocking the output when the output raw_io function returns an error.
	therefore we temporarily have to check the output programming correctness in the user library. *
		if (mode == FD_OUT_MODE_DELAY || mode == FD_OUT_MODE_DISABLED)
	/*{
		if(pulse->rep < 0 || pulse->rep > 16) /* delay mode allows trains of 1 to 16 pulses. *
			return -EINVAL;

		if(a[FD_ATTR_OUT_START_L] == 0 && a[FD_ATTR_OUT_START_COARSE] < (600 / 8)) // 600 ns min delay
			return -EINVAL;
	}*/

	/* we need to fill the nsample field of the control */


	


		
	/*pos=0, neg=0;
	ch = eep = 0;*/

	/*mprintf("Timer init\n");
	usleep(5*1000*1000);
	mprintf("Timer end\n");*/
	
	//kernel_dev(0, "%s: Trace error check", KBUILD_MODNAME);*/
	
	while(1){
		check_stack();
		shell_interactive();
	}
	
	//while(eep != 0){
		//ui_update();
		/*eep=verify_lpj(5*1000*1000*20);
		if (eep>0){
			pos++;	
		}
		else {
			neg++;
		}
		if (ch*eep < 0){
			mprintf("pos = %lu, neg= %lu\n", pos, neg);
			if (pos != 0 && neg != 0)
				pos = neg = 0;
		}
		ch=eep;*/
	//}
	/*i=1000;
	while(i++ < 10)
		verify_lpj(i);*/
	mprintf("The end#\n");
}
