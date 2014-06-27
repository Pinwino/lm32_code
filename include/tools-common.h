/*
 * Simple code that is repeated over several tools
 */

enum fd_command_help {
	FD_CMD_PULS_HELP = 0,
	FD_CMD_TIME_HELP,
	FD_CMD_TERM_HELP,
	FD_CMD_STAT_HELP,
};


extern void tools_getopt_d_i(int argc, char **argv, enum fd_command_help param);
extern int 	tools_need_help(int argc, char **argv);
extern int 	str_vector_length (char ** str);

#define TOOLS_UMODE_USER    0
#define TOOLS_UMODE_RAW     1
#define TOOLS_UMODE_FLOAT   2

extern void tools_report_time(char *name, struct fdelay_time *t, int umode);
extern void report_output_config(int channel, struct fdelay_pulse *p, int umode);

extern void help (enum fd_command_help param); /* This is needed in all tools */
