/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
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
#include <pixertool/pixertool.h>
#include <pixertool/ffmpeg-pixertool.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <avcodec.h>
#ifdef HAVE_SWSCALE
#include <swscale.h>
#endif
#ifdef __cplusplus
}
#endif

#include <stdlib.h>
#include <string.h>

/*
 * Private functions
 */
/**
 * Convert data from NV12 to YUV420P.
 *
 * @param data data to convert
 * @param width image widht
 * @param height image height
 * @return converted data
 */

static uint8_t * _nv12_to_yuv420p(uint8_t * data, unsigned width, unsigned height);

piximage * pix_alloc(pixosi pix, unsigned width, unsigned height) {
	piximage * image = (piximage *) malloc(sizeof(piximage));

	avcodec_init();

	image->width = width;
	image->height = height;
	image->palette = pix;

	image->data = (uint8_t *) av_malloc(pix_size(pix, width, height) * sizeof(uint8_t));

	if (!image->data) {
		free(image);
		image = NULL;
	}

	return image;
}

void pix_free(piximage * ptr) {
	if (ptr) {
		if (ptr->data) {
			av_free(ptr->data);
		}
		free(ptr);
	}
}

unsigned pix_size(pixosi pix, unsigned width, unsigned height) {
	return avpicture_get_size(pix_ffmpeg_from_pix_osi(pix), width, height);

}

const char *pix_get_fmt_name(int pixfmt) {
  return avcodec_get_pix_fmt_name(pixfmt);
}

pixerrorcode pix_convert(int flags, piximage * img_dst, piximage * img_src) {
	uint8_t * buf_source = img_src->data;
	int need_avfree = 0;
#ifdef HAVE_SWSCALE
	struct SwsContext *convert_context;
#endif

	//If the format is NV12, transforming it
	if (img_src->palette == PIX_OSI_NV12) {
		buf_source = _nv12_to_yuv420p(img_src->data, img_src->width, img_src->height);
		need_avfree = 1;
		img_src->palette = PIX_OSI_YUV420P;
	}
	////

	int need_resize = 0;

	//Check if the piximage needs to be resized
	if ((img_src->width != img_dst->width) || (img_src->height != img_dst->height)) {
		need_resize = 1;
	}
	////

	//int len_target = pix_size(img_dst->palette, img_src->width, img_src->height);

	enum PixelFormat pix_fmt_source = pix_ffmpeg_from_pix_osi(img_src->palette);
	enum PixelFormat pix_fmt_target = pix_ffmpeg_from_pix_osi(img_dst->palette);

	AVPicture avp_source, avp_target;
	avpicture_fill(&avp_source,  buf_source, pix_fmt_source, img_src->width, img_src->height);
	avpicture_fill(&avp_target, img_dst->data, pix_fmt_target, img_dst->width, img_dst->height);

	//FIXME Only flip other planes if the destination palette is YUV420
	if ((flags & PIX_FLIP_HORIZONTALLY) && (img_src->palette == PIX_OSI_YUV420P)) {
		avp_source.data[0] += avp_source.linesize[0] * (img_src->height - 1);
		avp_source.linesize[0] *= -1;

		if (pix_fmt_source == PIX_FMT_YUV420P) {
			avp_source.data[1] += avp_source.linesize[1] * (img_src->height / 2 - 1);
			avp_source.linesize[1] *= -1;
			avp_source.data[2] += avp_source.linesize[2] * (img_src->height / 2 - 1);
			avp_source.linesize[2] *= -1;
		}
	}

	//Resizing picture if needed. Needs test
	if (need_resize) {
		//resampling only works yuv420P -> yuv420P in current ffmpeg
		if (pix_fmt_source != PIX_FMT_YUV420P) {
			return PIX_NOK;
		}

#ifndef HAVE_SWSCALE
	      //TODO optimize this part but will need the preparation of contexts
               ImgReSampleContext * resample_context = img_resample_init(img_dst->width, img_dst->height,
                       img_src->width, img_src->height);

               if (!resample_context) {
                       return PIX_NOK;
               }

               AVPicture avp_tmp_target;

               //we need to prepare a tmp buffer
               uint8_t * buf_tmp_target = (uint8_t *)av_malloc(avpicture_get_size(pix_fmt_source, img_dst->width, img_dst->height)  * sizeof(uint8_t));
               avpicture_fill(&avp_tmp_target, buf_tmp_target, pix_fmt_source, img_dst->width, img_dst->height);
               //

               //do the resampling
               img_resample(resample_context, &avp_tmp_target, &avp_source);
               img_resample_close(resample_context);
               //

                //do the conversion
		if (img_convert(&avp_target, pix_fmt_target,
		        &avp_tmp_target, pix_fmt_source,
	  	        img_dst->width, img_dst->height) == -1) {
			av_free(buf_tmp_target);
                        return PIX_NOK;
	        }

	        av_free(buf_tmp_target);
#else
		//do the conversion
		convert_context = sws_getContext(img_src->width, img_src->height, pix_fmt_source, img_dst->width, img_dst->height, pix_fmt_target, SWS_BICUBIC, NULL, NULL, NULL);
		if(!convert_context) {
			return PIX_NOK;
		}

		if (sws_scale(convert_context, avp_source.data, avp_source.linesize,
		    0, img_src->height, 
		    avp_target.data, avp_target.linesize) == -1) {
			sws_freeContext(convert_context);
			return PIX_NOK;
		}
		sws_freeContext(convert_context);
		//
#endif
	} else {
#ifndef HAVE_SWSCALE
               if (img_convert(&avp_target, pix_fmt_target,
                       &avp_source, pix_fmt_source,
                       img_src->width, img_src->height) == -1) {
			return PIX_NOK;
		}
#else
		convert_context = sws_getContext(img_src->width, img_src->height, pix_fmt_source, img_dst->width, img_dst->height, pix_fmt_target, SWS_BICUBIC, NULL, NULL, NULL);

		if(!convert_context) {
			return PIX_NOK;
		}

		if (sws_scale(convert_context, avp_source.data, avp_source.linesize,
		    0, img_src->height, 
		    avp_target.data, avp_target.linesize) == -1) {
			sws_freeContext(convert_context);
			return PIX_NOK;
		}

		sws_freeContext(convert_context);
#endif
	}
	////

	if (need_avfree) {
		av_free(buf_source);
	}
	return PIX_OK;
}

static uint8_t * _nv12_to_yuv420p(uint8_t * data, unsigned width, unsigned height) {

	uint8_t * buf_source = data;
	int len_target = (width * height * 3) / 2;
	uint8_t * buf_target = (uint8_t *) av_malloc(len_target * sizeof(uint8_t));
	memcpy(buf_target, buf_source, width * height);
	register unsigned i;
	for (i = 0 ; i < (width * height / 4) ; i++) {
		buf_target[(width * height) + i] = buf_source[(width * height) + 2 * i];
		buf_target[(width * height) + (width * height / 4) + i]
			= buf_source[(width * height) + 2 * i + 1];
	}
	return buf_target;
}

piximage * pix_copy(piximage * src) {
	piximage * result = pix_alloc(src->palette, src->width, src->height);
	memcpy(result->data, src->data, pix_size(src->palette, src->width, src->height));

	return result;
}
