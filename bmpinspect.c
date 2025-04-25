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
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>


#define BMPFILE_BM 0x4d42
#define BMPFILE_BA 0x4142
#define BMPFILE_CI 0x4943
#define BMPFILE_CP 0x5043
#define BMPFILE_IC 0x4349
#define BMPFILE_PT 0x5450


#define BMPFHSIZE       14
#define BMPIHSIZE_V3    40
#define BMPIHSIZE_V4   108
#define BMPIHSIZE_OS22  64
#define BMPIHSIZE_V5   124


enum BmpInfoVer {
	BMPINFO_CORE = 1,
	BMPINFO_OS21X,
	BMPINFO_OS22X,
	BMPINFO_V3,
	BMPINFO_V3_ADOBE1,
	BMPINFO_V3_ADOBE2,
	BMPINFO_V4,
	BMPINFO_V5,
};

struct Bmpfile {
	unsigned      type; /* "BM" */
	unsigned long size; /* bytes in file */
	unsigned      reserved1;
	unsigned      reserved2;
	unsigned long offbits;
};

struct Bmpinfo {
	/* BITMAPINFOHEADER (40 bytes) */
	unsigned long size; /* sizof struct */
	long          width;
	long          height;
	unsigned      planes;
	unsigned      bitcount;
	unsigned long compression;
	unsigned long sizeimage; /* 0 ok for uncompressed */
	long          xpelspermeter;
	long          ypelspermeter;
	unsigned long clrused;
	unsigned long clrimportant;
	/* BITMAPV4INFOHEADER (108 bytes) */
	unsigned long redmask;
	unsigned long greenmask;
	unsigned long bluemask;
	unsigned long alphamask;
	unsigned long cstype;
	long          redX;
	long          redY;
	long          redZ;
	long          greenX;
	long          greenY;
	long          greenZ;
	long          blueX;
	long          blueY;
	long          blueZ;
	unsigned long gammared;
	unsigned long gammagreen;
	unsigned long gammablue;
	/* BITMAPV5INFOHEADER (124 bytes) */
	unsigned long intent;
	unsigned long profiledata;
	unsigned long profilesize;
	unsigned long reserved;

	/* OS22XBITMAPHEADER */
	unsigned      units;           /* = 0 BRU_METRIC  */
	unsigned      orientation;     /* = 0 BRA_BOTTOMUP */
	unsigned      halftone_alg; /* BRH_NOTHALFTONED, BRH_ERRORDIFFUSION=1, BRH_PANDA=2, BRA_SUPERCIRCLE=3 */
	unsigned long halftone_parm1; /* for BRH_ERRORDIFFUSION: % error dampening; for PANDA/SUPERCIRCLE: x dimenstion of pattern in pixels */
	unsigned long halftone_parm2; /* PANDA/SUPERCIRCLE: y dimension of pattern in pixels, otherwise ignored */
	unsigned long color_encoding;  /* = 0 BCE_RGB, BCE_PALETTE = -1 */
	unsigned long app_id;

	/* internal only, not from file: */
	enum BmpInfoVer version;
};

#define IH_PROFILEDATA_OFFSET (14L + 112L)

#define MAX_ICCPROFILE_SIZE (1UL << 20)


#define BI_RGB             0
#define BI_RLE8            1
#define BI_RLE4            2
#define BI_BITFIELDS       3
#define BI_JPEG            4
#define BI_PNG             5
#define BI_ALPHABITFIELDS  6
#define BI_CMYK           11
#define BI_CMYKRLE8       12
#define BI_CMYKRLE4       13

#define BI_OS2_HUFFMAN_DUP  3 /* same as BI_BITFIELDS in Win BMP */
#define BI_OS2_RLE24_DUP    4 /* same as BI_JPEG in Win BMP      */

/* we set our own unique values: */
#define BI_OS2_HUFFMAN  1001
#define BI_OS2_RLE24    1002


#define LCS_CALIBRATED_RGB      0
#define LCS_sRGB                0x73524742 /* 'sRGB' */
#define LCS_WINDOWS_COLOR_SPACE 0x57696e20 /* 'Win ' */
#define PROFILE_LINKED          0x4c494e4b /* 'LINK' */
#define PROFILE_EMBEDDED        0x4d424544 /* 'MBED' */


#define LCS_GM_ABS_COLORIMETRIC 8
#define LCS_GM_BUSINESS         1
#define LCS_GM_GRAPHICS         2
#define LCS_GM_IMAGES           4



enum Type {
	U32 = 1,
	S32,
	U16,
	S16
};

#define DESCR_WIDTH 20

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

void print_hex_and_decimal_u32(unsigned long v);
void print_hex_and_decimal_s32(long v);
void print_hex_and_decimal_u16(unsigned v);
void print_hex_and_decimal_s16(int v);
void print_field(const void *var, enum Type type, const char *descr, bool flag, const char *name);

unsigned long u32_from_le(unsigned char *buf);
long s32_from_le(unsigned char *buf);
unsigned u16_from_le(unsigned char *buf);
int s16_from_le(unsigned char *buf);


const char* cstype_name(const struct Bmpinfo *ih);
const char* compression_name(const struct Bmpinfo *ih);
const char* intent_name(const struct Bmpinfo *ih);

const char* os2_halftonealg_name(const struct Bmpinfo *ih);
const char* os2_orientation_name(const struct Bmpinfo *ih);
const char* os2_units_name(const struct Bmpinfo *ih);
const char* os2_encoding_name(const struct Bmpinfo *ih);


const char* info_header_name(const struct Bmpinfo *ih);
bool determine_info_version(const struct Bmpfile *fh, struct Bmpinfo *ih);
bool read_file_header(FILE *file, struct Bmpfile *fh);
bool read_info_header(FILE *file, const struct Bmpfile *fh, struct Bmpinfo *ih);
uint64_t measure_file_size(FILE *file);


int main(int argc, char **argv)
{
	FILE          *file = NULL;
	struct Bmpfile fh;
	struct Bmpinfo ih;

	memset(&fh, 0, sizeof fh);
	memset(&ih, 0, sizeof ih);

	if (argc != 2) {
		printf("Usage: bmpinspect <bmp-file>\n");
		return 1;
	}

	if (!(file = fopen(argv[1], "rb"))) {
		perror(argv[1]);
		return 1;
	}

	if (!read_file_header(file, &fh))
		goto abort;

	if (fh.type != BMPFILE_BM) {
		printf("Invalid signature: 0x%04x\n", fh.type);
		goto abort;
	}

	if (!read_info_header(file, &fh, &ih))
		goto abort;

	int color_size = ih.version >= BMPINFO_OS22X ? 4 : 3;
	int max_colors = (fh.offbits - ih.size - 14) / color_size;
	int ncolors = 0, def_colors = 0, usable_colors = 0;
	if (ih.bitcount <= 8) {
		def_colors = 1 << ih.bitcount;
		usable_colors = MIN(def_colors, max_colors);
		if (ih.clrused > 0)
			ncolors = ih.clrused;
		else
			ncolors = def_colors;
	} else {
		max_colors = 0;
	}

	uint64_t filesize = measure_file_size(file);
	uint64_t maximgsize = filesize - fh.offbits;
	if (ih.cstype == PROFILE_EMBEDDED)
		maximgsize -= filesize - (14ULL + ih.profiledata);

	bool badimgsize = maximgsize < ih.sizeimage;
	bool badbitcount = ih.bitcount < 1 || (ih.bitcount > 32 && ih.bitcount != 64);

	printf("\nFile header:\n");
	print_field(&fh.type, U16, "type", false, NULL);
	print_field(&fh.size, U32, "size", fh.size != filesize, NULL);
	print_field(&fh.reserved1, U16, "reserved1", false, NULL);
	print_field(&fh.reserved2, U16, "reserved2", false, NULL);
	print_field(&fh.offbits, U32, "offbits", false, NULL);

	fclose(file);
	file = NULL;

	printf("\nInfo header: %s\n", info_header_name(&ih));

	printf("(size: %d)\n", (int) ih.size);
	print_field(&ih.width, S32, "width", false, NULL);
	print_field(&ih.height, S32, "height", false, NULL);
	print_field(&ih.planes, U16, "planes", false, NULL);
	print_field(&ih.bitcount, U16, "bitcount", badbitcount , NULL);

	if (ih.version >= BMPINFO_OS22X) {
		print_field(&ih.compression, U32, "compression", false, compression_name(&ih));
		print_field(&ih.sizeimage, U32, "image size", badimgsize, NULL);
		print_field(&ih.xpelspermeter, S32, "xpelspermeter", false, NULL);
		print_field(&ih.ypelspermeter, S32, "ypelspermeter", false, NULL);
		print_field(&ih.clrused, U32, "clrused", ncolors > usable_colors, NULL);
		print_field(&ih.clrimportant, U32, "clrimportant", false, NULL);
		if (ih.version >= BMPINFO_V4) {
			print_field(&ih.redmask, U32, "red mask", false, NULL);
			print_field(&ih.greenmask, U32, "green mask", false, NULL);
			print_field(&ih.bluemask, U32, "blue mask", false, NULL);
			print_field(&ih.alphamask, U32, "alpha mask", false, NULL);
			print_field(&ih.cstype, U32, "CS type", false, cstype_name(&ih));
			if (ih.version >= BMPINFO_V5) {
				print_field(&ih.intent, U32, "rendering intent", false, intent_name(&ih));
				print_field(&ih.profiledata, U32, "profile data", false, NULL);
				print_field(&ih.profilesize, U32, "profile size", false, NULL);
				print_field(&ih.reserved, U32, "reserved", false, NULL);
			}
		}
	}
	if (ih.version == BMPINFO_OS22X) {
		print_field(&ih.units, U16, "units", false, os2_units_name(&ih));
		print_field(&ih.orientation, U16, "orientation", false, os2_orientation_name(&ih));
		print_field(&ih.halftone_alg, U16, "halftone alg.", false, os2_halftonealg_name(&ih));
		print_field(&ih.halftone_parm1, U32, "halftone parm1", false, NULL);
		print_field(&ih.halftone_parm2, U32, "halftone parm2", false, NULL);
		print_field(&ih.color_encoding, U32, "color encoding", false, os2_encoding_name(&ih));
		print_field(&ih.app_id, U32, "app_id", false, NULL);
	}



	if (fh.size == 14 + ih.size) {
		printf("\nfh.size == file header + info header (ok for OS/2)\n");
	}
	else if (filesize > 0 && filesize != fh.size) {
		printf("\nActual file size: 0x%08llx (%llu)!\n", (unsigned long long) filesize, (unsigned long long) filesize);
	}

	if (ncolors > max_colors) {
		printf("\nHeader claims %d colors, but there is only room for %d colors!\n", ncolors, max_colors);
	} else if (ncolors > usable_colors) {
		printf("\nOversized color palette (%d colors, instead of usable %d colors)!\n", ncolors, usable_colors);
	}

	puts("");

	return 0;
abort:
	if (file)
		fclose(file);
	return 1;
}


uint64_t measure_file_size(FILE *file)
{
	long pos, size;

	if (-1 == (pos = ftell(file))) {
		perror(__func__);
		return 0;
	}
	if (fseek(file, 0, SEEK_END)) {
		perror(__func__);
		return 0;
	}
	if (-1 == (size = ftell(file))) {
		size = 0;
		perror(__func__);
	}

	fseek(file, pos, SEEK_SET);

	return (uint64_t) size;
}



bool determine_info_version(const struct Bmpfile *fh, struct Bmpinfo *ih)
{
	switch (ih->size) {
	case  12: ih->version = BMPINFO_CORE; break;
	case  16:
	case  20:
	case  24:
	case  28:
	case  32:
	case  36:
	case  42:
	case  44:
	case  46:
	case  48:
	case  60:
	case  64: ih->version = BMPINFO_OS22X;     break;
	case  40: ih->version = BMPINFO_V3;        break;
	case  52: ih->version = BMPINFO_V3_ADOBE1; break;
	case  56: ih->version = BMPINFO_V3_ADOBE2; break;
	case 108: ih->version = BMPINFO_V4;        break;
	case 124: ih->version = BMPINFO_V5;        break;
	default:
		printf("Invalid info header size: %lu\n", ih->size);
		return false;
	}

	if (ih->version == BMPINFO_CORE || ih->version == BMPINFO_V3) {
		if (fh->size == 14 + ih->size) {
			ih->version = BMPINFO_OS21X;
		}
		else if (ih->compression == BI_OS2_HUFFMAN_DUP && ih->bitcount == 1) {
			ih->version = BMPINFO_OS21X;
		}
		else if (ih->compression == BI_OS2_RLE24_DUP && ih->bitcount == 24) {
			ih->version = BMPINFO_OS21X;
		}
	}

	return true;
}


const char* cstype_name(const struct Bmpinfo *ih)
{
	if (ih->version < BMPINFO_V4 && ih->cstype == 0)
		return "(none)";

	switch (ih->cstype) {
	case LCS_CALIBRATED_RGB:      return "LCS_CALIBRATED_RGB";
	case LCS_sRGB:                return "LCS_sRGB";
	case LCS_WINDOWS_COLOR_SPACE: return "LCS_WINDOWS_COLOR_SPACE";
	case PROFILE_LINKED:          return "PROFILE_LINKED";
	case PROFILE_EMBEDDED:        return "PROFILE_EMBEDDED";
	default: return "(unknown)";
	}
}


const char* intent_name(const struct Bmpinfo *ih)
{
	if (ih->version < BMPINFO_V5)
		return "(none)";

	switch (ih->intent) {
	case LCS_GM_BUSINESS:         return "LCS_GM_BUSINESS";
	case LCS_GM_GRAPHICS:         return "LCS_GM_GRAPHICS";
	case LCS_GM_IMAGES:           return "LCS_GM_IMAGES";
	case LCS_GM_ABS_COLORIMETRIC: return "LCS_GM_ABS_COLORIMETRIC";
	default: return "(unknown)";
	}
}


const char* compression_name(const struct Bmpinfo *ih)
{
	if (ih->version == BMPINFO_CORE || ih->version == BMPINFO_OS21X)
		return "(none)";

	if (ih->version == BMPINFO_OS22X) {
		switch (ih->compression) {
		case BI_RGB:             return "BCA_UNCOMP";
		case BI_RLE4:            return "BCA_RLE4";
		case BI_RLE8:            return "BCA_RLE8";
		case BI_OS2_HUFFMAN_DUP: return "BCA_HUFFMAN1D";
		case BI_OS2_RLE24_DUP:   return "BCA_RLE24";
		default:                 return "(invalid)";
		}
	}

	switch (ih->compression) {
	case BI_RGB:            return "BI_RGB";
	case BI_RLE8:           return "BI_RLE8";
	case BI_RLE4:           return "BI_RLE4";
	case BI_BITFIELDS:      return "BI_BITFIELDS";
	case BI_JPEG:           return "BI_JPEG";
	case BI_PNG:            return "BI_PNG";
	case BI_ALPHABITFIELDS: return "BI_ALPHABITFIELDS";
	case BI_CMYK:           return "BI_CMYK";
	case BI_CMYKRLE8:       return "BI_CMYKRLE8";
	case BI_CMYKRLE4:       return "BI_CMYKRLE4";
	}

	return ("(invalid)");
}

const char* info_header_name(const struct Bmpinfo *ih)
{
	switch (ih->version) {
	case BMPINFO_CORE: return "BITMAPCOREHEADER";
	case BMPINFO_OS21X: return "OS2 BITMAPINFOHEADER";
	case BMPINFO_OS22X: return "OS2 BITMAPINFOHEADER2";
	case BMPINFO_V3: return "BITMAPINFOHEADER";
	case BMPINFO_V3_ADOBE1: return "BITMAPINFOHEADER + RGB masks (Adobev2)";
	case BMPINFO_V3_ADOBE2: return "BITMAPINFOHEADER + RGBA masks (Adobev3)";
	case BMPINFO_V4: return "BITMAPV4HEADER";
	case BMPINFO_V5: return "BITMAPV5HEADER";
	default: return "(unknown header)";
	}
}



const char* os2_halftonealg_name(const struct Bmpinfo *ih)
{
	switch (ih->halftone_alg) {
	case 0: return "BCA_NOTHALFTONED";
	case 1: return "BCA_ERRORDIFFUSION";
	case 2: return "BCA_PANDA";
	case 3: return "BCA_SUPERCIRCLE";
	default: return "(unknown)";
	}
}

const char* os2_orientation_name(const struct Bmpinfo *ih)
{
	switch (ih->orientation) {
	case 0: return "BRA_BOTTOMUP";
	default: return "(unknown)";
	}
}

const char* os2_units_name(const struct Bmpinfo *ih)
{
	switch (ih->units) {
	case 0: return "BRU_METRIC";
	default: return "(unknown)";
	}
}

const char* os2_encoding_name(const struct Bmpinfo *ih)
{
	switch (ih->color_encoding) {
	case  0: return "BCE_RGB";
	case -1: return "BCE_PALETTE";
	default: return "(unknown)";
	}
}


bool read_file_header(FILE *file, struct Bmpfile *fh)
{
	unsigned char buffer[14] = { 0 };

	if (14 != fread(buffer, 1, sizeof buffer, file)) {
		perror("read file header");
		return false;
	}

	fh->type      = u16_from_le(buffer +  0);
	fh->size      = u32_from_le(buffer +  2);
	fh->reserved1 = u16_from_le(buffer +  6);
	fh->reserved2 = u16_from_le(buffer +  8);
	fh->offbits   = u32_from_le(buffer + 10);

	return true;
}


#define MAX_INFOHEADER_SIZE 124

bool read_info_header(FILE *file, const struct Bmpfile *fh, struct Bmpinfo *ih)
{
	unsigned char  buffer[MAX_INFOHEADER_SIZE] = { 0 };

	if (4 != fread(buffer, 1, 4, file)) {
		perror("read info header size");
		return false;
	}

	ih->size = u32_from_le(buffer);

	if (ih->size < 12 || ih->size > MAX_INFOHEADER_SIZE) {
		printf("Invalid info header size: %lu\n", ih->size);
		return false;
	}

	size_t nbytes = ih->size - 4;
	if (nbytes != fread(buffer + 4, 1, nbytes, file)) {
		perror("read info header");
		return false;
	}

	if (ih->size == 12) {
		ih->width         = s16_from_le(buffer +  4);
		ih->height        = s16_from_le(buffer +  6);
		ih->planes        = u16_from_le(buffer +  8);
		ih->bitcount      = u16_from_le(buffer + 10);
	}
	else {
		ih->width         = s32_from_le(buffer +  4);
		ih->height        = s32_from_le(buffer +  8);
		ih->planes        = u16_from_le(buffer + 12);
		ih->bitcount      = u16_from_le(buffer + 14);
		ih->compression   = u32_from_le(buffer + 16);
		ih->sizeimage     = u32_from_le(buffer + 20);
		ih->xpelspermeter = s32_from_le(buffer + 24);
		ih->ypelspermeter = s32_from_le(buffer + 28);
		ih->clrused       = u32_from_le(buffer + 32);
		ih->clrimportant  = u32_from_le(buffer + 36);
	}
	if (!determine_info_version(fh, ih))
		return false;

	if (ih->version > BMPINFO_V3) {
		/* BITMAPV4INFOHEADER (108 bytes) */
		ih->redmask       = u32_from_le(buffer +  40);
		ih->greenmask     = u32_from_le(buffer +  44);
		ih->bluemask      = u32_from_le(buffer +  48);
		ih->alphamask     = u32_from_le(buffer +  52);
		ih->cstype        = u32_from_le(buffer +  56);
		ih->redX          = s32_from_le(buffer +  60);
		ih->redY          = s32_from_le(buffer +  64);
		ih->redZ          = s32_from_le(buffer +  68);
		ih->greenX        = s32_from_le(buffer +  72);
		ih->greenY        = s32_from_le(buffer +  76);
		ih->greenZ        = s32_from_le(buffer +  80);
		ih->blueX         = s32_from_le(buffer +  84);
		ih->blueY         = s32_from_le(buffer +  88);
		ih->blueZ         = s32_from_le(buffer +  92);
		ih->gammared      = u32_from_le(buffer +  96);
		ih->gammagreen    = u32_from_le(buffer + 100);
		ih->gammablue     = u32_from_le(buffer + 104);

		if (ih->version > BMPINFO_V4) {
			/* BITMAPV5INFOHEADER (124 bytes) */
			ih->intent        = u32_from_le(buffer + 108);
			ih->profiledata   = u32_from_le(buffer + 112);
			ih->profilesize   = u32_from_le(buffer + 116);
			ih->reserved      = u32_from_le(buffer + 120);
		}
	}
	else if (ih->version == BMPINFO_OS22X) {
		ih->units          = u16_from_le(buffer + 40);
		ih->orientation    = u16_from_le(buffer + 44);
		ih->halftone_alg   = u16_from_le(buffer + 46);
		ih->halftone_parm1 = u32_from_le(buffer + 48);
		ih->halftone_parm2 = u32_from_le(buffer + 52);
		ih->color_encoding = u32_from_le(buffer + 56);
		ih->app_id         = u32_from_le(buffer + 60);
	}
	return true;
}



void print_hex_and_decimal_u32(unsigned long v)
{
	printf("0x%08lx (%lu)", v, v);
}

void print_hex_and_decimal_s32(long v)
{
	printf("0x%08lx (%ld)", (unsigned long)v, v);
}

void print_hex_and_decimal_u16(unsigned v)
{
	printf("0x%04x     (%u)", v, v);
}

void print_hex_and_decimal_s16(int v)
{
	printf("0x%04x     (%d)", (unsigned) v, v);
}

void print_field(const void *var, enum Type type, const char *descr, bool flag, const char *name)
{
	printf(" %c%-*s: ", flag ? '!' : ' ', DESCR_WIDTH, descr);
	switch (type)
	{
	case U32:
		print_hex_and_decimal_u32(*(unsigned long*)var);
		break;
	case S32:
		print_hex_and_decimal_s32(*(long*)var);
		break;
	case U16:
		print_hex_and_decimal_u16(*(unsigned*)var);
		break;
	case S16:
		print_hex_and_decimal_s16(*(int*)var);
		break;
	}
	if (name)
		printf(" %s\n", name);
	else
		printf("\n");
}

unsigned long u32_from_le(unsigned char *buf)
{
	return (unsigned long)buf[0]       |
	       (unsigned long)buf[1] <<  8 |
	       (unsigned long)buf[2] << 16 |
	       (unsigned long)buf[3] << 24;
}

long s32_from_le(unsigned char *buf)
{
	unsigned long u32 = u32_from_le(buf);

	if (u32 >= 0x80000000UL)
		return (long)(u32 - 0x80000000UL) - 0x7fffffffL - 1L;
	else
		return (long)u32;
}

unsigned u16_from_le(unsigned char *buf)
{
	return (unsigned)buf[0] | (unsigned)buf[1] <<  8;
}

int s16_from_le(unsigned char *buf)
{
	unsigned u16 = u16_from_le(buf);

	if (u16 >= 0x8000U)
		return (int)(u16 - 0x8000U) - 0x7fff - 1;
	else
		return (int)u16;
}
