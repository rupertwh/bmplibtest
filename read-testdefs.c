/* bmplibtest - read-testdefs.c
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
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>

#include "read-testdefs.h"


static int read_line(FILE *file, char *buf, int size);
static int read_line_filter_comments(FILE *file, char *buf, int size);


struct Test* read_testdefs(FILE *file)
{
	struct Test  *testlist = NULL;
	struct Test **tail = &testlist;
	int           linelen, testlen = 0;
	char          linebuf[1024], testbuf[1024];

	do {
		linelen = read_line_filter_comments(file, linebuf, sizeof linebuf);
		if (linelen > 0) {
			if (testlen + linelen  >= sizeof testbuf) {
				printf("Test definition too long, max is %d\n", (int) (sizeof testbuf - 1));
				exit(1);
			}
			strcpy(testbuf + testlen, linebuf);
			testlen += linelen;
		} else if (testlen == 0) {
			/* skip empty lines while not in a test definition */
			continue;
		} else {
			/* empty non-comment line ends test definition */
			char *tmp = malloc(testlen + sizeof *testlist);
			if (!tmp) {
				perror("malloc test list");
				exit(1);
			}
			*tail = (struct Test*) tmp;
			strcpy((*tail)->str, testbuf);
			tail = &(*tail)->next;
			testlen = 0;
		}
	} while (linelen || !feof(file));
	*tail = NULL;
	return testlist;
}

void free_testdefs(struct Test *testlist)
{
	struct Test *next;

	for (struct Test *test = testlist; test; test = next) {
		next = test->next;
		free(test);
	}
}


static int read_line_filter_comments(FILE *file, char *buf, int size)
{
	int  count;

	do {
		count = read_line(file, buf, size);
	} while ('#' == *buf);

	return count;
}

static int read_line(FILE *file, char *buf, int size)
{
	int  c, count = 0;
	bool empty = true;

	while (EOF != (c = getc(file))) {
		if ('\n' == c)
			break;

		if (empty) {
			if (strchr(" \t", c))
				continue;
			else
				empty = false;
		}

		if (count + 1 >= size) {
				printf("Input line too long, max is %d\n", size - 1);
				exit(1);
		}

		buf[count++] = c;
	}
	if (EOF == c && !feof(file)) {
		perror("reading defs file");
		exit(1);
	}

	buf[count] = 0;

	return count;
}
