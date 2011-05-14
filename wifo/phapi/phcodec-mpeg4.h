/*
 * phmedia -  Phone Api media streamer
 *
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

#ifndef PHCODEC_MPEG4_H
#define PHCODEC_MPEG4_H

#if 0
#include <phapi.h>
#include <phcall.h>
#include <phmedia.h>
#include <avcodec.h>
#include <phcodec-avcodec-wrapper.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

int mpeg4_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
int mpeg4_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
void *mpeg4_encoder_init(void *ctx);
void *mpeg4_decoder_init(void *ctx);
void mpeg4_decoder_cleanup(void *ctx);
void mpeg4_encoder_cleanup(void *ctx);

struct ph_mpeg4_encoder_ctx {
	ph_avcodec_meta_ctx_t meta;
	ph_avcodec_encoder_ctx_t encoder_ctx;
	uint8_t *data_enc;
	int max_frame_len;
};

struct ph_mpeg4_decoder_ctx {
	ph_avcodec_meta_ctx_t meta;
	ph_avcodec_decoder_ctx_t decoder_ctx;
	uint8_t *data_dec;
	int buf_index;
    int max_frame_len;
};


typedef struct ph_mpeg4_encoder_ctx ph_mpeg4_encoder_ctx_t;
typedef struct ph_mpeg4_decoder_ctx ph_mpeg4_decoder_ctx_t;


#ifdef __cplusplus
}
#endif

#endif
