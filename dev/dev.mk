obj-y += \
	dev/sdb.o 
	
obj-$(CONFIG_LEGACY_EEPROM) += dev/eeprom.o
obj-$(CONFIG_SDB_EEPROM) += dev/sdb-eeprom.o

obj-$(CONFIG_W1) +=		dev/w1.o	dev/w1-hw.o	dev/w1-shell.o
obj-$(CONFIG_W1) +=		dev/w1-temp.o	dev/w1-eeprom.o
obj-$(CONFIG_UART) +=		dev/uart.o
obj-$(CONFIG_UART_SW) +=	dev/uart-sw.o
