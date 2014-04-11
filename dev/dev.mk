obj-$(CONFIG_WR_NODE) += \
	dev/i2c.o \
	dev/pps_gen.o \
	dev/syscon.o \
	dev/sdb.o 

obj-$(CONFIG_LEGACY_EEPROM) += dev/eeprom.o
obj-$(CONFIG_SDB_EEPROM) += dev/sdb-eeprom.o

obj-$(CONFIG_UART) +=		dev/uart.o
obj-$(CONFIG_UART_SW) +=	dev/uart-sw.o
