/* bmplibtest - cmdline.c
 *
 * Copyright (c) 2024-2025, Rupert Weber.
 *
 * This file is part of bmplibtest.
 *
 * bmplibtest is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "config.h"
#include "cmdline.h"

#if defined(__GNUC__)
	#define MAY_BE_UNUSED __attribute__((unused))
#else
	#define MAY_BE_UNUSED
#endif

enum Optnum {
	OP_VERBOSE,
	OP_QUIET,
	OP_HELP,
};

struct Option {
	const enum Optnum op;
	const int         shortname;
	const char       *longname;
	const bool        has_arg;
} s_options[] = {
	{ OP_VERBOSE, 'v', "verbose", false },
	{ OP_QUIET,   'q', "quiet",   false },
	{ OP_HELP,    '?', "help",    false },
};

static MAY_BE_UNUSED void add_opt_arg_str(char **result, const char *arg);
static const int shortname(enum Optnum op);
static const char* longname(enum Optnum op);
static void print_option(enum Optnum op);


/********************************************************
 * 	do_opt
 *
 * 	Handle argument-less options, i.e. simple
 * 	switches.
 *      E.g. "-p", "--yes"
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
 *
 * 	Handle options that have an argument.
 * 	E.g. "-f filename", "--mode=release"
 *******************************************************/

static bool do_opt_arg(const char *arg, int op, bool longopt, struct Cmdline *cmdline)
{
	char  *endptr = NULL;

	if (!*arg) {
		fprintf(stderr, "Missing argument to --%s option: '%s'\n", s_options[op].longname, arg);
		return false;
	}


	switch (s_options[op].op) {
/*
		case OP_SOME_INT_ARG:
			cmdline->myintarg = strtol(arg, &endptr, 10);
			break;

		case OP_SOME_STR_ARG:
			add_opt_arg_str(&cmdline->mystrarg, arg);
			break;
*/

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
 * 	cmd_usage
 *******************************************************/

void cmd_usage(void)
{
	const char *debug;

#ifdef DEBUG
	debug = "-debug";
#else
	debug = "";
#endif

	printf("%s v%s%s\n", PROGRAM_NAME, PROGRAM_VERSION, debug);
	printf("\nUsage:\n");
	printf("\t%s [options] [testnums...]\n", PROGRAM_NAME);
	printf("\nOptions:\n");

	print_option(OP_VERBOSE);
	print_option(OP_QUIET);
	printf("\t\tBe more or less verbose. Repeat option to be even more verbose\n"
               "\t\tor quiet.\n\n");

	print_option(OP_HELP);
	printf("\t\tPrint this help screen.\n\n");
}


/*
 * =====================================================================================
 *
 *   Everything below this line is generic and should not have to be adapted
 *   for respective command line options.
 *
 */


#define MAX(a, b) (a) > (b) ? (a) : (b)
#define ALIGN_TO_POINTER(a)  (((a) + (sizeof(void*) - 1)) & (~(sizeof(void*) - 1)))

static bool next_arg(const char **parg);
static bool do_opt(int op, struct Cmdline *cmdline);
static bool do_opt_arg(const char *arg, int op, bool longopt, struct Cmdline *cmdline);
static bool add_string(struct Cmdline *cmdline, const char *arg);
static bool strict_strcmp(const char *s1, const char *s2, size_t len);

static int    s_current_arg, s_argc;
static char **s_argv;
static size_t s_size, s_used = 0;
static char  *s_buffer;


/********************************************************
 * 	print_option
 *******************************************************/

static void print_option(enum Optnum op)
{
	printf("\t-%c, --%s\n", shortname(op), longname(op));
}


/********************************************************
 * 	add_opt_arg_str
 *******************************************************/

static MAY_BE_UNUSED void add_opt_arg_str(char **result, const char *arg)
{
	size_t sz;

	sz = strlen(arg) + 1;
	if (s_used > s_size - sz) {
		printf("cmdline buffer too small!\n");
		exit(1);
	}
	*result = s_buffer + s_used;
	strcpy(*result, arg);
	s_used += sz;
}


/********************************************************
 * 	estimate_size
 *******************************************************/

static size_t estimate_size(int argc, char **argv)
{
	size_t size = 0;

	for (int i = 1; i < argc; i++)
		size += ALIGN_TO_POINTER(strlen(argv[i]) + 1 + sizeof(struct Cmdlinestr));

	size += sizeof(struct Cmdline);

	return size;
}


/********************************************************
 * 	cmd_parse
 *******************************************************/

struct Cmdline* cmd_parse(int argc, char **argv)
{
	const char       *arg = NULL, *equalsign;
	int               i, op;
	bool              rest_is_args = false;
	static const int  opnum = sizeof s_options / sizeof s_options[0];
	size_t            comparelen;
	struct Cmdline   *cmdline;

	s_current_arg = 0;
	s_argc = argc;
	s_argv = argv;

	s_size = estimate_size(argc, argv);

	if (!(s_buffer = malloc(s_size))) {
		perror("malloc");
		return NULL;
	}
	memset(s_buffer, 0, s_size);
	cmdline = (struct Cmdline*) s_buffer;
	s_used += sizeof *cmdline;

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
					if (s_options[i].longname && strict_strcmp(arg, s_options[i].longname,
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

					if (!do_opt_arg(arg, op, true, cmdline))
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
						if (s_options[i].shortname == (int) (unsigned char) *arg) {
							op = i;
							break;
						}
					}
					if (opnum == op) {
						fprintf(stderr, "Unknown option -%c\n", (int) (unsigned char) *arg);
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
			if (!add_string(cmdline, arg))
				goto abort;
		}
	}


	/* printf("Cmdline: allocated %lu, used %lu\n", (unsigned long)s_size, (unsigned long)s_used); */

	return cmdline;

abort:
	cmd_free(cmdline);
	return NULL;
}


/********************************************************
 * 	cmd_free
 *******************************************************/

void cmd_free(struct Cmdline *cmdline)
{
	if (cmdline)
		free(cmdline);
}


/********************************************************
 * 	add_string
 *******************************************************/

static bool add_string(struct Cmdline *cmdline, const char *arg)
{
	size_t             sz;
	struct Cmdlinestr *str;

	sz = sizeof *str + strlen(arg) + 1;

	s_used = ALIGN_TO_POINTER(s_used);

	if (s_used > s_size - sz) {
		fprintf(stderr, "Cmdline add_string() ran out of space\n");
		exit(1);
	}

	str = (struct Cmdlinestr*) (s_buffer + s_used);
	s_used += sz;

	str->next = cmdline->strlist;
	cmdline->strlist = str;

	strcpy(str->str, arg);

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


/********************************************************
 * 	strict_strcmp
 *******************************************************/

static bool strict_strcmp(const char *s1, const char *s2, size_t len)
{
	if (!(len < SIZE_MAX))
		return false;

	for (size_t i = 0; i < len; i++) {
		if (s1[i] != s2[i] || !(s1[i] && s2[i]))
			return false;
	}
	return true;
}


/********************************************************
 * 	shortname
 *******************************************************/

static int shortname(enum Optnum op)
{
	int n = sizeof s_options / sizeof s_options[0];

	for (int i = 0; i < n; i++) {
		if (op == s_options[i].op)
			return s_options[i].shortname;
	}
	return 0;
}


/********************************************************
 * 	longname
 *******************************************************/

static const char* longname(enum Optnum op)
{
	int n = sizeof s_options / sizeof s_options[0];

	for (int i = 0; i < n; i++) {
		if (op == s_options[i].op)
			return s_options[i].longname;
	}
	return "0";
}
