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
 * This is a wrapper around avcodec-powered phcodecs
 * It came to mind when i realized the only stuff that was changing between
 * two libavcodec codecs (in terms of usage) was the id and certain
 * parameters.
 *
 */

#define HAVE_MMX


#include <osip2/osip_mt.h>
#include <osip2/osip.h>
#include <ortp.h>
#include <telephonyevents.h>
#include <time.h>
#include <fcntl.h>



#include <avcodec.h>
#include <ortp.h>
#include <osip2/osip_mt.h>
#include <osipparser2/osip_list.h>
#include <webcam/webcam.h>

#include "phcodec.h"
#include "phapi.h"
#include "phcall.h"
#include "phmedia.h"
#include "phcodec-avcodec-wrapper.h"
#include "phmstream.h"
#include "phvstream.h"


void ph_avcodec_wrapper_init()
{
	avcodec_init();
	avcodec_register_all();
}

int phcodec_avcodec_decode(void *ctx, const void *src,
			int srcsize, void *dst, int dstsize) {

	int dec_len, got_picture = 0;
	ph_avcodec_decoder_ctx_t * decoder_t = (ph_avcodec_decoder_ctx_t *) ctx;

	dec_len = avcodec_decode_video(decoder_t->context,
		dst, &got_picture, (uint8_t *)src, srcsize);

	if (got_picture)
	{
		return dec_len;
	}

	return 0;
}

int phcodec_avcodec_encode(void *ctx, const void *src,
			int srcsize, void *dst, int dstsize) {

	ph_avcodec_encoder_ctx_t * encoder_t = (ph_avcodec_encoder_ctx_t *) ctx;
	int enc_len;

	enc_len = avcodec_encode_video(encoder_t->context,
		dst, dstsize, (AVFrame *)src);

	return enc_len;
}

int phcodec_avcodec_encoder_init(ph_avcodec_encoder_ctx_t *encoder_t, void *ctx, void *opaque) {
	ph_avcodec_meta_ctx_t *meta_t = (ph_avcodec_meta_ctx_t *) ctx;
	int dest_width, dest_height;

	dest_width = PHMEDIA_VIDEO_FRAME_WIDTH;
	dest_height = PHMEDIA_VIDEO_FRAME_HEIGHT;

	encoder_t->context = avcodec_alloc_context();
	encoder_t->encoder = avcodec_find_encoder(
			meta_t->avcodec_encoder_id);

	if (!encoder_t->encoder)
	{
		if (!encoder_t->encoder)
		{
#ifdef DEBUG
			printf("Couldn't find coded with id %d\n",
				meta_t->avcodec_encoder_id);
#endif
			return -1;
		}
	}

	encoder_t->context->pix_fmt = PIX_FMT_YUV420P;
	encoder_t->context->width = dest_width;
	encoder_t->context->height = dest_height;
    
    // time base unit for the presentation timestamps = millisecond
	encoder_t->context->time_base.num = 1;
    encoder_t->context->time_base.den = 1000;
    
	encoder_t->context->max_b_frames = 0;
	//encoder_t->context->dsp_mask = (FF_MM_MMX|FF_MM_MMXEXT|FF_MM_SSE|FF_MM_SSE2);
	//encoder_t->context->dsp_mask = (FF_MM_FORCE|FF_MM_MMX|FF_MM_MMXEXT|FF_MM_SSE|FF_MM_SSE2);
	encoder_t->context->dct_algo = FF_DCT_AUTO;
	encoder_t->context->idct_algo = FF_IDCT_AUTO;
	encoder_t->context->opaque = opaque;
	//VOXOX CHANGE by ASV 04-24-2009: the new version of FFMPEG doesn't use rtp_mode
	#if (defined OS_MACOSX) || (defined OS_LINUX)
		//encoder_t->context->rtp_mode = 1;
	#else 
		encoder_t->context->rtp_mode = 1;
	#endif
	encoder_t->context->rtp_payload_size = 1000;
	encoder_t->context->rtp_callback = phcodec_avcodec_video_rtp_callback;

	encoder_t->resized_pic = avcodec_alloc_frame();
	encoder_t->sampled_frame = avcodec_alloc_frame();

	return 0;
}

int phcodec_avcodec_decoder_init(ph_avcodec_decoder_ctx_t * decoder_t, void *ctx) {
	ph_avcodec_meta_ctx_t *meta_t = (ph_avcodec_meta_ctx_t *) ctx;

	int dest_width, dest_height;

	dest_width = PHMEDIA_VIDEO_FRAME_WIDTH;
	dest_height = PHMEDIA_VIDEO_FRAME_HEIGHT;


	decoder_t->context = avcodec_alloc_context();
	decoder_t->pictureIn = avcodec_alloc_frame();

	decoder_t->decoder = avcodec_find_decoder(meta_t->avcodec_decoder_id);

	if (!decoder_t->decoder)
	{
		return -1;
	}

	decoder_t->context->width = dest_width;
	decoder_t->context->height = dest_height;
	//decoder_t->context->dsp_mask = (FF_MM_MMX|FF_MM_MMXEXT|FF_MM_SSE|FF_MM_SSE2);
	decoder_t->context->idct_algo = FF_IDCT_AUTO;

	if (avcodec_open(decoder_t->context, decoder_t->decoder) < 0) 
	{
		return -1;
	}
	return 0;
}


void phcodec_avcodec_video_rtp_callback(struct AVCodecContext * context, void *data,
	          int size, int packetNumber ) {

	phvstream_t *video_stream = (phvstream_t *)context->opaque;
    // time base unit of ->pts has been defined as "millisecond"
    // ts = fix_random + sum (90000 / local_fps)
    // => ts = 0 + sum (90 * delta_ts_millisec)
    uint32_t ts = (uint32_t) ( 90 * context->coded_frame->pts );
	int eof;

	/*
	 * Set the P bit of h263 header to 1 (start of picture)
	 *
	 */

	/*
	if (video_stream->outq_count == 0) {
		((unsigned char *) data)[1] |= 32;
	}
	*/

	video_stream->mbCounter += packetNumber;
	eof = (video_stream->mbCounter == QCIF_MACROBLOCKS_IN_ONE_FRAME);
	if (eof)
	{
		video_stream->mbCounter = 0;
	}

	video_stream->rtpCallback(video_stream, data, size, ts, eof);
}
