obj-$(CONFIG_WR_NODE) += \
	shell/shell.o \
	shell/cmd_sdb.o \
	#shell/cmd_fmc_fdelay_pulse.o \
	shell/cmd_help.o 

obj-n +=			shell/cmd_ip.o
obj-$(CONFIG_CMD_SLEEP) +=			shell/cmd_sleep.o
