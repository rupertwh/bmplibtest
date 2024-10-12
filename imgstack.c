/* bmplibtest - imgstack.c
 *
 * Copyright (c) 2024-2025, Rupert Weber.
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


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <bmplib.h>

#include "imgstack.h"


static struct Image **imgstack  = NULL;
static int            imgcount  = 0;
static size_t         stacksize = 0;

static const int alloc_step = 3;

bool imgstack_push(struct Image *img)
{
	size_t         newsize;
	struct Image **tmp;

	if (stacksize < (imgcount+1) * sizeof *imgstack) {
		newsize = stacksize + alloc_step * sizeof *imgstack;
		if (!(tmp = realloc(imgstack, newsize))) {
			perror("realloc imgstack");
			return false;
		}
		imgstack  = tmp;
		stacksize = newsize;
	}

	imgstack[imgcount++] = img;
	return true;
}


struct Image* imgstack_get(int pos)
{
	/* pos: 0 == last, 1 == before last... */

	if (pos >= imgcount) {
		printf("imgstack: invalid pos %d. (stack count=%d)\n", pos, imgcount);
		return NULL;
	}

	return imgstack[imgcount - pos - 1];
}

void imgstack_delete(void)
{
	if (imgcount < 1) {
		printf("imgastack: called delete() on empty stack\n");
		return;
	}
	img_free(imgstack[imgcount - 1]);
	imgcount--;
}


void imgstack_clear(void)
{
	while (imgcount > 0) {
		imgstack_delete();
	}
}


void imgstack_destroy(void)
{
	imgstack_clear();
	if (imgstack) {
		free(imgstack);
		imgstack = NULL;
	}
}

void img_free(struct Image *img)
{
	if (img) {
		if (img->buffer)
			free(img->buffer);
		if (img->palette)
			free(img->palette);
	}
	free(img);
}

