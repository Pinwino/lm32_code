# choose your board here.
BOARD = spec

# and don't touch the rest unless you know what you're doing.
CROSS_COMPILE ?= lm32-elf-

CC =		$(CROSS_COMPILE)gcc
OBJDUMP =	$(CROSS_COMPILE)objdump
OBJCOPY =	$(CROSS_COMPILE)objcopy
SIZE =		$(CROSS_COMPILE)size

-include $(CURDIR)/.config

OBJS_WRC = 	wrc_main.o \
		wrc_ptp.o \
		monitor/monitor.o

PTP_NOPOSIX = ptp-noposix

INCLUDE_DIRS = -I$(PTP_NOPOSIX)/wrsw_hal \
		-I$(PTP_NOPOSIX)/libptpnetif \
		-I$(PTP_NOPOSIX)/softpll \
		-Iinclude

CFLAGS = -include $(CURDIR)/include/generated/autoconf.h

CFLAGS_PTPD  = -ffreestanding \
	-DPTPD_FREESTANDING \
	-DWRPC_EXTRA_SLIM \
	-DPTPD_MSBF \
	-DPTPD_DBG \
	-DPTPD_NO_DAEMON \
	-DNEW_SINGLE_WRFSM \
	-DPTPD_TRACE_MASK=0 \
	-include $(PTP_NOPOSIX)/compat.h \
	-include $(PTP_NOPOSIX)/PTPWRd/dep/trace.h \
	-include $(PTP_NOPOSIX)/libposix/ptpd-wrappers.h

OBJS_PTPD = $(PTP_NOPOSIX)/PTPWRd/arith.o \
	$(PTP_NOPOSIX)/PTPWRd/bmc.o \
	$(PTP_NOPOSIX)/PTPWRd/dep/msg.o \
	$(PTP_NOPOSIX)/PTPWRd/dep/net.o \
	$(PTP_NOPOSIX)/PTPWRd/dep/sys.o \
	$(PTP_NOPOSIX)/PTPWRd/dep/timer.o \
	$(PTP_NOPOSIX)/PTPWRd/dep/wr_servo.o \
	$(PTP_NOPOSIX)/PTPWRd/dep/servo.o \
	$(PTP_NOPOSIX)/PTPWRd/protocol.o \
	$(PTP_NOPOSIX)/PTPWRd/wr_protocol.o \
	$(PTP_NOPOSIX)/libposix/freestanding-startup.o \
	$(PTP_NOPOSIX)/libposix/freestanding-wrapper.o \
	$(PTP_NOPOSIX)/libposix/net.o \
	$(PTP_NOPOSIX)/softpll/softpll_ng.o

CFLAGS_PLATFORM  = -mmultiply-enabled -mbarrel-shift-enabled
LDFLAGS_PLATFORM = -mmultiply-enabled -mbarrel-shift-enabled \
	-nostdlib -T arch/lm32/ram.ld

OBJS_PLATFORM = arch/lm32/crt0.o arch/lm32/irq.o arch/lm32/debug.o

include shell/shell.mk
include tests/tests.mk
include lib/lib.mk
include sockitowm/sockitowm.mk
include dev/dev.mk


CFLAGS += $(CFLAGS_PLATFORM) $(CFLAGS_EB) $(CFLAGS_PTPD) $(INCLUDE_DIRS) \
	-ffunction-sections -fdata-sections -Os -Iinclude \
	-include include/trace.h \
	$(PTPD_CFLAGS) -I$(PTP_NOPOSIX)/PTPWRd -I. -Isoftpll

LDFLAGS = $(LDFLAGS_PLATFORM) \
	-ffunction-sections -fdata-sections -Wl,--gc-sections -Os -Iinclude

OBJS = $(OBJS_PLATFORM) $(OBJS_WRC) $(OBJS_PTPD) \
	$(OBJS_SHELL) $(OBJS_TESTS) $(OBJS_LIB) \
	$(OBJS_SOCKITOWM) $(OBJS_SOFTPLL) $(OBJS_DEV)

OUTPUT = wrc

REVISION=$(shell git describe --dirty --always)

all: tools $(OUTPUT).ram $(OUTPUT).vhd $(OUTPUT).elf

.PRECIOUS: %.elf %.bin
.PHONY: all tools clean

$(OUTPUT).elf: silentoldconfig $(OBJS)
	$(CC) $(CFLAGS) -DGIT_REVISION=\"$(REVISION)\" -c revision.c
	${CC} -o $@ revision.o $(OBJS) $(LDFLAGS)
	${OBJDUMP} -d $(OUTPUT).elf > $(OUTPUT)_disasm.S
	$(SIZE) $@

%.bin: %.elf
	${OBJCOPY} -O binary $^ $@

%.ram: tools %.bin
	./tools/genraminit $*.bin 0 > $@

%.vhd: tools %.bin
	./tools/genramvhd -s 90112 $*.bin > $@

$(OBJS): include/board.h

include/board.h:
	ln -sf ../boards/$(BOARD)/board.h include/board.h


clean:
	rm -f $(OBJS) $(OUTPUT).elf $(OUTPUT).bin $(OUTPUT).ram include/board.h

%.o:		%.c
	${CC} $(CFLAGS) $(PTPD_CFLAGS) $(INCLUDE_DIR) $(LIB_DIR) -c $*.c -o $@

tools:
	$(MAKE) -C tools

# following targets from Makefile.kconfig
silentoldconfig:
	@mkdir -p include/config
	$(MAKE) -f Makefile.kconfig $@

scripts_basic config %config:
	$(MAKE) -f Makefile.kconfig $@

.config: silentoldconfig
