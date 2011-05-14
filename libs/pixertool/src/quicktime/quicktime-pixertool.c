/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <pixertool/quicktime-pixertool.h>

#include <Quicktime/Quicktime.h>

typedef struct _pix_quicktime_table_elt {
	pixosi osi;
	unsigned qtime;
} pix_quicktime_table_elt;

pix_quicktime_table_elt pix_quicktime_table[] = {
	{ PIX_OSI_BGR24, k24BGRPixelFormat},
	{ PIX_OSI_ARGB32, k32ARGBPixelFormat },
	{ PIX_OSI_RGBA32, k32RGBAPixelFormat }
};
	//TODO: verify format compatibility with MacOS/Windows then implement remaining formats
	/*
	{ PIX_OSI_YUV420P, kYUV420PixelFormat }
	{ PIX_OSI_RGB24, k24RGBPixelFormat}
	case k4444YpCbCrA8RPixelFormat:
	case kCMYKPixelFormat:
	case k422YpCbCr8PixelFormat:
	case kUYVY422PixelFormat:
	case kYUV211PixelFormat:
	case kYVU9PixelFormat:
	case k64ARGBPixelFormat:
	case k48RGBPixelFormat:
	case k32AlphaGrayPixelFormat:
	case k16GrayPixelFormat:
	case k4444YpCbCrA8PixelFormat:
	*/

pixosi pix_quicktime_to_pix_osi(int pix) {
	pixosi palette = PIX_OSI_UNSUPPORTED;
	register unsigned i;

	for (i = 0 ; i < sizeof(pix_quicktime_table) / sizeof(pix_quicktime_table_elt) ; i++) {
		if (pix == pix_quicktime_table[i].qtime) {
			palette =  pix_quicktime_table[i].osi;
			break;
		}
	}

	return palette;
}

int pix_quicktime_from_pix_osi(pixosi pix) {
	int palette = -1;
	register unsigned i;

	for (i = 0 ; i < sizeof(pix_quicktime_table) / sizeof(pix_quicktime_table_elt) ; i++) {
		if (pix == pix_quicktime_table[i].osi) {
			palette =  pix_quicktime_table[i].qtime;
			break;
		}
	}

	return palette;
}

const char * pix_quicktime_to_string(unsigned pix) {
	static char pixfmt[512];

	//Description are taken from Apple documentation
	switch (pix) {
	case k16LE555PixelFormat:
		strcpy(pixfmt, "16 bit LE RGB 555 (PC)");
		break;
	case k16LE5551PixelFormat:
		strcpy(pixfmt, "16 bit LE RGB 5551");
		break;
	case k16BE565PixelFormat:
		strcpy(pixfmt, "16 bit BE RGB 565");
		break;
	case k16LE565PixelFormat:
		strcpy(pixfmt, "16 bit LE RGB 565");
		break;
	case k24BGRPixelFormat:
		strcpy(pixfmt, "24 bit BGR");
		break;
	case k32BGRAPixelFormat:
		strcpy(pixfmt, "32 bit BGRA (Matrox)");
		break;
	case k32ABGRPixelFormat:
		strcpy(pixfmt, "32 bit ABGR");
		break;
	case k32ARGBPixelFormat:
		strcpy(pixfmt, "32 bit ARGB");
		break;
	case k32RGBAPixelFormat:
		strcpy(pixfmt, "32 bit RGBA");
		break;
	case k64ARGBPixelFormat:
		strcpy(pixfmt, "ARGB 16-bit big-endian samples");
		break;
	case k48RGBPixelFormat:
		strcpy(pixfmt, "RGB 16-bit big-endian samples");
		break;
	case k32AlphaGrayPixelFormat:
		strcpy(pixfmt, "AlphaGray 16-bit big-endian samples");
		break;
	case k16GrayPixelFormat:
		strcpy(pixfmt, "Grayscale 16-bit big-endian samples");
		break;
	case kYUVSPixelFormat:
		strcpy(pixfmt, "YUV 4:2:2 byte ordering 16-unsigned");
		break;
	case kYUVUPixelFormat:
		strcpy(pixfmt, "YUV 4:2:2 byte ordering 16-signed");
		break;
	case kYVU9PixelFormat:
		strcpy(pixfmt, "YVU9 Planar 9");
		break;
	case kYUV411PixelFormat:
		strcpy(pixfmt, "YUV 4:1:1 Interleaved 16");
		break;
	case kYVYU422PixelFormat:
		strcpy(pixfmt, "YVYU 4:2:2 byte ordering 16");
		break;
	case kUYVY422PixelFormat:
		strcpy(pixfmt, "UYVY 4:2:2 byte ordering 16");
		break;
	case kYUV211PixelFormat:
		strcpy(pixfmt, "YUV 2:1:1 Packed 8");
		break;
	case kYUV420PixelFormat:
		strcpy(pixfmt, "Planar");
		break;
	case kCMYKPixelFormat:
		strcpy(pixfmt, "CMYK 8-bit");
		break;
	case k422YpCbCr8PixelFormat:
		strcpy(pixfmt, "Component Y'CbCr 8-bit 4:2:2, ordered Cb Y'0 Cr Y'1");
		break;
	case k4444YpCbCrA8PixelFormat:
		strcpy(pixfmt, "Component Y'CbCrA 8-bit 4:4:4:4, ordered Cb Y' Cr A");
		break;
	case k4444YpCbCrA8RPixelFormat:
		strcpy(pixfmt, "Component Y'CbCrA 8-bit 4:4:4:4, full range alpha, zero biased yuv, ordered A Y' Cb Cr");
		break;
	default:
		strcpy(pixfmt, "not found");
	}

	return pixfmt;
}
