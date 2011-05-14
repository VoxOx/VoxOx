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

#include "config.h"
#include <pixertool/ffmpeg-pixertool.h>
#include <stdlib.h>
#include <avcodec.h>
#ifdef HAVE_SWSCALE
#include <swscale.h>
#endif

/** Used by pix_convert_avpicture */
static AVPicture * pictureBuffer = NULL;

/** Called at program exit */
static void pix_ffmpeg_cleanup(void);
pixosi pix_ffmpeg_to_pix_osi(int pix) {
	pixosi palette;
	switch(pix) {
	case PIX_FMT_RGB24:
		palette = PIX_OSI_RGB24;
		break;
	case PIX_FMT_BGR24:
		palette = PIX_OSI_BGR24;
		break;
	case PIX_FMT_RGB555:
		palette = PIX_OSI_RGB555;
		break;
	case PIX_FMT_RGB565:
		palette = PIX_OSI_RGB565;
		break;
	case PIX_FMT_RGBA32:
		palette = PIX_OSI_RGBA32;
		break;
	case PIX_FMT_YUV420P:
		palette = PIX_OSI_YUV420P;
		break;
	case PIX_FMT_YUV422P:
		palette = PIX_OSI_YUV422P;
		break;
	case PIX_FMT_YUV422:
		palette = PIX_OSI_YUY2;
		break;
	case PIX_FMT_YUV444P:
		palette = PIX_OSI_YUV444P;
		break;
/* DOES not compile on MacOS X with current version of fink/ffmpeg (11/9/2006)
    case PIX_FMT_UYVY422:
        palette = PIX_OSI_UYVY;
        break;
*/	default:
		palette = PIX_OSI_UNSUPPORTED;
	}
	return palette;
}

enum PixelFormat pix_ffmpeg_from_pix_osi(pixosi pix) {

	enum PixelFormat palette;

	switch(pix) {
	case PIX_OSI_BGR24:
		palette = PIX_FMT_BGR24;
		break;
	case PIX_OSI_RGB24:
		palette = PIX_FMT_RGB24;
		break;
	case PIX_OSI_RGB555:
		palette = PIX_FMT_RGB555;
		break;
	case PIX_OSI_RGB565:
		palette = PIX_FMT_RGB565;
		break;
	case PIX_OSI_ARGB32:

	case PIX_OSI_RGB32:

	case PIX_OSI_RGBA32:
		palette = PIX_FMT_RGBA32;
		break;
	case PIX_OSI_YUV420P:
		palette = PIX_FMT_YUV420P;
		break;
	case PIX_OSI_YUV422P:
		palette = PIX_FMT_YUV422P;
		break;
	case PIX_OSI_YUV444P:
		palette = PIX_FMT_YUV444P;
		break;
	case PIX_OSI_YUV422:

	case PIX_OSI_YUYV:

	case PIX_OSI_YUY2:
		palette = PIX_FMT_YUV422;
		break;

/* DOES not compile on MacOS X with current version of fink/ffmpeg (11/9/2006)
	case PIX_OSI_UYVY:
		palette = PIX_FMT_UYVY422;
		break;
*/	default:
		palette = PIX_OSI_UNSUPPORTED;
	}
	return palette;
}

void pix_fill_avpicture(AVPicture * dst , piximage * src) {
	avpicture_fill(dst, src->data, pix_ffmpeg_from_pix_osi(src->palette),
		src->width, src->height);
}

pixerrorcode pix_convert_avpicture(int flags, piximage * img_dst, AVPicture * img_src, pixosi src_fmt) {
#ifdef HAVE_SWSCALE
	struct SwsContext *convert_context;
#endif
	pixosi desiredPalette = pix_ffmpeg_from_pix_osi(img_dst->palette);

	if (!pictureBuffer) {
		pictureBuffer = (AVPicture *) malloc(sizeof(AVPicture));
		atexit(pix_ffmpeg_cleanup);
	}

	avpicture_fill(pictureBuffer, img_dst->data, desiredPalette, img_dst->width, img_dst->height);

#ifndef HAVE_SWSCALE
       img_convert(pictureBuffer, desiredPalette,
               img_src, pix_ffmpeg_from_pix_osi(src_fmt),
               img_dst->width, img_dst->height);
#else

	convert_context = sws_getContext(img_dst->width, img_dst->height, pix_ffmpeg_from_pix_osi(src_fmt),
		    img_dst->width, img_dst->height, desiredPalette,
		    SWS_BICUBIC, NULL, NULL, NULL);
	if(!convert_context) {
		return PIX_NOK;
	}

	sws_scale(convert_context, img_src->data, img_src->linesize, 
		      0, img_dst->height,
		      pictureBuffer->data, pictureBuffer->linesize);
	sws_freeContext(convert_context);
#endif

	//pictureBuffer->data[0] should contain only valid data
	return PIX_OK;
}



void pix_ffmpeg_cleanup(void) {
	if (pictureBuffer) {
		free(pictureBuffer);
		pictureBuffer = NULL;
	}
}
