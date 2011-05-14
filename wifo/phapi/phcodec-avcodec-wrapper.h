/*
 * Copyright (C) 2005 David Ferlier <david.ferlier@wengo.fr>
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

#ifndef PHCODEC_AVCODEC_WRAPPER_H
#define PHCODEC_AVCODEC_WRAPPER_H

#include "phapi.h"
#include "phcall.h"
#include "phmedia.h"
#include <avcodec.h>

#define PHCODEC_DEFAULT_BITRATE 1000000
// 99 = 11*9 = (176/16)*(144/16) = a partition of macroblocs ( 1 MB = four 8x8 squares)
// cf RFC 2190, 2.Definitions
#define QCIF_MACROBLOCKS_IN_ONE_FRAME 99

typedef struct ph_avcodec_encoder_ctx ph_avcodec_encoder_ctx_t;
typedef struct ph_avcodec_decoder_ctx ph_avcodec_decoder_ctx_t;
typedef struct ph_avcodec_meta_ctx ph_avcodec_meta_ctx_t;

int phcodec_avcodec_decode(void *ctx, const void *src,
		int srcsize, void *dst, int dstsize);
int phcodec_avcodec_encode(void *ctx, const void *src,
		int srcsize, void *dst, int dstsize);

int phcodec_avcodec_encoder_init(ph_avcodec_encoder_ctx_t *encoder_t, void *ctx, void *opaque);

int phcodec_avcodec_decoder_init(ph_avcodec_decoder_ctx_t *decctx, void *ctx);

void phcodec_avcodec_video_rtp_callback(struct AVCodecContext * context, void *data,
	          int size, int packetNumber );

struct ph_avcodec_encoder_ctx {
	AVPicture pictureOut;
	AVFrame *resized_pic, *sampled_frame;
	AVCodecContext *context;
	AVCodec* encoder;
	int maxencodesize;
};

struct ph_avcodec_decoder_ctx {
	AVCodecContext *context;
	AVFrame *pictureIn;
	AVCodec* decoder;
	int maxencodesize;
};

struct ph_avcodec_meta_ctx {
	int avcodec_encoder_id;
	int avcodec_decoder_id;
	int frame_width;
	int frame_height;
	int frame_rate;
};


#endif
