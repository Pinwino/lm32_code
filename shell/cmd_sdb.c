#include "shell.h"
#include "syscon.h"
#include "hw/memlayout.h"

static int cmd_show_sdb(const char *args[])
{
	sdb_print_devices();
	return 0;
}

DEFINE_WRC_COMMAND(show_sdb) = {
	.name = "show_sdb",
	.exec = cmd_show_sdb,
};
