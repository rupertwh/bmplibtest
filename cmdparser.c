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



#define WHITESPACE " \t\n\r"
#define CMDSEP ";"
#define ARGSEP ","





static const int parse_ignore(const char *str, const char *ignore);


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
	int             total = 0;

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
		arglist = &(*arglist)->next;
		*cmdstr += len;
		if (strchr(ARGSEP, **cmdstr))
			(*cmdstr)++; /* skip separator */

	} while (true);

	return true;
}



static const int parse_ignore(const char *str, const char *ignore)
{
	int n = 0;

	while (str[n] && strchr(ignore, str[n]))
		n++;

	return n;
}
