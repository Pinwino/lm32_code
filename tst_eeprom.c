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
#define mprintf pp_printf
#define vprintf pp_vprintf
#define sprintf pp_sprintf
#define jiffies (unsigned long) timer_get_tics
#define HZ 100
#define udelay usleep
#define msleep timer_delay_ms


struct fd_dev fd;
struct fmc_device fmc_loc;

	
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

/* The reset function (by Tomasz) */
static void fd_do_reset(struct fd_dev *fd, int hw_reset)
{
	uint32_t val, adr;
	if (hw_reset) {
		val= FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_CORE_MASK;
		adr=FD_REG_RSTR;
		mprintf("\t\VAL %08X ADR %08X\n", val, adr);
		fd_writel(fd, val, adr);
		udelay(10000);
		val= FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_CORE_MASK | FD_RSTR_RST_FMC_MASK;
		adr=FD_REG_RSTR;
		mprintf("\t\VAL %08X ADR %08X\n", val, adr);
		fd_writel(fd, val, adr);
		/* TPS3307 supervisor needs time to de-assert master reset */
		//msleep(600);
		return;
	}
	
	val = FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_FMC_MASK;
	adr = FD_REG_RSTR;
	mprintf("\t\VAL %08X ADR %08X\n", val, adr);
	fd_writel(fd, val,  adr);
	udelay(1000);
	val = FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_FMC_MASK | FD_RSTR_RST_CORE_MASK;
	adr = FD_REG_RSTR;
	mprintf("\t\VAL %08X ADR %08X\n", val, adr);
	fd_writel(fd, val, adr);
	udelay(1000);
}

/* *************************************************** */

/* ************** RESET FUNCTION FOR FD ************** */
int fd_reset_again(struct fd_dev *fd)
{
	unsigned long j;

	mprintf("********** Reset again ***********\n");
	/* Reset the FD core once we have proper reference/TDC clocks */
	fd_do_reset(fd, 0 /* not hw */);

	j = jiffies + 2 * HZ;
	while (time_before(jiffies, j)) {
		if (fd_readl(fd, FD_REG_GCR) & FD_GCR_DDR_LOCKED)
			break;
		msleep(10);
	}
	if (time_after_eq(jiffies, j)) {
		mprintf("%s: timeout waiting for GCR lock bit\n", __func__);
	}

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

 
int main(void)
{
	const unsigned int main_registers[] = {FD_REG_TM_SECH, FD_REG_TM_SECL, FD_REG_TM_CYCLES};
	
	const unsigned int port_base_adress[] = {0x300, 0x400}; 
	const unsigned int port_registers[] = {FD_REG_DCR, FD_REG_U_STARTH, FD_REG_U_STARTL, FD_REG_C_START, FD_REG_F_START, FD_REG_U_ENDH, FD_REG_U_ENDL, FD_REG_C_END, FD_REG_F_END, FD_REG_U_DELTA, FD_REG_C_DELTA, FD_REG_F_DELTA, FD_REG_RCR};

	const unsigned int pps_vals[]	= {0x0, 0x0, 0x0, 0xbebc21, 0x760, 0x0, 0x0, 0xbebc27, 0xb60, 0x1, 0x0, 0x000, 0x10000};
	const unsigned int XMhz_vals[]	= {0x0, 0x0, 0x0, 0xbebc1b, 0x34a, 0x0, 0x0, 0xbebc21, 0x74a, 0x0, 0x6, 0x400, 0x10000};
	const unsigned int crtl []		= {0x81, 0x92, 0x83, 0x87};
	
		
	int i, j, *dir, *ind, sel, ch, eep;
	size_t size=0;
	
	_endram = ENDRAM_MAGIC;
	
	sdb_find_devices();
	uart_init_sw();
	uart_init_hw();
	
	shell_init();
	
	fd.fd_regs_base = BASE_FINE_DELAY;
	fd.fd_owregs_base= fd.fd_regs_base + 0x500;		/* regs_base + 0x500 */
	
	mprintf("\n\n**********************************************************\n");
	mprintf("WR-Siggen: starting up...\n");
	//mprintf("\tUart base adress %08X\n", BASE_UART);
	//mprintf("\tFinde Dalay base adress %08X\n\n", fd.fd_regs_base);
	//mprintf("FD starting initilization...\n");
		
	
	fd_i2c_init(&fd);		
	/* Hack */
	fd.fmc = &fmc_loc; 
	fd.fmc->eeprom_len = SPEC_I2C_EEPROM_SIZE;
	mprintf("LEN = %d\n", fd.fmc->eeprom_len);
	fd.fmc->eeprom = malloc((size_t) (fd.fmc->eeprom_len));
	fd_eeprom_read(&fd, 0x50, 0, fd.fmc->eeprom, (size_t) (fd.fmc->eeprom_len));
	fd_handle_eeprom_calibration(&fd);
	
	free(fd.fmc->eeprom);
	
	mprintf("\tFD Core Reset\n");
	fd_do_reset(&fd, 1);
	mprintf("\tFD Core Reset Done\n\n");
	
	mprintf("\tInit SPI\n");
		fd_spi_init(&fd);
	mprintf("\tSPI initialized\n\n");
	
	mprintf("\tInit GPIO\n");
		fd_gpio_init(&fd);
	mprintf("\tGPIO initialized\n\n");
	
	mprintf("\tInit gpio-default\n");
		fd_gpio_defaults(&fd);
	mprintf("\tgpio-default initialized\n\n");
	
	mprintf("\tInit PLL\n");
		fd_pll_init(&fd);
	mprintf("\tPLL initialized\n\n");
	
	mprintf("\tInit OneWire\n");
		fd_onewire_init (&fd);
	mprintf("\tOneWire initialized\n");
	
	mprintf("\tInit gpio-default\n");
		fd_gpio_defaults(&fd);
	mprintf("\tgpio-default initialized\n\n");
	
	mprintf("\tReset Again\n");
		fd_reset_again(&fd);
	mprintf("\tFD Reset Again Done\n\n");
	
	mprintf("\tInit ACAM\n");
		fd_acam_init(&fd);
	mprintf("\tACAM initialized\n");
	
	mprintf("\tInit TIMER\n");
		fd_time_init(&fd);
	mprintf("\tTIMER initialized\n");
	
	mprintf("\tInit I2C\n");
		eep=fd_i2c_init(&fd);
	mprintf("\I2C initialized\n");
	
		
	mprintf("\tSet all output enable stages...\n");
		for (ch = 1; ch <= FD_CH_NUMBER; ch++){
		mprintf("\t\tEnable channel %d\n", ch);
		fd_gpio_set(&fd, FD_GPIO_OUTPUT_EN(ch));
		}
	mprintf("\tSet all output enable stages\n\n");
	
	mprintf("Starting port programming\n");
	ind=pps_vals;	
	for (j=0; j<ARRAY_SIZE(port_base_adress); j++){
		mprintf("\tChannel base adress %08X\n", port_base_adress[j]);
		for (i=0x0; i<(sizeof(port_registers)/sizeof(port_registers[0])); i++)
		{			
				dir = BASE_FINE_DELAY + port_base_adress[j] + port_registers[i];
				*dir = *(ind+i);
				
				mprintf("\t\tIter %d Dir %08X val %08X\n", i, dir, *dir);
		}
		
		ind=XMhz_vals;
	}
		

	for (j=0; j<ARRAY_SIZE(port_base_adress); j++)
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
	mprintf("\t\tIter %d Dir %08X val %08X\n", i, dir, *dir);
	mprintf("**** no_posix & ppsi ****\n");
		
	check_stack();

}
