/*
 * Copyright (C) 2005 Wengo SAS
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

/**
 * MPEG4 encoder for phapi
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
#include "phcodec-avcodec-wrapper.h"
#include "phcodec-mpeg4.h"


ph_avcodec_meta_ctx_t * _mpeg4_meta_init(ph_avcodec_meta_ctx_t * meta, phvstream_t *s);


int mpeg4_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize) {
	ph_mpeg4_decoder_ctx_t * mpeg4t = (ph_mpeg4_decoder_ctx_t *) ctx;
	return phcodec_avcodec_decode(&mpeg4t->decoder_ctx, src, srcsize,
				dst, dstsize);
}

int mpeg4_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize) {
	ph_mpeg4_encoder_ctx_t * mpeg4t = (ph_mpeg4_encoder_ctx_t *) ctx;
	return phcodec_avcodec_encode(&mpeg4t->encoder_ctx, src, srcsize,
				dst, dstsize);
}

ph_avcodec_meta_ctx_t * _mpeg4_meta_init(ph_avcodec_meta_ctx_t * meta, phvstream_t *s) {
	meta->avcodec_encoder_id = CODEC_ID_MPEG4;
	meta->avcodec_decoder_id = CODEC_ID_MPEG4;
	meta->frame_width = webcam_get_width(s->wt);
	meta->frame_height = webcam_get_height(s->wt);

	return meta;
}

void *mpeg4_encoder_init(void *ctx) {
	phvstream_t *s = (phvstream_t *)ctx;
	ph_mpeg4_encoder_ctx_t * mpeg4t;

	mpeg4t = (ph_mpeg4_encoder_ctx_t *) calloc(sizeof(ph_mpeg4_encoder_ctx_t), 1);
	_mpeg4_meta_init(&mpeg4t->meta, s);
	mpeg4t->max_frame_len = 8096;
	mpeg4t->data_enc = (uint8_t *) av_malloc (mpeg4t->max_frame_len);

	if(phcodec_avcodec_encoder_init(&mpeg4t->encoder_ctx, &mpeg4t->meta, s) < 0)
	{
		av_free(mpeg4t->data_enc);
		free(mpeg4t);
		return 0;
	}

	mpeg4t->encoder_ctx.context->flags |= CODEC_FLAG_QP_RD;
	//mpeg4t->encoder_ctx.context->flags |= CODEC_FLAG_H263P_UMV;
	//mpeg4t->encoder_ctx.context->flags |= CODEC_FLAG_4MV;
	//mpeg4t->encoder_ctx.context->flags |= CODEC_FLAG_AC_PRED;
	//mpeg4t->encoder_ctx.context->flags |= CODEC_FLAG_QSCALE;
	mpeg4t->encoder_ctx.context->mb_decision = FF_MB_DECISION_RD;
	//mpeg4t->encoder_ctx.context->gop_size = 600;
	//mpeg4t->encoder_ctx.context->max_b_frames = 0;
	mpeg4t->encoder_ctx.context->qblur = 0.5;
	mpeg4t->encoder_ctx.context->qcompress = 0.5;
	mpeg4t->encoder_ctx.context->b_quant_offset = 1.25;
	mpeg4t->encoder_ctx.context->b_quant_factor = 1.25;
	mpeg4t->encoder_ctx.context->i_quant_offset = 0.0;
	mpeg4t->encoder_ctx.context->i_quant_factor = -0.8;
	mpeg4t->encoder_ctx.context->thread_count = 1;

	if (avcodec_open(mpeg4t->encoder_ctx.context,
			mpeg4t->encoder_ctx.encoder) < 0)
	{
		return 0;
	}

	return mpeg4t;
}

void *mpeg4_decoder_init(void *ctx) {
	phvstream_t *s = (phvstream_t *)ctx;
	ph_mpeg4_decoder_ctx_t * mpeg4t;

	mpeg4t = (ph_mpeg4_decoder_ctx_t *) calloc(sizeof(ph_mpeg4_decoder_ctx_t), 1);
  	_mpeg4_meta_init(&mpeg4t->meta, s);
 	if(phcodec_avcodec_decoder_init(&mpeg4t->decoder_ctx, &mpeg4t->meta) < 0)
 	{
 		free(mpeg4t);
 		return 0;
 	}
	mpeg4t->data_dec = (uint8_t *) av_malloc (100000);
	mpeg4t->buf_index = 0;
	mpeg4t->max_frame_len = 8096;

	return mpeg4t;
}

void mpeg4_encoder_cleanup(void *ctx) {
	ph_mpeg4_encoder_ctx_t * encoder = (ph_mpeg4_encoder_ctx_t *) ctx;

	av_free(encoder->encoder_ctx.resized_pic);
	av_free(encoder->encoder_ctx.sampled_frame);
	av_free(encoder->data_enc);
	av_free(encoder->encoder_ctx.context);
	free(encoder);
}

void mpeg4_decoder_cleanup(void *ctx) {
	ph_mpeg4_decoder_ctx_t * decoder = (ph_mpeg4_decoder_ctx_t *) ctx;
	av_free(decoder->data_dec);
	av_free(decoder->decoder_ctx.pictureIn);
	av_free(decoder->decoder_ctx.context);
	free(decoder);
}
