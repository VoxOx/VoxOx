/*
 * phmedia video control
 *
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
 *
 * @author David Ferlier <david.ferlier@wengo.fr>
 *
 */

#include <osip2/osip_mt.h>
#include <osip2/osip.h>
#include <ortp.h>
#include <telephonyevents.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <webcam/webcam.h>
#include <avcodec.h>

#include "phcodec.h"
#include "phapi.h"
#include "phcall.h"
#include "phmedia.h"
#include "phmstream.h"
#include "phvstream.h"
#include "phcodec-h263.h"


int phVideoControlSetCameraFlip(int flip) {
	phcall_t *ca;
	phvstream_t *sess;
	phConfig_t *cfg = phGetConfig();
	cfg->video_config.video_camera_flip_frame = flip;
	return 0;
}

int phVideoControlChangeFps(int callid, int fps) {
	phcall_t *ca;
	ph_h263_encoder_ctx_t *enc_ctx;
	phvstream_t *stream;

	ca = ph_locate_call_by_cid(callid);
	if (!ca) 
	{
		return -PH_BADCID;
	}

	stream = (phvstream_t *) ca->ph_video_stream;

	if (!ca->ph_video_stream || !stream->ms.running)
	{
		return -PH_ERROR;
	}

	if (ca->video_payload != PH_MEDIA_H263_PAYLOAD)
	{
		return -PH_ERROR;
	}

	enc_ctx = (ph_h263_encoder_ctx_t *) stream->ms.encoder_ctx;

	/*
	 * If the webcam is initialized, change its fps
	 *
	 */

	if (stream->wt)
	{
		webcam_set_fps(stream->wt, fps);
	}

	/*
	 * Now update the encoder frame rate parameters and hope for the best
	 *
	 */

	enc_ctx->encoder_ctx.context->time_base.den = fps;
	enc_ctx->encoder_ctx.context->time_base.num = 1;

	return 0;
}

int phVideoControlChangeQuality(int callid, int quality) {
	phcall_t *ca;
	ph_h263_encoder_ctx_t *enc_ctx;
	ph_avcodec_encoder_ctx_t * av_enc_t;
	phvstream_t *stream;

	ca = ph_locate_call_by_cid(callid);
	if (!ca)
	{
#ifdef DEBUG
		printf("Can't find call (phchangefps)\n");
#endif
		return -PH_BADCID;
	}

	stream = (phvstream_t *) ca->ph_video_stream;

	if (!ca->ph_video_stream || !stream->ms.running)
	{
		return -PH_ERROR;
	}

	if (ca->video_payload != PH_MEDIA_H263_PAYLOAD)
	{
		return -PH_ERROR;
	}

	return 0;
/*
	enc_ctx = (ph_h263_encoder_ctx_t *) stream->ms.encoder_ctx;
	av_enc_t = &enc_ctx->encoder_ctx;

	av_enc_t->sampled_frame->quality = FF_LAMBDA_MAX / quality;
	av_enc_t->resized_pic->quality = FF_LAMBDA_MAX / quality;

	av_enc_t->context->qcompress = (float)(quality / 100.0);

	av_enc_t->context->b_quant_offset = (float)(2.0 - quality / 100.0);
	av_enc_t->context->b_quant_factor = (float)(2.0 - quality / 100.0);

	av_enc_t->context->i_quant_factor =  (float)(-1.0 + quality / 100.0);
	av_enc_t->context->i_quant_offset = (float)(0.0);

	av_enc_t->context->gop_size = 600;
	return 0;
*/
}


int phVideoControlSetBitrate(int callid, int maxrate, int minrate) {
	phcall_t *ca;
	ph_h263_encoder_ctx_t *enc_ctx;
	ph_avcodec_encoder_ctx_t * av_enc_t;
	phvstream_t *stream;

	ca = ph_locate_call_by_cid(callid);
	if (!ca)
	{
		return -PH_BADCID;
	}

	stream = (phvstream_t *) ca->ph_video_stream;

	if (!ca->ph_video_stream || !stream->ms.running)
	{
		return 0;
	}

	if (ca->video_payload != PH_MEDIA_H263_PAYLOAD)
	{
		return 0;
	}

	enc_ctx = (ph_h263_encoder_ctx_t *) stream->ms.encoder_ctx;
	av_enc_t = &enc_ctx->encoder_ctx;
	av_enc_t->context->rc_min_rate = minrate;
	av_enc_t->context->rc_max_rate = maxrate;
	av_enc_t->context->bit_rate = maxrate;
	av_enc_t->context->rc_buffer_size = maxrate * 20;
	return 0;
}


int phVideoControlSetWebcamCaptureResolution(int width, int height) {
	phConfig_t *cfg = phGetConfig();
    if (cfg) {
        cfg->video_config.video_webcam_capture_width = width;
        cfg->video_config.video_webcam_capture_height = height;
        return 0;
    }
    return -1;
}

void phVideoControlCodecSet(int callid, struct phVideoCodecConfig *pvcc) {
	phcall_t *ca;
	ph_h263_encoder_ctx_t *enc_ctx;
	ph_avcodec_encoder_ctx_t * av_enc_t;
	phvstream_t *stream;

	ca = ph_locate_call_by_cid(callid);
	if (!ca)
	{
		return;
	}

	stream = (phvstream_t *) ca->ph_video_stream;
	if (!ca->ph_video_stream || !stream->ms.running)
	{
		return;
	}

	enc_ctx = (ph_h263_encoder_ctx_t *) stream->ms.encoder_ctx;
	av_enc_t = &enc_ctx->encoder_ctx;

	av_enc_t->context->rc_min_rate = pvcc->minrate;
	av_enc_t->context->rc_max_rate = pvcc->maxrate;
	av_enc_t->context->gop_size = pvcc->gopsize;
	av_enc_t->context->qmin = pvcc->qmin;
	av_enc_t->context->qmax = pvcc->qmax;
	av_enc_t->context->i_quant_offset = (float) pvcc->i_offset;
	av_enc_t->context->b_quant_offset = (float) pvcc->b_offset;
	av_enc_t->context->i_quant_factor = (float) pvcc->i_factor;
	av_enc_t->context->b_quant_factor = (float) pvcc->b_factor;
	av_enc_t->context->max_b_frames = pvcc->max_b_frame;
	av_enc_t->context->qcompress = pvcc->compress;
	av_enc_t->sampled_frame->quality = pvcc->f_quality;
	av_enc_t->resized_pic->quality = pvcc->f_quality;

}

void phVideoControlCodecGet(int callid, struct phVideoCodecConfig *pvcc) {
	phcall_t *ca;
	ph_h263_encoder_ctx_t *enc_ctx;
	ph_avcodec_encoder_ctx_t * av_enc_t;
	phvstream_t *stream;

	ca = ph_locate_call_by_cid(callid);
	if (!ca)
	{
		return;
	}

	stream = (phvstream_t *) ca->ph_video_stream;
	if (!ca->ph_video_stream || !stream->ms.running)
	{
		return;
	}

	enc_ctx = (ph_h263_encoder_ctx_t *) stream->ms.encoder_ctx;
	av_enc_t = &enc_ctx->encoder_ctx;

	pvcc->minrate = av_enc_t->context->rc_min_rate;
	pvcc->maxrate = av_enc_t->context->rc_max_rate;
	pvcc->gopsize = av_enc_t->context->gop_size;
	pvcc->qmin = av_enc_t->context->qmin;
	pvcc->qmax = av_enc_t->context->qmax;
	pvcc->i_offset = av_enc_t->context->i_quant_offset;
	pvcc->b_offset = av_enc_t->context->b_quant_offset;
	pvcc->i_factor = av_enc_t->context->i_quant_factor;
	pvcc->b_factor = av_enc_t->context->b_quant_factor;
	pvcc->max_b_frame = av_enc_t->context->max_b_frames;
	pvcc->compress = av_enc_t->context->qcompress;
	pvcc->f_quality = av_enc_t->sampled_frame->quality;
}
