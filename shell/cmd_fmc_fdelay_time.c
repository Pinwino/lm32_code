#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "fdelay-lib.h"

#include <shell.h>

#include <opt.h>

#include "tools-common.h"
#define CONFIG_SYS_64BIT_VSPRINTF

extern struct fd_dev fd; 

/*void help(char *name)
{


		//exit(1);
}*/

int main_time(const char **argv)
{
	struct fdelay_board *b;
	struct fdelay_time t;
	int nboards, i, get = 0, host = 0, wr_on = 0, wr_off = 0, argc=0;
	int index = -1, dev = -1;
	char *s;
	optind = 0;

	while(argv[argc++] != NULL){}


	/* Standard part of the file (repeated code) */
	if (tools_need_help(argc, argv))
		help(FD_CMD_TIME_HELP);

	mprintf("optind %i, argc %i\n", optind, argc);
	tools_getopt_d_i(argc, argv, FD_CMD_TIME_HELP);
	mprintf("optind %i, argc %i\n", optind, argc);


	/* Parse the mandatory extra argument */
	mprintf("%i, %i\n", optind, argc);
	if (optind + 1 != argc - 1)
		help(FD_CMD_TIME_HELP);
		
	/* Crappy parser */
	if (!strcmp(s, "get"))
		get = 1;
	else if (!strcmp(s, "wr"))
		wr_on = 1;
	else if (!strcmp(s, "local"))
		wr_off = 1;
	else {
		unsigned long long nano;
		unsigned long long sec;

		memset(&t, 0, sizeof(t));
		printf("Sending %s\n", s);
		i = sscanf_addhoc_replacement(s, &sec, &nano);
		printf("argumenst %i, sec %llu nano %llu\n", i, sec, nano);
		if (i < 1) {
			fprintf(stderr, "%s: Not a number \"%s\"\n",
				argv[0], s);
			return -1;
			//exit(1);
		}
		t.utc = sec;
		t.coarse = nano * 1000 * 1000 * 1000 / 8;
		printf("argumenst %i, t.utc %llu t.coarse %llu\n", i, t.utc, t.coarse);
	}

	if (get) {
		if (fd_time_get(&fd, &t, NULL) < 0) {
			fprintf(stderr, "%s: fdelay_get_time(): %s\n", argv[0],
				strerror(errno));
			return -1;
			//exit(1);
		}

		int err = fd_zio_conf_set(&fd, NULL, NULL, FD_CMD_WR_QUERY);
		printf("WR Status: ");
		switch(err)
		{
			case -ENODEV: 	printf("disabled.\n"); break;
			case -ENOLINK: 	printf("link down.\n"); break;
			case -EAGAIN: 	printf("synchronization in progress.\n"); break;
			case 0: 	printf("synchronized.\n"); break;
			default:   	printf("error: %s\n", strerror(errno)); break;
		}
		//printf("Time: %lli.%09li\n", (long long)t.utc, (long)t.coarse * 8);
		printf("Time: %i.%09i\n", t.utc, t.coarse * 8);
		printf("Time: %i", t.utc << 32);
		printf(".%09i\n", t.coarse * 8);
		printf("Time: %i.%09i\n", (long long)t.utc, (long)t.coarse * 8);
		
		//fdelay_close(b);
		//fdelay_exit();
		return 0;
	}


	if (wr_on) {
		//setbuf(stdout, NULL);
		printf("Locking the card to WR: ");
		
		mprintf("\nCall with FD_CMD_WR_ENABLE = %i\n", FD_CMD_WR_ENABLE);
		int err = fd_zio_conf_set(&fd, NULL, NULL, FD_CMD_WR_ENABLE);
		
		if(err == -ENOTSUP)
		{
			fprintf(stderr, "%s: no support for White Rabbit (check the gateware).\n",
				argv[0]);
			return -1;
			//exit(1);
		} else if (err) {
			fprintf(stderr, "%s: fdelay_wr_mode(): %s\n",
				argv[0], strerror(errno));
			return -1;
			//exit(1);
		}

		while ((err = fd_zio_conf_set(&fd, NULL, NULL, FD_CMD_WR_QUERY)) != 0) {
			if( err == -ENOLINK )
			{
				fprintf(stderr, "%s: no White Rabbit link (check the cable and the switch).\n",
					argv[0]);
				return -1;
				//exit(1);
			}
			printf(".");
			usleep(1000*1000);
		}

		printf(" locked!\n");
		return 0;
	}

	if (wr_off) {
		if (fd_zio_conf_set(&fd, NULL, NULL, FD_CMD_WR_DISABLE) < 0) {
			fprintf(stderr, "%s: fdelay_wr_mode(): %s\n",
				argv[0], strerror(errno));
			return -1;
			//exit(1);
		}
		return 0;
	}


	if (fd_time_set(&fd, &t, NULL) < 0) {
		fprintf(stderr, "%s: fdelay_set_time(): %s\n",
			argv[0], strerror(errno));
		return -1;
	}
	return 0;
}

DEFINE_WRC_COMMAND(time) = {
	.name = "time",
	.exec = main_time,
};
