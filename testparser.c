/* bmplibtest - testparser.c
 *
 * Copyright (c) 2025, Rupert Weber.
 *
 * This file is part of bmplibtest.
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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "allocate.h"
#include "testparser.h"

static void ignore_comment(FILE *file);
static int  read_keyword(FILE *file, char *buffer, int size);

struct read_text_args
{
	FILE       *file;
	char       *buffer;
	int         size;
	const char *endswith;
	const char *invalid;
	const char *valid; /* if non-NULL, only these chars are valid */
	int         minlen; /* minum length before considering endswith */
	bool        keep_ending_char;
	bool newline_to_space; /* convert newline and following whitespace to a single space */
	bool allow_comments_within;
	bool no_eof; /* must not encounter EOF */
};
#define read_text(...) read_text_(&(struct read_text_args) { __VA_ARGS__ })
static int read_text_(struct read_text_args *args);

static void parse_command(FILE *file, const char *cmdname);
static void parse_actionlist(FILE *file);
static void parse_action(FILE *file, const char *actname);
static void parse_action_args(FILE *file);

static int  read_char(FILE *file);
static void unread_char(FILE *file, int c);

static void prettyprint(void);
static void dumpall(void);

static size_t line    = 1;
static size_t pos     = 0;
static size_t prevpos = 0;

static struct Command   *cmdlisthead    = NULL;
static struct Command  **cmdlist        = &cmdlisthead;
static struct Action   **curractionlist = NULL;
static struct Argument **currarglist    = NULL;

#define WHITESPACE " \t\r\n"

struct Command *parse_test_definitions(FILE *file)
{
	char keyword[32] = { 0 };

	while (read_keyword(file, keyword, sizeof keyword))
	{
		parse_command(file, keyword);
		memset(keyword, 0, sizeof keyword);
	}

	return cmdlisthead;
}

void print_test_definitions(enum TestPrintStyle style)
{
	switch (style)
	{
	case PRINTSTYLE_DUMP:
		dumpall();break;

	case PRINTSTYLE_PRETTY:
		prettyprint();break;

	default:
		fprintf(stderr, "%s(): invalid print style option %d\n",
		        __func__, (int)style);
		break;
	}
}

void free_cmdlist(void)
{
	free_all();
}

static void dumpall(void)
{
	struct Command  *cmd;
	struct Action   *action;
	struct Argument *arg;
	int              count = 0;

	for (cmd = cmdlisthead; cmd; cmd = cmd->next)
	{
		printf("Test %02d: '%s'\n", ++count, cmd->descr);
		for (action = cmd->actionlist; action; action = action->next)
		{
			printf(" +-'%s'\n", action->actname);
			for (arg = action->arglist; arg; arg = arg->next)
			{
				if (arg->argvalue && *arg->argvalue)
					printf("  +-'%s':'%s'\n", arg->argname, arg->argvalue);
				else
					printf("  +-'%s'\n", arg->argname);
			}
		}
	}
}

static void prettyprint(void)
{
	struct Command  *cmd;
	struct Action   *action;
	struct Argument *arg;
	int              count = 0;

	puts("\n# Test definitions:\n");
	for (cmd = cmdlisthead; cmd; cmd = cmd->next)
	{
		printf("# Test %02d:\n", ++count);
		printf("test (%s) {\n", cmd->descr);
		for (action = cmd->actionlist; action; action = action->next)
		{
			printf("\t%-13s { ", action->actname);
			for (arg = action->arglist; arg; arg = arg->next)
			{
				if (arg != action->arglist)
					printf(", ");
				if (arg->argvalue && *arg->argvalue)
					printf("%s: %s", arg->argname, arg->argvalue);
				else
					printf("%s", arg->argname);
			}
			if (action->arglist)
				puts(" }");
			else
				puts("}");
		}
		puts("}\n");
	}
	puts("\n");
}

static void add_argument(const char *argname, const char *argvalue)
{
	assert(argname != NULL);
	assert(argvalue != NULL);
	int namelen = (int)strlen(argname);
	int vallen  = (int)strlen(argvalue);

	if (!currarglist)
	{
		fprintf(stderr, "%s(): there is no current argument list! (%s)\n",
		                                                 __func__, argname);
		exit(1);
	}

	*currarglist = allocate(sizeof **currarglist, true);

	(*currarglist)->argname = allocate(namelen + 1, false);
	strcpy((*currarglist)->argname, argname);

	(*currarglist)->argvalue = allocate(vallen + 1, false);
	strcpy((*currarglist)->argvalue, argvalue);

	currarglist = &(*currarglist)->next;
}

static void add_action(const char *actname, int len)
{
	if (!curractionlist)
	{
		fprintf(stderr, "%s(): there is no current action list! (%s)\n",
		                                                __func__, actname);
		exit(1);
	}

	*curractionlist = allocate(sizeof **curractionlist, true);

	(*curractionlist)->actname = allocate(len + 1, false);
	strcpy((*curractionlist)->actname, actname);

	currarglist    = &(*curractionlist)->arglist;
	curractionlist = &(*curractionlist)->next;
}

static void action_done(void)
{
	if (!curractionlist)
	{
		fprintf(stderr, "%s(): there is no current action list to finalize!\n", __func__);
		exit(1);
	}

	currarglist = NULL;
}

static void add_command(const char *descr, int len, const char *cmdname)
{
	if (curractionlist)
	{
		fprintf(stderr, "%s(): there's already an unfinalized command! (%s)\n",
		        __func__, descr);
		exit(1);
	}

	*cmdlist = allocate(sizeof **cmdlist, true);

	(*cmdlist)->descr = allocate(len + 1, false);
	strcpy((*cmdlist)->descr, descr);

	if (!strcmp(cmdname, "test"))
	{
		(*cmdlist)->type = COMMAND_TEST;
	}
	else
	{
		fprintf(stderr, "%s(): Unknown command '%s' on line %zu\n",
		        __func__, cmdname, line);
		exit(1);
	}

	curractionlist = &(*cmdlist)->actionlist;
	cmdlist        = &(*cmdlist)->next;
}

static void command_done(void)
{
	if (!curractionlist)
	{
		fprintf(stderr, "%s(): there is no current command to finalize!\n", __func__);
		exit(1);
	}
	curractionlist = NULL;
}

static void parse_command(FILE *file, const char *cmdname)
{
	int  c;
	int  descr_len  = 0;
	char descr[112] = { 0 };
	bool has_descr = false, has_actionlist = false;

	/* A command keyword has been read, we now expect an optional
	 * description in parentheses, an opening brace '{' followed by a
	 * command list (which may be empty), and after the command list a
	 * closing brace '}'. We ignore white space and comments.
	 */

	while (EOF != (c = read_char(file)))
	{
		if ('}' == c)
		{
			if (!has_actionlist)
			{
				fprintf(stderr, "%s(): unexpected closing brace on line %zu, pos %zu\n",
				        __func__, line, pos);
				exit(1);
			}
			break;
		}

		if (strchr(WHITESPACE, c))
			continue;

		if ('#' == c)
		{
			ignore_comment(file);
			continue;
		}

		if ('(' == c)
		{
			if (has_descr)
			{
				fprintf(stderr, "%s(): command already has description '%s' (line %zu, pos %zu)\n",
				        __func__, descr, line, pos);
				exit(1);
			}
			descr_len = read_text(.file = file, .buffer = descr,
			                      .size = sizeof descr, .endswith = ")",
			                      .invalid = "{}(", .newline_to_space = true,
			                      .keep_ending_char      = false,
			                      .allow_comments_within = true,
			                      .no_eof                = true);
			has_descr = true;
			continue;
		}

		if ('{' == c)
		{
			if (has_actionlist)
			{
				fprintf(stderr, "%s(): cannot have nested commands, line %zu, pos %zu\n",
				        __func__, line, pos);
				exit(1);
			}
			add_command(descr, descr_len, cmdname);
			parse_actionlist(file);
			has_actionlist = true;
			continue;
		}
		fprintf(stderr, "%s(): Invalid char '%c' on line %zu, pos %zu\n",
		        __func__, c, line, pos);
		exit(1);
	}

	if (EOF == c)
	{
		if (feof(file))
			fprintf(stderr, "%s(): EOF while reading command '%s'\n",
			        __func__, descr);
		else
			perror(__func__);
		exit(1);
	}

	command_done();
}

static void parse_actionlist(FILE *file)
{
	int  c;
	char actname[48] = { 0 };
	int  len;

	/* We are inside the braces '{...}' of a command. We expect a list of actions or a closing
	 * brace '}' which ends the action list.
	 * For every action keyword we encounter, call parse_action().
	 * Ignore spaces and comments.
	 */

	while (EOF != (c = read_char(file)))
	{
		if ('}' == c)
		{
			unread_char(file, c);
			return;
		}

		if (strchr(WHITESPACE, c))
		{
			continue;
		}

		if ('#' == c)
		{
			ignore_comment(file);
			continue;
		}

		unread_char(file, c);

		len = read_keyword(file, actname, sizeof actname);
		if (len == 0)
		{
			fprintf(stderr, "%s(): Panic, empty action name around line %zu, pos %zu\n",
			        __func__, line, pos);
			exit(1);
		}
		add_action(actname, len);
		parse_action(file, actname);
		action_done();
	}
}

static void parse_action(FILE *file, const char *actname)
{
	int c;

	while (EOF != (c = read_char(file)))
	{
		if (strchr(WHITESPACE, c))
			continue;

		if ('#' == c)
		{
			ignore_comment(file);
			continue;
		}

		if ('{' == c)
		{
			parse_action_args(file);
			return;
		}

		fprintf(stderr, "%s(): Invalid char '%c' on line %zu, pos %zu, expected action arguments\n",
		        __func__, c, line, pos);
		exit(1);
	}

	if (EOF == c)
	{
		if (!feof(file))
			perror(__func__);
		else
			fprintf(stderr, "%s(): EOF while reading action '%s'\n",
			        __func__, actname);
		exit(1);
	}
}

static void parse_action_args(FILE *file)
{
	int  c;
	char arg[48] = { 0 }, val[48] = { 0 };
	bool have_arg = false;

	while (EOF != (c = read_char(file)))
	{
		if (strchr(WHITESPACE, c))
			continue;

		if ('#' == c)
		{
			ignore_comment(file);
			continue;
		}

		if (',' == c || '}' == c)
		{
			if (have_arg)
			{
				add_argument(arg, val);
				arg[0] = val[0] = 0;
				have_arg        = false;
			}

			if ('}' == c)
			{
				break;
			}
			continue;
		}

		if (!have_arg)
		{
			if (':' == c)
			{
				fprintf(stderr, "%s(): stray ':' on line %zu, pos %zu\n",
				        __func__, line, pos);
				exit(1);
			}
			unread_char(file, c);
			if (read_text(.file = file, .buffer = arg, .size = sizeof arg,
			              .no_eof = true, .endswith = ":,}" WHITESPACE,
			              .invalid = "{(", .keep_ending_char = true))
			{
				have_arg = true;
			}
		}
		else
		{
			if (':' == c)
			{
				read_text(.file = file, .buffer = val, .size = sizeof val,
				          .no_eof = true, .endswith = ",}" WHITESPACE,
				          .invalid = "{(:", .keep_ending_char = true);
			}
		}
	}
}

static int read_text_(struct read_text_args *args)
{
	int  c;
	int  len = 0, limbo_len = 0;
	char limbo_space[112] = { 0 };

	assert(args->size > 0);

	while (len < args->size - 1 && EOF != (c = read_char(args->file)))
	{
		if (len == 0 && strchr(WHITESPACE, c))
			continue;

		if (len == 0 && '#' == c)
		{
			ignore_comment(args->file);
			continue;
		}

		if (len >= args->minlen && strchr(args->endswith, c))
		{
			if (args->keep_ending_char)
				unread_char(args->file, c);
			break;
		}

		if ('#' == c)
		{
			if (args->allow_comments_within)
			{
				ignore_comment(args->file);
				continue;
			}
			unread_char(args->file, c);
			break;
		}

		if (args->valid && !strchr(args->valid, c))
		{
			fprintf(stderr, "%s(): invalid character '%c' on line %zu, pos %zu\n",
			        __func__, c, line, pos);
			exit(1);
		}

		if (args->invalid && strchr(args->invalid, c))
		{
			fprintf(stderr, "%s(): invalid character '%c' on line %zu, pos %zu\n",
			        __func__, c, line, pos);
			exit(1);
		}

		if (strchr(WHITESPACE, c))
		{
			if (args->newline_to_space && strchr("\r\n", c))
			{
				while (EOF != (c = read_char(args->file)))
				{
					if (strchr(WHITESPACE, c))
						continue;
					unread_char(args->file, c);
					break;
				}
				c = ' ';
			}

			if ((size_t)limbo_len + 1 >= sizeof limbo_space - 1)
			{
				fprintf(stderr, "%s(): ran out of limbo space on line %zu\n",
				        __func__, line);
				exit(1);
			}
			limbo_space[limbo_len++] = c;
			continue;
		}

		if (limbo_len > 0)
		{
			if (len + limbo_len > args->size - 2)
			{
				fprintf(stderr, "%s(): buffer too short to fit limbo space on line %zu\n",
				        __func__, line);
				exit(1);
			}
			strcat(args->buffer, limbo_space);
			len += limbo_len;
			memset(limbo_space, 0, sizeof limbo_space);
			limbo_len = 0;
		}
		args->buffer[len++] = c;
	}
	args->buffer[len] = 0;

	if (len == args->size - 1)
	{
		fprintf(stderr, "%s(): buffer too short on line %zu\n", __func__, line);
		exit(1);
	}

	if (EOF == c)
	{
		if (!feof(args->file))
		{
			perror(__func__);
			exit(1);
		}
		if (args->no_eof)
		{
			fprintf(stderr, "%s(): EOF while reading text: '%s'\n",
			        __func__, args->buffer);
			exit(1);
		}
	}

	return len;
}

static int read_keyword(FILE *file, char *buffer, int size)
{
	static const char valid[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

	int len = read_text(.file = file, .buffer = buffer, .size = size,
	                    .endswith = "({,;:\"'`#" WHITESPACE, .minlen = 1,
	                    .valid = valid, .keep_ending_char = true,
	                    .allow_comments_within = false);

	if (len == 0 && !feof(file))
	{
		fprintf(stderr, "%s(): Invalid keyword on line %zu, pos %zu\n", __func__, line, pos + 1);
		exit(1);
	}
	return len;
}

static void ignore_comment(FILE *file)
{
	int c;

	while (EOF != (c = read_char(file)))
	{
		if (c == '\n')
		{
			unread_char(file, c);
			return;
		}
	}
}

static int read_char(FILE *file)
{
	int c;

	c = getc(file);
	if ('\n' == c)
	{
		line++;
		prevpos = pos;
		pos     = 0;
	}
	else
	{
		pos++;
	}

	return c;
}

static void unread_char(FILE *file, int c)
{
	if ('\n' == c)
	{
		line--;
		pos = prevpos;
	}
	else
	{
		pos--;
	}

	ungetc(c, file);
}
