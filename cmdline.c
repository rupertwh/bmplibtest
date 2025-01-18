
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>

#include "config.h"

#include "cmdline.h"

#define MAX(a, b) (a) > (b) ? (a) : (b)

enum Optnum {
	OP_VERBOSE,
	OP_QUIET,
	OP_HELP,
};

struct Option {
	const enum Optnum op;
	const int         shortname;
	const char       *longname;
	const int         has_arg;
} s_options[] = {
	{ OP_VERBOSE, 'v', "verbose", 0 },
	{ OP_QUIET,   'q', "quiet",   0 },
	{ OP_HELP,    '?', "help",    0 },
};


static int    s_current_arg, s_argc;
static char **s_argv;

static bool next_arg(const char **parg);
static bool do_opt(int op, struct Cmdline *cmdline);
static bool do_opt_arg(const char *arg, int op, int longopt, struct Cmdline *cmdline);
static bool add_file(struct Cmdline *cmdline, const char *arg);


/********************************************************
 * 	do_opt
 *******************************************************/

static bool do_opt(int op, struct Cmdline *cmdline)
{
	switch (s_options[op].op) {
		case OP_VERBOSE:
			cmdline->verbose++;
			break;

		case OP_QUIET:
			cmdline->verbose--;
			break;

		case OP_HELP:
			cmdline->help = true;
			break;

		default:
			fprintf(stderr, "do_opt(): Impossible option %s!\n", s_options[op].longname);
			return false;
	}
	return true;
}



/********************************************************
 * 	do_opt_arg
 *******************************************************/

static bool do_opt_arg(const char *arg, int op, int longopt, struct Cmdline *cmdline)
{
	char *endptr = NULL;

	if (!*arg) {
		fprintf(stderr, "Missing argument to --%s option: '%s'\n", s_options[op].longname, arg);
		return false;
	}

	switch (s_options[op].op) {

		default:
			fprintf(stderr, "do_opt_arg(): Impossible option %s!\n", s_options[op].longname);
			return false;
	}

	if (endptr && *endptr != '\0') {
		fprintf(stderr, "Invalid numerical argument to --%s option: '%s'\n", s_options[op].longname, arg);
		return false;
	}

	return true;
}



/********************************************************
 * 	cmd_parse
 *******************************************************/

bool cmd_parse(int argc, char **argv, struct Cmdline *cmdline)
{
	const char       *arg = NULL, *equalsign;
	int               i, op;
	bool              rest_is_args = false;
	static const int  opnum = sizeof s_options / sizeof s_options[0];
	size_t            comparelen;

	s_current_arg = 0;
	s_argc = argc;
	s_argv = argv;

	while (next_arg(&arg))
	{
		if ('-' == *arg && !rest_is_args) {
			/* short or long option */
			arg++;
			if ('-' == *arg) {
				/* long option */
				arg++;

				if (!*arg) {
					rest_is_args = true;
					continue;
				}
				equalsign = strchr(arg, '=');
				comparelen = equalsign ? equalsign - arg : strlen(arg);

				for (i = 0, op = opnum; i < opnum; i++) {
					if (s_options[i].longname && !strncmp(arg, s_options[i].longname,
					                             MAX(comparelen, strlen(s_options[i].longname)))) {
						op = i;
						break;
					}
				}
				if (op == opnum) {
					fprintf(stderr, "Unknown option --%s\n", arg);
					goto abort;
				}

				if (s_options[op].has_arg) {
					/* long option with argument */

					if (!equalsign) {
						fprintf(stderr, "Option --%s needs an argument!\n", s_options[op].longname);
						goto abort;
					}
					arg = equalsign + 1;

					if (!do_opt_arg(arg, op, 1, cmdline))
						goto abort;

				} else {
					/* long option without argument */
					if (equalsign) {
						fprintf(stderr, "Option --%s cannot have an argument!\n", s_options[op].longname);
						goto abort;
					}

					if (!do_opt(op, cmdline))
						goto abort;

				}

			} else {
				/*short option(s) */
				while (*arg) {
					for (i = 0, op = opnum; i < opnum; i++) {
						if (s_options[i].shortname == (int) *arg) {
							op = i;
							break;
						}
					}
					if (opnum == op) {
						fprintf(stderr, "Unknown option -%c\n", (int) *arg);
						goto abort;
					}

					arg++;
					if (s_options[op].has_arg) {
						if (!*arg) {
							if (!next_arg(&arg)) {
								fprintf(stderr, "Missing argument for -%c option\n", s_options[op].shortname);
								goto abort;
							}
						}
						if (!do_opt_arg(arg, op, 0, cmdline))
							goto abort;
						break; /* no more options after an arg */
					} else {
						if (!do_opt(op, cmdline))
							goto abort;
					}
				}
			}

		} else {  /* non-option argument */
			if (!add_file(cmdline, arg))
				goto abort;
		}
	}
	return true;

abort:
	/*fprintf(stderr, "Invalid argument '%s'\n", arg);*/
	cmd_free(cmdline);
	return false;
}



/********************************************************
 * 	cmd_usage
 *******************************************************/

void cmd_usage(void)
{
	char *progname, *version, *debug;

#ifdef DEBUG
	debug = " debug";
#else
	debug = "";
#endif

	progname = PROGRAM_NAME;
	version  = PROGRAM_VERSION;

	printf("%s v%s%s\n", progname, version, debug);
	printf("\nUsage:\n");
	printf("\t%s [options] [testnums...]\n", progname);
	printf("\nOptions:\n");

	printf("\t-%c, --%s\n", s_options[OP_VERBOSE].shortname,
	                        s_options[OP_VERBOSE].longname);
	printf("\t-%c, --%s\n", s_options[OP_QUIET].shortname,
	                        s_options[OP_QUIET].longname);
	printf("\t\tBe more or less verbose. Repeat option to be even more verbose\n"
		   "\t\tor quiet.\n");
	printf("\t\t(-%c%c to -%c%c)\n\n", s_options[OP_QUIET].shortname,
	                                   s_options[OP_QUIET].shortname,
	                                   s_options[OP_VERBOSE].shortname,
	                                   s_options[OP_VERBOSE].shortname);

	printf("\t\t-%c%c\tsuper quiet, not even error messages\n",
	                                                         s_options[OP_QUIET].shortname,
	                                                         s_options[OP_QUIET].shortname);
	printf("\t\t-%c\tquiet, only fatal/serious error messages\n",
	                                                         s_options[OP_QUIET].shortname);
	printf("\t\t(none)\tbasic progress information\n");
	printf("\t\t-%c\tverbose, print detailed information\n", s_options[OP_VERBOSE].shortname);
	printf("\t\t-%c%c\tsuper verbose, print lots and lots of info\n\n",
	                                                         s_options[OP_VERBOSE].shortname,
	                                                         s_options[OP_VERBOSE].shortname);

	printf("\t-%c, --%s\n", s_options[OP_HELP].shortname,
	                        s_options[OP_HELP].longname);
	printf("\t\tPrint this help screen.\n\n");
}



/********************************************************
 * 	cmd_free
 *******************************************************/

void cmd_free(struct Cmdline *cmdline)
{
	if (cmdline->file) {
		for (int i = 0; i < cmdline->nfiles; i++) {
			if (cmdline->file[i]) {
				free(cmdline->file[i]);
				cmdline->file[i] = NULL;
			}
		}
		cmdline->nfiles = 0;
		free(cmdline->file);
		cmdline->file = NULL;
	}	
}



/********************************************************
 * 	add_file
 *******************************************************/

static bool add_file(struct Cmdline *cmdline, const char *arg)
{
	char      **tmp;
	static int  nalloc = 0, newsize;
	const int   inc = 5;

	if (nalloc < cmdline->nfiles + 1) {
		if (nalloc >= INT_MAX - inc) {
			perror("add_file() crazy big memory needed!");
			return false;
		}
		tmp = realloc(cmdline->file, (newsize = nalloc + inc) * sizeof *cmdline->file);
		if (!tmp) {
			perror("add_file()");
			return false;
		}
		nalloc = newsize;
		cmdline->file = tmp;
	}

	if (!(cmdline->file[cmdline->nfiles] = malloc(strlen(arg) + 1))) {
		perror("add_file() malloc()");
		return false;
	}

	strcpy(cmdline->file[cmdline->nfiles++], arg);

	return true;
}



/********************************************************
 * 	next_arg
 *******************************************************/

static bool next_arg(const char **parg)
{
	if (!parg)
		return false;

	if (++s_current_arg >= s_argc)
		return false;

	*parg = s_argv[s_current_arg];
	return true;
}
