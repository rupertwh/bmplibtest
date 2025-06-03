/* bmplibtest - bmpinspect.c
 *
 * Copyright (c) 2024-2025, Rupert Weber.
 *
 * bmpinspect is free software: you can redistribute it and/or modify
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
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#include <bmplib.h>

#define DESCR_WIDTH 20

const char *bmtype_descr(BMPIMAGETYPE type);

int main(int argc, char **argv)
{
	FILE     *file = NULL;
	BMPHANDLE h;

	if (argc < 2)
	{
		printf("Usage: bmparray <bmp-file>\n");
		return 1;
	}

	bool ret = true;
	for (int i = 1; i < argc; i++)
	{
		if (!(file = fopen(argv[i], "rb")))
		{
			perror(argv[i]);
			ret = false;
			break;
		}

		printf("\n--- %s ---\n", argv[i]);

		if (!(h = bmpread_new(file)))
		{
			fprintf(stderr, "Failed to get bmp handle\n");
			fclose(file);
			ret = false;
			continue;
		}

		if (BMP_RESULT_ARRAY != bmpread_load_info(h))
		{
			fprintf(stderr, "Not a BMP array (BA)\n");
			bmp_free(h);
			fclose(file);
			ret = false;
			continue;
		}

		int n = bmpread_array_num(h);

		for (int j = 0; j < n; j++)
		{
			struct BmpArrayInfo ai = { 0 };

			if (BMP_RESULT_OK != bmpread_array_info(h, &ai, j))
			{
				fprintf(stderr, "%s\n", bmp_errmsg(h));
				continue;
			}

			printf("%02d: %s\n", j, bmtype_descr(ai.type));
			printf("    %d x %d, %d colors\n", ai.width, ai.height, ai.ncolors);
			if (ai.screenwidth || ai.screenheight)
				printf("    Screen: %d x %d\n", ai.screenwidth, ai.screenheight);

			puts("");
		}

		bmp_free(h);
		fclose(file);
		file = NULL;
	}
	return !ret;
}

const char *bmtype_descr(BMPIMAGETYPE type)
{
	switch (type)
	{
	case BMP_IMAGETYPE_BM  : return "Windows or OS/2 BMP [BM]";
	case BMP_IMAGETYPE_BA  : return "OS/2 bitmap array [BA]";
	case BMP_IMAGETYPE_CI  : return "OS/2 color icon [CI]";
	case BMP_IMAGETYPE_CP  : return "OS/2 color pointer [CP]";
	case BMP_IMAGETYPE_IC  : return "OS/2 icon (b/w) [IC]";
	case BMP_IMAGETYPE_PT  : return "OS/2 pointer (b/w) [PT]";
	case BMP_IMAGETYPE_NONE: return "not specified";
	default                : return "(unknown)";
	}
}
