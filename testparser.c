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

#include "testparser.h"


#define ALIGN8(a) (((size_t)(a) + 7) & ~(size_t)7)

static void buffers_ensure_space(int nbytes, bool aligned);
static void ignore_comment(FILE *file);
static int read_keyword(FILE *file, char *buffer, int size);

struct read_text_args {
	FILE       *file;
	char       *buffer;
	int         size;
	const char *endswith;
	const char *invalid;
	bool        may_eof;
	bool        keep_ending_char;
};
#define read_text(...) read_text_(&(struct read_text_args){ __VA_ARGS__ })
static int read_text_(struct read_text_args *args);

static void ct_test(FILE *file);
static void test_commandlist(FILE *file);
static void test_command(FILE *file, const char *cmdname);
static void test_command_args(FILE *file, const char *cmdname);

static int read_char(FILE *file);
static void unread_char(FILE *file, int c);


static size_t line    = 1;
static size_t pos     = 0;
static size_t prevpos = 0;

static struct Test          *testlist = NULL;
static struct Test         **testlisthead = &testlist;
static struct Test          *currtest = NULL;
static struct TestCommand  **currcmdlist = NULL;
static struct TestArgument **currarglist = NULL;


static char  *s_currbuffer = NULL;
static int    s_used = 0;

static int    idx     = 0;
static char **buffers = NULL;


struct Test* parse_test_definitions(FILE *file)
{
	char keyword[30];

	while (read_keyword(file, keyword, sizeof keyword)) {
		if (!strcmp(keyword, "test")) {
			ct_test(file);
		} else {
			printf("Unkown keyword on line %zu: '%s'\n", line, keyword);
			exit(1);
		}
	}

	#ifdef NEVER
	dumpall();
	#endif

	return testlist;
}

void free_testlist(void)
{
	for (int i = 0; i < idx; i++)
		free(buffers[i]);
	free(buffers);
}



static void buffers_ensure_space(int nbytes, bool aligned)
{
	static const int   size    = 8 * 1024; /* size of a single buffer      */
	static const int   nmax    = 1000;     /* max number of buffers        */
	static const int   nincr   = 16;       /* number of buffers to grow by */
	static int         nalloc  = 0;

	int available = size - (aligned ? (int) ALIGN8(s_used) : s_used);

	if (s_currbuffer && nbytes <= available)
		return;

	if (nbytes > size) {
		printf("%s(): requested too large of a buffer: %d\n", __func__, nbytes);
		exit(1);
	}

	if (idx >= nalloc) {
		int newalloc = nalloc + nincr;

		if (newalloc > nmax) {
			printf("%s(): exceeded max. number of pool buffers (%d)\n", __func__, newalloc);
			exit(1);
		}

		char **tmp = realloc(buffers, newalloc * sizeof *buffers);
		if (!tmp) {
			perror(__func__);
			exit(1);
		}
		buffers = tmp;
		nalloc  = newalloc;
	}

	if (!(s_currbuffer = malloc(size))) {
		perror(__func__);
		exit(1);
	}
	memset(s_currbuffer, 0, size);
	s_used = 0;
	buffers[idx++] = s_currbuffer;
}



#ifdef NEVER
static void dumpall(void)
{
	struct Test         *test;
	struct TestCommand  *cmd;
	struct TestArgument *arg;

	for (test = testlist; test; test = test->next) {
		printf("Test: '%s'\n", test->descr);
		for (cmd = test->cmdlist; cmd; cmd = cmd->next) {
			printf(" +-'%s'\n", cmd->cmdname);
			for (arg = cmd->arglist; arg; arg = arg->next) {
				if (arg->argvalue && *arg->argvalue)
					printf("  +-'%s':'%s'\n", arg->argname, arg->argvalue);
				else
					printf("  +-'%s'\n", arg->argname);
			}
		}
	}

	printf("Memory used: %zu of %zu\n", testbuf_used, testbuffer_size);
}
#endif


static void add_argument(const char *argname, const char *argvalue)
{
	assert(argname != NULL);
	assert(argvalue != NULL);
	int namelen = (int) strlen(argname);
	int vallen  = (int) strlen(argvalue);

	if (!currarglist) {
		fprintf(stderr, "%s(): there is no current argument list! (%s)\n", __func__, argname);
		exit(1);
	}

	buffers_ensure_space(sizeof **currarglist, true);
	*currarglist = (struct TestArgument*) (s_currbuffer + ALIGN8(s_used));
	s_used = ALIGN8(s_used) + sizeof **currarglist;

	buffers_ensure_space(namelen + 1, false);
	(*currarglist)->argname = s_currbuffer + s_used;
	strcpy((*currarglist)->argname, argname);
	s_used += namelen + 1;

	buffers_ensure_space(vallen + 1, false);
	(*currarglist)->argvalue = s_currbuffer + s_used;
	strcpy((*currarglist)->argvalue, argvalue);
	s_used += vallen + 1;

	currarglist = &(*currarglist)->next;
}


static void add_command(const char *cmdname, int len)
{
	if (!currcmdlist) {
		fprintf(stderr, "%s(): there is no current command list! (%s)\n", __func__, cmdname);
		exit(1);
	}

	buffers_ensure_space(sizeof **currcmdlist, true);
	*currcmdlist = (struct TestCommand*) (s_currbuffer + ALIGN8(s_used));
	s_used = ALIGN8(s_used) + sizeof **currcmdlist;

	buffers_ensure_space(len + 1, false);
	(*currcmdlist)->cmdname = s_currbuffer + s_used;
	strcpy((*currcmdlist)->cmdname, cmdname);
	s_used += len + 1;

	currarglist = &(*currcmdlist)->arglist;
	currcmdlist = &(*currcmdlist)->next;


}

static void command_done(void)
{
	if (!currcmdlist) {
		fprintf(stderr, "%s(): there is no current command list to finalize!\n", __func__);
		exit(1);
	}

	currarglist = NULL;
}


static void add_test(const char *descr, int len)
{
	if (currtest) {
		fprintf(stderr, "%s(): there's already an unfinalized test! (%s)\n", __func__, descr);
		exit(1);
	}

	buffers_ensure_space(sizeof *currtest, true);
	currtest = (struct Test*) (s_currbuffer + ALIGN8(s_used));
	s_used = ALIGN8(s_used) + sizeof *currtest;

	buffers_ensure_space(len + 1, false);
	currtest->descr = s_currbuffer + s_used;
	strcpy(currtest->descr, descr);
	s_used += len + 1;

	currcmdlist = &currtest->cmdlist;
}

static void test_done(void)
{
	if (!currtest) {
		fprintf(stderr, "%s(): there is no current test to finalize!\n", __func__);
		exit(1);
	}

	*testlisthead = currtest;
	testlisthead = &currtest->next;
	currtest = NULL;
	currcmdlist = NULL;
}



static void ct_test(FILE *file)
{
	int   c;
	bool  has_descr = false;
	int   descr_len = 0;
	char  descr[120] = { 0 };


	while (EOF != (c = read_char(file))) {
		if ('(' == c && !has_descr) {
			descr_len = read_text(.file = file, .buffer = descr, .size = sizeof descr,
			                      .endswith = ")", .invalid = "{}(\r\n",
			                      .may_eof = false, .keep_ending_char = false);
			has_descr = true;
			continue;
		}
		if ('#' == c) {
			ignore_comment(file);
			continue;
		}
		if ('{' == c) {
			add_test(descr, descr_len);
			test_commandlist(file);
			break;
		}
	}
	if (EOF == c) {
		if (feof(file))
			fprintf(stderr, "%s(): EOF while reading test '%s'\n", __func__, descr);
		else
			perror(__func__);
		exit(1);
	}

	test_done();
}


static void test_commandlist(FILE *file)
{
	int  c;
	char command[80] = { 0 };
	int  len;

	while (EOF != (c = read_char(file))) {
		if ('}' == c) {
			return;
		}
		if ('#' == c) {
			ignore_comment(file);
			continue;
		}
		if (strchr(" \t\r\n", c)) {
			continue;
		}
		if ('{' == c) {
			fprintf(stderr, "%s(): invalid char '%c' in line %zu\n", __func__, c, line);
			exit(1);
		}

		unread_char(file, c);

		len = read_keyword(file, command, sizeof command);
		if (len == 0) {
			fprintf(stderr, "%s(): Panic, empty command name around line %zu\n", __func__, line);
			exit(1);
		}
		add_command(command, len);
		test_command(file, command);
		command_done();
	}


}


static void test_command(FILE *file, const char *cmdname)
{
	int  c;

	while (EOF != (c = read_char(file))) {
		if ('}' == c) {
			return;
		}
		if ('#' == c) {
			ignore_comment(file);
			continue;
		}

		if (strchr(" \t\r\n", c))
			continue;

		if ('{' == c) {
			test_command_args(file, cmdname);
			return;
		}
	}

	if (EOF == 'c') {
		if (!feof(file))
			perror(__func__);
		else
			fprintf(stderr, "%s(): EOF while reading commad '%s'\n", __func__, cmdname);
		exit(1);
	}

}


static void test_command_args(FILE *file, const char *cmdname)
{
	int    c;
	char   arg[80] = { 0 }, val[80] = { 0 };
	bool   have_arg = false;


	while (EOF != (c = read_char(file))) {
		if ('#' == c) {
			ignore_comment(file);
			continue;
		}
		if (strchr(" \t\r\n", c))
			continue;

		if (',' == c || '}' == c) {
			if (have_arg) {
				add_argument(arg, val);
				arg[0] = val[0] = 0;
				have_arg = false;
			}
			if ('}' == c) {
				break;
			}
			continue;
		}

		if (!have_arg) {
			if (':' == c) {
				fprintf(stderr, "%s(): stray ':' on line %zu, pos %zu\n", __func__, line, pos);
				exit(1);
			}
			unread_char(file, c);
			if (read_text(.file = file, .buffer = arg, .size = sizeof arg,
			              .endswith = ":,} \t\n", .invalid = "{(", .may_eof = false, .keep_ending_char = true)) {
				have_arg = true;
			}
		} else {
			if (':' == c) {
				read_text(.file = file, .buffer = val, .size = sizeof val,
				          .endswith = ",} \t\n", .invalid = "{(:", .may_eof = false, .keep_ending_char = true);
			}
		}

	}

}


static int read_text_(struct read_text_args *args)
{
	int  c;
	int  len = 0, limbo_len = 0;
	char limbo_space[100] = { 0 };

	assert(args->size > 0);

	while (len < args->size - 1 && EOF != (c = read_char(args->file))) {
		if (len == 0 && strchr(" \t\r\n", c))
			continue;
		if (len == 0 && '#' == c) {
			ignore_comment(args->file);
			continue;
		}

		if (strchr(args->endswith, c)) {
			if (args->keep_ending_char)
				unread_char(args->file, c);
			break;
		}

		if ('#' == c) {
			unread_char(args->file, c);
			break;
		}

		if (strchr(args->invalid, c)) {
			fprintf(stderr, "%s(): invalid character '%c' on line %zu\n", __func__, c, line);
			exit(1);
		}

		if (strchr(" \t", c)) {
			if (len == 0)
				continue;
			if (limbo_len + 1 == sizeof limbo_space - 1) {
				fprintf(stderr, "%s(): ran out of limbo space on line %zu\n", __func__, line);
				exit(1);
			}
			limbo_space[limbo_len++] = c;
			continue;
		}

		if (limbo_len > 0) {
			if (len + limbo_len > args->size - 2) {
				fprintf(stderr, "%s(): buffer too short to fit limbo space on line %zu\n", __func__, line);
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

	if (len == args->size - 1) {
		fprintf(stderr, "%s(): buffer too short on line %zu\n", __func__, line);
		exit(1);
	}
	if (EOF == 'c') {
		if (!feof(args->file)) {
			perror("read_text");
			exit(1);
		}
		if (!args->may_eof) {
			fprintf(stderr, "%s(): EOF while reading text: '%s'\n", __func__, args->buffer);
			exit(1);
		}
	}

	return len;
}


static int read_keyword(FILE *file, char *buffer, int size)
{
	return read_text(.file = file, .buffer = buffer, .size = size,
	                 .endswith = "({ \t\r\n,;:\"'`", .invalid = "})",
	                 .may_eof = false, .keep_ending_char = true);

}


static void ignore_comment(FILE *file)
{
	int c;

	while (EOF != (c = read_char(file))) {
		if (c == '\n') {
			unread_char(file, c);
			return;
		}
	}
}


static int read_char(FILE *file)
{
	int c;

	c = getc(file);
	if ('\n' == c) {
		line++;
		prevpos = pos;
		pos = 0;
	}

	return c;
}

static void unread_char(FILE *file, int c)
{
	if ('\n' == c) {
		line--;
		pos = prevpos;
	}

	ungetc(c, file);
}
