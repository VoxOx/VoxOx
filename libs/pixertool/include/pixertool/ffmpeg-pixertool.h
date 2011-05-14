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

#ifndef OWFFMPEG_PIXERTOOL_H
#define OWFFMPEG_PIXERTOOL_H

#include <pixertool/pixertool.h>
#include "config.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef HAVE_SWSCALE
#include <swscale.h>
#endif
#include <avcodec.h>
#ifdef __cplusplus
}
#endif

#include <avutil.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Palette identifier conversion.
 *
 * Converts from ffmpeg palette identifier to
 * pixer palette identifier
 *
 * @param pix palette identifier to convert
 * @return equivalent pixosi identifier
 */
PIXERTOOL_API pixosi pix_ffmpeg_to_pix_osi(int pix);
/**
 * Palette identifier conversion.
 *
 * Converts from pixer palette identifier to
 * ffmpeg palette identifier
 *
 * @param pix palette identifier to convert
 * @return equivalent ffmpeg identifier
 */

PIXERTOOL_API enum PixelFormat pix_ffmpeg_from_pix_osi(pixosi pix);

/**
 * Fills an AVPicture with a piximage.
 *
 * Data are not copied so src pointer must stay valid
 *
 * @param dst AVPicture to fill
 * @param src the piximage to convert
 */
PIXERTOOL_API void pix_fill_avpicture(AVPicture * dst, piximage * src);

/**
 * Converts an AVPicture to an img_dst.
 *
 * This function exists because of AVPicture buffer size. They are not
 * sometimes bigger than real picture size so img_src->data[0] is unusable to
 * get the picture data.
 * Size of the picture are taken from the img_dst.
 * This method does not auto resize the picture as pix_convert does
 * @param flags setup conversion. Flags are described in pixertool.h
 * @param img_dst destination image. Must be allocated.
 * @param img_src source image
 * @param src_fmt format of the AVPicture
 * @return the converted image. NULL if format is not supported
 */

PIXERTOOL_API pixerrorcode pix_convert_avpicture(int flags, piximage * img_dst, AVPicture * img_src, pixosi src_fmt);

#ifdef __cplusplus
}
#endif
#endif	//OWFFMPEG_PIXERTOOL_H
