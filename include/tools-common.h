/*
 * Simple code that is repeated over several tools
 */

extern void tools_getopt_d_i(int argc, char **argv,
			     int *dev, int *index);
extern int  tools_need_help(int argc, char **argv);

#define TOOLS_UMODE_USER    0
#define TOOLS_UMODE_RAW     1
#define TOOLS_UMODE_FLOAT   2

extern void tools_report_time(char *name, struct fdelay_time *t, int umode);
extern void report_output_config(int channel, struct fdelay_pulse *p, int umode);

extern void help(char *name); /* This is mandatory in all tools */
