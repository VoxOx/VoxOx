/*
 * Copyright (C) 2005-2006 Wengo SAS
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with dpkg; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Support for H264 in phapi
 *
 */

#include <avcodec.h>
#include <ortp.h>
#include <osip2/osip_mt.h>
#include <osipparser2/osip_list.h>
#include <webcam/webcam.h>

#include "phcodec.h"
#include "phapi.h"
#include "phcall.h"
#include "phmedia.h"
#include "phmstream.h"
#include "phvstream.h"
#include "phcodec-avcodec-wrapper.h"
#include "phcodec-h264.h"


int h264_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize) {
	ph_h264_decoder_ctx_t * h264t = (ph_h264_decoder_ctx_t *) ctx;
	return phcodec_avcodec_decode(&h264t->decoder_ctx, src, srcsize,
				dst, dstsize);
}

int h264_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize) {
	ph_h264_encoder_ctx_t * h264t = (ph_h264_encoder_ctx_t *) ctx;
	return phcodec_avcodec_encode(&h264t->encoder_ctx, src, srcsize,
				dst, dstsize);
}

ph_avcodec_meta_ctx_t * _h264_meta_init(ph_avcodec_meta_ctx_t * meta, phvstream_t *s) {
	phConfig_t *cfg;
	cfg = phGetConfig();

	meta->avcodec_encoder_id = CODEC_ID_H264;
	meta->avcodec_decoder_id = CODEC_ID_H264;
	meta->frame_width = PHMEDIA_VIDEO_FRAME_WIDTH;
	meta->frame_height = PHMEDIA_VIDEO_FRAME_HEIGHT;

	return meta;
}

int h264_encoder_init(void *ctx) {
	phvstream_t *s = (phvstream_t *)ctx;
	ph_h264_encoder_ctx_t * h264t;

	h264t = (ph_h264_encoder_ctx_t *) malloc(sizeof(ph_h264_encoder_ctx_t));
	_h264_meta_init(&h264t->meta, s);
 	h264t->max_frame_len = 8096;
	h264t->data_enc = (uint8_t *) av_malloc (h264t->max_frame_len);

	if(phcodec_avcodec_encoder_init(&h264t->encoder_ctx, &h264t->meta, s) < 0)
	{
		av_free(h264t->data_enc);
		free(h264t);
		return 0;
	}

	if (avcodec_open(h264t->encoder_ctx.context,
			h264t->encoder_ctx.encoder) < 0)
	{
		return -1;
	}

	return h264t;
}

int h264_decoder_init(void *ctx) {
	phvstream_t *s = (phvstream_t *)ctx;
	ph_h264_decoder_ctx_t * h264t;

	h264t = (ph_h264_decoder_ctx_t *) malloc(sizeof(ph_h264_decoder_ctx_t));
	_h264_meta_init(&h264t->meta, s);
	h264t->data_dec = (uint8_t *) av_malloc (100000);
	h264t->buf_index = 0;
	h264t->max_frame_len = 8096;
 	if(phcodec_avcodec_decoder_init(&h264t->decoder_ctx, &h264t->meta) < 0)
 	{
 		free(h264t);
 		return 0;
 	}

	return h264t;
}


void h264_encoder_cleanup(void *ctx) {
	ph_h264_encoder_ctx_t * encoder = (ph_h264_encoder_ctx_t *) ctx;

	av_free(encoder->encoder_ctx.resized_pic);
	av_free(encoder->encoder_ctx.sampled_frame);
	av_free(encoder->data_enc);
	av_free(encoder->encoder_ctx.context);
	free(encoder);
}

void h264_decoder_cleanup(void *ctx) {
	ph_h264_decoder_ctx_t * decoder = (ph_h264_decoder_ctx_t *) ctx;
	av_free(decoder->data_dec);
	av_free(decoder->decoder_ctx.pictureIn);
	av_free(decoder->decoder_ctx.context);
	free(decoder);
}
