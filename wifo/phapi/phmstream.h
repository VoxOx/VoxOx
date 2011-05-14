#ifndef __PHMSTREAM_H__
#define __PHMSTREAM_H__

#include <wtimer.h>
#include "phcodec.h"

#ifdef __cplusplus
extern "C"
{
#endif


#if !defined(WIN32)
#define TYPE(val) .type = (val)
#define CLOCK_RATE(val) .clock_rate = (val)
#define BITS_PER_SAMPLE(val) .bits_per_sample = (val)
#define ZERO_PATTERN(val)   .zero_pattern = (val)
#define PATTERN_LENGTH(val) .pattern_length = (val)
#define NORMAL_BITRATE(val) .normal_bitrate = (val)
#define MIME_TYPE(val) .mime_type = (val)
#else
#define TYPE(val) (val)
#define CLOCK_RATE(val) (val)
#define BITS_PER_SAMPLE(val) (val)
#define ZERO_PATTERN(val)   (val)
#define PATTERN_LENGTH(val) (val)
#define NORMAL_BITRATE(val)  (val)
#define MIME_TYPE(val)  (val)
#endif

struct ph_msession_s;
struct phcall_s;
struct phmstream
{
  struct _RtpSession *rtp_session;
  struct ph_msession_s *mses;
 
  phcodec_t *codec;
  void      *encoder_ctx;
  void      *decoder_ctx;

  char  remote_ip[32];
  int	remote_port;

  int ptime;

  int payload;
  int running;
  int suspended;
  int lastsuspended;

  unsigned long rxtstamp;
  unsigned long txtstamp;
  unsigned long rxts_inc;

  /** used only for video. TODO: replace with io_timer */
  struct osip_thread *media_io_thread;

  /** used only for audio. TODO: share with video */
  struct timer_impl * media_io_timer_impl;
  struct timer *media_io_timer;

  void      (*endCallback)(void *ca, int arg);
};
typedef struct phmstream phmstream_t;

extern int ph_media_use_tunnel;
phcodec_t *ph_media_lookup_codec(int payload);
int ph_media_get_clock_rate(int payload);


void ph_tvsub(register struct timeval *out, register struct timeval *in);
void ph_tvdiff(struct timeval *diff, struct timeval *out, struct timeval *in);


#if defined(WIN32) && !defined(HAVE_STRUCT_TIMESPEC)
struct timespec {
	long tv_sec;
	long tv_nsec;
};
#define HAVE_STRUCT_TIMESPEC
#endif

#ifndef TIMEVAL_TO_TIMESPEC
#define TIMEVAL_TO_TIMESPEC(tv, ts) {                                   \
        (ts)->tv_sec = (tv)->tv_sec;                                    \
	        (ts)->tv_nsec = (tv)->tv_usec * 1000;                           \
}
#endif /*TIMEVAL_TO_TIMESPEC*/

#ifndef TIMESPEC_TO_TIMEVAL
#define TIMESPEC_TO_TIMEVAL(tv, ts) {                                   \
        (tv)->tv_sec = (ts)->tv_sec;                                    \
	        (tv)->tv_usec = (ts)->tv_nsec / 1000;                           \
}
#endif /*TIMESPEC_TO_TIMEVAL*/

#ifdef __cplusplus
}
#endif


void ph_msession_video_stop(struct ph_msession_s *s);
int  ph_msession_video_start(struct ph_msession_s *s, const char *device);
void ph_msession_audio_stop(struct ph_msession_s *s, const char *device);
int  ph_msession_audio_start(struct ph_msession_s *s, const char *device);
int ph_msession_audio_resume(struct ph_msession_s *s, int resumewhat, const char *deviceId);
void ph_msession_audio_suspend(struct ph_msession_s *s, int suspendwhat, const char *deviceId);
int ph_msession_audio_conf_start(struct ph_msession_s *s1, struct ph_msession_s *s2, const char *deviceId);
int ph_msession_audio_conf_stop(struct ph_msession_s *s1, struct ph_msession_s *s2);

#if 0
int ph_media_audio_start(phcall_t *ca, int port,
				   void (*dtmfCallback)(phcall_t *ca, int event), 
				   void (*endCallback)(phcall_t *ca, int event),
			 const char * deviceId, unsigned vad, int cng, int jitter, int noaec);



void ph_media_audio_stop(phcall_t *ca);

#endif
#endif

