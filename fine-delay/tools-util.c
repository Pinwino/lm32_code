#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <opt.h>

#include "fdelay-lib.h"
#include "tools-common.h"

#include "kernel2lm32_layer.h"

#define printf mprintf
#define sprintf pp_sprintf

//extern void help(char *name); /* This is mandatory in all tools */

int str_vector_length (char ** str){
	int len = 0;
	
	while(str[len++] != NULL){}
	mprintf("len = %i\n", len);
	
	return (len-1);
}

void help (enum fd_command_help param){
	switch (param){
		case FD_CMD_PULS_HELP:
			mprintf("pulse: Use \"pulse [<opts>]\"\n");
			mprintf(
					"   -o <output>     ouput channel: 1..4 (default 1)\n"
					"   -c <count>      default is 0 and means forever\n"
					"   -m <mode>       \"pulse\" (default), \"delay\", \"disable\"\n"
					"   -r <reltime>    relative time,  e.g. \"10m+20u\" -- use m,u,n,p and add/sub\n"
					"   -D <date>       absolute time, <secs>:<nano>\n"
					"   -T <period>     period, e.g. \"50m-20n\" -- use m,u,n,p and add/sub\n"
					"   -w <width>      like period; defaults to 50%% period\n"
					"   -t              wait for trigger before //exiting\n"
					"   -p              pulse per seconds (sets -D -T -w)\n"
					"   -1              10MHz (sets -D -T -w)\n"
					"   -v              verbose (report action)\n");
		break;
		case FD_CMD_TIME_HELP:
			mprintf("time: a tool for manipulating the FMC Fine Delay time base.\n");
			mprintf("Use: \"time <command>\"\n");
			mprintf("   where the <command> can be:\n"
					"     get                    - shows current time and White Rabbit status.\n"
					"     local                  - sets the time source to the card's local oscillator.\n"
					"     wr                     - sets the time source to White Rabbit.\n"
					"     seconds:[nanoseconds]: - sets local time to the given value.\n");
		break;
		case FD_CMD_TERM_HELP:
			mprintf("term: Use \"term [on|off]\n");
			mprintf("   on: enables termination\n");
			mprintf("  off: disables termination (default value)\n");
		break;
		case FD_CMD_STAT_HELP:
			mprintf("status: reports channel programming\n");
			mprintf("Use: \"status [-r]\"\n");
			mprintf("   -r: display raw hardware configuration\n");
		break;
	}
}

void tools_getopt_d_i(int argc, char **argv, enum fd_command_help param)
{
	char *rest;
	int opt;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		if (opt =='h') {
			help (param);
		}
	}
}

int tools_need_help(int argc, char **argv)
{
	if (argc != 1)
		return 0;
	if (!strcmp(argv[0], "--help"))
		return 1;
	return 0;
}

//crappy adhoc replacement for sscanf just to save time and  optimize memory usage
int sscanf_addhoc_replacement(char *str, unsigned long long *var1, unsigned long long *var2){ 
	char *c, *ptr, *rest, tok[]= ":";
	int processed = 0;
	c=str;

	if ((ptr = strpbrk (c, tok)) > 0){
		ptr[0] = 0;
		*var1=strtol(c, &rest, 0);
		c = ptr+1;
		if (!(rest && *rest))
		{
			processed++;
			while (ptr++){
				if (isalpha(ptr[0]) || (ptr[0] == '/0')){
					if (ptr[0] != 0)
						ptr[0]=0;
					break;
				}
			}
			*var2=strtol(c, &rest, 0);
			if (! (rest && *rest))
				processed ++;
		} //if ((offset = strpbrk (c, tok) - c) > 0)	
	}
	return processed;
}

void tools_report_time(char *name, struct fdelay_time *t, int umode)
{
	unsigned long long picoseconds = t->coarse * 8000ULL +
		t->frac * 8000ULL / 4096ULL;

	printf("%s ", name);
	switch(umode) {
	case TOOLS_UMODE_USER:
		printf ("%10llu:%03llu,%03llu,%03llu,%03llu ps\n",
			(long long)(t->utc),
			((picoseconds)  / (1000LL * 1000 * 1000)),
			((picoseconds)  / (1000LL * 1000) % 1000),
			((picoseconds)  / (1000LL) % 1000),
			((picoseconds)  % 1000LL));
		break;
	case TOOLS_UMODE_FLOAT:
		printf ("float %10llu.%012llu\n", (long long)(t->utc),
			(picoseconds << 32) | (picoseconds >> 32));
		break;
	case TOOLS_UMODE_RAW:
		printf("raw   utc %10lli,  coarse %9li,  frac %9li\n",
		       (long long)t->utc, (long)t->coarse, (long)t->frac);
		break;
	}
}

static struct fdelay_time fd_ts_sub(struct fdelay_time a, struct fdelay_time b)
{
	struct fdelay_time rv;
	int f, c = 0;
	int64_t u = 0;

	f = a.frac - b.frac;
	if(f < 0)
	{
	    f += 4096;
	    c--;
	}

	c += a.coarse - b.coarse;
	if(c < 0)
	{
		c += 125 * 1000 * 1000;
		u--;
	}

	u += a.utc - b.utc;
	rv.utc = u;
	rv.coarse = c;
	rv.frac = f;
	rv.seq_id = 0;
	rv.channel = 0;
	return rv;
}


static void report_output_config_human(int channel, struct fdelay_pulse *p)
{
	struct fdelay_time width;

	printf("Channel %i: ", FDELAY_OUTPUT_HW_TO_USER(channel));

	int m = p->mode & 0x7f;
	
	switch(m)
	{
		case FD_OUT_MODE_DISABLED:
			printf("disabled\n");
			return;
		case FD_OUT_MODE_PULSE:
			printf("pulse generator mode");
			break;
		case FD_OUT_MODE_DELAY:
			printf("delay mode");
			break;
		default:
			printf("unknown mode\n");
			return;
	} 

	if(p->mode & 0x80) 
		printf(" (triggered) ");

	tools_report_time(m == FD_OUT_MODE_DELAY ? "\n  delay:           " : "\n  start at:        ", 
			  &p->start, TOOLS_UMODE_USER);

	width = fd_ts_sub(p->end, p->start);
	tools_report_time("  pulse width:     ", &width, TOOLS_UMODE_USER);

	if(p->rep != 1)
	{
		printf("  repeat:                    ");
		if(p->rep == -1)
			printf("infinite\n");
		else
			printf("%d times\n", p->rep);
		tools_report_time("  period:          ", &p->loop, TOOLS_UMODE_USER);
	}
}

void report_output_config_raw(int channel, struct fdelay_pulse *p, int umode)
{
	char mode[80];
	int m = p->mode & 0x7f;
	
	if (m == FD_OUT_MODE_DISABLED) strcpy(mode, "disabled");
	else if (m == FD_OUT_MODE_PULSE) strcpy(mode, "pulse");
	else if (m == FD_OUT_MODE_DELAY) strcpy(mode, "delay");
	else sprintf(mode, "%i (0x%04x)", p->mode, p->mode);

	if (p->mode & 0x80) 
	strcat(mode, " (triggered)");
	printf("Channel %i, mode %s, repeat %i %s\n",
		FDELAY_OUTPUT_HW_TO_USER(channel), mode,
		p->rep, p->rep == -1 ? "(infinite)" : "");
	tools_report_time("start", &p->start, umode);
	tools_report_time("end  ", &p->end, umode);
	tools_report_time("loop ", &p->loop, umode);
}

void report_output_config(int channel, struct fdelay_pulse *p, int umode)
{
    switch(umode)
    {
	case TOOLS_UMODE_USER: 
		report_output_config_human(channel, p);
		break;
	case TOOLS_UMODE_RAW: 
	case TOOLS_UMODE_FLOAT: 
		report_output_config_raw(channel, p, umode);
	default: 
	    break;
    }
}
