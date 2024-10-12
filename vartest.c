/* bmplibtest - vartest.c
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>

#include <png.h>
#include <bmplib.h>

#include "imgstack.h"
#include "cmdparser.h"

const char sampledir[] = "/home/rw/source/bmpsuite-2.8/";
const char refdir[]    = "/home/rw/source/bmplibtest/refs/";
const char tmpdir[]    = "/home/rw/source/bmplibtest/tmp/";




char* testdef[] = {

	"name{Load 1-bit indexed bl/grn}"
	"loadbmp{sample,g/pal1bg.bmp}; loadpng{ref,ref_8bit_2bg.png};"
	"compare{};",

	"name{Load 1-bit indexed b/w}; "
	"loadbmp{sample,g/pal1.bmp}; loadpng{ref,ref_8bit_2bw.png};"
	"compare{ }",

	"name{Load 1-bit indexed w/b}; "
	"loadbmp{sample,g/pal1wb.bmp}; loadpng{ref,ref_8bit_2bw.png};"
	"compare{ }",

	"name{Load 4-bit indexed}; "
	"loadbmp{sample,g/pal4.bmp}; loadpng{ref,ref_8bit_12c.png};"
	"compare{ }",

	"name{Load 4-bit indexed gs }; "
	"loadbmp{sample,g/pal4gs.bmp}; loadpng{ref,ref_8bit_12gs.png};"
	"compare{ }",

	"name{Load 4-bit RLE}; "
	"loadbmp{sample,g/pal4rle.bmp}; loadpng{ref,ref_8bit_12c_alpha.png};"
	"compare{ }",

	"name{Load 8-bit indexed all zero}; "
	"loadbmp{sample,g/pal8-0.bmp}; loadpng{ref,ref_8bit_252c.png};"
	"compare{ }",

	"name{Load 8-bit indexed}; "
	"loadbmp{sample,g/pal8.bmp}; loadpng{ref,ref_8bit_252c.png};"
	"compare{ }",

	"name{Load 8-bit indexed gs}; "
	"loadbmp{sample,g/pal8gs.bmp}; loadpng{ref,ref_8bit_252gs.png};"
	"compare{ }",

	"name{Load 8-bit indexed nonsquare}; "
	"loadbmp{sample,g/pal8nonsquare.bmp}; loadpng{ref,ref_8bit_252c_nonsquare.png};"
	"compare{ }",

	"name{Load 8-bit indexed OS/2}; "
	"loadbmp{sample,g/pal8os2.bmp}; loadpng{ref,ref_8bit_252c.png};"
	"compare{ }",

	"name{Load 8-bit indexed OS/2 v2 16}; "
	"loadbmp{sample,q/pal8os2v2-16.bmp}; loadpng{ref,ref_8bit_252c.png};"
	"compare{ }",

	"name{Load 8-bit indexed OS/2 v2 40}; "
	"loadbmp{sample,q/pal8os2v2-40sz.bmp}; loadpng{ref,ref_8bit_252c.png};"
	"compare{ }",

	"name{Load 8-bit indexed OS/2 v2-sz}; "
	"loadbmp{sample,q/pal8os2v2-sz.bmp}; loadpng{ref,ref_8bit_252c.png};"
	"compare{ }",

	"name{Load 8-bit RLE}; "
	"loadbmp{sample,g/pal8rle.bmp}; loadpng{ref,ref_8bit_252c_alpha.png};"
	"compare{ }",

	"name{Load 8-bit indexed topdown}; "
	"loadbmp{sample,g/pal8topdown.bmp}; loadpng{ref,ref_8bit_252c.png};"
	"compare{ }",

	"name{Load 8-bit indexed V4}; "
	"loadbmp{sample,g/pal8v4.bmp}; loadpng{ref,ref_8bit_252c.png};"
	"compare{ }",

	"name{Load 8-bit indexed V5}; "
	"loadbmp{sample,g/pal8v5.bmp}; loadpng{ref,ref_8bit_252c.png};"
	"compare{ }",

	"name{Load 8-bit indexed w124}; "
	"loadbmp{sample,g/pal8w124.bmp}; loadpng{ref,ref_8bit_252c_w124.png};"
	"compare{ }",

	"name{Load 8-bit indexed w125}; "
	"loadbmp{sample,g/pal8w125.bmp}; loadpng{ref,ref_8bit_252c_w125.png};"
	"compare{ }",

	"name{Load 8-bit indexed w126}; "
	"loadbmp{sample,g/pal8w126.bmp}; loadpng{ref,ref_8bit_252c_w126.png};"
	"compare{ }",

	"name{Load 16-bit RGB 565}; "
	"loadbmp{sample,g/rgb16-565.bmp}; loadpng{ref,ref_8bit_rgb16-565.png};"
	"compare{ }",

	"name{Load 16-bit RGB 565 + color table}; "
	"loadbmp{sample,g/rgb16-565pal.bmp}; loadpng{ref,ref_8bit_rgb16-565.png};"
	"compare{ }",

	"name{Load 16-bit RGB 565 BITFIELDS}; "
	"loadbmp{sample,g/rgb16bfdef.bmp}; loadpng{ref,ref_8bit_rgb16.png};"
	"compare{ }",

	"name{Load 16-bit RGB}; "
	"loadbmp{sample,g/rgb16.bmp}; loadpng{ref,ref_8bit_rgb16.png};"
	"compare{ }",

	"name{Load 24-bit RGB + Save}; "
	"loadbmp{sample,g/rgb24.bmp}; loadpng{ref,ref_8bit_255c.png};"
	"compare{ };"
	"savebmp{tmp,rgb24out.bmp};"
	"loadbmp{tmp,rgb24out.bmp};"
	"compare{ }",

	"name{Load 24-bit RGB + color table}; "
	"loadbmp{sample,g/rgb24pal.bmp}; loadpng{ref,ref_8bit_255c.png};"
	"compare{ }",

	"name{Load 32-bit BITFIELDS unusual order}; "
	"loadbmp{sample,g/rgb32bf.bmp}; loadpng{ref,ref_8bit_255c.png};"
	"compare{ }",

	"name{Load 32-bit BITFIELDS}; "
	"loadbmp{sample,g/rgb32bfdef.bmp}; loadpng{ref,ref_8bit_255c.png};"
	"compare{ }",

	"name{Load 32-bit RGB}; "
	"loadbmp{sample,g/rgb32.bmp}; loadpng{ref,ref_8bit_255c.png};"
	"compare{ }",

	"name{Load Huffman}; "
	"loadbmp{sample,q/pal1huffmsb.bmp}; loadpng{ref,ref_8bit_2bw.png};"
	"compare{ }",

	"name{Load Huffman as float}; "
	"loadbmp{sample,q/pal1huffmsb.bmp,format:float}; "
	"convertformat{int,8};"
	"loadpng{ref,ref_8bit_2bw.png};"
	"compare{ }",

	"name{Load RLE24}; "
	"loadbmp{sample,q/rgb24rle24.bmp}; savebmp{tmp,rle24.bmp}",

	"name{Load 64-bit}; "
	"loadbmp{sample,q/rgba64.bmp,format:float}; convertformat{int,8}; "
	"savebmp{tmp,rgba64_to_16bit.bmp}",
	//"loadpng{ref,ref_8bit_255c_alpha64.png};"
	//"compare{ }",

	"name{Save 32-bit RGB (load as float)}; "
	"loadbmp{sample,g/rgb32.bmp,format:float}; "
	"savebmp{tmp,rgb32.bmp}; "
	"delete{ }; "
	"loadbmp{tmp,rgb32.bmp}; loadpng{ref,ref_8bit_255c.png}; "
	"compare{ }",


	"name{Save 64-bit RGB}; "
	"loadbmp{sample,g/rgb32.bmp}; "
	"convertgamma{srgb,linear}"
	"savebmp{tmp,rgb64.bmp,64bit: }",

	"name{Save 32-bit RGB}; "
	"loadbmp{sample,g/rgb32.bmp,format:float};"
	"savebmp{tmp,rgb32_16.bmp,format:int,bufferbits:16}",

	"name{Load 64-bit RGB}; "
	"loadbmp{sample,q/rgba64.bmp};"
	"savebmp{tmp,rgb64-to-24.bmp,format:int,bufferbits:8}",

	"name{Load 64-bit RGB float linear + convert}; "
	"loadbmp{sample,q/rgba64.bmp,format:float,conv64:linear};"
	"convertgamma{linear,srgb};"
	"savebmp{tmp,rgb64fltlin-to-24.bmp,format:int,bufferbits:8}",

	"name{Load 64-bit RGB s2.13 linear + convert}; "
	"loadbmp{sample,q/rgba64.bmp,format:s2.13,conv64:linear};"
	"convertgamma{linear,srgb};"
	"savebmp{tmp,rgb64s2.13lin-to-24.bmp,format:int,bufferbits:8}",

	"name{Load 64-bit RGB s2.13}; "
	"loadbmp{sample,q/rgba64.bmp,format:s2.13,conv64:srgb};"
	"savebmp{tmp,rgb64s2.13lin-to-24.bmp,format:int,bufferbits:8}",

	"name{create HDR 64-bit};"
	"loadpng{ref,almdudler.png};"
	"convertformat{float,0};"
	"convertgamma{srgb,linear};"
	"exposure{fstops:2};"
	"savebmp{tmp,hdr-64bit.bmp,64bit: }",

	"name{create dark 16-bit};"
	"loadpng{ref,almdudler.png};"
	"convertformat{float,0};"
	"convertgamma{srgb,linear};"
	"exposure{fstops:-8};"
	"convertgamma{linear,srgb};"
	"convertformat{int,16};"
	"savebmp{tmp,dark16.bmp,outbits:r10g11b11a0}",


	"name{Save big Huffman}; "
	"loadbmp{ref,sw-big.bmp,rgb:index};"
	"savebmp{tmp,sw-huffout.bmp,rle:auto,allow:huff}",

	"name{Save big Huffman as RGB}; "
	"loadbmp{ref,sw-big.bmp,rgb:index};"
	"flatten{ };"
	"savebmp{tmp,sw-huffoutrgb.bmp}",

	"name{Save 8-bit RLE8}; "
	"loadbmp{sample,g/pal8.bmp,rgb:index};"
	"savebmp{tmp,rle8.bmp,rle:auto};"
	"loadbmp{tmp,rle8.bmp,rgb:index};"
	"compare{ }",

	"name{Save 4-bit RLE4}; "
	"loadbmp{sample,g/pal4.bmp,rgb:index};"
	"savebmp{tmp,rle4.bmp,rle:auto};"
	"loadbmp{tmp,rle4.bmp,rgb:index};"
	"compare{ }",

	"name{Save 8-bit RLE24}; "
	"loadbmp{sample,g/rgb24.bmp};"
	"savebmp{tmp,rle24.bmp,rle:auto,allow:rle24};"
	"loadbmp{tmp,rle24.bmp,undef:leave};"
	"compare{ }",

	"name{Save bigger 8-bit RLE24}; "
	"loadbmp{ref,90s.bmp};"
	"savebmp{tmp,90s_out.bmp,rle:auto,allow:rle24};"
	"loadbmp{tmp,90s_out.bmp,undef:leave};"
	"compare{ }",

//	"name{Save small Huffman}; "
//	"loadbmp{sample,q/pal1huffmsb.bmp,rgb:index};"
//	"savebmp{tmp,pal1huffout.bmp,rle:auto,allow:huff}",
//
//	"name{re-Save bigl Huffman}; "
//	"loadbmp{tmp,sw-huffout.bmp};"
//	"savebmp{tmp,sw-outout.bmp}",


//	"name{Save 16-bit PNG as 64bit BMP with exposureed contrast}; "
//	"loadpng{ref,photo16.png}; "
//	"convertformat{float,0};"
//	"convertgamma{srgb,linear};"
//	"exposure{fstops:2};"
//	"savebmp{tmp,photo-64bit.bmp,64bit: }",

//	"name{Load huge BMP}; "
//	"loadbmp{ref,my_test_image_empty.bmp,insane:yes}",

};


const unsigned char checkmark[] = {0x20, 0xE2, 0x9C, 0x93, 0};

static bool perform(const char *action, struct Cmdarg *args);
static bool perform_loadbmp(struct Cmdarg *args);
static bool perform_loadpng(struct Cmdarg *args);
static bool perform_savebmp(struct Cmdarg *args);
static bool perform_compare(void);
static bool perform_delete(void);
static bool perform_convertgamma(struct Cmdarg *args);
static bool perform_flatten(struct Cmdarg *args);
static bool perform_exposure(struct Cmdarg *args);
static bool perform_convertformat(struct Cmdarg *args);
static void convert_format(BMPFORMAT format, int bits);
static void set_exposure(double fstops, int symmetric);
static struct Image* pngfile_read(FILE *file);




int main(int argc, char *argv[])
{
	int          numtest;
	int          bad = 0, good = 0;

	numtest = sizeof testdef / sizeof testdef[0];

	for (int i = 0; i < numtest; i++) {
		bool        failed = false;
		bool        first  = true;
		const char *cmdstr = testdef[i];

		imgstack_clear();
		printf("\n===== Test %02d: ", i);

		do {
			char cmdname[160];
			if (next_command(&cmdstr, cmdname, sizeof cmdname)) {
				struct Cmdarg *args;
				char           argbuf[1024];

				if (arglist_from_cmdstr(&cmdstr, argbuf, sizeof argbuf, &args)) {
					if (!strcmp(cmdname, "name")) {
						if (first)
							printf("%s\n", args ? args->arg : "(none)");
					} else {
						if (!perform(cmdname, args))
							failed = true;
					}
				} else {
					printf("**Error splitting args\n");
					failed = true;
					break;
				}
				first = false;
			} else {
				/* no more commands, possibly some filler */
				break;
			}
		} while (*cmdstr);
		if (failed) {
			bad++;
			printf("****failed\n");
		}
		else {
			good++;
			printf("----passed\n");
		}
	}

	printf("\nBad : %d\nGood: %d\n %s\n", bad, good, bad ? " ***!!!***" : (char*)checkmark);
	imgstack_destroy();
	return bad;
}


static bool perform(const char *action, struct Cmdarg *args)
{
	if (!strcmp("loadbmp", action))
		return perform_loadbmp(args);
	else if (!strcmp("loadpng", action))
		return perform_loadpng(args);
	else if (!strcmp("savebmp", action))
		return perform_savebmp(args);
	else if (!strcmp("compare", action))
		return perform_compare();
	else if (!strcmp("delete", action))
		return perform_delete();
	else if (!strcmp("convertgamma", action))
		return perform_convertgamma(args);
	else if (!strcmp("convertformat", action))
		return perform_convertformat(args);
	else if (!strcmp("flatten", action))
		return perform_flatten(args);
	else if (!strcmp("exposure", action))
		return perform_exposure(args);
	else {
		printf("Unkown action: %s\n", action);
	}
	return false;
}






static bool perform_loadbmp(struct Cmdarg *args)
{
	const char   *dir = NULL, *fname = NULL;
	const char   *dirpath;
	char         *optname, *optvalue;
	char          path[1024];
	FILE         *file = NULL;
	struct Image *img  = NULL;
	BMPHANDLE     h    = NULL;
	BMPRESULT     res;

	bool          set_undef = false;
	BMPUNDEFINED  undefmode;

	bool          set_format = false;
	BMPFORMAT     format     = BMP_FORMAT_INT;

	bool          set_conv64 = false;
	BMPCONV64     conv64;

	bool          insane       = false;
	bool          line_by_line = false;
	bool          index        = false;

	if (args) {
		dir  = args->arg;
		args = args->next;
	}
	if (args) {
		fname = args->arg;
		args  = args->next;
	}

	if (!(fname && *fname)) {
		printf("loadbmp: invalid filespec\n");
		goto abort;
	}

	if (!strcmp(dir, "sample"))
		dirpath = sampledir;
	else if (!strcmp(dir, "tmp"))
		dirpath = tmpdir;
	else if (!strcmp(dir, "ref"))
		dirpath = refdir;
	else {
		printf("loadbmp: Invalid dir '%s'\n", dir);
		goto abort;
	}
	if (sizeof path < snprintf(path, sizeof path, "%s%s", dirpath, fname)) {
		printf("path too small!");
		exit(1);
	}

	while (args && args->arg) {
		optname = args->arg;
		optvalue = strchr(optname, ':');
		if (!optvalue) {
			printf("loadbmp: invalid option '%s'\n", optname);
			goto abort;
		}
		*optvalue++ = 0;

		if (!strcmp(optname, "line")) {
			if (!strcmp(optvalue, "whole"))
				line_by_line = false;
			else if (!strcmp(optvalue, "line"))
				line_by_line = true;
			else {
				printf("loadbmp: invalid line mode %s\n", optvalue);
				goto abort;
			}
		} else if (!strcmp(optname, "rgb")) {
			if (!strcmp(optvalue, "rgb"))
				index = false;
			else if (!strcmp(optvalue, "index"))
				index = true;
			else {
				printf("loadbmp: invalid rgb mode %s\n", optvalue);
				goto abort;
			}
		} else if (!strcmp(optname, "undef")) {
			set_undef = true;
			if (!strcmp(optvalue, "alpha"))
				undefmode = BMP_UNDEFINED_TO_ALPHA;
			else if (!strcmp(optvalue, "leave"))
				undefmode = BMP_UNDEFINED_LEAVE;
			else {
				printf("loadbmp: invalid undef mode %s\n", optvalue);
				goto abort;
			}
		} else if (!strcmp(optname, "conv64")) {
			set_conv64 = true;
			if (!strcmp(optvalue, "srgb"))
				conv64 = BMP_CONV64_SRGB;
			else if (!strcmp(optvalue, "linear"))
				conv64 = BMP_CONV64_LINEAR;
			else {
				printf("loadbmp: invalid conv64 mode %s\n", optvalue);
				goto abort;
			}
		} else if (!strcmp(optname, "format")) {
			set_format = true;
			if (!strcmp(optvalue, "int"))
				format = BMP_FORMAT_INT;
			else if (!strcmp(optvalue, "float"))
				format = BMP_FORMAT_FLOAT;
			else if (!strcmp(optvalue, "s2.13"))
				format = BMP_FORMAT_S2_13;
			else {
				printf("loadbmp: invalid number format %s\n", optvalue);
				goto abort;
			}
		} else if (!strcmp(optname, "insane")) {
			if (!strcmp(optvalue, "yes"))
				insane = true;
			else {
				printf("loadbmp: invalid insanity value. must be 'yes'");
				goto abort;
			}
		}
		else {
			printf("loadbmp: unknown option %s\n", optname);
			goto abort;
		}
		args = args->next;
	}

	if (!(file = fopen(path, "rb"))) {
		perror(path);
		goto abort;
	}

	if (!(h = bmpread_new(file))) {
		printf("Couldn't get bmpread handle\n");
		goto abort;
	}

	if ((res = bmpread_load_info(h))) {
		if (res == BMP_RESULT_INSANE && insane)
			bmpread_set_insanity_limit(h, bmpread_buffersize(h));
		else {
			printf("%s\n", bmp_errmsg(h));
			goto abort;
		}
	}

	if (set_undef)
		bmpread_set_undefined(h, undefmode);

	if (!(img = malloc(sizeof *img))) {
		perror("malloc");
		goto abort;
	}
	memset(img, 0, sizeof *img);

	if (set_conv64) {
		if (bmpread_set_64bit_conv(h, conv64)) {
			printf("%s\n", bmp_errmsg(h));
			goto abort;
		}
	}
	if (set_format) {
		if (bmp_set_number_format(h, format)) {
			printf("%s\n", bmp_errmsg(h));
			goto abort;
		}
	}
	img->format = format;

	if (index) {
		fflush(stdout);
		img->numcolors = bmpread_num_palette_colors(h);
		if (img->numcolors > 0) {
			if (bmpread_load_palette(h, &img->palette)) {
				printf("%s\n", bmp_errmsg(h));
				goto abort;
			}
		}
	}

	img->width    = bmpread_width(h);
	img->height   = bmpread_height(h);
	img->channels = bmpread_channels(h);
	img->bitsperchannel = bmpread_bitsperchannel(h);
	img->buffersize     = bmpread_buffersize(h);

	if (line_by_line) {
		if (!(img->buffer = malloc(img->buffersize))) {
			perror("buffer");
			goto abort;
		}
		unsigned char *line;
		for (int y = 0; y < img->height; y++) {
			line = img->buffer + (uint64_t) y * img->width * img->channels * img->bitsperchannel / 8;
			if (bmpread_load_line(h, &line)) {
				printf("%s\n", bmp_errmsg(h));
				goto abort;
			}
		}

	} else {
		if (bmpread_load_image(h, &img->buffer)) {
			printf("%s\n", bmp_errmsg(h));
			goto abort;
		}
	}

	printf("Image %s loaded\n", path);
	bmp_free(h); h = NULL;
	fclose(file); file = NULL;

	if (!imgstack_push(img))
		goto abort;

	return true;

abort:
	if (file)
		fclose(file);
	if (h)
		bmp_free(h);
	if (img)
		free(img);

	return false;
}


static bool perform_savebmp(struct Cmdarg *args)
{
	const char   *dir = NULL, *fname = NULL;
	const char   *dirpath;
	char         *optname, *optvalue;
	char          path[1024];
	FILE         *file = NULL;
	struct Image *img  = NULL;
	BMPHANDLE     h    = NULL;

	bool          set_format = false;
	BMPFORMAT     format;

	bool          set_rle = false;
	BMPRLETYPE    rle;

	int           bufferbits  = 0;
	bool          set_outbits = false;
	int           outbits[4]  = {0,0,0,0};
	bool          set_64bit   = false;
	bool          allow_huff  = false;
	bool          allow_2bit  = false;
	bool          allow_rle24 = false;

	if (args) {
		dir  = args->arg;
		args = args->next;
	}
	if (args) {
		fname = args->arg;
		args  = args->next;
	}

	if (!(fname && *fname)) {
		printf("savebmp: invalid filespec\n");
		goto abort;
	}

	if (!strcmp(dir, "sample"))
		dirpath = sampledir;
	else if (!strcmp(dir, "tmp"))
		dirpath = tmpdir;
	else if (!strcmp(dir, "ref"))
		dirpath = refdir;
	else {
		printf("loadbmp: Invalid dir '%s'", dir);
		goto abort;
	}
	if (sizeof path < snprintf(path, sizeof path, "%s%s", dirpath, fname)) {
		printf("path too small!");
		exit(1);
	}

	while (args && args->arg) {
		optname = args->arg;
		optvalue = strchr(optname, ':');
		if (!optvalue) {
			printf("savebmp: invalid option '%s'\n", optname);
			goto abort;
		}
		*optvalue++ = 0;

		if (!strcmp(optname, "bufferbits")) {
			bufferbits = atol(optvalue);
			switch (bufferbits) {
			case 8:
			case 16:
			case 32:
				/* ok */
				break;
			default:
				printf("savebmp: invalid bufferbits (%d)\n", bufferbits);
				goto abort;
			}
		}
		else if (!strcmp(optname, "format")) {
			set_format = true;
			if (!strcmp(optvalue, "int"))
				format = BMP_FORMAT_INT;
			else if (!strcmp(optvalue, "float"))
				format = BMP_FORMAT_FLOAT;
			else if (!strcmp(optvalue, "s2.13"))
				format = BMP_FORMAT_S2_13;
			else {
				printf("savebmp: invalid number format %s\n", optvalue);
				goto abort;
			}
		}
		else if (!strcmp(optname, "rle")) {
			set_rle = true;
			if (!strcmp(optvalue, "auto"))
				rle = BMP_RLE_AUTO;
			else if (!strcmp(optvalue, "rle8"))
				rle = BMP_RLE_RLE8;
			else if (!strcmp(optvalue, "none"))
				rle = BMP_RLE_NONE;
			else {
				printf("savebmp: invalid rle option %s\n", optvalue);
				goto abort;
			}
		}
		else if (!strcmp(optname, "allow")) {
			if (!strcmp(optvalue, "huff"))
				allow_huff = true;
			else if (!strcmp(optvalue, "2bit"))
				allow_2bit = true;
			else if (!strcmp(optvalue, "rle24"))
				allow_rle24 = true;
			else {
				printf("savebmp: invalid allow option %s\n", optvalue);
				goto abort;
			}
		}
		else if (!strcmp(optname, "outbits")) {
			set_outbits = true;
			int col;
			char *str;
			while (*optvalue) {
				switch (*optvalue) {
				case 'r': col = 0; break;
				case 'g': col = 1; break;
				case 'b': col = 2; break;
				case 'a': col = 3; break;
				default:
					printf("savebmp: invalid outbits %s\n", optvalue);
					goto abort;
				}
				outbits[col] = strtol(++optvalue, &str, 10);
				if (str <= optvalue) {
					printf("hmmmmmmm....\n");
					break;
				}
				optvalue = str;
			}
		}
		else if (!strcmp(optname, "64bit")) {
			set_64bit = true;
		}
		else {
			printf("savebmp: unknown option %s\n", optname);
			goto abort;
		}
		args = args->next;
	}

	if (!(img = imgstack_get(0))) {
		goto abort;
	}

	if (!(file = fopen(path, "wb"))) {
		perror(path);
		goto abort;
	}

	if (!(h = bmpwrite_new(file))) {
		printf("Couldn't get bmpwrite handle\n");
		goto abort;
	}

	if (set_64bit) {
		if (bmpwrite_set_64bit(h)) {
			printf("setting 64bit: %s\n", bmp_errmsg(h));
			goto abort;
		}
	}

	if (set_outbits) {
		if (bmpwrite_set_output_bits(h, outbits[0], outbits[1], outbits[2], outbits[3])) {
			printf("setting 64bit: %s\n", bmp_errmsg(h));
			goto abort;
		}
	}

	if (allow_2bit) {
		bmpwrite_allow_2bit(h);
	}

	if (allow_huff) {
		bmpwrite_allow_huffman(h);
	}

	if (allow_rle24) {
		bmpwrite_allow_rle24(h);
	}

	if (img->palette) {
		if (bmpwrite_set_palette(h, img->numcolors, img->palette)) {
			printf("setting palette: %s\n", bmp_errmsg(h));
			goto abort;
		}
	}
	if (set_rle) {
		if (bmpwrite_set_rle(h, rle)) {
			printf("setting rle: %s\n", bmp_errmsg(h));
			goto abort;
		}
	}
	if (set_format && format != img->format) {
		if (format == BMP_FORMAT_INT && !bufferbits) {
			printf("cannot set output INT w/o specifying bits\n");
			exit(1);
		}
		convert_format(format, bufferbits);
	}

	if (img->format)
		bmp_set_number_format(h, img->format);

	if (bmpwrite_set_dimensions(h, img->width, img->height, img->channels, img->bitsperchannel)) {
		printf("set dimensions: %s\n", bmp_errmsg(h));
		goto abort;
	}

	if (bmpwrite_save_image(h, img->buffer)) {
		printf("%s\n", bmp_errmsg(h));
		goto abort;
	}

	bmp_free(h);
	fclose(file);

	return true;

abort:
	if (file)
		fclose(file);
	if (h)
		bmp_free(h);

	return false;
}


static bool perform_flatten(struct Cmdarg *args)
{
	struct Image  *img;
	size_t         new_size, offnew, offold;
	unsigned char *tmp;

	img = imgstack_get(0);

	if (!(img->palette && img->channels == 1 && img->bitsperchannel == 8)) {
		printf("Cannot flatten RGB image\n");
		return false;
	}

	img->channels = 3;
	new_size = img->width * img->height * img->channels;
	if (!(tmp = realloc(img->buffer, new_size))) {
		perror("flatten");
		return false;
	}
	img->buffer = tmp;
	img->buffersize = new_size;

	offnew = new_size;
	offold = img->width * img->height;
	while (offold > 0) {
		offnew -= 3;
		offold -= 1;
		for (int c = 2; c >= 0; c--) {
			img->buffer[offnew + c] = img->palette[4 * img->buffer[offold] + c];
		}
	}

	free (img->palette);
	img->palette = NULL;
	img->numcolors = 0;
	return true;
}

static bool perform_exposure(struct Cmdarg *args)
{
	char   *opt, *optval;
	double  fstops = 0.0;
	bool    symmetric = false;


	while (args && args->arg) {
		opt = args->arg;
		optval = strchr(opt, ':');
		if (optval)
			*optval++ = 0;

		if (!strcmp(opt, "fstops")) {
			fstops = atof(optval);
		}
		else if (!strcmp(opt,"symmetric")) {
			symmetric = true;
		}
		args = args->next;
	}
	if (fstops == 0.0)
		return false;
	set_exposure(fstops, symmetric);
	return true;
}



static void convert_srgb_to_linear(void);
static void convert_linear_to_srgb(void);


static bool perform_convertgamma(struct Cmdarg *args)
{
	const char   *from, *to;

	if (args) {
		from = args->arg;
		args = args->next;
	}
	if (args) {
		to   = args->arg;
		args = args->next;
	}

	if (!(to && *to)) {
		printf("convertgamma: need from,to\n");
		return false;
	}

	if (!strcmp(from, "srgb")) {
		if (!strcmp(to, "linear"))
			convert_srgb_to_linear();
		else {
			printf("Unknown conversion to %s\n", to);
			return false;
		}
	}
	else if (!strcmp(from, "linear")) {
		if (!strcmp(to, "srgb"))
			convert_linear_to_srgb();
		else {
			printf("Unknown conversion to %s\n", to);
			return false;
		}
	}
	else {
		printf("Unknown conversion from %s\n", from);
		return false;
	}
	return true;
}

static double srgb_to_linear(double d);
static double linear_to_srgb(double d);
static void convert_gamma(double (*func)(double));

static void convert_srgb_to_linear(void)
{
	convert_gamma(srgb_to_linear);
}

static void convert_linear_to_srgb(void)
{
	convert_gamma(linear_to_srgb);
}


static double srgb_to_linear(double d)
{
	if (d <= 0.04045)
		d = d / 12.92;
	else
		d = pow((d+0.055)/1.055, 2.4);
	return d;
}


static double linear_to_srgb(double d)
{
	if (d <= 0.0031308)
		d *= 12.92;
	else
		d = 1.055 * pow(d, 1.0/2.4) - 0.055;
	return d;
}

static void convert_gamma(double (*func)(double))
{
	struct Image *img;
	int channels, colorchannels;
	size_t        i, px, npixels;
	img = imgstack_get(0);


	npixels = img->width * img->height;
	channels = img->channels;
	if (channels == 2)
		colorchannels = 1;
	else if (channels == 4)
		colorchannels = 3;
	else
		colorchannels = channels;

	for (px = 0; px < npixels; px++) {
		size_t offs = px * channels;
		for (i = 0; i < colorchannels; i++) {
			double   d;
			uint16_t u16;

			switch(img->format) {
			case BMP_FORMAT_FLOAT:
				d = ((float*)img->buffer)[offs + i];
				break;
			case BMP_FORMAT_S2_13:
				d = ((((int32_t)((uint16_t*)img->buffer)[offs+i])<<16)>>16) / 8192.0;
				break;
			case BMP_FORMAT_INT:
				switch (img->bitsperchannel) {
				case 8:
					d = ((uint8_t*)img->buffer)[offs+i] / (double) 0xffU;
					break;
				case 16:
					d = ((uint16_t*)img->buffer)[offs+i] / (double) 0xffffU;
					break;
				case 32:
					d = ((uint32_t*)img->buffer)[offs+i] / (double) 0xffffffffUL;
					break;
				default:
					printf("Waaaaaaaaaaaaaaa\n");
					exit(1);
				}
				break;
			}

			d = (*func)(d);

			switch (img->format) {
			case BMP_FORMAT_FLOAT:
				((float*)img->buffer)[offs+i] = (float) d;
				break;
			case BMP_FORMAT_S2_13:
				if (d < -4.0)
					u16 = 0x8000;
				else if (d >= 4.0)
					u16 = 0x7fff;
				else
					u16 = d * 8192.0 + 0.5;
				((uint16_t*)img->buffer)[offs+i] = u16;
				break;
			case BMP_FORMAT_INT:
				switch (img->bitsperchannel) {
				case 8:
					((uint8_t*)img->buffer)[offs+i]  = d * (double) 0xffU + 0.5;
					break;
				case 16:
					((uint16_t*)img->buffer)[offs+i] = d * (double) 0xffffU + 0.5;
					break;
				case 32:
					((uint32_t*)img->buffer)[offs+i] = d * (double) 0xffffffffUL + 0.5;
					break;
				}
				break;
			}
		}
	}
}




static void set_exposure(double fstops, int symmetric)
{
	struct Image *img;
	int channels, colorchannels;
	size_t        i, px, npixels;
	img = imgstack_get(0);


	npixels = img->width * img->height;
	channels = img->channels;
	if (channels == 2)
		colorchannels = 1;
	else if (channels == 4)
		colorchannels = 3;
	else
		colorchannels = channels;

	for (px = 0; px < npixels; px++) {
		size_t offs = px * channels;
		for (i = 0; i < colorchannels; i++) {
			double   d;
			uint16_t u16;

			switch(img->format) {
			case BMP_FORMAT_FLOAT:
				d = ((float*)img->buffer)[offs + i];
				break;
			case BMP_FORMAT_S2_13:
				d = ((((int32_t)((uint16_t*)img->buffer)[offs+i])<<16)>>16) / 8192.0;
				break;
			case BMP_FORMAT_INT:
				printf("Cannot exposure contrast on int images!\n");
				exit(1);
			}

			d = d * pow(2, fstops);
			if (symmetric)
				d -= pow(2, fstops-1);

			switch (img->format) {
			case BMP_FORMAT_FLOAT:
				((float*)img->buffer)[offs+i] = (float) d;
				break;
			case BMP_FORMAT_S2_13:
				if (d < -4.0)
					u16 = 0x8000;
				else if (d >= 4.0)
					u16 = 0x7fff;
				else
					u16 = d * 8192.0 + 0.5;
				((uint16_t*)img->buffer)[offs+i] = u16;
				break;
			default:
				break;
			}
		}
	}
}


static bool perform_convertformat(struct Cmdarg *args)
{
	const char *to = NULL;
	int         bits = 0;

	if (args) {
		to   = args->arg;
		args = args->next;
	}
	if (args) {
		bits = atol(args->arg);
		args = args->next;
	}

	if (!(to && *to)) {
		printf("convertformat: need format\n");
		return false;
	}

	if (!strcmp(to, "float")) {
		convert_format(BMP_FORMAT_FLOAT, 0);
	}
	else if (!strcmp(to, "s2.13")) {
		convert_format(BMP_FORMAT_S2_13, 0);
	}
	else if (!strcmp(to, "int")) {
		convert_format(BMP_FORMAT_INT, bits);
	}
	else {
		printf("Unknown conversion to %s\n", to);
		return false;
	}
	return true;
}


static void convert_format(BMPFORMAT format, int bits)
{
	struct Image *img;
	size_t        newsize, nvals;
	uint8_t      *tmp;

	if (format == BMP_FORMAT_FLOAT)
		bits = 32;
	else if (format == BMP_FORMAT_S2_13)
		bits = 16;
	else if (!(bits == 8 || bits == 16 || bits == 32)) {
		printf("convert: invalid bit-number: %d\n", bits);
		exit(1);
	}

	img = imgstack_get(0);
	if (img->format == format && img->bitsperchannel == bits)
		return;

	nvals = (size_t) img->width * (size_t) img->height * (size_t) img->channels;
	newsize = nvals * (size_t) bits / 8;

	if (newsize > img->buffersize) {
		if (!(tmp = realloc(img->buffer, newsize))) {
			perror("realloc buffer for conv");
			exit(1);
		}
		img->buffer = tmp;
	}

	for (int i = 0; i < nvals; i++) {
		double   d;
		uint16_t u16;
		int      offs = img->bitsperchannel >= bits ? i : (nvals - i - 1);

		switch (img->format) {
		case BMP_FORMAT_FLOAT:
			d = ((float*)img->buffer)[offs];
			break;
		case BMP_FORMAT_S2_13:
			d = ((((int32_t)((uint16_t*)img->buffer)[offs])<<16)>>16) / 8192.0;
			break;
		case BMP_FORMAT_INT:
			switch (img->bitsperchannel) {
			case 8:
				d = ((uint8_t*)img->buffer)[offs] / (double) 0xffU;
				break;
			case 16:
				d = ((uint16_t*)img->buffer)[offs] / (double) 0xffffU;
				break;
			case 32:
				d = ((uint32_t*)img->buffer)[offs] / (double) 0xffffffffUL;
				break;
			default:
				printf("Waaaaaaaaaaaaaaa\n");
				exit(1);
			}
		}

		switch (format) {
		case BMP_FORMAT_FLOAT:
			((float*)img->buffer)[offs] = (float) d;
			break;
		case BMP_FORMAT_S2_13:
			if (d < -4.0)
				u16 = 0x8000;
			else if (d >= 4.0)
				u16 = 0x7fff;
			else
				u16 = d * 8192.0 + 0.5;
			((uint16_t*)img->buffer)[offs] = u16;
			break;
		case BMP_FORMAT_INT:
			switch (bits) {
			case 8:
				((uint8_t*)img->buffer)[offs]  = d * (double) 0xffU + 0.5;
				break;
			case 16:
				((uint16_t*)img->buffer)[offs] = d * (double) 0xffffU + 0.5;
				break;
			case 32:
				((uint32_t*)img->buffer)[offs] = d * (double) 0xffffffffUL + 0.5;
				break;
			}
		}
	}

	if (newsize < img->buffersize) {
		tmp = realloc(img->buffer, newsize);
		if (tmp) {
			img->buffer = tmp;
			img->buffersize = newsize;
		}
	}
	img->bitsperchannel = bits;
	img->format = format;
}



static bool perform_delete(void)
{
	imgstack_delete();
	return true;
}


static bool perform_compare(void)
{
	int           i;
	size_t        off, size;
	struct Image *img[2];
	uint8_t      *p8l, *p8r;
	uint16_t     *p16l, *p16r;
	uint32_t     *p32l, *p32r;
	int           bytes_per_pixel;

	for (i = 0; i < 2; i++) {
		img[i] = imgstack_get(i);
		if (!img[i])
			return false;
	}

	if(!(img[0]->width          == img[1]->width &&
	     img[0]->height         == img[1]->height &&
	     img[0]->channels       == img[1]->channels &&
	     img[0]->bitsperchannel == img[1]->bitsperchannel)) {
		printf("compare: dimensions don't match: %dx%dx%d@%d vs %dx%dx%d@%d\n",
		       img[0]->width, img[0]->height, img[0]->channels, img[0]->bitsperchannel,
		       img[1]->width, img[1]->height, img[1]->channels, img[1]->bitsperchannel);
		return false;
	}

	bytes_per_pixel = img[0]->bitsperchannel * img[0]->channels / 8;

	size = (size_t)img[0]->width * (size_t)img[0]->height * (size_t)img[0]->channels;

	switch (img[0]->bitsperchannel) {
	case 8:
		p8l = (uint8_t*) img[0]->buffer;
		p8r = (uint8_t*) img[1]->buffer;
		for (off = 0; off < size; off++) {
			if (p8l[off] != p8r[off]) {
				printf("compare: pixels don't match (%d vs %d @ %d,%d)\n",
				        (int) p8l[off], (int) p8r[off],
				        (int) ((off / bytes_per_pixel) % img[0]->width),
				        (int) ((off / bytes_per_pixel) / img[0]->width));
				return false;
			}
		}
		break;

	case 16:
		p16l = (uint16_t*) img[0]->buffer;
		p16r = (uint16_t*) img[1]->buffer;
		for (off = 0; off < size; off++) {
			if (p16l[off] != p16r[off]) {
				printf("compare: pixels don't match\n");
				return false;
			}
		}
		break;

	case 32:
		p32l = (uint32_t*) img[0]->buffer;
		p32r = (uint32_t*) img[1]->buffer;
		for (off = 0; off < size; off++) {
			if (p32l[off] != p32r[off]) {
				printf("compare: pixels don't match\n");
				return false;
			}
		}
		break;
	default:
		printf("Invalid bitsperchannel (%d) for comparison",img[0]->bitsperchannel);
		return false;
	}
	return true;
}






static bool perform_loadpng(struct Cmdarg *args)
{
	const char   *dir = NULL, *fname = NULL;
	const char   *dirpath;
	char          path[1024];
	FILE         *file = NULL;
	struct Image *img = NULL;

	if (args) {
		dir  = args->arg;
		args = args->next;
	}
	if (args) {
		fname = args->arg;
		args  = args->next;
	}

	if (!(fname && *fname)) {
		printf("loadpng: invalid filespec\n");
		goto abort;
	}

	if (!strcmp(dir, "sample"))
		dirpath = sampledir;
	else if (!strcmp(dir, "tmp"))
		dirpath = tmpdir;
	else if (!strcmp(dir, "ref"))
		dirpath = refdir;
	else {
		printf("loadpng: Invalid dir '%s'", dir);
		goto abort;
	}
	if (sizeof path < snprintf(path, sizeof path, "%s%s", dirpath, fname)) {
		printf("path too small!");
		exit(1);
	}

	if (!(file = fopen(path, "rb"))) {
		perror(path);
		goto abort;
	}

	if (!(img = pngfile_read(file)))
		goto abort;

	fclose(file);

	img->format = BMP_FORMAT_INT;

	if (!imgstack_push(img))
		goto abort;

	return true;

abort:
	if (file)
		fclose(file);
	if (img)
		free(img);

	return false;
}


static struct Image* pngfile_read(FILE *file)
{
	struct Image  *img = NULL;
	png_structp    png_ptr = NULL;
	png_infop      info_ptr = NULL;
	png_bytep     *row_pointers = NULL;
	png_uint_32    width, height;
	int            bit_depth, color_type, interlace_method, compression_method, filter_method;
	int            y;

	if (!(png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
		printf("Couldn't create PNG read struct\n");
		goto abort;
	}

	if (!(info_ptr = png_create_info_struct(png_ptr))) {
		printf("Couldn't create PNG info struct\n");
		goto abort;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("PNG reading failed\n");
		goto abort;
	}

	png_init_io(png_ptr, file);

	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_method,
	             &compression_method, &filter_method);


	if (!(img = malloc(sizeof *img))) {
		perror("allocate png image");
		goto abort;
	}
	memset(img, 0, sizeof *img);

	if (bit_depth < 8) {
		if (color_type == PNG_COLOR_TYPE_PALETTE ||
		    color_type == PNG_COLOR_TYPE_GRAY ||
		    png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
			png_set_expand(png_ptr);
		else
			png_set_packing(png_ptr);
	}

	switch (color_type) {
	case PNG_COLOR_TYPE_PALETTE:
		png_set_palette_to_rgb(png_ptr); /* == png_set_expand */
		img->channels = 3;
		break;

	case PNG_COLOR_TYPE_GRAY:
		img->channels = 1;
		break;

	case PNG_COLOR_TYPE_GRAY_ALPHA:
		img->channels = 2;
		break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
		img->channels = 4;
		break;

	case PNG_COLOR_TYPE_RGB:
		img->channels = 3;
		break;

	default:
		printf("Invalid PNG color type!\n");
		goto abort;
	}

	png_set_interlace_handling(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_method,
	             &compression_method, &filter_method);

	if (!(bit_depth == 8 || bit_depth == 16)) {
		printf("Invalid bit depth: %d\n", bit_depth);
		goto abort;
	}

	if (width > INT_MAX || height > INT_MAX) {
		printf("Invalid PNG dimensions %lux%lu\n", (unsigned long) width,
		                                           (unsigned long) height);
		goto abort;
	}
	img->bitsperchannel = bit_depth;
	img->width          = (int) width;
	img->height         = (int) height;

	if (!(row_pointers = malloc(height * sizeof *row_pointers))) {
		perror("Allocating memory for PNG row pointers");
		goto abort;
	}
	memset(row_pointers, 0, height * sizeof *row_pointers);

	img->buffersize = width * height * img->channels * (bit_depth / 8);
	if (!(img->buffer = malloc(img->buffersize))) {
		perror("allocate PNG buffer");
		goto abort;
	}
	memset(img->buffer, 0, img->buffersize);

	for (y = 0; y < height; y++) {
		row_pointers[y] = img->buffer + y * width * img->channels * (bit_depth / 8);
	}

	png_read_image(png_ptr, row_pointers);

	png_read_end(png_ptr, NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	free(row_pointers);

	if (bit_depth == 16) {
		int      lobyte, hibyte;
		uint16_t val;
		size_t   pixels = width * height * img->channels;

		for (size_t off = 0; off/2 < pixels; off += 2) {
			hibyte = img->buffer[off];
			lobyte = img->buffer[off+1];
			val = (hibyte << 8) + lobyte;
			*((uint16_t*)(img->buffer+off)) = val;
		}
	}


	return img;

abort:
	if (img) {
		if (img->buffer)
			free(img->buffer);
		free (img);
	}

	if (png_ptr)
		png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : NULL, NULL);

	if (row_pointers)
		free(row_pointers);

	return NULL;
}
