/* bmplibtest - cmdparser.c
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
#include <ctype.h>
#include <stdbool.h>

#include "cmdparser.h"


/*
 * a simple parser for test definitions.
 *
 * Test definitions consist of a string that contains a series of commands
 * to execute.
 *
 * A single command has the form:
 *     <command-name>{<arg1>,<arg2>,<arg3name>:<arg3val>,<arg4name>:<arg4val>,...};
 *
 * Arguments can be simple strings (<arg1> and <arg2> in the example) or key/value pairs,
 * separated by a colon (:).
 *
 * e.g.  loadbmp{sample,picture.bmp};
 *
 * or    name{Load 4-bit RLE};
 *       loadbmp{sample,g/pal4rle.bmp};
 *       loadpng{ref,ref_8bit_12c_alpha.png};
 *       compare{}
 *
 * or    name    {Load 24-bit RGB + Save}
 *       loadbmp {sample, g/rgb24.bmp}; loadpng {ref, ref_8bit_255c.png}
 *       compare { }
 *       savebmp {tmp, rgb24out.bmp}; loadbmp{tmp, rgb24out.bmp}
 *       compare { }
 *
 * or    name    {Load 64-bit RGB s2.13}
 *       loadbmp {sample, q/rgba64.bmp, format:s2.13, conv64:srgb}
 *       savebmp {tmp, rgb64s2.13lin-to-24.bmp, format:int, bufferbits:8}
 *
 * all whitespace (including newlines) around commands and arguments will be
 * trimmed, the semicolon is optional
 *
 */




#define WHITESPACE " \t\n\r"
#define CMDSEP ";"
#define ARGSEP ","
#define VALSEP ':'


static const int parse_ignore(const char *str, const char *ignore);
static char* rtrim(char *str, int len);
static void split_arg(struct Cmdarg *arg);


bool next_command(const char **list, char *cmd, int size)
{
	int len = 0;

	*list += parse_ignore(*list, CMDSEP WHITESPACE);

	while (**list && len + 1 < size && isalnum(*(unsigned char*)*list))
		cmd[len++] = *(*list)++;

	cmd[len] = 0;

	return len > 0;
}


bool arglist_from_cmdstr(const char **cmdstr, char *buf, int size, struct Cmdarg **arglist)
{
	int total = 0;

	*cmdstr += parse_ignore(*cmdstr, WHITESPACE);

	if (**cmdstr != '{') {
		printf("Expected '{': %s\n", *cmdstr);
		return false;
	}
	(*cmdstr)++;

	do {
		int len = 0;

		*cmdstr += parse_ignore(*cmdstr, WHITESPACE);

		if (**cmdstr == '}') {
			(*cmdstr)++;
			*arglist = NULL;
			break;
		}
		if (!**cmdstr) {
			printf("argument list ends unexpectedly\n");
			*arglist = NULL;
			return false;
		}

		while ((*cmdstr)[len] && (*cmdstr)[len] != '}' &&
			   !strchr(ARGSEP, (unsigned char)(*cmdstr)[len]))
			len++;

		total = (total + 7) & 0xfffffff8;  /* align on 8-byte boundary */
		*arglist = (struct Cmdarg*) &buf[total];

		total += sizeof **arglist + len + 1;
		if (total > size) {
			printf("arglist buffer too small! Need >=%d, have %d\n", total, size);
			*arglist = NULL;
			return false;
		}

		(*arglist)->arg = (char*)*arglist + sizeof **arglist;

		strncpy((*arglist)->arg, *cmdstr, len);
		(*arglist)->arg[len] = 0;
		rtrim((*arglist)->arg, len);
		(*arglist)->val = NULL;

		split_arg(*arglist);

		arglist = &(*arglist)->next;
		*cmdstr += len;
		if (strchr(ARGSEP, **cmdstr))
			(*cmdstr)++; /* skip separator */

	} while (true);

	return true;
}

static void split_arg(struct Cmdarg *arg)
{
	int n;

	for (int i = 0; arg->arg[i]; i++) {
		if (arg->arg[i] == VALSEP) {

			arg->arg[i] = 0;
			rtrim(arg->arg, i);

			arg->val = arg->arg + i + 1;
			for (n = 0; arg->val[n] && strchr(WHITESPACE, (unsigned char)arg->val[n]); n++)
				;
			arg->val += n;
			break;
		}
	}
}

static char* rtrim(char *str, int len)
{
	if (len == 0)
		len = strlen(str);

	for (int i = len - 1; i >= 0; i--) {
		if (strchr(WHITESPACE, (unsigned char)str[i]))
			str[i] = 0;
		else
			break;
	}
	return str;
}

static const int parse_ignore(const char *str, const char *ignore)
{
	int n = 0;

	while (str[n] && strchr(ignore, str[n]))
		n++;

	return n;
}
