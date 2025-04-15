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

#include "defs.h"
#include "imgstack.h"
#include "cmdparser.h"
#include "read-testdefs.h"
#include "conf.h"


const unsigned char checkmark[] = {0x20, 0xE2, 0x9C, 0x93, 0};

static bool perform(const char *action, struct Cmdarg *args);
static bool perform_loadbmp(struct Cmdarg *args);
static bool perform_loadpng(struct Cmdarg *args);
static bool perform_savebmp(struct Cmdarg *args);
static bool perform_swap(void);
static bool perform_duplicate(void);
static bool perform_compare(struct Cmdarg *args);
static bool perform_delete(void);
static bool perform_convertgamma(struct Cmdarg *args);
static bool perform_flatten(struct Cmdarg *args);
static bool perform_exposure(struct Cmdarg *args);
static bool perform_convertformat(struct Cmdarg *args);
static bool perform_invertpalette(struct Cmdarg *args);
static void convert_format(BMPFORMAT format, int bits);
static void set_exposure(double fstops);
static struct Image* pngfile_read(FILE *file);
static void trim_trailing_slash(char *str);
static bool perform_addalpha(void);
static inline uint16_t float_to_s2_13(double d);
static inline double s2_13_to_double(uint16_t s2_13);
bool bmpresult_from_str(const char *str, BMPRESULT *res);



static struct Conf *conf;


int main(int argc, char *argv[])
{
	int  testnum = 0;
	int  bad = 0, good = 0;
	bool only_selected_tests;
	struct Test *testlist;
	FILE        *file;

	if (!(conf = conf_parse_cmdline(argc, argv))) {
		printf("try -? or --help\n");
		return 1;
	}

	if (conf->help) {
		conf_usage();
		return 0;
	}

	only_selected_tests = conf->strlist != NULL;

	trim_trailing_slash(conf->sampledir);
	trim_trailing_slash(conf->refdir);
	trim_trailing_slash(conf->tmpdir);

	if (conf->verbose > 0) {
		printf("test definitions: %s\n", conf->testfile);
		printf("samples: %s\n", conf->sampledir);
		printf("ref    : %s\n", conf->refdir);
		printf("tmp    : %s\n", conf->tmpdir);
	}

	if (!conf->testfile) {
		printf("No testfile specified\n");
		printf("try -? or --help\n");
		return 1;
	}
	if (!(file = fopen(conf->testfile, "r"))) {
		perror(conf->testfile);
		return 1;
	}

	testlist = read_testdefs(file);
	fclose(file);

	for (struct Test *test = testlist ; test; test = test->next) {
		bool failed = false;
		bool first  = true;
		const char      *cmdstr = test->str;
		struct Confstr **str;

		testnum++;

		/* if test numbers have been given on command line, skip all other tests */
		if (only_selected_tests) {
			bool  found = false;
			char *endptr;
			for (str = &conf->strlist; *str; str = &(*str)->next) {
				if (*(*str)->str && testnum == strtol((*str)->str, &endptr, 10)) {
					if (endptr && *endptr != '\0')
						continue;
					*str = (*str)->next;
					found = true;
					break;
				}
			}
			if (!found)
				continue;
		}

		imgstack_clear();

		if (conf->verbose > 0) {
			printf("\n===== Test %02d: ", testnum);
		}


		do {
			char cmdname[160];
			if (next_command(&cmdstr, cmdname, sizeof cmdname)) {
				struct Cmdarg *args;
				char           argbuf[1024];

				if (arglist_from_cmdstr(&cmdstr, argbuf, sizeof argbuf, &args)) {
					if (!strcmp(cmdname, "name")) {
						if (first && conf->verbose > 0)
							printf("%s\n", args ? args->arg : "(none)");
						else if (!first) {
							printf("Warning: name{} command not as first"
							       " command of test definition!\n");
						}
					} else {
						if (conf->verbose > 1) {
							struct Cmdarg *a;
							printf("-+'%s'\n", cmdname);
							for (a = args; a != NULL; a = a->next) {
								if (a->val)
									printf(" +--'%s':'%s'\n", a->arg, a->val);
								else
									printf(" +--'%s'\n", a->arg);
							}
						}
						if (!perform(cmdname, args)) {
							failed = true;
							break;
						}
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
			if (conf->verbose > 0)
				printf("****failed\n");
		} else {
			good++;
			if (conf->verbose > 0)
				printf("----passed\n");
		}
	}

	free_testdefs(testlist);

	if (conf->strlist) {
		printf("\nThe following specified tests didn't exist:\n");
		for (struct Confstr *str = conf->strlist; str; str = str->next) {
			printf(" - '%s'\n", str->str);
		}
		putchar('\n');
	}

	if (conf->verbose > -1)
		printf("\nBad : %d\nGood: %d\n %s\n", bad, good, bad ? " ***!!!***" : (char*)checkmark);
	imgstack_destroy();
	conf_free(conf);
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
	else if (!strcmp("swap", action))
		return perform_swap();
	else if (!strcmp("duplicate", action))
		return perform_duplicate();
	else if (!strcmp("compare", action))
		return perform_compare(args);
	else if (!strcmp("delete", action))
		return perform_delete();
	else if (!strcmp("addalpha", action))
		return perform_addalpha();
	else if (!strcmp("convertgamma", action))
		return perform_convertgamma(args);
	else if (!strcmp("convertformat", action))
		return perform_convertformat(args);
	else if (!strcmp("invertpalette", action))
		return perform_invertpalette(args);
	else if (!strcmp("flatten", action))
		return perform_flatten(args);
	else if (!strcmp("exposure", action))
		return perform_exposure(args);
	else
		printf("Unkown action: %s\n", action);

	return false;
}


static bool perform_loadbmp(struct Cmdarg *args)
{
	bool           success = false;
	const char    *dir = NULL, *fname = NULL;
	const char    *dirpath;
	char          *optname, *optvalue;
	char           path[1024];
	FILE          *file = NULL;
	struct Image  *img  = NULL;
	BMPHANDLE      h    = NULL;
	BMPRESULT      res;
	bool           set_undef = false;
	BMPUNDEFINED   undefmode;
	bool           set_format = false;
	BMPFORMAT      format     = BMP_FORMAT_INT;
	bool           set_conv64 = false;
	BMPCONV64      conv64;
	bool           set_huff_t4black = false;
	int            huff_t4black = 1;
	bool           insane       = false;
	bool           line_by_line = false;
	bool           index        = false;
	bool           loadicc = false;
	bool           icc_loadonly = false;
	BMPRESULT      expected = BMP_RESULT_OK;

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
		dirpath = conf->sampledir;
	else if (!strcmp(dir, "tmp"))
		dirpath = conf->tmpdir;
	else if (!strcmp(dir, "ref"))
		dirpath = conf->refdir;
	else {
		printf("loadbmp: Invalid dir '%s'\n", dir);
		goto abort;
	}
	if (sizeof path < snprintf(path, sizeof path, "%s/%s", dirpath, fname)) {
		printf("path too small!");
		exit(1);
	}

	while (args && args->arg) {
		optname  = args->arg;
		optvalue = args->val;
		if (!optvalue) {
			printf("loadbmp: invalid option '%s'\n", optname);
			goto abort;
		}

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
		} else if (!strcmp(optname, "expect")) {
			if (!bmpresult_from_str(optvalue, &expected)) {
				printf("loadbmp: invalid expected result '%s'.", optvalue);
				goto abort;
			}
		} else if (!strcmp(optname, "iccprofile")) {
			if (!strcmp(optvalue, "loadonly")) {
				icc_loadonly = true;
			}
			else if (!strcmp(optvalue, "apply")) {
				icc_loadonly = false;
			}
			else {
				printf("loadbmp: invalid option '%s' for iccprofile.", optvalue);
				goto abort;
			}
			loadicc = true;
		} else if (!strcmp(optname, "huff-t4black")) {
			huff_t4black = !!atoi(optvalue);
			set_huff_t4black = true;
		} else {
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

	if (set_huff_t4black)
		bmp_set_huffman_t4black_value(h, huff_t4black);

	if ((res = bmpread_load_info(h))) {
		if (res == BMP_RESULT_INSANE && insane)
			bmpread_set_insanity_limit(h, bmpread_buffersize(h));
		else {
			success = (res == expected);
			if (!success)
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

	img->xdpi = bmpread_resolution_xdpi(h);
	img->ydpi = bmpread_resolution_ydpi(h);

	if (loadicc) {

		img->iccprofile_size = bmpread_iccprofile_size(h);
		if (!img->iccprofile_size) {
			printf("no valid profile in file\n");
			goto abort;
		}
		if ((res = bmpread_load_iccprofile(h, &img->iccprofile))) {
			success = (res == expected);
			if (!success)
				printf("%s\n", bmp_errmsg(h));
			goto abort;
		}
		if (conf->verbose > 1)
			printf("Successfully loaded profile (size %lu bytes).\n", (unsigned long) img->iccprofile_size);
	}

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
			if ((res = bmpread_load_palette(h, &img->palette))) {
				success = (res == expected);
				if (!success)
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
			if ((res = bmpread_load_line(h, &line))) {
				success = (res == expected);
				if (!success)
					printf("%s\n", bmp_errmsg(h));
				goto abort;
			}
		}

	} else {
		if ((res = bmpread_load_image(h, &img->buffer))) {
			success = (res == expected);
			if (!success)
				printf("%s\n", bmp_errmsg(h));
			goto abort;
		}
	}

	if (conf->verbose > 1)
		printf("Image %s loaded\n", path);
	bmp_free(h); h = NULL;
	fclose(file); file = NULL;

	if (!imgstack_push(img))
		goto abort;

	img = NULL;
	success = (expected == BMP_RESULT_OK);

	/* fall through */
abort:
	if (file)
		fclose(file);
	if (h)
		bmp_free(h);
	if (img)
		img_free(img);

	return success;
}


static bool perform_savebmp(struct Cmdarg *args)
{
	const char   *fname = NULL;
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
	bool          set_huff_fgidx = false;
	int           huff_fgidx = 1;
	bool          set_huff_t4black = false;
	int           huff_t4black = 1;
	bool          icc_embed = false;

	if (args) {
		fname = args->arg;
		args  = args->next;
	}

	if (!(fname && *fname)) {
		printf("savebmp: invalid filespec\n");
		goto abort;
	}

	dirpath = conf->tmpdir;

	if (sizeof path < snprintf(path, sizeof path, "%s/%s", dirpath, fname)) {
		printf("path too small!");
		exit(1);
	}

	while (args && args->arg) {
		optname  = args->arg;
		optvalue = args->val;
		if (!optvalue) {
			printf("savebmp: invalid option '%s'\n", optname);
			goto abort;
		}

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
		} else if (!strcmp(optname, "format")) {
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
		} else if (!strcmp(optname, "rle")) {
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
		} else if (!strcmp(optname, "allow")) {
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
		} else if (!strcmp(optname, "huff-fgidx")) {

			huff_fgidx = !!atoi(optvalue);
			set_huff_fgidx = true;

		} else if (!strcmp(optname, "huff-t4black")) {

			huff_t4black = !!atoi(optvalue);
			set_huff_t4black = true;

		} else if (!strcmp(optname, "outbits")) {
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
		} else if (!strcmp(optname, "64bit")) {
			if (!strcmp(optvalue, "yes"))
				set_64bit = true;
			else if (!strcmp(optvalue, "no"))
				set_64bit = false;
			else {
				printf("savebmp: invalid 64bit option %s\n", optvalue);
				goto abort;
			}

		}  else if (!strcmp(optname, "iccprofile")) {
			if (!strcmp(optvalue, "embed"))
				icc_embed = true;
			else {
				printf("savebmp: invalid iccprofile option %s\n", optvalue);
				goto abort;
			}

		} else {
			printf("savebmp: unknown option %s\n", optname);
			goto abort;
		}
		args = args->next;
	}

	if (!(img = imgstack_get(0)))
		exit(1);

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

	if (set_huff_fgidx) {
		bmpwrite_set_huffman_img_fg_idx(h, huff_fgidx);
	}

	if (set_huff_t4black) {
		bmp_set_huffman_t4black_value(h, huff_t4black);
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

	if (icc_embed) {
		if (img->iccprofile_size <= 0) {
			printf("Source image has no ICC profile.");
			goto abort;
		}
		if (bmpwrite_set_iccprofile(h, img->iccprofile_size, img->iccprofile)) {
			printf("Couldn't set ICC profile: %s\n", bmp_errmsg(h));
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

	if (img->xdpi || img->ydpi) {
		bmpwrite_set_resolution(h, img->xdpi, img->ydpi);
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


static bool perform_swap(void)
{
	if (!imgstack_swap())
		exit(1);

	return true;
}

static bool perform_duplicate(void)
{
	struct Image *img, *newimg = NULL;

	if (!(img = imgstack_get(0)))
		exit(1);

	if (!(newimg = malloc(sizeof *newimg))) {
		perror("malloc");
		goto abort;
	}
	memset(newimg, 0, sizeof *newimg);

	if (!(newimg->buffer = malloc(img->buffersize))) {
		perror("malloc");
		goto abort;
	}

	if (img->palette) {
		if (!(newimg->palette = malloc(img->numcolors * 4))) {
			perror("malloc");
			goto abort;
		}
		memcpy(newimg->palette, img->palette, img->numcolors * 4);
		newimg->numcolors = img->numcolors;
	}

	memcpy(newimg->buffer, img->buffer, img->buffersize);

	newimg->buffersize     = img->buffersize;
	newimg->width          = img->width;
	newimg->height         = img->height;
	newimg->channels       = img->channels;
	newimg->bitsperchannel = img->bitsperchannel;
	newimg->format         = img->format;
	newimg->orientation    = img->orientation;

	if (!imgstack_push(newimg))
		goto abort;

	return true;
abort:
	if (newimg) {
		if (newimg->buffer)
			free(newimg->buffer);
		if (newimg->palette)
			free(newimg->palette);
		free(newimg);
	}
	return false;
}


static bool perform_addalpha(void)
{
	struct Image  *img;
	size_t         new_size, offnew, offold;
	unsigned char *tmp;
	int            bytesperchannel;

	if (!(img = imgstack_get(0)))
		exit(1);

	if (!(img->channels == 3)) {
		printf("Can add alpha channel only to RGB image\n");
		return false;
	}

	bytesperchannel = img->bitsperchannel / 8;
	new_size = (uint64_t) img->width * img->height * 4 * bytesperchannel;

	if (!(tmp = realloc(img->buffer, new_size))) {
		perror("add alpha");
		return false;
	}
	img->buffer     = tmp;
	img->buffersize = new_size;
	img->channels   = 4;

	offnew = img->width * img->height * 4;
	offold = img->width * img->height * 3;

	while (offold > 0) {

		offnew -= 4;
		offold -= 3;

		for (int c = 2; c >= 0; c--) {
			switch (bytesperchannel) {
			case 1:
				((uint8_t*)img->buffer)[offnew + c] = ((uint8_t*)img->buffer)[offold + c];
				break;
			case 2:
				((uint16_t*)img->buffer)[offnew + c] = ((uint16_t*)img->buffer)[offold + c];
				break;
			case 4:
				((uint32_t*)img->buffer)[offnew + c] = ((uint32_t*)img->buffer)[offold + c];
				break;
			default:
				printf("wahhh\n");
				exit(1);
			}
		}
		switch (img->format) {
		case BMP_FORMAT_FLOAT:
			((float*)img->buffer)[offnew + 3] = 1.0;
			break;
		case BMP_FORMAT_S2_13:
			((int16_t*)img->buffer)[offnew + 3] = 8192;
			break;
		case BMP_FORMAT_INT:
			switch (bytesperchannel) {
			case 1:
				img->buffer[offnew + 3] = 0xff;
				break;
			case 2:
				((uint16_t*)img->buffer)[offnew + 3] = 0xffff;
				break;
			case 4:
				((uint32_t*)img->buffer)[offnew + 3] = 0xffffffff;
				break;
			}
		}
	}
	return true;
}


static bool perform_flatten(struct Cmdarg *args)
{
	struct Image  *img;
	size_t         new_size, offnew, offold;
	unsigned char *tmp;

	if (!(img = imgstack_get(0)))
		exit(1);

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

	while (args && args->arg) {
		opt    = args->arg;
		optval = args->val;

		if (!strcmp(opt, "fstops")) {
			fstops = atof(optval);
		} else {
			if (conf->verbose > -2)
				printf("exposure: unkown option %s\n", opt);
			return false;
		}
		args = args->next;
	}
	if (fstops != 0.0)
		set_exposure(fstops);
	return true;
}



static void convert_srgb_to_linear(void);
static void convert_linear_to_srgb(void);

static bool perform_convertgamma(struct Cmdarg *args)
{
	const char   *from = NULL, *to = NULL;

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
	} else if (!strcmp(from, "linear")) {
		if (!strcmp(to, "srgb"))
			convert_linear_to_srgb();
		else {
			printf("Unknown conversion to %s\n", to);
			return false;
		}
	} else {
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

	if (!(img = imgstack_get(0)))
		exit(1);

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
			double   d = 0;

			switch(img->format) {
			case BMP_FORMAT_FLOAT:
				d = ((float*)img->buffer)[offs + i];
				break;
			case BMP_FORMAT_S2_13:
				d = s2_13_to_double(((uint16_t*)img->buffer)[offs + i]);
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
				((uint16_t*)img->buffer)[offs+i] = float_to_s2_13(d);
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


static void set_exposure(double fstops)
{
	struct Image *img;
	int channels, colorchannels;
	size_t        px, npixels;

	if (!(img = imgstack_get(0)))
		exit(1);

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
		for (int c = 0; c < colorchannels; c++) {
			double   d = 0;

			switch(img->format) {
			case BMP_FORMAT_FLOAT:
				d = ((float*)img->buffer)[offs + c];
				break;
			case BMP_FORMAT_S2_13:
				d = s2_13_to_double(((int16_t*)img->buffer)[offs + c]);
				break;
			case BMP_FORMAT_INT:
				switch (img->bitsperchannel) {
				case 8:  d = (double) ( (uint8_t*)img->buffer)[offs + c] / ((1<< 8)-1);    break;
				case 16: d = (double) ((uint16_t*)img->buffer)[offs + c] / ((1<<16)-1);    break;
				case 32: d = (double) ((uint32_t*)img->buffer)[offs + c] / ((1ULL<<32)-1); break;
				}
			}

			d = d * pow(2, fstops);

			switch (img->format) {
			case BMP_FORMAT_FLOAT:
				((float*)img->buffer)[offs+c] = (float) d;
				break;
			case BMP_FORMAT_S2_13:
				((uint16_t*)img->buffer)[offs+c] = float_to_s2_13(d);
				break;
			case BMP_FORMAT_INT:
				switch (img->bitsperchannel) {
				case 8:  ( (uint8_t*)img->buffer)[offs + c] =  (uint8_t) (d * ((1<< 8)-1) + 0.5);    break;
				case 16: ((uint16_t*)img->buffer)[offs + c] = (uint16_t) (d * ((1<<16)-1) + 0.5);    break;
				case 32: ((uint32_t*)img->buffer)[offs + c] = (uint32_t) (d * ((1ULL<<32)-1) + 0.5); break;
				}
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
	} else if (!strcmp(to, "s2.13")) {
		convert_format(BMP_FORMAT_S2_13, 0);
	} else if (!strcmp(to, "int")) {
		convert_format(BMP_FORMAT_INT, bits);
	} else {
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

	if (!(img = imgstack_get(0)))
		exit(1);

	if (img->format == format && img->bitsperchannel == bits)
		return;

	nvals = (size_t) img->width * img->height * img->channels;
	newsize = nvals * bits / 8;

	if (newsize > img->buffersize) {
		if (!(tmp = realloc(img->buffer, newsize))) {
			perror("realloc buffer for conv");
			exit(1);
		}
		img->buffer = tmp;
		img->buffersize = newsize;
	}

	for (int i = 0; i < nvals; i++) {
		double   d = 0;
		int      offs = img->bitsperchannel >= bits ? i : (nvals - i - 1);

		switch (img->format) {
		case BMP_FORMAT_FLOAT:
			d = ((float*)img->buffer)[offs];
			break;
		case BMP_FORMAT_S2_13:
			d = s2_13_to_double(((int16_t*)img->buffer)[offs]);
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
			((uint16_t*)img->buffer)[offs] = float_to_s2_13(d);
			break;
		case BMP_FORMAT_INT:
			switch (bits) {
			case 8:
				((uint8_t*)img->buffer)[offs]  = (uint8_t) (d * (double) 0xffU + 0.5);
				break;
			case 16:
				((uint16_t*)img->buffer)[offs] = (uint16_t) (d * (double) 0xffffU + 0.5);
				break;
			case 32:
				((uint32_t*)img->buffer)[offs] = (uint32_t) (d * (double) 0xffffffffUL + 0.5);
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

static bool perform_invertpalette(struct Cmdarg *args)
{
	struct Image *img;

	if (!(img = imgstack_get(0)))
		exit(1);

	if (!(img->palette && img->numcolors > 1)) {
		printf("invert-palette: image is not indexed\n");
		exit(1);
	}

	for (int i = 0; i < img->numcolors / 2; i++) {
		for (int j = 0; j < 4; j++) {
			int tmp;
			tmp = img->palette[4 * i + j];
			img->palette[4 * i + j] = img->palette[4 * (img->numcolors - i - 1) + j];
			img->palette[4 * (img->numcolors - i - 1) + j] = tmp;
		}
	}

	for (size_t i = 0; i < img->width * img->height; i++) {
		img->buffer[i] = img->numcolors - img->buffer[i] - 1;
	}

	return true;
}


static bool perform_delete(void)
{
	imgstack_delete();
	return true;
}


static bool perform_compare(struct Cmdarg *args)
{
	int           i, fuzz = 0;
	size_t        off, size;
	struct Image *img[2];
	const char   *opt, *optval;

	while (args && args->arg) {
		opt    = args->arg;
		optval = args->val;

		if (!strcmp(opt, "fuzz")) {
			fuzz = atol(optval);
		} else {
			printf("Warning: unknown option '%s' for compare\n", opt);
		}
		args = args->next;
	}

	for (i = 0; i < 2; i++) {
		img[i] = imgstack_get(i);
		if (!img[i])
			exit(1);
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

	if (img[0]->format != img[1]->format && conf->verbose > -2) {
		printf("compare: Warning! Images have different pixel formats!\n");
	}

	size = (size_t)img[0]->width * img[0]->height * img[0]->channels;

	for (off = 0; off < size; off++) {
		switch (img[0]->bitsperchannel) {
		case 8:
			if (fuzz < abs(((uint8_t*)img[0]->buffer)[off] - ((uint8_t*)img[1]->buffer)[off])) {
				printf("compare: pixels don't match (%u vs %u @ %u,%u)\n",
				        (unsigned) ((uint8_t*)img[0]->buffer)[off],
				        (unsigned) ((uint8_t*)img[1]->buffer)[off],
				        (unsigned) ((off / img[0]->channels) % img[0]->width),
				        (unsigned) ((off / img[0]->channels) / img[0]->width));
				return false;
			}
			break;

		case 16:
			if (fuzz < abs(((uint16_t*)img[0]->buffer)[off] - ((uint16_t*)img[1]->buffer)[off])) {
				printf("compare: pixels don't match (%u vs %u @ %u,%u)\n",
				        (unsigned) ((uint16_t*)img[0]->buffer)[off],
				        (unsigned) ((uint16_t*)img[1]->buffer)[off],
				        (unsigned) ((off / img[0]->channels) % img[0]->width),
				        (unsigned) ((off / img[0]->channels) / img[0]->width));
				return false;
			}
			break;

		case 32:
			if (fuzz < abs(((uint32_t*)img[0]->buffer)[off] - ((uint32_t*)img[1]->buffer)[off])) {
				printf("compare: pixels don't match (%u vs %u @ %u,%u)\n",
				        (unsigned) ((uint32_t*)img[0]->buffer)[off],
				        (unsigned) ((uint32_t*)img[1]->buffer)[off],
				        (unsigned) ((off / img[0]->channels) % img[0]->width),
				        (unsigned) ((off / img[0]->channels) / img[0]->width));
				return false;
			}
			break;
		default:
			printf("Invalid bitsperchannel (%d) for comparison", img[0]->bitsperchannel);
			return false;
		}
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
		dirpath = conf->sampledir;
	else if (!strcmp(dir, "tmp"))
		dirpath = conf->tmpdir;
	else if (!strcmp(dir, "ref"))
		dirpath = conf->refdir;
	else {
		printf("loadpng: Invalid dir '%s'", dir);
		goto abort;
	}
	if (sizeof path < snprintf(path, sizeof path, "%s/%s", dirpath, fname)) {
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

static void trim_trailing_slash(char *str)
{
	size_t len;

	if (!str)
		exit(1);

	len = strlen(str);

	while (len > 0 && '/' == str[len - 1])
		str[--len] = 0;
}

static inline uint16_t float_to_s2_13(double d)
{
	uint16_t u16;

	if (d <= -4.0)
		u16 = 0x8000;
	else if (d >= 4.0)
		u16 = 0x7fff;
	else {
		d   = round(d * 8192.0);
		u16 = (uint16_t) (0xffff & (int32_t)d);
	}
	return u16;
}

static inline double s2_13_to_double(uint16_t s2_13)
{
	return ((int16_t)s2_13) / 8192.0;
}

bool bmpresult_from_str(const char *str, BMPRESULT *res)
{
	if (!strcmp(str, "BMP_RESULT_OK"))
		*res = BMP_RESULT_OK;
	else if (!strcmp(str, "BMP_RESULT_INVALID"))
		*res = BMP_RESULT_INVALID;
	else if (!strcmp(str, "BMP_RESULT_TRUNCATED"))
		*res = BMP_RESULT_TRUNCATED;
	else if (!strcmp(str, "BMP_RESULT_INSANE"))
		*res = BMP_RESULT_INSANE;
	else if (!strcmp(str, "BMP_RESULT_PNG"))
		*res = BMP_RESULT_PNG;
	else if (!strcmp(str, "BMP_RESULT_JPEG"))
		*res = BMP_RESULT_JPEG;
	else if (!strcmp(str, "BMP_RESULT_ERROR"))
		*res = BMP_RESULT_ERROR;
	else
		return false;

	return true;
}


