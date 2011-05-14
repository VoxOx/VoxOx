/*
 * phmedia -  Phone Api media streamer
 *
 * Copyright (C) 2005-2006 WENGO SAS
 * Copyright (C) 2004 Vadim Lebedev <vadim@mbdsys.com>
 * Copyright (C) 2002,2003   Aymeric Moizard <jack@atosc.org>
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

#include "phglobal.h"
#include "phlog.h"
#include <osip2/osip_mt.h>
#include <osip2/osip.h>
#ifdef OS_POSIX
#include <sys/ioctl.h>
#include <sys/time.h>
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <ortp.h>
#include <telephonyevents.h>
#include "phapi.h"
#include "phcall.h"
#include "phmedia.h"

#include "phcodec.h"
#include "phmstream.h"

#ifdef _MSC_VER
#define strncasecmp strnicmp
#endif


int ph_transport_common_sendto(ph_rtp_transport_data_t *data, OWSLSocket sock, 
							   const void *buffer, size_t len, int flags, 
							   const struct sockaddr *to, socklen_t tolen)
{
	int error = 0;
	char * buf_tmp;
	int len_tmp, sent;

	buf_tmp = malloc(len);
	memcpy(buf_tmp, buffer, len);
	len_tmp = len;

	if (data->rtp_pre_send_cb)
	{
		data->rtp_pre_send_cb(data->externalID, &error, buf_tmp, &len_tmp);
	}

	sent = owsl_send(sock, buf_tmp, len_tmp, 0);

	free(buf_tmp);

	return len_tmp == sent ? len : -1;
}

int ph_rtp_transport_sendto(struct _RtpTransport *t, const void *buffer, size_t len, int flags,
							const struct sockaddr *to, socklen_t tolen)
{
	ph_rtp_transport_data_t *data = t->data;

	if (!data)
	{
		return -1;
	}

	return ph_transport_common_sendto(data, data->rtp_sock, buffer, len, flags, to, tolen);
}

int ph_rtcp_transport_sendto(struct _RtpTransport *t, const void *buffer, size_t len, int flags,
							 const struct sockaddr *to, socklen_t tolen)
{
	ph_rtp_transport_data_t *data = t->data;

	if (!data)
	{
		return -1;
	}

	return ph_transport_common_sendto(data, data->rtcp_sock, buffer, len, flags, to, tolen);
}

int ph_transport_common_recvfrom(ph_rtp_transport_data_t *data, OWSLSocket sock,
								 void *buffer, size_t len, int flags, 
								 struct sockaddr *from, socklen_t *fromlen)
{
	int i = 0;
	int ret = 0;
	int error = 0;
	
	i = owsl_recv(sock, buffer, len, 0);
	if (i < 1)
	{
		return 0;
	}

	if (data->rtp_post_recv_cb)
	{
		data->rtp_post_recv_cb(data->externalID, &error, buffer, &i);
	}

	return i;
}

int ph_rtp_transport_recvfrom(struct _RtpTransport *t, void *buffer, size_t len, int flags, 
							  struct sockaddr *from, socklen_t *fromlen)
{
	ph_rtp_transport_data_t *data = t->data;

	if (!data)
	{
		return -1;
	}

	return ph_transport_common_recvfrom(data, data->rtp_sock, buffer, len, flags, from, fromlen);
}

int ph_rtcp_transport_recvfrom(struct _RtpTransport *t, void *buffer, size_t len, int flags, 
							   struct sockaddr *from, socklen_t *fromlen)
{
	ph_rtp_transport_data_t *data = t->data;

	if (!data)
	{
		return -1;
	}

	return ph_transport_common_recvfrom(data, data->rtcp_sock, buffer, len, flags, from, fromlen);
}

int ph_media_cleanup()
{
  return 0;
}

int ph_msession_start(struct ph_msession_s *s, const char *deviceid)
{
  int ret1,ret2 = -1;

  DBG_MEDIA_ENGINE("MEDIA_ENGINE: entering ph_msession_start\n");

  pthread_mutex_lock(&s->critsec_mstream_init);

  ret1 = ph_msession_audio_start(s, deviceid);
#ifdef PHAPI_VIDEO_SUPPORT
  ret2 = ph_msession_video_start(s, deviceid);
#endif

  pthread_mutex_unlock(&s->critsec_mstream_init);

  if (!ret1 && !ret2)
  {
    return 0;
  }

  return ret1 ? ret1 : ret2;
}

int ph_msession_conf_start(struct ph_msession_s *s1, struct ph_msession_s *s2, const char *device)
{
  int ret;
  DBG_MEDIA_ENGINE("MEDIA_ENGINE: entering ph_msession_conf_start\n");
  pthread_mutex_lock(&s1->critsec_mstream_init);
  pthread_mutex_lock(&s2->critsec_mstream_init);
  ret = ph_msession_audio_conf_start(s1, s2, device);
  pthread_mutex_unlock(&s2->critsec_mstream_init);
  pthread_mutex_unlock(&s1->critsec_mstream_init);
  return ret;
}

int ph_msession_conf_stop(struct ph_msession_s *s1, struct ph_msession_s *s2)
{
  int ret;
  DBG_MEDIA_ENGINE("MEDIA_ENGINE: entering ph_msession_conf_stop\n");
  pthread_mutex_lock(&s1->critsec_mstream_init);
  pthread_mutex_lock(&s2->critsec_mstream_init);
  ret = ph_msession_audio_conf_stop(s1, s2);
  pthread_mutex_unlock(&s2->critsec_mstream_init);
  pthread_mutex_unlock(&s1->critsec_mstream_init);
  return ret;
}


int ph_msession_suspend(struct ph_msession_s *s,  int traffictype, const char *device)
{
  DBG_MEDIA_ENGINE("MEDIA_ENGINE: entering ph_msession_suspend\n");
  pthread_mutex_lock(&s->critsec_mstream_init);
#ifdef PHAPI_VIDEO_SUPPORT
  ph_msession_video_stop(s);
#endif
  ph_msession_audio_suspend(s, traffictype, device);
  pthread_mutex_unlock(&s->critsec_mstream_init);
  return (0);
}

/**
 * @brief resume a media session
 */
int ph_msession_resume(struct ph_msession_s *s, int traffictype, const char *device)
{
  int ret = 0;

  DBG_MEDIA_ENGINE("MEDIA_ENGINE: entering ph_msession_resume\n");
  pthread_mutex_lock(&s->critsec_mstream_init);

  ret = ph_msession_audio_resume(s, traffictype, device);
  if (ret == 0)
  {
#ifdef PHAPI_VIDEO_SUPPORT
    ret = ph_msession_video_start(s, "");
#endif
  }

  pthread_mutex_unlock(&s->critsec_mstream_init);
  return ret;
}

int ph_msession_stopped(struct ph_msession_s *s)
{
	return (s->activestreams == 0);
}

int ph_msession_audio_stopped(struct ph_msession_s *s)
{
	return !(s->activestreams & (1 << PH_MSTREAM_AUDIO1));
}

int ph_msession_video_stopped(struct ph_msession_s *s)
{
	return !(s->activestreams & (1 << PH_MSTREAM_VIDEO1));
}

void ph_msession_stop(struct ph_msession_s *s, const char *adevid)
{
	DBG_MEDIA_ENGINE("MEDIA_ENGINE: entering ph_msession_stop\n");
	pthread_mutex_lock(&s->critsec_mstream_init);
	
#ifdef PHAPI_VIDEO_SUPPORT
	if (!ph_msession_video_stopped(s))
	{
		ph_msession_video_stop(s);
	}
#endif
	if (!ph_msession_audio_stopped(s))
	{
		ph_msession_audio_stop(s, adevid);
	}
	
	pthread_mutex_unlock(&s->critsec_mstream_init);
}

static PayloadType ilbc =
{
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(8000),
  BITS_PER_SAMPLE( 0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH( 0),
  NORMAL_BITRATE( 13330),
  MIME_TYPE ("ILBC")
};

static PayloadType amr={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(8000),
  BITS_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(12000),    /* set mode MR 122 as default */
  MIME_TYPE ("AMR")
};

static PayloadType amr_wb={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(16000),
  BITS_PER_SAMPLE(320),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(24000),   /* set mode 2385 as default */
  MIME_TYPE ("AMR-WB")
};

static PayloadType cng_8={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(8000),
  BITS_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(8000),
  MIME_TYPE ("CN")
};

static PayloadType g726={
    PAYLOAD_AUDIO_PACKETIZED,
    CLOCK_RATE(8000),
    BITS_PER_SAMPLE(0),
    ZERO_PATTERN(NULL),
    PATTERN_LENGTH(0),
    NORMAL_BITRATE(32000),
    MIME_TYPE ("G726-32")
};

static PayloadType g722={
    PAYLOAD_AUDIO_PACKETIZED,
    CLOCK_RATE(16000),
    BITS_PER_SAMPLE(0),
    ZERO_PATTERN(NULL),
    PATTERN_LENGTH(0),
    NORMAL_BITRATE(60000),
    MIME_TYPE ("G722")
};


static PayloadType cng_16={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(16000),
  BITS_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(8000),
  MIME_TYPE ("CN")
};

static PayloadType mspeex_nb={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(8000),
  BITS_PER_SAMPLE(320),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(15000),    /* set mode MR 122 as default */
  MIME_TYPE ("SPEEX")
};

static PayloadType mspeex_wb={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(16000),
  BITS_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(28000),
  MIME_TYPE ("SPEEX")
};

static PayloadType g726_wb={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(16000),
  BITS_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(28000),
  MIME_TYPE ("G726-64wb")
};

static PayloadType h263p={
  PAYLOAD_VIDEO,
  90000,
  0,
  NULL,
  0,
  256000,
  CODEC_H263P_MIME_STRING
};

static PayloadType h263flv1={
  PAYLOAD_VIDEO,
  90000,
  0,
  NULL,
  0,
  256000,
  CODEC_FLV1_MIME_STRING
};

static PayloadType h264={
  PAYLOAD_VIDEO,
  90000,
  0,
  NULL,
  0,
  256000,
  CODEC_H264_MIME_STRING
};

static PayloadType mpeg4 ={
  PAYLOAD_VIDEO,
  90000,
  0,
  NULL,
  0,
  256000,
  "MP4V-ES"
};

int
ph_media_init(const char *pluginpath)
{
  static int first_time = 1;
  RtpProfile *profile;

  if (!first_time)
  {
      return 0;
  }

  ortp_init();
  ortp_set_log_level_mask(0);
  ph_media_audio_init();
  ph_media_codecs_init(pluginpath);

  /* initialize audio payloads (ortp needs this) */
  profile = &av_profile;
  rtp_profile_set_payload(profile,PH_MEDIA_DTMF_PAYLOAD, &payload_type_telephone_event);

  rtp_profile_set_payload(profile,PH_MEDIA_ILBC_PAYLOAD, &ilbc);//VOXOX CHANGE by ASV 06_22_2009: eliminated the ilbc macro to work on Mac

  rtp_profile_set_payload(profile,PH_MEDIA_G722_PAYLOAD, &g722);
  rtp_profile_set_payload(profile,PH_MEDIA_G726_PAYLOAD, &g726);
#ifdef ENABLE_AMR
  rtp_profile_set_payload(profile,PH_MEDIA_AMR_PAYLOAD, &amr);
#endif

#ifdef ENABLE_AMR_WB
  rtp_profile_set_payload(profile,PH_MEDIA_AMR_WB_PAYLOAD, &amr_wb);
#endif

  rtp_profile_set_payload(profile,PH_MEDIA_CN_PAYLOAD, &cng_8);
  rtp_profile_set_payload(profile,PH_MEDIA_SPEEXNB_PAYLOAD, &mspeex_nb);
  rtp_profile_set_payload(profile,PH_MEDIA_SPEEXWB_PAYLOAD, &mspeex_wb);
  rtp_profile_set_payload(profile,PH_MEDIA_G726WB_PAYLOAD, &g726_wb);

#ifdef PHAPI_VIDEO_SUPPORT
  rtp_profile_set_payload(profile,PH_MEDIA_H263P_PAYLOAD, &h263p);
  rtp_profile_set_payload(profile,PH_MEDIA_H263FLV1_PAYLOAD, &h263flv1);
  rtp_profile_set_payload(profile,PH_MEDIA_H264_PAYLOAD, &h264);
  rtp_profile_set_payload(profile,PH_MEDIA_MPEG4_PAYLOAD, &mpeg4);
#endif

  //ortp_scheduler_init();
  ortp_set_log_file(NULL);

  first_time = 0;
  return 0;
}

int ph_media_supported_payload(ph_media_payload_t *pt, const char *ptstring)
{
  PayloadType *rtppt;
  RtpProfile *profile = &av_profile;

  pt->number = rtp_profile_get_payload_number_from_rtpmap(profile, ptstring);
  if (pt->number == -1)
  {
    return 0;
  }

  rtppt = rtp_profile_get_payload(profile, pt->number);

  strncpy(pt->string, rtppt->mime_type, sizeof(pt->string));
  pt->rate = rtppt->clock_rate;

  return 1;
}

phcodec_t *ph_media_lookup_codec_bymime(const char *mime, int rate)
{
  phcodec_t *codec = ph_codec_list;
  int plen = strlen(mime);

  plen = strlen(mime);
  DBG_CODEC_LOOKUP("we need to match against : <%s/%d> in phcodec list\n", mime, rate);

  while(codec)
  {
    int mlen;

    DBG_CODEC_LOOKUP("....trying <%s/%d>\n", codec->mime, codec->clockrate);
    mlen = strlen(codec->mime);
    if(mlen == plen)
    {
      if (!strncasecmp(codec->mime, mime, mlen))
      {
        if (!codec->clockrate || !rate)
        {
          DBG_CODEC_LOOKUP("....<%s/%d> matched\n", codec->mime, codec->clockrate);
          return codec;
        }

        if (codec->clockrate == rate)
        {
          DBG_CODEC_LOOKUP("....<%s/%d> matched\n", codec->mime, codec->clockrate);
          return codec;
        }
      }
    }
    codec = codec->next;
  }

  DBG_CODEC_LOOKUP("could not find phapi codec ref <%s> in phcodec list\n", mime);
  return 0;
}

/* find a codec object corresponding to given payload */
phcodec_t *ph_media_lookup_codec(int payload)
{
  RtpProfile *profile = &av_profile;
  PayloadType *pt = rtp_profile_get_payload(profile, payload);

  DBG_CODEC_LOOKUP("CODEC LOOKUP: ph_media_lookup_codec\n");

  if (!pt)
  {
    DBG_CODEC_LOOKUP("could not find payload %d in ortp profile list\n", payload);
    return 0;
  }

  if (!pt->mime_type)
  {
    DBG_CODEC_LOOKUP("fatal error - NULL mime type for codec %d in ortp\n", payload);
    return 0;
  }

  return ph_media_lookup_codec_bymime(pt->mime_type, pt->clock_rate);
}

/*
 given the mime string of type ENCODING/RATE return TRUE if we have a codec able to handle this
 payload
*/
int ph_media_can_handle_payload(const char *mime)
{
  const char *slash;
  char mime2[64];
  int mlen;

  if (!mime)
  {
    return 0;
  }

  slash = strchr(mime, '/');
  if (!slash)
  {
    return 0 != ph_media_lookup_codec_bymime(mime, 0);
  }

  mlen = slash - mime;
  if (mlen > (sizeof(mime2) - 1))
  {
    return 0;
  }

  strncpy(mime2, mime, mlen);
  mime2[mlen] = 0;
  return 0 != ph_media_lookup_codec_bymime(mime2, atoi(slash+1));
}

void ph_tvsub(register struct timeval *out, register struct timeval *in)
{
  out->tv_usec -= in->tv_usec;

  while(out->tv_usec < 0)
  {
    --out->tv_sec;
    out->tv_usec += 1000000;
  }

  out->tv_sec -= in->tv_sec;
}

void
ph_tvdiff(register struct timeval *diff, register struct timeval *out, register struct timeval *in)
{
  diff->tv_sec = out->tv_sec;
  diff->tv_usec = out->tv_usec;

  diff->tv_usec -= in->tv_usec;

  while(diff->tv_usec < 0)
  {
    --diff->tv_sec;
    diff->tv_usec += 1000000;
  }

  diff->tv_sec -= in->tv_sec;
}

int
ph_timeval_substract (struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec)
  {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000)
  {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

int ph_media_get_clock_rate(int payload)
{
  RtpProfile *profile = &av_profile;
  PayloadType *pt;
#ifdef SPEEX_OVER_G729_HACK
  if (payload == PH_MEDIA_G729_PAYLOAD)
  {
    payload = PH_MEDIA_SPEEXWB_PAYLOAD;
  }
#endif
  pt  = rtp_profile_get_payload(profile, payload);
  return pt->clock_rate;
}


// deprecated branch of code (before msession & conf addition)
#if 0
int ph_media_start(phcall_t *ca, int port, int videoport,
           void (*dtmfCallback)(phcall_t *ca, int event),
           void (*endCallback)(phcall_t *ca, int event),
               phFrameDisplayCbk frameDisplay,
       const char * deviceId, unsigned vad, int cng, int jitter, int noaec)

{

  int ret;

  ret = ph_media_audio_start(ca, port, dtmfCallback, endCallback, deviceId, vad, cng, jitter, noaec);

#ifdef PHAPI_VIDEO_SUPPORT
  if (!ret && (frameDisplay != 0))
{
    ph_media_video_start(ca, videoport, frameDisplay);
}
  else
  {
    ph_media_video_stop(ca);
  }
#endif

  return ret;

}

void ph_media_stop(phcall_t *ca)
{
  int i;

#ifdef PHAPI_VIDEO_SUPPORT
  ph_media_video_stop(ca);
#endif
  ph_media_audio_stop(ca);
}
#endif  //if 0

void ph_clear_msession_streams_fmtps(struct ph_msession_s *s)
{
	int iStreamsIndex;
	int iPayloadsIndex;
	
	if (s)
	{
		for (iStreamsIndex = 0; 
				iStreamsIndex < PH_MSESSION_MAX_STREAMS; iStreamsIndex++)
		{
			for (iPayloadsIndex = 0; 
					iPayloadsIndex < PH_MSTREAM_PARAMS_MAX_PAYLOADS; iPayloadsIndex++)
			{
				if (s->streams[iStreamsIndex].ipayloads[iPayloadsIndex].fmtp)
				{
					osip_free(s->streams[iStreamsIndex].ipayloads[iPayloadsIndex].fmtp);
					s->streams[iStreamsIndex].ipayloads[iPayloadsIndex].fmtp = NULL;
				}
				
				if (s->streams[iStreamsIndex].opayloads[iPayloadsIndex].fmtp)
				{
					osip_free(s->streams[iStreamsIndex].opayloads[iPayloadsIndex].fmtp);
					s->streams[iStreamsIndex].opayloads[iPayloadsIndex].fmtp = NULL;
				}
			}
		}
	}
}
