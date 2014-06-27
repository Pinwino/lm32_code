obj-y += \
	dev/sdb.o 
	
obj-y += dev/eeprom.o
obj-n += dev/sdb-eeprom.o

obj-n +=		dev/w1.o	dev/w1-hw.o	dev/w1-shell.o
obj-n +=		dev/w1-temp.o	dev/w1-eeprom.o
obj-$(CONFIG_UART) +=		dev/uart.o
obj-$(CONFIG_UART_SW) +=	dev/uart-sw.o
