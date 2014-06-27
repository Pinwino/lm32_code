#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "fdelay-lib.h"

#include <shell.h>

#include "tools-common.h"

extern int fd_zio_info_output(struct fd_dev *fd, int ch, enum fd_zattr_in_idx option, uint32_t *usr_val);
extern struct fd_dev fd;


int main_status(const char **argv)
{
	struct fdelay_board *b;
	struct fdelay_pulse p;
	int nboards, ch, index = -1, dev = -1, raw = 0, opt, optind=0, argc=-1;
	while (argv[++argc] != NULL) {}

	/* Standard part of the file (repeated code) */
	if (tools_need_help(argc, argv))
		help(FD_CMD_STAT_HELP);

	while ((opt = getopt(argc, argv, "rh")) != -1) {
		char *rest;
		switch (opt) {
		case 'r':
			raw = 1;
			break;
		case 'h':
			help(FD_CMD_STAT_HELP);
			return 0;
		}
	}


	int i;
	for (ch = 1; ch <= 4; ch++) {
		if (fdelay_get_config_pulse(&fd, FDELAY_OUTPUT_USER_TO_HW(ch), &p) <0){
			fprintf(stderr, "status: get_config(channel %i): %s\n", ch, strerror(errno));
		}
		/* pass hw number again, as the function is low-level */
		report_output_config(FDELAY_OUTPUT_USER_TO_HW(ch),
				    &p, raw ? TOOLS_UMODE_RAW : TOOLS_UMODE_USER);
	}
	return 0;
}

DEFINE_WRC_COMMAND(status) = {
	.name = "status",
	.exec = main_status,
};
