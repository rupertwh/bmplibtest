#ifndef CMDLINE_H
#define CMDLINE_H


struct Cmdline {
	int            verbose;
	int            nfiles;
	char         **file;
	bool           help;
};



bool cmd_parse(int argc, char **argv, struct Cmdline *cmdline);
void cmd_free(struct Cmdline *cmdline);
void cmd_usage(void);

#endif
