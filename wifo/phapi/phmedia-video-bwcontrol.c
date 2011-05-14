/*
 * phmedia video bandwidth control
 *
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
 * @author David Ferlier
 *
 * This is a first attempt of a bandwidth regulation algorithm.
 *
 * NOT REALLY WORKING FOR NOW
 *
 */

#include <osip2/osip_mt.h>
#include <osip2/osip.h>
#include <ortp.h>
#include <telephonyevents.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <webcam/webcam.h>
#include <avcodec.h>

#include "phlog.h"
#include "phcodec.h"
#include "phapi.h"
//#include "phcall.h"
#include "phmedia.h"
#include "phmstream.h"
#include "phvstream.h"
#include "phcodec-h263.h"


struct bwcontrol_stat {
	struct timeval b_time;
	int rate;
	unsigned long long drop_count;
	unsigned long long  unavail_count;
	unsigned long long  outoftime_count;
};

#define KB_PER_SEC(k)	(k * 8 * 1024)
#define ABSOLUTE_MAX_RATE	KB_PER_SEC(64)
#define ABSOLUTE_MIN_RATE	KB_PER_SEC(4)

//extern phcall_t ph_calls[PH_MAX_CALLS];

void dump_rr(rtcp_rr_t *r) {
	printf("Dumping new RR\n");
	printf("FRACTION: %d\n", report_block_get_fraction_lost(r->rb));
	printf("LOST: %d\n", report_block_get_cum_packet_loss(r->rb));
	printf("LAST_SEQ: %d\n", report_block_get_high_ext_seq(r->rb));
	printf("JITTER: %d\n", report_block_get_interarrival_jitter(r->rb));
	printf("LSR: %d\n", report_block_get_last_SR_time(r->rb));
	printf("DLSR: %d\n", report_block_get_last_SR_delay(r->rb));
}

/*
void dump_sr(rtcp_t *r) {
	int fixed_mask = (pow(2, 8) - 1);
	int fraction_left, fraction_right;
	float f;

	printf("Mask %d\n", fixed_mask);

	fraction_left = ((r->r.sr.ntp_frac & (1 << 7)) >> 7);
	fraction_right = (r->r.sr.ntp_frac & fixed_mask);

	printf("Dumping new SR\n");
	printf("FRAC %d\n", r->r.sr.ntp_frac);
	printf("FRAC point %d\n", fraction_left);
	printf("FRAC else %d\n", fraction_right);

	f = (float) ((float) fraction_left + (float)(fraction_right / pow(10, 8)));
	printf("FRACTION ! %f\n", f);
}

void ph_video_rtcp_rr_received(RtpSession *session, rtcp_t * r) {
	//dump_rr(&r->r.sr.rr[0]);
	//dump_rr(&r->r.rr.rr[1]);
}

void ph_video_rtcp_sr_sent(RtpSession *session, rtcp_t * r) {
	printf("RTCP sr sent\n");
	dump_sr(r);


	//printf("Frac %d\n", r->r.sr.ntp_frac);
}

void ph_video_rtcp_sr_received(RtpSession *session, rtcp_t * r) {
	printf("RTCP sr received\n");
}

void ph_video_rtcp_rr_sent(RtpSession *session, rtcp_t * r) {
	phvstream_t *s;
	rtcp_t *rr;

	printf("RTCP rr sent\n");

	s = (phvstream_t *) (session->user_data);

	if (s && s->ms.rtp_session == session)
	  {
	    rr = (rtcp_t *) malloc (sizeof(rtcp_t));
	    memcpy(rr, r, sizeof(rtcp_t));
	    osip_list_add(&s->rr_sent_q, rr, -1);
	  }
}
*/

void ph_video_bwcontrol_apply_user_params(phvstream_t *stream) {
	ph_h263_encoder_ctx_t *enc_ctx;
	ph_avcodec_encoder_ctx_t * av_enc_t;
	int q_compress, b_quant_offset, b_quant_factor, i_quant_factor, i_quant_offset;
	int quality, fps, gop_size, b_rate, img_quality;
	ph_mstream_params_t *msp = &stream->ms.mses->streams[PH_MSTREAM_VIDEO1];


	switch (msp->videoconfig) {
		case PHAPI_VIDEO_LINE_128KBPS:
			DBG_MEDIA_ENGINE_VIDEO("...video_bwcontrol: 128kbps\n");
			quality = 45;
			b_rate = KB_PER_SEC(15);
			fps = 10;
			gop_size = 1;
			img_quality = 900;
			break;

		case PHAPI_VIDEO_LINE_256KBPS:
			DBG_MEDIA_ENGINE_VIDEO("...video_bwcontrol: 256kbps\n");
			b_rate = KB_PER_SEC(30);
			quality = 45;
			fps = 12;
			gop_size = 7;
			img_quality = 260;
			break;

		case PHAPI_VIDEO_LINE_512KBPS:
			DBG_MEDIA_ENGINE_VIDEO("...video_bwcontrol: 512kbps\n");
			b_rate = KB_PER_SEC(60);
			quality = 60;
			fps = 17;
			gop_size = 9;
			img_quality = 20;
			break;

		case PHAPI_VIDEO_LINE_1024KBPS:
			DBG_MEDIA_ENGINE_VIDEO("...video_bwcontrol: 1024kbps\n");
			b_rate = KB_PER_SEC(120);
			quality = 110;
			fps = 25;
			gop_size = 13;
			img_quality = 3;
			break;
		default:
			DBG_MEDIA_ENGINE_VIDEO("...video_bwcontrol: default mode\n");
			quality = 45;
			b_rate = KB_PER_SEC(15);
			fps = 10;
			gop_size = 1;
			img_quality = 900;
			break;
	}

	gop_size = fps;

	enc_ctx = (ph_h263_encoder_ctx_t *) stream->ms.encoder_ctx;
	av_enc_t = &enc_ctx->encoder_ctx;

	av_enc_t->resized_pic->quality = img_quality;

#if 0
	av_enc_t->context->qcompress = (float)(quality / 100.0);
	av_enc_t->context->qblur = (float)(quality / 100.0);
#endif

	av_enc_t->context->b_quant_offset = (float)(2.0 - quality / 100.0);
	av_enc_t->context->b_quant_factor = (float)(2.0 - quality / 100.0);

	av_enc_t->context->i_quant_factor =  (float)(-1.0 + quality / 100.0);
	av_enc_t->context->i_quant_offset = (float)(0.0);


	av_enc_t->context->rc_min_rate = b_rate;
	av_enc_t->context->rc_max_rate = b_rate;
	av_enc_t->context->rc_buffer_size = b_rate * 64;
	av_enc_t->context->bit_rate = (b_rate * 3) >> 2;
	av_enc_t->context->bit_rate = b_rate;
	av_enc_t->context->bit_rate_tolerance = b_rate << 3;
	av_enc_t->context->max_qdiff = 3;
	av_enc_t->context->rc_qsquish = 0;
	av_enc_t->context->rc_eq = "tex^qComp";

	av_enc_t->context->gop_size = gop_size;

	av_enc_t->context->time_base.den = fps;
	av_enc_t->context->time_base.num = 1;

	stream->fps = fps;
	stream->fps_interleave_time = (1000/fps);

	if (stream->wt)
	{
		webcam_set_fps(stream->wt, fps);
	}
}

void *
ph_video_bwcontrol_thread(void *p)
{
	phvstream_t *stream = (phvstream_t*)p;
	ph_h263_encoder_ctx_t *enc_ctx;
	ph_avcodec_encoder_ctx_t * av_enc_t;
	rtp_stats_t *st;
//	rtcp_t *rr, *rr2;
	struct bwcontrol_stat *bws, *bws2 = NULL, *bws3 = NULL;
	struct timeval now_time;
	int pos, it, d_rate;

	osip_list_t pkts_drop_list;
	osip_list_init(&pkts_drop_list);

	st = &stream->ms.rtp_session->rtp.stats;
	enc_ctx = (ph_h263_encoder_ctx_t *) stream->ms.encoder_ctx;
	av_enc_t = &enc_ctx->encoder_ctx;

	while (stream->ms.running) {
		bws = (struct bwcontrol_stat *) malloc (sizeof(struct bwcontrol_stat));
		gettimeofday(&bws->b_time, 0);
		bws->drop_count = st->cum_packet_loss;
		bws->unavail_count = st->unavaillable;

		bws->rate = av_enc_t->context->rc_max_rate;

		osip_list_add(&pkts_drop_list, bws, -1);
		pos = osip_list_size(&pkts_drop_list);

		bws2 = (struct bwcontrol_stat *) osip_list_get(
				&pkts_drop_list, pos - 1);
		d_rate = bws2->rate;

/*
// TODO: Wengo - use the new RTCP api
		pos = osip_list_size(&stream->rr_sent_q);

		rr = (rtcp_t *) osip_list_get(&stream->rr_sent_q, pos-1);

		for (it = pos - 2; it > pos - 5; it -= 1) {
			rr = (rtcp_t *) osip_list_get(&stream->rr_sent_q, it);
			if (!rr)
			{
				continue;
			}

			printf("Current lost %d, item lost %d\n",
				rr->r.rr.rr[0].lost, rr2->r.rr.rr[0].lost);

			if (rr->r.rr.rr[0].lost > rr2->r.rr.rr[0].lost)
			{
				d_rate -= KB_PER_SEC(1);
			}
			else
			{
				d_rate += KB_PER_SEC(1);
			}
		}
*/
		/*
		if (pos > 8) {
			for (it = 0; it < pos - 5; it += 1) {
				osip_list_remove(&pkts_drop_list, it);
			}
		}
		*/

		printf("Actual rate %d, new rate %d\n",
				av_enc_t->context->rc_max_rate, d_rate);

		if (d_rate >= ABSOLUTE_MIN_RATE &&
			d_rate <= ABSOLUTE_MAX_RATE && stream->ms.running)
		{
			av_enc_t->context->rc_max_rate = d_rate;
			av_enc_t->context->rc_min_rate = d_rate;
			av_enc_t->context->bit_rate = d_rate;
		}

#ifdef WIN32
		Sleep(1000);
#else
		usleep(500000);
#endif
	}

	return NULL;
}
