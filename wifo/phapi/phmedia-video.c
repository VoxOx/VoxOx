/*
 * phmedia -  Phone Api media streamer (video part)
 *
 * Copyright (C) 2005-2006 Wengo SAS
 * Copyright (C) 2004 Vadim Lebedev <vadim@mbdsys.com>
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
 * @author Jerome Wagner <jerome.wagner@wengo.fr>
 * @author David Ferlier <david.ferlier@wengo.fr>
 */

#define _WIN32_WINNT 0x5000

#include <osip2/osip_mt.h>
#include <osip2/osip.h>
#include <ortp.h>
#include <telephonyevents.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <webcam/webcam.h>
#include <avcodec.h>
#include <pixertool/ffmpeg-pixertool.h>
#include <pixertool/pixertool.h>

#include <svoip_phapi.h>

#include "phlog.h"
#include "phcodec.h"
#include "phapi.h"
// #include "phcall.h"
#include "phmedia.h"
#include "phmstream.h"
#include "phvstream.h"
#include "phcodec-h263.h"

/* sVoIP */
extern void sVoIP_phapi_recvRtp(int cid, void *error, void *buffer, int *len);
extern void sVoIP_phapi_sendRtp(int cid, void *error, void *buffer, int *len);

/* external transport functions to use with ORTP */
int ph_rtp_transport_recvfrom(struct _RtpTransport *t, void *buf, size_t len, int flags, 
							  struct sockaddr *from, socklen_t *fromlen);
int ph_rtcp_transport_recvfrom(struct _RtpTransport *t, void *buf, size_t len, int flags, 
							   struct sockaddr *from, socklen_t *fromlen);
int ph_rtp_transport_sendto(struct _RtpTransport *t, const void *buf, size_t len, int flags, 
							const struct sockaddr *from, socklen_t tolen);
int ph_rtcp_transport_sendto(struct _RtpTransport *t, const void *buf, size_t len, int flags, 
							 const struct sockaddr *from, socklen_t tolen);

/* If we don't have a webcam, send a YUV image filled with this value.
 * It produces a gray image.
 */
#define NOWEBCAM_FILL_VALUE 128

#include "nowebcam.yuv"

void tvsub_phapi(register struct timeval *out, register struct timeval *in);

/**
 * @brief Function called by ffmpeg when a RTP slice is available
 *
 * This is called by ffmpeg in SLICE_STRUCT mode. When a fragment of data
 * is ready to be sent, it calls this function. What we just do is send
 * the frame on RTP :)
 *
 * @param ctx		The video stream
 * @param data		The packet's data
 * @param size		The size of the packet
 * @param ts		The timestamp to use in the rtp packet
 * @param eof		End Of File (if it's the last packet of a serie)
 *
 */

void phmedia_video_rtpsend_callback (void *ctx, void *data, int size,
				unsigned long ts, int eof)
{
	phvstream_t *video_stream = (phvstream_t *) ctx;
	mblk_t *m1 ;
	static char h323header[] = {0x00, 0x40, 0x00, 0x19};
	int needH263Header = 1;
	int headerSize = 0;

	//H263HeaderHack: We add a 4 bytes header to every H263 RTP payload to be in compliant 
	//with H263 stardard. If the codec used is not H263, do not do it.
	if (strcmp(video_stream->ms.codec->mime, CODEC_H263_MIME_STRING) != 0 && 
				strcmp(video_stream->ms.codec->mime, CODEC_H263P_MIME_STRING) != 0) {
		needH263Header = 0;
	}

	headerSize = RTP_FIXED_HEADER_SIZE;
	if (needH263Header) {
		headerSize += sizeof(h323header);
	}
	m1 = rtp_session_create_packet (video_stream->ms.rtp_session,
			headerSize, (char *)data, size);
	if (!m1)
		return;

	if (needH263Header) {
		memcpy(m1->b_rptr+RTP_FIXED_HEADER_SIZE, h323header, sizeof(h323header));
	}

	if (eof) {
		rtp_set_markbit(m1, 1);
	}

	rtp_session_sendm_with_ts(video_stream->ms.rtp_session, m1, ts);
}

/**
 * @brief Initializes the webcam for a given stream
 *
 * This setups the webcam, and sets the resolution decided by the user.
 *
 * @param vstream		The video stream
 * @return	return code of webcamOpen (WEBCAM_CODE_OK on ok, NOK on nok)
 *
 */

WebcamErrorCode ph_media_video_initialize_webcam(phvstream_t *vstream) {
	phConfig_t *cfg = phGetConfig();
	WebcamErrorCode err;

    DBG_MEDIA_ENGINE_VIDEO("webcam init: trying to set video device (%s)...\n...", cfg->video_config.video_device);
	err = webcam_set_device(vstream->wt, cfg->video_config.video_device);

	if (err == WEBCAM_OK)
	{
		
	    DBG_MEDIA_ENGINE_VIDEO("OK\n");
        // beware: you must initialize these video_config parameters
        // while debugging, you can always try PHMEDIA_VIDEO_FRAME_WIDTH, PHMEDIA_VIDEO_FRAME_HEIGHT. 320,240 is a very good guess
		webcam_set_resolution(vstream->wt,
			cfg->video_config.video_webcam_capture_width,
			cfg->video_config.video_webcam_capture_height);

        // preferred palette output for webcam is YUV420P
		webcam_set_palette(vstream->wt, PIX_OSI_YUV420P);
	}

    DBG_MEDIA_ENGINE_VIDEO("webcam init: end of initialization\n");
	return err;
}

/**
 * @brief Function called by the webcam driver
 *
 * This function is called by the webcam driver. In this function we
 * convert the frame to YUV420 Planar (which is the format required
 * by ffmpeg and H.263), resize it to a fixed size (for now 176x144),
 * encode it. The encode function calls the RTP callback when it has
 * a RTP frame to be sent
 *
 * @param userdata		The video stream
 * @param buf_frameready	Pointer to the webcam frame
 * @param width_frameready			width of the served frame
 * @param height_frameready			height of the served frame
 * @param pix_osi_frameready			palette of the served frame on the osi lib plane
 * @param len_frameready			Length (in bytes) of the served frame buffer
 * @return not relevant
 *
 */

void webcam_frame_callback(webcam *sender, piximage *image, void *userdata) {
	phm_videoframe_t *phmvf;
	phvstream_t *video_stream = (phvstream_t *) userdata;
	(void)sender;

	if (!video_stream || !video_stream->ms.running)
	{
		return;
	}

	phmvf = ph_media_video_new_video_frame(image);

#ifdef PH_VIDEO_USELOCK
	osip_mutex_lock(video_stream->mtx);
#endif
	osip_list_add(&video_stream->webcam_frames_q, phmvf, -1);

#ifdef PH_VIDEO_USELOCK
	osip_mutex_unlock(video_stream->mtx);
#endif
}


phm_videoframe_t *ph_media_video_new_video_frame(piximage *image) {
	phm_videoframe_t *phmvf;

	phmvf = (phm_videoframe_t*) malloc(sizeof(phm_videoframe_t));
	phmvf->image.palette = image->palette;
	phmvf->image.data = image->data;
	phmvf->image.width = image->width;
	phmvf->image.height = image->height;

	return phmvf;
}


void ph_media_free_video_frame(phm_videoframe_t *ptr) {
	if (ptr)
	{
		free(ptr);
	}
}


/**
 * @brief Function called to both encode and send one full picture frame on the tx path of the stream
 *
 * This function is used to encode and send a frame on the network
 *
 * @param video_stream	the overall phmedia stream structure for video streams
 * @param phmvf	this is the structure holding the framebuffer and some metadata information
 * @param cache	0/1 : the yuv frame must be cached on the video stream. This is used for local image incrustation for example. It is done here to avoid converting to YUV twice
 * @return not relevant
 *
 */
int ph_media_video_send_frame(phvstream_t *video_stream, phm_videoframe_t *phmvf, int cache) {
	ph_h263_encoder_ctx_t *video_encoder = (ph_h263_encoder_ctx_t *) video_stream->ms.encoder_ctx;
	struct timeval now_time;
	unsigned enclen = 0;
    unsigned delta_ts = 0;
	struct _piximage *image_captured;
	AVFrame *avf_prepared_for_encoding;
	unsigned len_prepared_for_encoding = pix_size (PIX_OSI_YUV420P,
		PHMEDIA_VIDEO_FRAME_WIDTH,
		PHMEDIA_VIDEO_FRAME_HEIGHT);

	image_captured = &(phmvf->image);

	// if asked, a copy of the frame is put in cache on the stream
	if (cache)
	{
		// TODO: check that size is ok for the processing buffer
		memcpy(video_stream->local_frame_cache->data, image_captured->data,
			pix_size(image_captured->palette, image_captured->width, image_captured->height));
	}

	// if captured image is not PIX_OSI_YUV420P, force it !
	if (image_captured->palette != PIX_OSI_YUV420P)
	{
		pix_convert(PIX_NO_FLAG, video_stream->image_wrong_pix, &(phmvf->image));
		image_captured = video_stream->image_wrong_pix;
	}
	//
	
	// from here, the image is PIX_OSI_YUV420P
	
	// we prepare the captured image for the encoder
	pix_fill_avpicture((AVPicture *)video_encoder->encoder_ctx.sampled_frame,
		image_captured);

	// adjust the sampled size to the 176x144 forced size
	if (image_captured->width  == PHMEDIA_VIDEO_FRAME_WIDTH &&
		image_captured->height == PHMEDIA_VIDEO_FRAME_HEIGHT)
	{
		// no more work is needed. Image is ready to go to the encoder
		avf_prepared_for_encoding = video_encoder->encoder_ctx.sampled_frame;
	}
	else
	{
		// more work needed. Webcam capture a frame that we need to resize
		// before sending it to the decoder
        // PIX_FLIP_HORIZONTALLY, PIX_NO_FLAG
		pix_convert((phcfg.video_config.video_camera_flip_frame)?PIX_FLIP_HORIZONTALLY:PIX_NO_FLAG, video_stream->image_ready_for_network, image_captured);
		pix_fill_avpicture((AVPicture *)video_encoder->encoder_ctx.resized_pic,
		video_stream->image_ready_for_network);
		avf_prepared_for_encoding = video_encoder->encoder_ctx.resized_pic;
	}
	//


    // calculate the delta timestamp
    gettimeofday(&now_time, 0);
    ph_tvsub(&now_time, &video_stream->last_encode_time);
    delta_ts = now_time.tv_usec / 1000; // in millisec
    gettimeofday(&video_stream->last_encode_time, 0);
    if (!video_stream->num_encoded_frames)
    {
        delta_ts = 1000; // 1 sec
    }

	// MINHPQ: If delta_ts is 0, the new timestamp will be equal to previous timestamps and 
	// ffmpeg doesn't like it. (It won't encode the picture).
	if (delta_ts <= 0) {
		delta_ts = 1; // 1 milisecond. 
	}

	// encode the frame and re-adjust tstamps
	video_stream->num_encoded_frames += 1;
	video_stream->txtstamp += delta_ts;
    avf_prepared_for_encoding->pts = video_stream->txtstamp;
	enclen = video_stream->ms.codec->encode(video_encoder,
		avf_prepared_for_encoding, len_prepared_for_encoding,
		video_encoder->data_enc, video_encoder->max_frame_len);
	//

	return 1;
}

/**
 * @brief used to allocate static buffers that will hold the images
 * 
 * It is in this function that the callback format for images is decided
 */
void ph_media_video_alloc_processing_buffers(phvstream_t *vstream, pixosi nego_pix, unsigned nego_width, unsigned nego_height) {
	vstream->pix_proc_source = nego_pix;
	vstream->width_proc_source = nego_width;
	vstream->height_proc_source = nego_height;

	// used for storing the local capture	
	vstream->local_frame_cache = pix_alloc(nego_pix, nego_width, nego_height);

	// needed when the capture needs resizing before sending on the TX
	vstream->image_ready_for_network = pix_alloc(PIX_OSI_YUV420P, PHMEDIA_VIDEO_FRAME_WIDTH, PHMEDIA_VIDEO_FRAME_HEIGHT);

    // buffer for the local capture event sent to the registered gui clients
	vstream->frame_event.frame_local = pix_alloc(PIX_OSI_YUV420P, nego_width, nego_height);

    // buffer for the remote image event sent to the registered gui clients
	vstream->frame_event.frame_remote = pix_alloc(PIX_OSI_YUV420P, PHMEDIA_VIDEO_FRAME_WIDTH, PHMEDIA_VIDEO_FRAME_HEIGHT);

	vstream->image_wrong_pix = pix_alloc(PIX_OSI_YUV420P, nego_width, nego_height);
}

void ph_media_video_check_processing_buffers(phvstream_t *vstream, pixosi pix_source, unsigned width_source, unsigned height_source){
	if ((width_source != vstream->width_proc_source) || (height_source != vstream->height_proc_source) || (pix_source != vstream->pix_proc_source))
	{
		ph_media_video_free_processing_buffers(vstream);
		ph_media_video_alloc_processing_buffers(vstream, pix_source, width_source, height_source);
	}
}

void ph_media_video_free_processing_buffers(phvstream_t *vstream){
		// NOTE: the 2 following in-place buffers are "static" and should
		// be deleted by the client
		//pix_free(vstream->frame_event.frame_local);
		//pix_free(vstream->frame_event.frame_remote);
		pix_free(vstream->image_ready_for_network);
		pix_free(vstream->local_frame_cache);
		pix_free(vstream->image_wrong_pix);
}


/**
 * @brief Video stream start function
 *
 * This function starts the webcam, creates a rtp session (tunneled or not),
 * and setups various things like frame & rtp callbacks.
 *
 * It also starts the video thread, which is responsible for handling frames
 * coming on the rtp socket.
 *
 * @param ca		The call the stream is to be started on
 * @param video_port	The port number the rtp socket will listen on
 * @param frameDisplay	Frame callback, called when a frame is available
 * @return		0 if ok
 */

#define LOCAL_YUV_FILE	"nowebcam.yuv"

int ph_msession_video_start(struct ph_msession_s *s, const char *deviceid)
{
	RtpSession *video_session;
	RtpProfile *profile;
	phvstream_t *video_stream;
	phcodec_t *codec;
	// vars the onewaycam frame server :
	int fs_x, fs_y, yuv_local_len;
	uint8_t *fs_buffer;
	FILE *f_local;
	ph_mstream_params_t *sp = &s->streams[PH_MSTREAM_VIDEO1];


	DBG_MEDIA_ENGINE("MEDIA ENGINE: ph_msession_video_start devid=%s\n", deviceid);

	if ((s->newstreams & (1 << PH_MSTREAM_VIDEO1)))
	{
		return 0;
	}

	if (!sp->localport || !sp->remoteport)
	{
		return 0;
	}

	DBG_MEDIA_ENGINE("Starting video stream from port: %d to %s:%d\n",
		sp->localport, sp->remoteaddr, sp->remoteport);

	// the function is cut into 2 branches :
	//   - branch1: video stream is already open (RE-INVITE for example)
	//   - branch2: video stream is not already open

	// begin branch1
	if (sp->streamerData)
	{
		DBG_MEDIA_ENGINE("ph_msession_video_start: reuse a current media stream\n");
		video_stream = (phvstream_t*) sp->streamerData;
		if (video_stream->ms.remote_port == sp->remoteport)
		{
			if (!strcmp(video_stream->ms.remote_ip, sp->remoteaddr))
			{
				return 0;
			}
		}

		strcpy(video_stream->ms.remote_ip, sp->remoteaddr);
		video_stream->ms.remote_port = sp->remoteport;
		rtp_session_reset(video_stream->ms.rtp_session);

		{
			rtp_session_set_remote_addr(video_stream->ms.rtp_session,
				video_stream->ms.remote_ip,
				video_stream->ms.remote_port);
		}

		return 0;

	}
	// end branch1

	DBG_MEDIA_ENGINE("ph_msession_video_start: create/init a new media stream\n");
	// begin branch2
	profile = &av_profile;

	// we should be able to find a codec structure based on the negociated video payload
	if (!sp->ipayloads[0].number)
	{
		return -1;
	}

	codec = ph_media_lookup_codec(sp->ipayloads[0].number);
	if (!codec) {
		return -1;
	}

	// init phase : a stream structure is created and initialized
	video_stream = (phvstream_t *)osip_malloc(sizeof(phvstream_t));
	memset(video_stream, 0, sizeof(phvstream_t));

	gettimeofday(&video_stream->stat_ts_decoded_over_5s, 0);
	osip_list_init(&video_stream->rr_sent_q);
	osip_list_init(&video_stream->webcam_frames_q);
	osip_list_init(&video_stream->received_packets_q);

	video_stream->ms.codec = codec;
	video_stream->rtpCallback = &phmedia_video_rtpsend_callback;
	video_stream->mtx = osip_mutex_init();

	if (codec->encoder_init)
	{
		video_stream->ms.encoder_ctx = codec->encoder_init(video_stream);
		if (!video_stream->ms.encoder_ctx) {
			DBG_MEDIA_ENGINE_VIDEO("unable to initialize video encoder\n");
			osip_free(video_stream);
			return -1;
		}
	}

	if (codec->decoder_init)
	{
		video_stream->ms.decoder_ctx = codec->decoder_init(video_stream);
		if (!video_stream->ms.decoder_ctx) {
			DBG_MEDIA_ENGINE_VIDEO("unable to initialize video decoder\n");
			osip_free(video_stream);
			return -1;
		}
	}

#ifdef PHAPI_VIDEO_LOCAL_HACK
	strcpy(sp->remoteaddr, "127.0.0.1");
	sp->remoteport = video_port;
	sp->flags &= ~PH_MSTREAM_FLAG_TUNNEL;
#endif

	video_session = rtp_session_new(RTP_SESSION_SENDRECV);

	video_session->rcv.pt = sp->ipayloads[0].number;

	rtp_session_set_scheduling_mode(video_session, SCHEDULING_MODE);
	rtp_session_set_blocking_mode(video_session, BLOCKING_MODE);

	rtp_session_set_profile(video_session, profile);
	rtp_session_set_jitter_compensation(video_session, 0);
	
	video_session->rtp.max_rq_size = 4096;


	// Julien Wengo - Set external transport functions to the ORTP session
	//rtp_session_set_local_addr(session, "0.0.0.0", sp->localport);
	//rtp_session_set_remote_addr(session, sp->remoteaddr,sp->remoteport);
	{
		OWSLSocket rtp_sock, rtcp_sock;
		RtpTransport *rtp_transport, *rtcp_transport;
		ph_rtp_transport_data_t *user_data;
		struct sockaddr_in laddr_rtp, laddr_rtcp, raddr_rtp, raddr_rtcp;

		// RTP socket
		if ((rtp_sock = owsl_socket(OWSL_AF_IPV4, OWSL_MODE_DATAGRAM, OWSL_CIPHERING_DISABLED)))
		{
			laddr_rtp.sin_addr.s_addr = inet_addr("0.0.0.0");
			laddr_rtp.sin_port = htons((short)sp->localport);
			laddr_rtp.sin_family = AF_INET;

			if (owsl_bind(rtp_sock, (struct sockaddr *) &laddr_rtp, sizeof(laddr_rtp)))
			{
				owsl_close(rtp_sock);
				return -1;
			}

			raddr_rtp.sin_addr.s_addr = inet_addr(sp->remoteaddr);
			raddr_rtp.sin_port = htons((short)sp->remoteport);
			raddr_rtp.sin_family = AF_INET;

			if (owsl_connect(rtp_sock, (struct sockaddr *) &raddr_rtp, sizeof(raddr_rtp)))
			{
				owsl_close(rtp_sock);
				return -1;
			}

			owsl_blocking_mode_set(rtp_sock, OWSL_NON_BLOCKING);
		}
		else
		{
			return -1;
		}

		// RTCP socket
		if ((rtcp_sock = owsl_socket(OWSL_AF_IPV4, OWSL_MODE_DATAGRAM, OWSL_CIPHERING_DISABLED)))
		{
			laddr_rtcp.sin_addr.s_addr = inet_addr("0.0.0.0");
			laddr_rtcp.sin_port = htons((short)sp->localport + 1);
			laddr_rtcp.sin_family = AF_INET;

			if (owsl_bind(rtcp_sock, (struct sockaddr *) &laddr_rtcp, sizeof(laddr_rtcp)))
			{
				owsl_close(rtcp_sock);
				return -1;
			}

			raddr_rtcp.sin_addr.s_addr = inet_addr(sp->remoteaddr);
			raddr_rtcp.sin_port = htons((short)sp->remoteport + 1);
			raddr_rtcp.sin_family = AF_INET;

			if (owsl_connect(rtcp_sock, (struct sockaddr *) &raddr_rtcp, sizeof(raddr_rtcp)))
			{
				owsl_close(rtcp_sock);
				return -1;
			}

			owsl_blocking_mode_set(rtcp_sock, OWSL_NON_BLOCKING);
		}
		else
		{
			return -1;
		}

		user_data = malloc(sizeof(ph_rtp_transport_data_t));
		memset(user_data, 0, sizeof(ph_rtp_transport_data_t));
		user_data->rtp_sock = rtp_sock;
		user_data->rtcp_sock = rtcp_sock;

		/* sVoIP integration */
		// SPIKE_SRTP: Check that the call is crypted, and set the callbacks
		// for the RTP session. Use externalID to give CID to the RTP functions.
		if (s && s->cbkInfo) 
		{
			phcall_t *jc;

			jc = (phcall_t*)s->cbkInfo;
			if (sVoIP_phapi_isCrypted(jc->extern_cid))
			{
				user_data->externalID = jc->extern_cid;
				user_data->rtp_post_recv_cb = sVoIP_phapi_recvRtp;
				user_data->rtp_pre_send_cb = sVoIP_phapi_sendRtp;
			}
		}
		/* sVoIP */

		rtp_transport = malloc(sizeof(RtpTransport));
		memset(rtp_transport, 0, sizeof(RtpTransport));
		rtp_transport->t_sendto = ph_rtp_transport_sendto;
		rtp_transport->t_recvfrom = ph_rtp_transport_recvfrom;
		rtp_transport->data = user_data;

		rtcp_transport = malloc(sizeof(RtpTransport));
		memset(rtcp_transport, 0, sizeof(RtpTransport));
		rtcp_transport->t_sendto = ph_rtcp_transport_sendto;
		rtcp_transport->t_recvfrom = ph_rtcp_transport_recvfrom;
		rtcp_transport->data = user_data;
		
		rtp_session_set_transports(video_session, rtp_transport, rtcp_transport);
	}

	rtp_session_set_payload_type(video_session, sp->ipayloads[0].number);

  // HACK
  // wt is always initialized because the webcam object holds the convertImage function...
  video_stream->wt = webcam_get_instance();

	// choice of the frame server and init sequence
	DBG_MEDIA_ENGINE_VIDEO("choosing type of frame server...\n");
	if (sp->traffictype & PH_MSTREAM_TRAFFIC_OUT)
	{
		DBG_MEDIA_ENGINE_VIDEO("...would like a real webcam\n");
		if (ph_media_video_initialize_webcam(video_stream))
		{
			DBG_MEDIA_ENGINE_VIDEO("...got a real webcam\n");
			video_stream->phmfs_webcam.state = 1;
		}
		else
		{
			DBG_MEDIA_ENGINE_VIDEO("...got a virtual webcam\n");
			video_stream->phmfs_onewaycam.state = 1; // init
		}
	}
	else
	{
		DBG_MEDIA_ENGINE_VIDEO("...would like and got a virtual webcam\n");
		video_stream->phmfs_onewaycam.state = 1; // init
	}


	// choice 1: start sequence of the webcam frame server
	if (video_stream->phmfs_webcam.state == 1)
	{
		DBG_MEDIA_ENGINE_VIDEO("start the real webcam engine\n");
		// create processing buffer after nego with the webcam
		ph_media_video_alloc_processing_buffers(video_stream,
				webcam_get_palette(video_stream->wt),
				webcam_get_width(video_stream->wt),
				webcam_get_height(video_stream->wt)
			);
		//

		// start the engine
		webcam_add_callback(video_stream->wt, webcam_frame_callback, (void *)video_stream);
		DBG_MEDIA_ENGINE_VIDEO("call to webcam_start_capture\n");
		webcam_start_capture(video_stream->wt);
		video_stream->phmfs_webcam.state = 2;
		//
	}

	// choice 2: start sequence of the virtual webcam frame server
	if (video_stream->phmfs_onewaycam.state == 1)
	{

		// note :	this "virtual" webcam may seem a little strange, but it facilitates NAT traversal right now
		//			and makes it possible to have only the tx or rx having a cam

		DBG_MEDIA_ENGINE_VIDEO("start the virtual webcam engine\n");

		// create processing buffer after nego with the webcam
		ph_media_video_alloc_processing_buffers(video_stream,
				PIX_OSI_YUV420P, 176, 144);
		//
		
		// create the still image for the virtual webcam
		fs_buffer = (uint8_t*)av_malloc((176*144*3)/2);
		video_stream->phmfs_onewaycam.buffer = fs_buffer;
		yuv_local_len = avpicture_get_size(PIX_FMT_YUV420P, 176, 144);
		//memcpy(video_stream->phmfs_onewaycam.buffer, pic_yuv, yuv_local_len);
		memset(video_stream->phmfs_onewaycam.buffer, NOWEBCAM_FILL_VALUE, yuv_local_len);
		//
		
		/*
		// just as an example of a directly generated YUV kind of rainbow
		for(fs_y=0;fs_y<144;fs_y++) {
		for(fs_x=0;fs_x<176;fs_x++) {
		fs_buffer[fs_y * 176 + fs_x] = 128;
		}
		}
		for(fs_y=0;fs_y<144/2;fs_y++) {
		for(fs_x=0;fs_x<176/2;fs_x++) {
		fs_buffer[176*144 + fs_y * 176/2 + fs_x] = 255 * fs_x * 2 / 176;
		fs_buffer[176*144 + (176*144/4) + fs_y * 176/2 + fs_x] = 255 * fs_y * 2 / 144;
		}
		}
		*/

		// start the engine
		video_stream->phmfs_onewaycam.state = 2;
		//
	}

	video_stream->ms.running = 1;
	video_stream->frameDisplayCallback = s->frameDisplayCbk;
	video_stream->ms.rtp_session = video_session;
	video_stream->ms.payload = sp->ipayloads[0].number;
	video_stream->ms.mses = s;
	video_stream->ms.codec = codec;
	video_stream->mbCounter = 0;
	video_stream->rxtstamp = 0;
	sp->flags |= PH_MSTREAM_FLAG_RUNNING;
	s->activestreams |= (1 << PH_MSTREAM_VIDEO1);



	strcpy(video_stream->ms.remote_ip, sp->remoteaddr);
	video_stream->ms.remote_port = sp->remoteport;


	video_session->user_data = video_stream;
	/*
	* If the user wants us to automatically adjust the bandwidth, then do it */

	if (sp->videoconfig  == PHAPI_VIDEO_LINE_AUTOMATIC)
	{

// TODO : WENGO - replace phapi RTCP
/* 		/\* Place callbacks for RRs *\/ */
/* 		video_session->rtcp_rr_sent_cb = ph_video_rtcp_rr_sent; */
/* 		video_session->rtcp_rr_received_cb = ph_video_rtcp_rr_received; */

/* 		/\* Place callbacks for SRs *\/ */
/* 		video_session->rtcp_sr_sent_cb = ph_video_rtcp_sr_sent; */
/* 		video_session->rtcp_sr_received_cb = ph_video_rtcp_sr_received; */

		/* Start the Automatic BW control thread */
		video_stream->media_bw_control_thread = osip_thread_create(20000,
			ph_video_bwcontrol_thread, video_stream);
	}

	DBG_MEDIA_ENGINE_VIDEO("applying user defined quality settings...\n");
	ph_video_bwcontrol_apply_user_params(video_stream);

	sp->streamerData  = video_stream;

	// start the engine !
	// 40 = 1000/25 = 25 frame per second
	video_stream->ms.media_io_thread = osip_thread_create(20000,
		ph_video_io_thread, video_stream);

	return 0;
	// end branch2
}


/**
 * used to group the rtp slices falling between 2 seqnumbers and try to decode the resulting buffer.
 *
 *
 * @return 0/1 : a frame was successfully decoded and flushed to the corresponding callback
 */
int ph_media_video_flush_queue(phvstream_t *stream, unsigned long seqnumber_start, unsigned long seqnumber_end) {
	struct timeval now_time;
	ph_h263_decoder_ctx_t *video_decoder;
	AVFrame *picIn;
	phVideoFrameReceivedEvent_t *f_event;
	int len_received, len;
	int q_size, it;
	unsigned long ts_packet;
	int counter = 0;
	static AVPicture *rgb = NULL;
	struct ph_video_stream_packet *phvs;

	phcodec_t *codec = stream->ms.codec;
	video_decoder = (ph_h263_decoder_ctx_t *) stream->ms.decoder_ctx;
	picIn = video_decoder->decoder_ctx.pictureIn;

	q_size = osip_list_size(&stream->received_packets_q);

	video_decoder->buf_index = 0;

	for (it = 0; it < q_size; it += 1) {
		phvs = (struct ph_video_stream_packet *) osip_list_get(
			&stream->received_packets_q, it);

		if (!phvs)
		{
			continue;
		}

		if (phvs->seqnumber < seqnumber_start)
		{
			continue;
		}

		if (phvs->seqnumber > seqnumber_end)
		{
			continue;
		}

		if (!phvs->mp->b_cont)
		{
			continue;
		}

		len_received = phvs->mp->b_cont->b_wptr - phvs->mp->b_cont->b_rptr;

#if 0

		if (video_decoder->buf_index + len_received >= MAX_DEC_BUFFER_SIZE)
		{
			video_decoder->buf_index = 0;
			break;
		}
#endif

		if (len_received <= 4)
		{
			continue;
		}

		
		if (strcmp(codec->mime, CODEC_H263_MIME_STRING) != 0 && 
				strcmp(codec->mime, CODEC_H263P_MIME_STRING) != 0) //see H263HeaderHack comment above
		{
			memcpy(video_decoder->data_dec + video_decoder->buf_index,
							phvs->mp->b_cont->b_rptr, len_received);
			video_decoder->buf_index += len_received;
		}
		else {
			memcpy(video_decoder->data_dec + video_decoder->buf_index,
							phvs->mp->b_cont->b_rptr+4, len_received-4);
			video_decoder->buf_index += (len_received-4);
		}
		counter +=1;
	}

	for (it = q_size - 1; it >= 0; it -= 1) {
		phvs = (struct ph_video_stream_packet *) osip_list_get(
			&stream->received_packets_q, it);
		if (phvs && phvs->seqnumber <= seqnumber_end)
		{
			osip_list_remove(&stream->received_packets_q, it);
			freemsg(phvs->mp);
			free(phvs);
		}

	}

	/*
	 * Time to check if we didn't overflow the fps set on the encoder
	 *
	 */
	if (video_decoder->buf_index != 0)
	{
		len = codec->decode(video_decoder, video_decoder->data_dec,
			video_decoder->buf_index, picIn,
			38016);

		if (!len)
		{
      		DBG_MEDIA_ENGINE("Can't decode !\n");
			return 0;
		}
		else
		{
			stream->stat_num_decoded_total_over_5s++;
			gettimeofday(&now_time, 0);
			ph_tvsub(&now_time, &stream->last_decode_time);
			if (picIn->pict_type != FF_I_TYPE && (now_time.tv_usec / 1000) < stream->fps_interleave_time)
			{
				stream->stat_num_decoded_dropped_over_5s++;
				return 0;
			}
			gettimeofday(&stream->last_decode_time, 0);

			gettimeofday(&now_time, 0);
      if ((now_time.tv_sec-stream->stat_ts_decoded_over_5s.tv_sec) >= 5)
      {
        DBG_MEDIA_ENGINE("decoder over last 5 sec: total fps: %d, callback fps: %d\n",
          stream->stat_num_decoded_total_over_5s/5, (stream->stat_num_decoded_total_over_5s - stream->stat_num_decoded_dropped_over_5s)/5);
        stream->stat_num_decoded_total_over_5s = 0;
        stream->stat_num_decoded_dropped_over_5s = 0;
        gettimeofday(&stream->stat_ts_decoded_over_5s, 0);
      }

			pix_convert_avpicture(PIX_NO_FLAG, stream->frame_event.frame_remote, picIn, PIX_OSI_YUV420P);
			pix_convert((phcfg.video_config.video_camera_flip_frame)?PIX_FLIP_HORIZONTALLY:PIX_NO_FLAG, stream->frame_event.frame_local, stream->local_frame_cache);

			stream->frameDisplayCallback(stream->ms.mses->cbkInfo, &stream->frame_event);

			return 1;

		}

	}

	return 0;
}

/**
 * @brief Tries to read queued video rtp packets from the socket
 *
 * This function reads all awaiting packets on the video rtp session
 * and tries to generate a complete data packet.
 *
 * We need to explain the (simple) algorithm. To send a video frame on a
 * rtp socket, we have to fragment it into multi packets. FFmpeg handles
 * this part by slicing each packet into 500 bytes chunks. The last packet
 * of a serie has its rtp markbit set to 1. We read and copy packets data
 * till we get the markbit. Once we have it, we pass the whole buffer to
 * the decoder, and call the frame display callback (typically set by
 * the user of the PHAPI stack
 *
 * @param stream		The stream to read on
 * @param timestamp		The RTP timestamp to use on recv()
 * @param ts_inc		The ts_inc, to be set to the next ts incr
 *
 */

void
ph_handle_video_network_data(phvstream_t *stream, unsigned long timestamp,
			unsigned long *ts_inc)
{
	struct ph_video_stream_packet *phvs;

	unsigned long rx_observed_timestamp;
	unsigned long rx_observed_seqnumber;
	int rx_observed_markbit;
	mblk_t *mp;
	int flushed;

	*ts_inc = 0;
	while ((stream->ms.running)&&((mp = rtp_session_recvm_with_ts(stream->ms.rtp_session,timestamp))!=NULL))
	{


		if (mp->b_cont)
		{
			rx_observed_timestamp = rtp_get_timestamp(mp);
			rx_observed_seqnumber = rtp_get_seqnumber(mp);
			rx_observed_markbit = rtp_get_markbit(mp);

			if (stream->rx_trace_initialize == 0)
			{
				stream->rx_trace_initialize = 1;
				stream->rx_current_timestamp = rx_observed_timestamp;
				stream->rx_current_seqnumber = rx_observed_seqnumber;
			}

			if (rx_observed_timestamp < stream->rx_current_timestamp) {
				continue;
			}

			phvs = (struct ph_video_stream_packet *) malloc(sizeof(struct ph_video_stream_packet));
			phvs->mp = mp;
			phvs->timestamp = rx_observed_timestamp;
			phvs->seqnumber = rx_observed_seqnumber;
			osip_list_add(&stream->received_packets_q, phvs, -1);

			if (rx_observed_markbit)
			{
				flushed = ph_media_video_flush_queue(stream, stream->rx_current_seqnumber, rx_observed_seqnumber);
				stream->rx_trace_initialize = 0;
				*ts_inc = 90000;
				stream->stat_markbit_hit +=1;
				//printf("frame decode hit: (markbit: %d), (ts: %d)\n", stream->stat_markbit_hit, stream->stat_timestamp_hit);
				return;
			}

			// beware of this algorithm with clients that put a different ts on each slice (!)
			if (!rx_observed_markbit && (rx_observed_timestamp > stream->rx_current_timestamp))
			{
				flushed = ph_media_video_flush_queue(stream, stream->rx_current_seqnumber, rx_observed_seqnumber-1);
				stream->rx_current_timestamp = rx_observed_timestamp;
				stream->rx_current_seqnumber = rx_observed_seqnumber;
				*ts_inc = 90000;
				stream->stat_timestamp_hit +=1;
				return;
			}

		}

	}

}

void ph_video_handle_data(phvstream_t *stream) {
	unsigned long rxts_inc = 0;
	int q_size, it;
	phm_videoframe_t *phmvf;
	phm_videoframe_t *fs_phmvf;

	stream->threadcount_encoding += 1;

	if (!stream->ms.running)
	{
		return;
	}

	// lock mutex : mainly used to protect the webcam_frames_q
#ifdef PH_VIDEO_USELOCK
	osip_mutex_lock(stream->mtx);
#endif

	// pull data from the rx rtp stream and push them to consumers (gui,..)

	ph_handle_video_network_data(stream, stream->rxtstamp,
		&rxts_inc);

	// adjust the rtp timestamp synchro for the next pull
	stream->rxtstamp += rxts_inc;

	// if the 'onewaycam' frame server is activated, it must produce a frame now
	if ((stream->phmfs_onewaycam.state == 2) && (stream->threadcount_encoding%15==0))
	{
		fs_phmvf = (phm_videoframe_t*) malloc(sizeof(phm_videoframe_t));
		//fs_phmvf->image = (piximage *)malloc(sizeof(piximage));
		fs_phmvf->image.data = stream->phmfs_onewaycam.buffer;
		fs_phmvf->image.width = 176;
		fs_phmvf->image.height = 144;
		fs_phmvf->image.palette = PIX_OSI_YUV420P;

		osip_list_add(&stream->webcam_frames_q, fs_phmvf, -1);

	}


	// iterate over the available local frames in order to flush them into to the encoder
	q_size = osip_list_size(&stream->webcam_frames_q);
	if (q_size > 1)
	{
    	DBG_MEDIA_ENGINE("webcam_frames_q overrun - %d\n", q_size);
	}
	for (it = 0; it < q_size; it += 1) {
		phmvf = (phm_videoframe_t *) osip_list_get(&stream->webcam_frames_q, 0);
		if (!phmvf)
		{
			continue;
		}
		if (it == (q_size-1))
		{
			//printf("calling send_frame\n");
			ph_media_video_send_frame(stream, phmvf, 1);
		}

		ph_media_free_video_frame(phmvf);

		osip_list_remove(&stream->webcam_frames_q, 0);
	}

#ifdef PH_VIDEO_USELOCK
	// unlock mutex that was previously locked
	osip_mutex_unlock(stream->mtx);
#endif
}


/**
 * @brief Stops the video stream
 *
 * This is called by ph_media_stop() when the call is finished and that video
 * has been negotiated and started. It stops the webcam driver, waits for
 * thread to finish, and destroys the rtp session
 *
 * @param ca		The call on which we need to stop the video stream
 *
 */

void ph_msession_video_stop(struct ph_msession_s *s)
{
  struct ph_mstream_params_s *msp = &s->streams[PH_MSTREAM_VIDEO1];
  phvstream_t *stream = (phvstream_t *) msp->streamerData;

  if (!(s->activestreams & (1 << PH_MSTREAM_VIDEO1)))
  {
      return;
  }

  s->activestreams &= ~(1 << PH_MSTREAM_VIDEO1);

  if (!stream)
  {
    return;
  }

  stream->ms.running = 0;
  msp->streamerData = 0;
  msp->flags &= ~PH_MSTREAM_FLAG_RUNNING;

  // if a thread was needed in the threading model, wait and destroy it
  if (stream->ms.media_io_thread)
  {
	osip_thread_join(stream->ms.media_io_thread);
	osip_free(stream->ms.media_io_thread);
	stream->ms.media_io_thread = 0;
  }
    
  if (msp->videoconfig == PHAPI_VIDEO_LINE_AUTOMATIC)
  {
    osip_thread_join(stream->media_bw_control_thread);
  }

  DBG_MEDIA_ENGINE_VIDEO("call to webcam_release\n");
  webcam_release(stream->wt);
  stream->phmfs_webcam.state = 0;
  stream->wt = 0;

  if (stream->phmfs_onewaycam.state == 2)
  {
    av_free(stream->phmfs_onewaycam.buffer);
    stream->phmfs_onewaycam.state = 0;
  }

  if (stream->ms.codec->encoder_cleanup)
  {
    stream->ms.codec->encoder_cleanup(stream->ms.encoder_ctx);
  }

  if (stream->ms.codec->decoder_cleanup)
  {
    stream->ms.codec->decoder_cleanup(stream->ms.decoder_ctx);
  }

  ph_media_video_free_processing_buffers(stream);

  // WENGO
  ortp_set_log_file(stdout);
  //ortp_session_stats_display(stream->ms.rtp_session);
  rtp_stats_display(&stream->ms.rtp_session->rtp.stats, "Session statistics");
  ortp_set_log_file(NULL);

  // Wengo Julien - Add OWSL support
  if (stream->ms.rtp_session->rtp.tr)
  {
	ph_rtp_transport_data_t *user_data = stream->ms.rtp_session->rtp.tr->data;

	if (user_data)
	{
		owsl_close(user_data->rtp_sock);
		owsl_close(user_data->rtcp_sock);
	}
  }

  rtp_session_destroy(stream->ms.rtp_session);

  memset(stream, 0, sizeof(phvstream_t));
  osip_free(stream);
}


/**
* @brief Video read thread
*
* This thread is responsible for reading frames from the rtp socket and
* calling the appropriate function. Since we have no real blocking-read
* in ortp, we also have to sleep() an appropriate ammount of time.
*
* @param p             The video stream (type phvstream_t)
* @return              NULL when the thread is finished
*
*/
void *
ph_video_io_thread(void *p)
{
	phvstream_t *stream = (phvstream_t*)p;
	struct timeval elapsed_time, sleeptime, start_time, end_time, loop_time;
	struct timespec sleepns;

	loop_time.tv_sec = 0;
	loop_time.tv_usec = 40000;
	
	while (stream->ms.running) {

		gettimeofday(&start_time, 0);

		if (!stream->ms.running)
		{
			break;
		}
		
		ph_video_handle_data(stream);
		
		gettimeofday(&end_time, 0);

		ph_timeval_substract (&elapsed_time, &end_time, &start_time);
		if (!ph_timeval_substract (&sleeptime, &loop_time, &elapsed_time))
		{
		
			TIMEVAL_TO_TIMESPEC(&sleeptime, &sleepns);
#ifdef WIN32
			Sleep(sleepns.tv_sec * 1000 + sleepns.tv_nsec / 1000000);
#else
			nanosleep(&sleepns, 0);
#endif
		}
	}
	
return NULL;
}
