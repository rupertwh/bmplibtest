/* bmplibtest - imgstack.h
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

struct Image
{
	unsigned char *buffer;
	size_t         buffersize;
	unsigned char *palette;
	int            numcolors;
	unsigned char *iccprofile;
	size_t         iccprofile_size;
	int            width;
	int            height;
	int            channels;
	int            bitsperchannel;
	int            xdpi;
	int            ydpi;
	BMPFORMAT      format;
	BMPORIENT      orientation;
};

bool          imgstack_push(struct Image *img);
struct Image *imgstack_get(int pos);
bool          imgstack_swap(void);
void          imgstack_delete(void);
void          imgstack_clear(void);
void          img_free(struct Image *img);
void          imgstack_destroy(void);
