/* Simple demo that acts on the termination of the first board */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "fdelay-lib.h"
#include <opt.h>

#include "tools-common.h"
#include <shell.h>

extern int fd_zio_info_tdc(struct fd_dev *fd, enum fd_zattr_in_idx option,
			     uint32_t *usr_val);

extern int fd_zio_conf_tdc(struct fd_dev *fd, enum fd_zattr_in_idx option,
			     uint32_t usr_val);
			     
extern struct fd_dev fd;


/*void help(char *name)
{
	fprintf(stderr, "%s: Use \"%s [-i <index>] [-d <dev>] [on|off]\n",
		name, name);
	//exit(1);
}*/

int main_term (const char **argv)
{
	struct fdelay_board *b;
	int nboards, hwval, newval;
	int index = -1, dev = -1, argc = 0;
	optind = 0;

	while(argv[argc++] != NULL){}

	/* Standard part of the file (repeated code) */
	if (tools_need_help(argc, argv))
		help(FD_CMD_TERM_HELP);

	/*nboards = fdelay_init();

	if (nboards < 0) {
		fprintf(stderr, "%s: fdelay_init(): %s\n", argv[0],
			strerror(errno));
		//exit(1);
	}
	if (nboards == 0) {
		fprintf(stderr, "%s: no boards found\n", argv[0]);
		//exit(1);
	}
	if (nboards == 1)*/
		index = 0; /* so it works with no arguments */

	printk("PRE ----->  optind = %i, argc = %i\n", optind, argc);
	tools_getopt_d_i(argc, argv, FD_CMD_TERM_HELP);
	printk("%s\n", argv[optind]);

	/*if (index < 0 && dev < 0) {
		fprintf(stderr, "%s: several boards, please pass -i or -d\n",
			argv[0]);
		//exit(1);
	}

	/* Parse the extra argument, if any */
	newval = -1;
	
	if (optind + 1 == argc - 1) {
		char *s = argv[optind];
		printk("%s\n",s);
		if (!strcmp(s, "0") || !strcmp(s, "off"))
		    newval = 0;
		else if (!strcmp(s, "1") || !strcmp(s, "on"))
			newval = 1;
		else
			help(FD_CMD_TERM_HELP);
	}
	/* Finally work */
	/*b = fdelay_open(index, dev);
	if (!b) {
		fprintf(stderr, "%s: fdelay_open(): %s\n", argv[0],
			strerror(errno));
		//exit(1);
	}*/

	fd_zio_info_tdc(&fd, FD_ATTR_TDC_FLAGS, &hwval);
	printk("hwval = %i, newval = %i \n", hwval, newval);
	int err = 0;

	switch(newval) {
	case 1:
		hwval |= FD_TDCF_TERM_50;
		//err = fdelay_set_config_tdc(b, hwval);
		printk("case 1\n");
		fd_zio_conf_tdc(&fd, FD_ATTR_TDC_FLAGS, hwval);	
		break;
	case 0:
		hwval &= ~FD_TDCF_TERM_50;
		printk("case 0\n");
		fd_zio_conf_tdc(&fd, FD_ATTR_TDC_FLAGS, hwval);	
		//err = fdelay_set_config_tdc(b, hwval);
		break;
	}
	
	if (err)
	{
		fprintf(stderr, "%s: error setting termination: %s", argv[0], strerror(errno));
		//exit(1);
	}

	/*printf("%s: termination is %s\n", argv[0],
	       hwval & FD_TDCF_TERM_50 ? "on" : "off");*/

	printf("%s: termination is %s\n", argv[0],
	       hwval & FD_TDCF_TERM_50 ? "on" : "off");

	//fdelay_close(b);
	//fdelay_exit();
	return 0;
}

DEFINE_WRC_COMMAND(term) = {
	.name = "term",
	.exec = main_term,
};
