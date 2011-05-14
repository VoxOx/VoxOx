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

#ifndef PIXERTOOL_H
#define PIXERTOOL_H

#include <pixertool/pixertooldll.h>

#include <stdint.h>

/**
 * Tools to convert image data from a format to another.
 *
 * This library also provides an OS independent way to
 * identify palette.
 *
 * @file pixertool.h
 * @author Philippe Bernery
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Palette identifiers.
 *
 * See http://www.fourcc.org for details about formats
 */
typedef enum {
	PIX_OSI_UNSUPPORTED = -1,
	PIX_OSI_YUV420P = 0, /* this value: 0 should not be modified. It is currently used as 0 in phmedia-video.c */
	PIX_OSI_I420,
	PIX_OSI_YUV422,
	PIX_OSI_YUV411,
	PIX_OSI_YUV422P,
	PIX_OSI_YUV444P,
	PIX_OSI_YUV424P,
	PIX_OSI_YUV41P,
	PIX_OSI_YUY2,
	PIX_OSI_YUYV,
	PIX_OSI_YVYU,
	PIX_OSI_UYVY,
	PIX_OSI_YV12,
	PIX_OSI_RGB555,
	PIX_OSI_RGB565,
	PIX_OSI_RGB1,
	PIX_OSI_RGB4,
	PIX_OSI_RGB8,
	PIX_OSI_RGBA32, //18
	PIX_OSI_RGB32, //19
	PIX_OSI_ARGB32, //20
	PIX_OSI_RGB24, //21
	PIX_OSI_BGR24, //22
	PIX_OSI_NV12 //23
} pixosi;

/** Flags for pix_convert. */
enum {
	PIX_NO_FLAG,
	/** Flip image horizontally */
	PIX_FLIP_HORIZONTALLY
};

/** Error code. */
typedef enum {
	PIX_OK,
	PIX_NOK
} pixerrorcode;

/** Image. */
typedef struct _piximage {
	uint8_t * data;
	unsigned width;
	unsigned height;
	pixosi palette;
} piximage;

/**
 * Allocates memory for image.
 *
 * @param width width of image
 * @param height height of image
 * @param pix desired palette
 * @return pointer to newly allocated data
 */
PIXERTOOL_API piximage * pix_alloc(pixosi pix, unsigned width, unsigned height);

/**
 * Free memory allocated with pix_alloc.
 *
 * @param ptr pointer to allocated data
 */
PIXERTOOL_API void pix_free(piximage * ptr);

/**
 * Computes size of image in memory.
 *
 * @param width width of picture
 * @param height height of picture
 * @param pix used palette
 * @return size of image
 */
PIXERTOOL_API unsigned pix_size(pixosi pix, unsigned width, unsigned height);

/**
 * Get the name of the palette/pixelformat
 *
 * @param pixfmt format identifier to get the name from
 * @return The name of the pixel format
 */
PIXERTOOL_API const char *pix_get_fmt_name(int pixfmt);

/**
 * Converts image data from original palette to desired palette.
 *
 * Resizes picture if img_dst size is different from img_src.
 * //TODO: beware of resize. it can only work when img_src is in YUV420
 *         (cf ffmpeg.avcodec.h function img_resample)
 *
 * @param flags setup conversion. Flags are described above
 * @param img_dst destination image. Must be allocated.
 * @param img_src source image
 * @return the converted image. NULL if format is not supported
 */
PIXERTOOL_API pixerrorcode pix_convert(int flags, piximage * img_dst, piximage * img_src);

/**
 * Copies a piximage.
 *
 * @param src the piximage to copy
 * @return the copy of the piximage
 */
PIXERTOOL_API piximage * pix_copy(piximage * src);

#ifdef __cplusplus
}
#endif

#endif	//PIXERTOOL_H
