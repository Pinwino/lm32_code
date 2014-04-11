#include "fine-daly.h"

int spec_eeprom_read(struct fmc_device *fmc, uint32_t offset,
		void *buf, size_t size)
{
	struct spec_dev *spec = fmc->carrier_data;
	int ret = size;
	uint8_t *buf8 = buf;
	uint32_t aux;
	unsigned char c;
	
	printk("****> spec_eeprom_read <****\n");
	printk("\tfmc->eeprom_addr = 0x%x\n", fmc->eeprom_addr);
	printk("\toffset = %lu\n", offset);
	printk("\tsize = %d\n", size);

	if (spec->flags & SPEC_FLAG_FAKE_EEPROM)
		return size; /* no hw access */

	if (offset > SPEC_I2C_EEPROM_SIZE)
		return -EINVAL;
	if (offset + size > SPEC_I2C_EEPROM_SIZE)
		return -EINVAL;

	/* Read it all in a single loop: hardware allows it */
	mi2c_start(fmc);
	if(mi2c_put_byte(fmc, fmc->eeprom_addr << 1) < 0) {
		mi2c_stop(fmc);
		return -EIO;
	}
	aux = (offset >> 8) & 0xff;
	printk("\t(offset >> 8) & 0xff = %lu\n", aux);
	mi2c_put_byte(fmc, aux);
	aux = offset & 0xff;
	printk("\toffset & 0xff = %lu\n", aux);
	mi2c_put_byte(fmc, aux);
	mi2c_stop(fmc);
	mi2c_start(fmc);
	aux=((fmc->eeprom_addr << 1) | 1);
	printk("\t(fmc->eeprom_addr << 1) | 1 = 0x%x\n", aux);
	mi2c_put_byte(fmc, aux);
	while (size--) {
		mi2c_get_byte(fmc, &c, size != 0);
		*buf8++ = c;
		//printk("read 0x%08x, %4i to go\n", c, size);
	}
	mi2c_stop(fmc);
	return ret;
}
