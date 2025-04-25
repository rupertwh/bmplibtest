/* bmplibtest - allocate.c
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


#define ALIGN8(a) (((size_t)(a) + 7) & ~(size_t)7)

static void ensure_space(int nbytes, bool aligned);

static const int   size    = 8 * 1024; /* size of a single buffer      */
static const int   nmax    = 1000;     /* max number of buffers        */
static const int   nincr   = 16;       /* number of buffers to grow by */
static int         nalloc  = 0;

static char  *currbuffer = NULL;
static int    used = 0;

static int    idx     = 0;
static char **buffers = NULL;



void* allocate(int size, bool align)
{
	char *ret;

	ensure_space(size, align);

	if (align) {
		ret = currbuffer + ALIGN8(used);
		used = ALIGN8(used) + size;
	}
	else {
		ret = currbuffer + used;
		used += size;
	}

	return ret;
}

void free_all(void)
{
	for (int i = 0; i < idx; i++)
		free(buffers[i]);
	free(buffers);

	currbuffer = NULL;
	buffers    = NULL;
	nalloc = 0;
	idx    = 0;
	used   = 0;
}


static void ensure_space(int nbytes, bool aligned)
{
	if (nbytes > size) {
		printf("%s(): requested too large of a buffer: %d\n", __func__, nbytes);
		exit(1);
	}
	if (nbytes < 1) {
		printf("%s(): requested invalid buffer size: %d\n", __func__, nbytes);
		exit(1);
	}

	int available = size - (aligned ? (int) ALIGN8(used) : used);
	if (currbuffer && nbytes <= available)
		return;

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

	if (!(currbuffer = malloc(size))) {
		perror(__func__);
		exit(1);
	}
	memset(currbuffer, 0, size);
	used = 0;
	buffers[idx++] = currbuffer;
}

