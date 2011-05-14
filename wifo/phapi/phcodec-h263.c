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
 * Support for H263 in phapi
 *
 * @author David Ferlier <david.ferlier@wengo.fr>
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
#include "phcodec-h263.h"

int h263_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize) {
	ph_h263_decoder_ctx_t * h263t = (ph_h263_decoder_ctx_t *) ctx;
	return phcodec_avcodec_decode(&h263t->decoder_ctx, src, srcsize,
				dst, dstsize);
}

int h263_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize) {
	ph_h263_encoder_ctx_t * h263t = (ph_h263_encoder_ctx_t *) ctx;
	return phcodec_avcodec_encode(&h263t->encoder_ctx, src, srcsize,
				dst, dstsize);
}

ph_avcodec_meta_ctx_t * _h263_meta_init(ph_avcodec_meta_ctx_t * meta, phvstream_t *s) {

	meta->avcodec_encoder_id = CODEC_ID_H263;
	meta->avcodec_decoder_id = CODEC_ID_H263;
	meta->frame_rate = 10;
	meta->frame_width = PHMEDIA_VIDEO_FRAME_WIDTH;
	meta->frame_height = PHMEDIA_VIDEO_FRAME_HEIGHT;
	return meta;
}

void *h263_encoder_init(void *ctx) {
	phvstream_t *s = (phvstream_t *)ctx;
	ph_h263_encoder_ctx_t * h263t;

	h263t = (ph_h263_encoder_ctx_t *) calloc(sizeof(ph_h263_encoder_ctx_t), 1);
	_h263_meta_init(&h263t->meta, s);
 	h263t->max_frame_len = MAX_ENC_BUFFER_SIZE;
	h263t->data_enc = (uint8_t *) av_malloc (h263t->max_frame_len);

	if(phcodec_avcodec_encoder_init(&h263t->encoder_ctx, &h263t->meta, s) < 0)
	{
		av_free(h263t->data_enc);
		free(h263t);
		return 0;
	}

	h263t->encoder_ctx.context->flags |= CODEC_FLAG_QP_RD;
	h263t->encoder_ctx.context->flags |= CODEC_FLAG_H263P_SLICE_STRUCT;
	h263t->encoder_ctx.context->flags |= CODEC_FLAG_QSCALE;

	//h263t->encoder_ctx.context->flags |= CODEC_FLAG_INPUT_PRESERVED;
	//h263t->encoder_ctx.context->flags |= CODEC_FLAG_EMU_EDGE;
	//h263t->encoder_ctx.context->flags |= CODEC_FLAG_PASS1;

	h263t->encoder_ctx.context->mb_decision = FF_MB_DECISION_RD;

	h263t->encoder_ctx.context->gop_size = 30;
	h263t->encoder_ctx.context->thread_count = 1;
#define DEFAULT_RATE	(16 * 8 * 1024)
	h263t->encoder_ctx.context->rc_min_rate = DEFAULT_RATE;
	h263t->encoder_ctx.context->rc_max_rate = DEFAULT_RATE;
	h263t->encoder_ctx.context->rc_buffer_size = DEFAULT_RATE * 64;
	h263t->encoder_ctx.context->bit_rate = DEFAULT_RATE;

	if (avcodec_open(h263t->encoder_ctx.context,
			h263t->encoder_ctx.encoder) < 0)
	{
		return 0;
	}

	return h263t;
}

void *h263_decoder_init(void *ctx) {
	phvstream_t *s = (phvstream_t *)ctx;
	ph_h263_decoder_ctx_t * h263t;

	h263t = (ph_h263_decoder_ctx_t *) calloc(sizeof(ph_h263_decoder_ctx_t), 1);
  	_h263_meta_init(&h263t->meta, s);
 	if(phcodec_avcodec_decoder_init(&h263t->decoder_ctx, &h263t->meta) < 0)
 	{
 		free(h263t);
 		return 0;
 	}
	h263t->max_frame_len = MAX_DEC_BUFFER_SIZE;
	h263t->data_dec = (uint8_t *) av_malloc (h263t->max_frame_len+FF_INPUT_BUFFER_PADDING_SIZE);
	memset(h263t->data_dec + h263t->max_frame_len, 0, FF_INPUT_BUFFER_PADDING_SIZE);
	h263t->buf_index = 0;
	return h263t;
}

void h263_encoder_cleanup(void *ctx) {
	ph_h263_encoder_ctx_t * encoder = (ph_h263_encoder_ctx_t *) ctx;

	//avcodec_flush_buffers(encoder->encoder_ctx.context);
	avcodec_close(encoder->encoder_ctx.context);
	av_free(encoder->encoder_ctx.resized_pic);
	av_free(encoder->encoder_ctx.sampled_frame);
	av_free(encoder->data_enc);
	av_free(encoder->encoder_ctx.context);
	free(encoder);
}

void h263_decoder_cleanup(void *ctx) {
	ph_h263_decoder_ctx_t * decoder = (ph_h263_decoder_ctx_t *) ctx;
	//avcodec_flush_buffers(decoder->decoder_ctx.context);
	avcodec_close(decoder->decoder_ctx.context);
	av_free(decoder->data_dec);
	av_free(decoder->decoder_ctx.pictureIn);
	av_free(decoder->decoder_ctx.context);
	free(decoder);
}
