#define SPEC_I2C_EEPROM_ADDR 0x50
#define SPEC_I2C_EEPROM_SIZE ((size_t)(8 * 1024))

extern int spec_eeprom_read(struct fmc_device *fmc, uint32_t offset,
			    void *buf, size_t size);
extern int spec_eeprom_write(struct fmc_device *fmc, uint32_t offset,
			     const void *buf, size_t size);
