/* bmplibtest - conf.c
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

#include "defs.h"
#include "config.h"
#include "conf.h"

enum Optnum
{
	OP_VERBOSE,
	OP_QUIET,
	OP_TESTFILE,
	OP_BMPSUITEDIR,
	OP_SAMPLEDIR,
	OP_REFDIR,
	OP_TMPDIR,
	OP_DUMP,
	OP_PRETTY,
	OP_HELP,
};

struct Option
{
	const enum Optnum op;
	const int         shortname;
	const char       *longname;
	const bool        has_arg;
	const char       *defaultstr;
	const char       *envname;
} s_options[] = {
	{     OP_VERBOSE, 'v',  "verbose", false,             NULL,                     NULL },
	{       OP_QUIET, 'q',    "quiet", false,             NULL,                     NULL },
	{    OP_TESTFILE, 'f',     "file",  true, "./testdefs.txt",    "BMPLIBTEST_TESTFILE" },
	{ OP_BMPSUITEDIR, 'b', "bmpsuite",  true,     "./bmpsuite", "BMPLIBTEST_BMPSUITEDIR" },
	{   OP_SAMPLEDIR, 's',  "samples",  true,      "./samples",   "BMPLIBTEST_SAMPLEDIR" },
	{      OP_REFDIR, 'r',     "refs",  true,         "./refs",      "BMPLIBTEST_REFDIR" },
	{      OP_TMPDIR, 't',      "tmp",  true,          "./tmp",      "BMPLIBTEST_TMPDIR" },
	{        OP_DUMP, 'd',     "dump", false,             NULL,                     NULL },
	{      OP_PRETTY, 'p',   "pretty", false,             NULL,                     NULL },
	{        OP_HELP, '?',     "help", false,             NULL,                     NULL },
};

static MAY_BE_UNUSED void add_opt_str(char **result, const char *arg);
static int                shortname(enum Optnum op);
static const char        *longname(enum Optnum op);
static const char        *envname(enum Optnum op);
static void               print_option(enum Optnum op);
static void print_option_with_arg(enum Optnum op, const char *argdescr);

/********************************************************
 * 	do_opt
 *
 * 	Handle argument-less options, i.e. simple
 * 	switches.
 *	E.g. "-p", "--yes"
 *******************************************************/

static bool do_opt(int op, struct Conf *cmdline)
{
	switch (s_options[op].op)
	{
	case OP_VERBOSE:
		cmdline->verbose++;
		break;

	case OP_QUIET:
		cmdline->verbose--;
		break;

	case OP_HELP:
		cmdline->help = true;
		break;

	case OP_DUMP:
		cmdline->dump = true;
		break;

	case OP_PRETTY:
		cmdline->pretty = true;
		break;

	default:
		printf("Something is boken\n");
		exit(1);
	}
	return true;
}

/********************************************************
 * 	do_opt_arg
 *
 * 	Handle options that have an argument.
 * 	E.g. "-f filename", "--mode=release"
 *******************************************************/

static bool do_opt_arg(const char *arg, int op, struct Conf *cmdline)
{
	char *endptr = NULL;

	if (!*arg)
	{
		fprintf(stderr, "Missing argument to --%s option: '%s'\n",
		        longname(op), arg);
		return false;
	}

	switch (s_options[op].op)
	{
	case OP_TESTFILE:
		add_opt_str(&cmdline->testfile, arg);
		break;

	case OP_BMPSUITEDIR:
		add_opt_str(&cmdline->bmpsuitedir, arg);
		break;

	case OP_SAMPLEDIR:
		add_opt_str(&cmdline->sampledir, arg);
		break;

	case OP_REFDIR:
		add_opt_str(&cmdline->refdir, arg);
		break;

	case OP_TMPDIR:
		add_opt_str(&cmdline->tmpdir, arg);
		break;

	default:
		printf("Something is boken\n");
		exit(1);
	}

	if (endptr && *endptr != '\0')
	{
		fprintf(stderr, "Invalid numerical argument to --%s option: '%s'\n",
		        longname(op), arg);
		return false;
	}

	return true;
}

/********************************************************
 * 	load_env_strings
 *******************************************************/

static void load_env_strings(struct Conf *cmdline)
{
	char *str;

	for (int i = 0; i < (int)ARRAY_SIZE(s_options); i++)
	{
		if (!s_options[i].envname)
			continue;
		if (!(str = getenv(s_options[i].envname)))
			continue;

		switch (s_options[i].op)
		{
		case OP_TESTFILE:
			add_opt_str(&cmdline->testfile, str);
			break;

		case OP_BMPSUITEDIR:
			add_opt_str(&cmdline->bmpsuitedir, str);
			break;

		case OP_SAMPLEDIR:
			add_opt_str(&cmdline->sampledir, str);
			break;

		case OP_REFDIR:
			add_opt_str(&cmdline->refdir, str);
			break;

		case OP_TMPDIR:
			add_opt_str(&cmdline->tmpdir, str);
			break;

		default:
			printf("Warning: env %s not used\n", s_options[i].envname);
			break;
		}
	}
}

/********************************************************
 * 	load_default_strings
 *******************************************************/

static void load_default_strings(struct Conf *cmdline)
{

	for (int i = 0; i < (int)ARRAY_SIZE(s_options); i++)
	{
		if (!s_options[i].defaultstr)
			continue;

		switch (s_options[i].op)
		{
		case OP_TESTFILE:
			add_opt_str(&cmdline->testfile, s_options[i].defaultstr);
			break;

		case OP_BMPSUITEDIR:
			add_opt_str(&cmdline->bmpsuitedir, s_options[i].defaultstr);
			break;

		case OP_SAMPLEDIR:
			add_opt_str(&cmdline->sampledir, s_options[i].defaultstr);
			break;

		case OP_REFDIR:
			add_opt_str(&cmdline->refdir, s_options[i].defaultstr);
			break;

		case OP_TMPDIR:
			add_opt_str(&cmdline->tmpdir, s_options[i].defaultstr);
			break;

		default:
			printf("Warning: default str for %s not used\n",
			       s_options[i].longname);
			break;
		}
	}
}

/********************************************************
 * 	conf_usage
 *******************************************************/

void conf_usage(void)
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
	printf("\n\tIf no test numbers are given, all available tests will be run.\n");
	printf("\t(Try -v, -d, or -p in order to find the test numbers.)\n");
	printf("\nOptions:\n");

	print_option_with_arg(OP_TESTFILE, "file");
	printf("\t\tText file with test definitions.\n\n");

	print_option_with_arg(OP_BMPSUITEDIR, "bmpsuite-dir");
	printf("\t\tDirectory with BMP Suite sample images in g/, q/, b/ subdirs.\n");
	printf("\t\t(see https://entropymine.com/jason/bmpsuite/)\n\n");

	print_option_with_arg(OP_SAMPLEDIR, "sample-dir");
	printf("\t\tDirectory with sample images.\n\n");

	print_option_with_arg(OP_REFDIR, "refs-dir");
	printf("\t\tDirectory with reference images.\n\n");

	print_option_with_arg(OP_TMPDIR, "tmp-dir");
	printf("\t\tDirectory where output images will be written.\n\n");

	print_option(OP_VERBOSE);
	print_option(OP_QUIET);
	printf("\t\tBe more or less verbose. Repeat option to be even more verbose\n"
	       "\t\tor quiet.\n\n");

	print_option(OP_DUMP);
	printf("\t\tPrint a dump of the parsed test list.\n\n");

	print_option(OP_PRETTY);
	printf("\t\tPretty-print the parsed test list in def file format.\n\n");

	print_option(OP_HELP);
	printf("\t\tPrint this help screen.\n\n");
}

/********************************************************
 * 	conf_free
 *******************************************************/

void conf_free(struct Conf *cmdline)
{
	if (!cmdline)
		return;
	if (cmdline->testfile)
		free(cmdline->testfile);
	if (cmdline->bmpsuitedir)
		free(cmdline->bmpsuitedir);
	if (cmdline->sampledir)
		free(cmdline->sampledir);
	if (cmdline->refdir)
		free(cmdline->refdir);
	if (cmdline->tmpdir)
		free(cmdline->tmpdir);

	free(cmdline);
}

/*
 * =====================================================================================
 *
 *   Everything below this line is generic and should not have to be adapted
 *   for respective command line options.
 *
 */

static bool next_arg(const char **parg);
static bool add_to_str_list(const char *arg);
static bool strict_strcmp(const char *s1, const char *s2, size_t len);
static int  capped_strlen(const char *str);

static int       s_current_arg, s_argc;
static char    **s_argv;
static int       s_size, s_used = 0;
static char     *s_buffer;
struct Confstr **s_listtail;

/********************************************************
 * 	print_option
 *******************************************************/

static void print_option(enum Optnum op)
{
	printf("\t-%c, --%s\n", shortname(op), longname(op));
}

/********************************************************
 * 	print_option_with_arg
 *******************************************************/

static void print_option_with_arg(enum Optnum op, const char *argdescr)
{
	const char *env = envname(op);

	if (env)
	{
		printf("\t-%c <%s>, --%s <%s>\n\t(env: %s)\n", shortname(op),
		       argdescr, longname(op), argdescr, env);
	}
	else
	{
		printf("\t-%c <%s>, --%s <%s>\n", shortname(op), argdescr,
		       longname(op), argdescr);
	}
}

/********************************************************
 * 	add_opt_str
 *******************************************************/

static MAY_BE_UNUSED void add_opt_str(char **result, const char *str)
{
	if (*result)
		free(*result);

	if (!(*result = malloc(capped_strlen(str) + 1)))
	{
		perror("opt arg malloc");
		exit(1);
	}
	strcpy(*result, str);
}

/********************************************************
 * 	estimate_size
 *******************************************************/

static int estimate_size(int argc, char **argv)
{
	int total_size = sizeof(struct Conf);

	for (int i = 1; i < argc; i++)
	{
		int item_size = ALIGN_TO_POINTER(capped_strlen(argv[i]) + 1 +
		                                 sizeof(struct Confstr));

		if (item_size >= INT_MAX - total_size)
		{
			printf("Crazy long argument list!!!\n");
			exit(1);
		}
		total_size += item_size;
	}
	return total_size;
}


static int s_find_opt_by_longname(const char *name, int len)
{
	for (int i = 0; i < (int) (sizeof s_options / sizeof s_options[0]); i++)
	{
		if (s_options[i].longname && strict_strcmp(name, s_options[i].longname,
		                             MAX(len, capped_strlen(s_options[i].longname))))
		{
			return i;
		}
	}
	return -1;
}

static int s_find_opt_by_shortname(int cname)
{
	for (int i = 0; i < (int) (sizeof s_options / sizeof s_options[0]); i++)
	{
		if (s_options[i].shortname == cname)
			return i;
	}
	return -1;
}



/********************************************************
 * 	conf_parse_cmdline
 *******************************************************/

struct Conf *conf_parse_cmdline(int argc, char **argv)
{
	const char      *arg = NULL;
	int              op;
	bool             rest_is_args = false;
	struct Conf     *conf;

	s_current_arg = 0;
	s_argc        = argc;
	s_argv        = argv;

	s_size = estimate_size(argc, argv);

	if (!(s_buffer = malloc(s_size)))
	{
		perror("malloc");
		return NULL;
	}
	memset(s_buffer, 0, s_size);
	conf = (struct Conf *)s_buffer;
	s_used += sizeof *conf;

	s_listtail = &conf->strlist;

	load_default_strings(conf);
	load_env_strings(conf);

	while (next_arg(&arg))
	{
		if ('-' == *arg && !rest_is_args)
		{
			/* short or long option */
			arg++;
			if ('-' == *arg)
			{
				/* long option */
				arg++;

				if (!*arg)
				{
					rest_is_args = true;
					continue;
				}

				int         comparelen;
				const char *equalsign = strchr(arg, '=');
				if (equalsign)
				{
					size_t optlen = equalsign - arg;
					if (optlen > (size_t)INT_MAX / 2)
					{
						fprintf(stderr, "option name way too long\n");
						goto abort;
					}
					comparelen = (int)optlen;
				}
				else
				{
					comparelen = capped_strlen(arg);
				}

				if (-1 == (op = s_find_opt_by_longname(arg, comparelen)))
				{
					fprintf(stderr, "Unknown option --%s\n", arg);
					goto abort;
				}

				if (s_options[op].has_arg)
				{
					/* long option with argument */
					if (!equalsign)
					{
						fprintf(stderr, "Option --%s needs an argument!\n", s_options[op].longname);
						goto abort;
					}
					arg = equalsign + 1;

					if (!do_opt_arg(arg, op, conf))
						goto abort;
				}
				else
				{
					/* long option without argument */
					if (equalsign)
					{
						fprintf(stderr, "Option --%s cannot have an argument!\n", s_options[op].longname);
						goto abort;
					}

					if (!do_opt(op, conf))
						goto abort;
				}
			}
			else
			{
				/*short option(s) */
				while (*arg)
				{
					if (-1 == (op = s_find_opt_by_shortname(*(unsigned char*)arg)))
					{
						fprintf(stderr, "Unknown option -%c\n", *(unsigned char*)arg);
						goto abort;
					}

					arg++;
					if (s_options[op].has_arg)
					{
						if (!*arg)
						{
							if (!next_arg(&arg))
							{
								fprintf(stderr, "Missing argument for -%c option\n",
								                                     s_options[op].shortname);
								goto abort;
							}
						}
						if (!do_opt_arg(arg, op, conf))
							goto abort;
						break; /* no more options after an argument */
					}
					else
					{
						if (!do_opt(op, conf))
							goto abort;
					}
				}
			}
		}
		else
		{
			/* non-option argument */
			if (!add_to_str_list(arg))
				goto abort;
		}
	}

	/* printf("Cmdline: allocated %lu, used %lu\n", (unsigned long)s_size, (unsigned long)s_used); */

	return conf;

abort:
	conf_free(conf);
	return NULL;
}

/********************************************************
 * 	add_to_str_list
 *******************************************************/

static bool add_to_str_list(const char *arg)
{
	int             sz;
	struct Confstr *item;

	sz = sizeof *item + capped_strlen(arg) + 1;

	s_used = ALIGN_TO_POINTER(s_used);

	if (s_used > s_size - sz)
	{
		fprintf(stderr, "Cmdline add_to_str_list() ran out of space\n");
		exit(1);
	}

	item    = (struct Confstr *)(s_buffer + s_used);
	s_used += sz;

	strcpy(item->str, arg);

	*s_listtail = item;
	s_listtail  = &item->next;

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

	for (size_t i = 0; i < len; i++)
	{
		if (s1[i] != s2[i] || !(s1[i] && s2[i]))
			return false;
	}
	return true;
}

/********************************************************
 * 	envname
 *******************************************************/

static const char *envname(enum Optnum op)
{
	for (int i = 0; i < (int)ARRAY_SIZE(s_options); i++)
	{
		if (op == s_options[i].op)
			return s_options[i].envname;
	}
	return "(not defined)";
}

/********************************************************
 * 	shortname
 *******************************************************/

static int shortname(enum Optnum op)
{
	for (int i = 0; i < (int)ARRAY_SIZE(s_options); i++)
	{
		if (op == s_options[i].op)
			return s_options[i].shortname;
	}
	return 0;
}

/********************************************************
 * 	longname
 *******************************************************/

static const char *longname(enum Optnum op)
{
	for (int i = 0; i < (int)ARRAY_SIZE(s_options); i++)
	{
		if (op == s_options[i].op)
			return s_options[i].longname;
	}
	return "(not defined)";
}

/********************************************************
 * 	capped_strlen
 *******************************************************/

static int capped_strlen(const char *str)
{
	size_t size = strlen(str);

	if (size > INT_MAX / 2)
	{
		printf("Crazy big string: %zu bytes\n", size);
		exit(1);
	}
	return (int)size;
}
