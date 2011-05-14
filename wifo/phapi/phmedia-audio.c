
/*
 * phmedia -  Phone Api media streamer
 *
 * Copyright (C) 2005-2007 Wengo SAS
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
#include <wtimer.h>
#include <ortp.h>
#include <telephonyevents.h>
#include <assert.h>
#include <limits.h>


#include <svoip_phapi.h> // sVoIP
extern void sVoIP_phapi_recvRtp(int cid, void *error, void *buffer, int *len);
extern void sVoIP_phapi_sendRtp(int cid, void *error, void *buffer, int *len);

#include "phapi.h"
#include "phevents.h"
#include "phmedia.h"
#include "phcall.h" // sVoIP
#include "phrecorder.h"
#include "phcodec.h"
#include "tonegen.h"
#include "phmbuf.h"

#include "phmstream.h"
#include "phastream.h"
#include "phaudiodriver.h"
#include "owpl_log.h"

#ifdef PH_USE_RESAMPLE
#include "phresample.h"
#endif

#define DO_CONF 1

#ifdef DO_CONF
#define CONF_LOCK DTMF_LOCK
#define CONF_UNLOCK DTMF_UNLOCK
#else
#define CONF_LOCK(x)
#define CONF_UNLOCK(x)
#endif

#ifdef USE_SPXEC
#include "spxec.h"
#endif

#define RTP_SESSION_LOCK(x)
#define RTP_SESSION_UNLOCK(x)


/** Default size of jitter buffer (ms). */
const int DEFAULT_JITTER_BUFFER = 60;

/** Environment variable setting jitter buffer (ms). */
const char* JITTER_BUFFER_ENV = "PH_JITTER_BUFFER_MS";

/** Default length of echo cancellation filter. */
const char* DFLT_ECHO_LENGTH = "120";

/** Default length of echo latency (TBD!). */
const char* DFLT_ECHO_LATENCY = "120";

/** Default filename fo PH_SEND_STREAM_FILE, %%d is a running index. */
const char* DFLT_SEND_STREAM_FILE = "sendstream%d.data";

#ifdef DO_ECHO_CAN

#define ECHO_SYNC_LOCK(x) if (s->ec) pthread_mutex_lock(&s->ecmux)
#define ECHO_SYNC_UNLOCK(x) if (s->ec) pthread_mutex_unlock(&s->ecmux)

/*
#define ECHO_SYNC_LOCK(x)
#define ECHO_SYNC_UNLOCK(x)
*/

#define NO_ECHO__SUPPRESSOR 1	
#define abs(x) ((x>=0)?x:(-x))



/**
 * Get the value from an integer environment variable
 *
 * @param variable Name of environment variable,
 * @param default_val Default value (!).
 * @return Parsed value of environment variable, or 
 *  default_val if variable doesn't exist or cannot be parsed.
 *
 */
int getenv_int( char* variable, int default_value );


void *create_AEC(int softboost, int smaplingrate);
short do_AEC(void *ec, short x, short y);
void kill_AEC(void *ec);

#endif  // DO_ECHO_CAN

pthread_mutex_t ph_audio_mux;

#define PH_MSESSION_AUDIO_LOCK() pthread_mutex_lock(&ph_audio_mux)
#define PH_MSESSION_AUDIO_UNLOCK() pthread_mutex_unlock(&ph_audio_mux)
#define audio_driver_has_fds()              \
	(ph_snd_driver.snd_driver_flags & PH_SNDDRVR_FDS)


#define CNG_TBL_SIZE 128
/* table for CNG generation */

const unsigned int tab_tx_cng[CNG_TBL_SIZE]={
2238721,1995262,1778279,1584893,1412538,1258925,1122018,1000000,
891251,794328,707946,630957,562341,501187,446684,398107,
354813,316228,281838,251189,223872,199526,177828,158489,
141254,125893,112202,100000,89125,79433,70795,63096,
56234,50119,44668,39811,35481,31623,28184,25119,
22387,19953,17783,15849,14125,12589,11220,10000,
8913,7943,7079,6310,5623,5012,4467,3981,
3548,3162,2818,2512,2239,1995,1778,1585,
1413,1259,1122,1000,891,794,708,631,
562,501,447,398,355,316,282,251,
224,200,178,158,141,126,112,100,
89,79,71,63,56,50,45,40,
35,32,28,25,22,20,18,16,
14,13,11,10,9,8,7,6,
6,5,4,4,4,3,3,3,
2,2,2,2,1,1,1,0
};

/* table for CNG applying */
unsigned int tab_rx_cng[CNG_TBL_SIZE];
static short sil_pkt[MAX_FRAME_SIZE];

static int ph_speex_hook_pt = -1;  /* payload code to be replaced by SPEEX WB */ 
static int ph_trace_mic = 0;       /* when nonzero show mean MIC signal level each sec */ 

static int ph_audio_play_cbk(phastream_t *stream, void *playbuf, int playbufsize);
static int ph_generate_comfort_noice(phastream_t *stream, void *buf);

int ph_ortp_session_object_size = sizeof(RtpSession);

/* external transport functions to use with ORTP */
int ph_rtp_transport_recvfrom(struct _RtpTransport *t, void *buf, size_t len, int flags, 
							  struct sockaddr *from, socklen_t *fromlen);
int ph_rtcp_transport_recvfrom(struct _RtpTransport *t, void *buf, size_t len, int flags, 
							   struct sockaddr *from, socklen_t *fromlen);
int ph_rtp_transport_sendto(struct _RtpTransport *t, const void *buf, size_t len, int flags, 
							const struct sockaddr *from, socklen_t tolen);
int ph_rtcp_transport_sendto(struct _RtpTransport *t, const void *buf, size_t len, int flags, 
							 const struct sockaddr *from, socklen_t tolen);

// helper functions for circular buffers (cb)
static void cb_put(struct circbuf *cb, char *data, int len);
static void cb_get(struct circbuf *cb, char **chunk1, int *chunk1len,  char **chunk2, int *chhunk2len, int len);
static void cb_init(struct circbuf *cb, int size);
static void cb_clean(struct circbuf *cb);

static int select_audio_device(const char *deviceId);
static void start_audio_device(struct ph_msession_s *s, phastream_t *stream);
int ph_msession_audio_conf_link(struct ph_msession_s *s1, struct ph_msession_s *s2);

phcodec_t *ph_media_lookup_codec(int payload);
void *ph_audio_io_thread(void *_p);

#define NOISE_LEN       0x4000
#define NOISE_B_LEN     (NOISE_LEN*2)

static short noise_pattern[NOISE_LEN];
static unsigned short noise_max;

int getenv_int( char* variable, int default_value )
{
	char* string;
	long  value;
	
	string = getenv( variable);
	if( ! string){
		return default_value;
	}
	value = strtol( string, NULL, 10);
	switch ( value) {
		case LONG_MAX:
		case LONG_MIN:
			owplLogWarn( "Cannot parse %s (%s)", 
				     variable, string );
			return default_value;
			
		default:
			owplLogDebug( "Setting %s: %ld", 
				      variable, value);
			return (int) value;
	}
	
	
}


static void 
cb_put(struct circbuf *cb, char *data, int len)
{
  int free = cb->cb_siz - cb->cb_cnt;
  int chunk1max, chunk2max;

  if (len > cb->cb_siz)
  {
      len = cb->cb_siz;
  }

  if (len > free)
  {
    /*
      not enough free space in the buffer,
      we need to free it
    */
    int needmore = len - free;

    /* advance the read pointer over data we're going to override */
    cb->cb_cnt -= needmore;
    cb->cb_rdx += needmore;
    if (cb->cb_rdx >= cb->cb_siz)
    {
      cb->cb_rdx -= cb->cb_siz;
    }
  }

  chunk1max = cb->cb_siz - cb->cb_wrx;
  if (len < chunk1max)
  {
      chunk1max = len;
  }

  chunk2max = len - chunk1max;

  memcpy(cb->cb_buf+cb->cb_wrx, data, chunk1max);

  cb->cb_cnt += chunk1max;
  cb->cb_wrx += chunk1max;
  if (cb->cb_wrx == cb->cb_siz)
  {
      cb->cb_wrx = 0;
  }

  if (chunk2max <= 0)
  {
      return;
  }

  memcpy(cb->cb_buf+cb->cb_wrx, data+chunk1max, chunk2max);

  cb->cb_cnt += chunk2max;
  cb->cb_wrx += chunk2max;
  return;
}


static void 
cb_get(struct circbuf *cb, char **chunk1, int *chunk1len,  char **chunk2, int *chunk2len, int len)
{
  int chunk1max = cb->cb_siz - cb->cb_rdx;

  if (len > cb->cb_cnt)
  {
      len = cb->cb_cnt;
  }

  *chunk2 = 0;
  *chunk2len = 0;

  *chunk1 = cb->cb_buf + cb->cb_rdx;
  if (len <= chunk1max)
  {
      *chunk1len = len;
      cb->cb_cnt -= len;
      cb->cb_rdx += len;
      if (cb->cb_rdx == cb->cb_siz)
      {
          cb->cb_rdx = 0;
      }
      return;
  }

  *chunk1len = chunk1max;
  cb->cb_rdx = 0;
  cb->cb_cnt -= chunk1max;

  len -= chunk1max;

  *chunk2 = cb->cb_buf;
  *chunk2len = len;
  cb->cb_cnt -= len;
  cb->cb_rdx += len;
}

static char zeroes[] =
{
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

void 
cb_zfill(struct circbuf *cb, int len)
{
  int x;
  const int mx = sizeof(zeroes);

  while(len > 0)
  {
    x = (len > mx) ? mx : len;

    cb_put(cb, zeroes, mx);
    len -= x;
  }
}

static void 
cb_init(struct circbuf *cb, int size)
{
  memset(cb, 0, sizeof(*cb));
  cb->cb_buf = (char *) malloc(size);
  cb->cb_siz = size;
}

static void 
cb_clean(struct circbuf *cb)
{
  free(cb->cb_buf);
  memset(cb, 0, sizeof(*cb));
}


static int 
cb_read(struct circbuf *cb, char *buf, int size)
{
  char *b1, *b2;
  int l1, l2;

  cb_get(cb, &b1, &l1, &b2, &l2, size);
  if (l1)
    memcpy(buf, b1, l1);
  if (l2)
    memcpy(buf + l1, b2, l2);

  return l1+l2;
}

void ph_gen_silence()
{
  int i;
  short *p = sil_pkt;
  for(i=0; i < MAX_FRAME_SIZE; i++)
  {
    *p++ = -32767;      /* lowest value */
  }
}

static unsigned int normalize(unsigned int maxval)
{
  int i=31;
  if(maxval < 0x7FFF)
  {
    return 0;
  }
  while((1<<i & maxval) == 0) 
  {
    i--;
  }
  return(i-15);
}

/* read file to choose 'friendly' white noise */

void ph_gen_noise()
{
  int i;
  unsigned int norm;
  unsigned long sum=0;

  norm = normalize(RAND_MAX);
  DBG_DYNA_AUDIO("no NOISE file, using random normalized %u\n", norm);
  for(i=0; i<NOISE_LEN; i++)
  noise_pattern[i] =  rand()>>norm;
  for(i=0; i<NOISE_LEN; i++) 
  {
    if(noise_max < abs(noise_pattern[i]))
    {
      noise_max = abs(noise_pattern[i]);
    }
    sum += abs(noise_pattern[i]);
  }
  DBG_DYNA_AUDIO("max noise %u mean %u\n", noise_max, sum/NOISE_LEN);
}

static void
ph_on_cng_packet(RtpSession *rtp_session, mblk_t *mp, struct ph_msession_s *s)
{
  short *noise;
  char *p;
  int i;
  unsigned short factor;
  unsigned int cng_level;
  phastream_t *stream = (phastream_t *)s->streams[PH_MSTREAM_AUDIO1].streamerData;

  rtp_session; // only to avoid compilation warnings

  if(stream->ms.running && stream->cngi.cng && mp != NULL)
  {
    /* first byte contains level, following fields (if any) are discarded */
    p = mp->b_rptr + RTP_FIXED_HEADER_SIZE;
    cng_level =  tab_tx_cng[*p]>>stream->cngi.long_pwr_shift;
    if(!cng_level)
    {
      cng_level = 1;
    }
    factor = noise_max/cng_level;
    if(!factor)
    {
      factor=1;
    }

    CNG_LOCK(stream);
    if(stream->ms.running && stream->cngi.noise)
    {
      noise = (short *)stream->cngi.noise;
      for(i=0; i<NOISE_LEN; i++)
      {
        noise[i] =  noise_pattern[i]/factor;
      }
    }
    CNG_UNLOCK(stream);
    DBG_DYNA_AUDIO("PHMEDIA:got CNG %u -> %u -> %u factor %u\n",*p, tab_tx_cng[*p], cng_level, factor);
    stream->cngi.got_cng =  1;
  }
  else
  {
    DBG_DYNA_AUDIO("PHMEDIA:got CNG, discarding\n");
  }
}

/**
 * @brief clean the Voice Activity Detector structures
 */
void ph_audio_vad_cleanup(phastream_t *stream)
{
  if ( stream->cngi.pwr )
  {
    osip_free(stream->cngi.pwr);
  }

  stream->cngi.pwr = 0;

  if ( stream->cngi.cng )
  {
    CNG_LOCK(stream);

    if(stream->cngi.noise)
    {
      osip_free(stream->cngi.noise);
    }

    stream->cngi.noise = 0;

    CNG_UNLOCK(stream);
#if 0
    if ( stream->cngi.cng_lock )
	{
      pthread_mutex_destroy(&stream->cng_lock);
	}
#endif
    }
  stream->cngi.cng = 0;
}

#ifdef DO_ECHO_CAN

void do_echo_update(phastream_t *s, char *data, int len);
void store_pcm(phastream_t *s, char *buf, int len);

void store_pcm(phastream_t *s, char *buf, int len)
{

  if (!s->ec)
  {
    return;
  }

  ECHO_SYNC_LOCK(s);
  if (s->underrun)
  {
    //audio_stream_get_out_space(s, &used);
    //DBG_DYNA_AUDIO("Detected Underrun: used = %d lat = %d\n", used, s->audio_loop_latency);
    cb_zfill(&s->pcmoutbuf, s->audio_loop_latency);
    s->underrun = 0;
  }

  cb_put(&s->pcmoutbuf, buf, len);
  s->sent_cnt += len;
  ECHO_SYNC_UNLOCK(s);
  DBG_DYNA_AUDIO_ECHO("PUT read, recv, sent: %d, %d, %d\n", s->read_cnt, s->recv_cnt, s->sent_cnt);
}

#define AEC do_AEC

void do_echo_update(phastream_t *s, char *micdata, int length)
{
  char *spkchunk1, *spkchunk2;
  int  spklen1, spklen2;
  short *spkpcm1, *spkpcm2;
  short *micpcm;
  int total = 0;
  int savedlen = length/2;
#if 0
  int used = 0;
#endif
#ifdef USE_SPXEC
  char tmpspk[2048];
  char tmpmic[2048];
#else
  short tmp;
#endif // USE_SPXEC

  if (!s->ec)
  {
    return;
  }

  DBG_DYNA_AUDIO_ECHO("echo pointers: %d, %d, %d\n", 2*s->spk_current_sample - s->read_cnt, length, s->sent_cnt - 2*s->mic_current_sample);
  // echo critical section : recovering the data that was previously saved from the speaker
  ECHO_SYNC_LOCK(s);
  s->recv_cnt += length;

  cb_get(&s->pcmoutbuf, &spkchunk1, &spklen1, &spkchunk2, &spklen2, length);
  s->read_cnt += (spklen1 + spklen2);
  ECHO_SYNC_UNLOCK(s);
  DBG_DYNA_AUDIO_ECHO("GET read (just read) - recv, sent (diff): %d (%d), - %d, %d (%d)\n",
    s->read_cnt, (spklen1 + spklen2), s->recv_cnt, s->sent_cnt, s->recv_cnt - s->sent_cnt);
  if (spklen1 + spklen2 < length)
  {
    s->underrun = 1;
#if 0
  audio_stream_get_out_space(s, &used); 
  DBG_DYNA_AUDIO_ECHO("UNDERRUN: current out queue length: %d \n", used);
#endif
  }

  if (spklen1 <= 0)
  {
    return;
  }

#if PSEUDO_AEC
    return;
#endif

#ifdef USE_SPXEC
  if (spklen1)
  {
    memcpy(tmpspk, spkchunk1, spklen1);
    if (spklen2)
    {
      memcpy(tmpspk+spklen1, spkchunk2, spklen2);
    }
  }

  if (spklen1 + spklen2 < length)
  {
    memset(tmpspk+spklen1+spklen2, 0, length - (spklen1 + spklen2));
  }
#endif // USE_SPXEC

  micpcm = (short *) micdata;
  length /= 2;

  spkpcm1 = (short *) spkchunk1;
  spklen1 /= 2;

  spkpcm2 = (short *) spkchunk2;
  spklen2 /= 2;

#ifdef USE_SPXEC

  total = spklen1 + spklen2;
  spxec_echo_cancel(s->ec, micpcm, (short *) tmpspk, (short *) tmpmic, 0);
  if (s->activate_recorder)
  {
    short *cleansignal = (short *) tmpmic;
    int N = length;

    spkpcm1 = (short *) tmpspk;
    while(N--)
    {
      ph_media_audio_recording_record_one(&s->recorder, *spkpcm1++, *micpcm++,  *cleansignal++);
    }
  }
  memcpy(micdata, tmpmic, length*2);

#else

  // adjust mic through echo cancellation. reference signal is the first chunk of the stored circular buffer
  // (this is the regular, common case)
  length -= spklen1;
  while(spklen1--) 
  {
    tmp = AEC(s->ec, *spkpcm1, *micpcm);

    if (s->activate_recorder)
    {
      ph_media_audio_recording_record_one(&s->recorder, *spkpcm1, *micpcm,  tmp);
    }
    *micpcm++ = tmp;
    spkpcm1++;
    total++;
  }

  // adjust mic through echo cancellation. reference signal is the second chunk of the stored circular buffer
  length -= spklen2;
  while(spklen2--) 
  {
    tmp = AEC(s->ec, *spkpcm2, *micpcm);

    if (s->activate_recorder)
    {
      ph_media_audio_recording_record_one(&s->recorder, *spkpcm2, *micpcm,  tmp);
    }
    *micpcm++ = tmp;
    spkpcm2++;
    total++;
  }

  // adjust mic through echo cancellation. reference signal is set to 0
  // (this is the unwanted worst case scenario found if there is a speaker underrun)
  while(length--) 
  {
    tmp = AEC(s->ec, 0, *micpcm);

    if (s->activate_recorder)
    {
      ph_media_audio_recording_record_one(&s->recorder, 0, *micpcm,  tmp);
    }
    *micpcm++ = tmp;
    total++;
  }

#endif
  if (total > savedlen)
  {
    DBG_DYNA_AUDIO_ECHO("do_echo_update: total=%d savedlen=%d\n", total, savedlen);
  }
}

#endif /* DO_ECHO_CAN */

#define TRACE_POWER 1

#ifdef TRACE_POWER
static unsigned int min_pwr;
static unsigned int max_pwr;
static int max_sil;
void print_pwrstats(phastream_t *s)
{
  if(s->cngi.pwr_size)
  {
    DBG_DYNA_AUDIO("\nPWR SUM: min %x max %x mean %x max_sil_cnt %d\n",
	  	min_pwr/s->cngi.pwr_size,
	  	max_pwr/s->cngi.pwr_size,
	  	s->cngi.mean_pwr/s->cngi.pwr_size,
	  	max_sil);
  }
}
#endif

/**
 * @brief finds greatest N such that 2^N <= val
 */
static int
calc_shift(int val)
{
  int ret=0;
  while((val=(val/2)))
  {
    ret++;
  }

  return ret;
}

/**
 * @brief update a previously initialized vadcng structure with a given sample buffer and decide whether there is silence or not
 */
static int
ph_vad_update0(struct vadcng_info *s, char *data, int len)
{
  int i;
  unsigned int power;
  short *p = (short *)data; 
  static int tracecnt = 0;

  /* calculate short term mean power/magnitude over the window */
  for(i = 0; i<len/2; i++)
  {
    s->mean_pwr -= s->pwr[s->pwr_pos];

    //s->pwr[s->pwr_pos] = p[i]*p[i]; // (un)comment for a "power" threshold   
    s->pwr[s->pwr_pos] = abs(p[i]); //(un)comment for a "magnitude" threshold

    s->mean_pwr += s->pwr[s->pwr_pos];       // no overflow as long as pwr size is less then 65536 */

#ifdef TRACE_POWER
    if (s->mean_pwr > max_pwr)
    {
      max_pwr = s->mean_pwr;
    }

    if(s->mean_pwr < min_pwr)
    {
      min_pwr = s->mean_pwr;
    }
#endif

    s->pwr_pos++;
    if (s->pwr_pos >= s->pwr_size)
    {
      s->pwr_pos = 0;
      if(s->cng)
      {
        /* now update the "long term" mean power, which is the sum of 64 mean powers calculated each pwr_size samples */
        s->long_mean_pwr -= s->long_pwr[s->long_pwr_pos];
        s->long_pwr[s->long_pwr_pos] =  s->mean_pwr>>s->pwr_shift;  
        s->long_mean_pwr += s->long_pwr[s->long_pwr_pos];
        s->long_pwr_pos++;
        if(s->long_pwr_pos >= LONG_PWR_WINDOW)
        {
          s->long_pwr_pos = 0;
        }
      }
    }
  }

  /* mean power in last PWR_WINDOW ms */
  power = s->mean_pwr>>s->pwr_shift;
  /* compare with threshold */
  if(power > s->pwr_threshold)
  {
      s->sil_cnt = 0;
  }
  else
  {
      s->sil_cnt += len/2;
  }

#ifdef TRACE_POWER
  if(s->sil_cnt > max_sil)
  {
      max_sil = s->sil_cnt;
  }

  if (ph_trace_mic && (tracecnt++ == 50))
  {
    DBG_DYNA_AUDIO("ph_media_audiuo: mean MIC signal: %d\n", power);
    tracecnt = 0;
  }
#endif

  if(s->sil_cnt > s->sil_max)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

char find_level(unsigned int pwr)
{
  int index=-1;
  int left=1;
  int right=CNG_TBL_SIZE;
  int mid;

  while(1)
  {
    if(left > right)
    {
      break;
    }
    mid = (left+right)>>1;
    if(pwr >= tab_tx_cng[mid] && pwr < tab_tx_cng[mid-1])
    {
      index = mid;
      break;
    }
    else
    {
      if(pwr > tab_tx_cng[mid])
      {
        right = mid-1;
      }
      else
      {
        left = mid+1;
      }
    }
  }
  return index;
}

int ph_astream_encoded_framesize_get(phastream_t *stream)
{
	int framesize = stream->ms.codec->encoded_framesize;

	if (stream->ptime)
	{
		// Default ptime is 20ms, modify the framesize to manage the new ptime
		framesize = (framesize * stream->ptime) / 20;
	}

	return framesize;
}
int ph_astream_decoded_framesize_get(phastream_t *stream)
{
	int framesize = stream->ms.codec->decoded_framesize;

	if (stream->ptime)
	{
		// Default ptime is 20ms, modify the framesize to manage the new ptime
		framesize = (framesize * stream->ptime) / 20;
	}

	return framesize;
}

static void
ph_send_cng(phastream_t *stream, unsigned long timestamp)
{
  mblk_t *mp;
  char level;
  int pt_bck; // JULIEN WENGO

  /* calculate level in dBov */ 
  level = find_level(stream->cngi.long_mean_pwr);
  if (level < 0)
  {
    DBG_DYNA_AUDIO("cng db invalid\n");
    return;
  }
  DBG_DYNA_AUDIO("PHMEDIA:send CNG %d\n", level);
  /* send CNG packet */
  // JULIEN WENGO - rtp_session_create_specific_payload_packet doesn't exist anymore
  //mp = rtp_session_create_specific_payload_packet(stream->ms.rtp_session, RTP_FIXED_HEADER_SIZE, stream->cngi.cng_pt, &level, 1);
  pt_bck = stream->ms.rtp_session->snd.pt;
  stream->ms.rtp_session->snd.pt = stream->cngi.cng_pt;
  mp = rtp_session_create_packet(stream->ms.rtp_session, RTP_FIXED_HEADER_SIZE, &level, 1);
  stream->ms.rtp_session->snd.pt = pt_bck;
  // JULIEN WENGO

  if (mp != NULL)
  {
    rtp_session_sendm_with_ts(stream->ms.rtp_session, mp, timestamp);
  }
}

static int
ph_generate_comfort_noice(phastream_t *stream, void *buf)
{
  struct timeval now, diff;
  phcodec_t *codec = stream->ms.codec;
  int framesize = ph_astream_decoded_framesize_get(stream);

  if (stream->clock_rate ==  stream->actual_rate)
  {
    framesize *= 2;
  }

  gettimeofday(&now, 0);
  ph_tvdiff(&diff, &now, &stream->last_rtp_recv_time);
  if (diff.tv_usec > NOISE_START_DELAY)
  {
    int lg,lg2,ret; // length in shorts
    int used = 0;

    ret = audio_stream_get_out_space(stream, &used);
    if (ret < 0)
    {
      DBG_DYNA_AUDIO("IOCTL error");
      return 0;
    }
    /* if less than 200ms of voice, send noise */
    if (used < framesize * 10)
    {
      /* leave place for 2 voice frame */
      //		lg = info.bytes - 2*codec->ph_astream_decoded_framesize_get(stream);
      lg = 2*framesize;

      if (ret < lg)
      {
          lg = used;
      }

      if (lg > NOISE_B_LEN)
      {
          lg = NOISE_B_LEN;
      }

      if (lg <= (NOISE_B_LEN - stream->cngi.nidx))
      {
        memcpy(buf, &stream->cngi.noise[stream->cngi.nidx], lg);  
        stream->cngi.nidx += lg;

        if (stream->cngi.nidx >=  NOISE_B_LEN)
        {
            stream->cngi.nidx = 0;
        }
      }
      else
      {
        lg2 = lg - (NOISE_B_LEN - stream->cngi.nidx);
        memcpy(buf, &stream->cngi.noise[stream->cngi.nidx], lg-lg2);  
        memcpy(lg - lg2 + (char*) buf, stream->cngi.noise, lg2);
        stream->cngi.nidx = lg2;
      }
    }
    return lg;
  }
  return 0;
}


/**
 * @brief ask for prepared samples for the SPK and play them
 *
 * prepared samples are decoded samples from the RX path mixed with
 * different things (files, conference,..)
 */
static void
ph_handle_network_data(phastream_t *stream)
{
  char data_in_dec[1024];
  int len;
  phcodec_t *codec = stream->ms.codec;
  int internal_framesize = ph_astream_decoded_framesize_get(stream);
  int internal_clockrate = stream->clock_rate;
  int played = 0;
#if 0
  int freespace;
  int usedspace;
#endif
  struct timeval now, now2;

  DBG_DYNA_AUDIO_RX("ph_handle_network_data :: start\n");
#ifdef DO_ECHO_CAN
  DBG_DYNA_AUDIO_ECHO("echo cirbuf size %d\n", stream->sent_cnt - stream->read_cnt);
#endif

#ifdef PH_FORCE_16KHZ
  if (internal_clockrate == 8000)
  {
    internal_framesize *= 2;
    internal_clockrate = 16000;
  }
#endif

#if 0
  freespace = audio_stream_get_out_space(stream, &used);
  if (used*2 >= ph_astream_decoded_framesize_get(stream))
  {
    return;
  }

 freespace = audio_stream_get_out_space(stream, &usedspace);
 if (!freespace)
 {
     return;
 }
#endif

  while (stream->ms.running)
  {
    gettimeofday(&now, 0);

    // try to read read to be played samples from the RX path
    len = ph_audio_play_cbk(stream, data_in_dec, internal_framesize);
	if(len)
	{
		DBG_DYNA_AUDIO("DYNA_AUDIO:ph_handle_network_data:got %d bytes\n",len);
		DBG_DYNA_AUDIO_RX("ph_handle_network_data:%u.%u :: read %d full size packets\n", now.tv_sec, now.tv_usec, len/internal_framesize);
	}
    if (!len)
    {
      break;
    }

    played += len;
    //freespace -= len;

    // try to write the decoded audio onto the speaker(out) device
    len = audio_stream_write(stream, data_in_dec, len);

    if (!len)
    {
      break;
    }

#ifdef DO_ECHO_CAN
    // SPIKE_AEC: record what is being played on the SPK for future reference
    if (!stream->using_out_callback)
    {
      store_pcm(stream, data_in_dec, len);
    }
#endif

    // exit loop if we've played 4 full size packets
    if (played >= internal_framesize * 4)
    {
      break;
    }

    // exit loop if we waited too much in the audio_stream_write
    gettimeofday(&now2, 0);
    if (now2.tv_sec > now.tv_sec || (now2.tv_usec - now.tv_usec) >= 10000)
    {
      DBG_DYNA_AUDIO("exit loop, we've waited too much in the audio_stream_write\n");
      break;
    }

  } // while loop: we try to get more data from the RX path

  DBG_DYNA_AUDIO_RX("ph_handle_network_data :: end\n");
}

#define SATURATE(x) ((x > 0x7fff) ? 0x7fff : ((x < ~0x7fff) ? ~0x7fff : x))

#ifdef PH_FORCE_16KHZ
void ph_downsample(void *rctx, void *framebuf, int framesize);
void ph_upsample(void *rctx, void *dbuf, void *framebuf, int framesize);
void *ph_resample_init();
void ph_resample_cleanup(void *ctx);
#endif


static int
prepare_reformat_buffer(phastream_t *stream)
{
	if (stream->reformatbuf.cb_buf)
		return;

  cb_init(&stream->reformatbuf, 1024*4);
}

/**
 * @brief catch a packet on the rtp RX path and decode it
 *
 * @param stream the concerned audio stream
 * @param mbf decoded samples are put in this buffer
 * @param clockrate decoded samples should be resampled if necessary to this clockrate
 * @return 0 or len in bytes of catched samples
 */
static int
ph_media_retrieve_decoded_frame(phastream_t *stream, ph_mediabuf_t *mbf, int clockrate)
{
  int decodedlen=0;
  int codedlen=0, resampledlen=0;
  mblk_t *mp;
  rtp_header_t *rtp;
  phcodec_t *codec = stream->ms.codec;
  int framesize = ph_astream_decoded_framesize_get(stream);
  int encoded_framesize = ph_astream_encoded_framesize_get(stream);
  int expected_decodedlen = framesize;
  int needreformat;
  int true_expected_decodedlen = expected_decodedlen;


#ifdef PH_FORCE_16KHZ
  if (stream->clock_rate != clockrate)
  {
	  true_expected_decodedlen = expected_decodedlen*2;
  }
#endif  
  DBG_DYNA_AUDIO_RX("expected_decoded_len :%d true_expected_decodedlen: %d)\n", expected_decodedlen, true_expected_decodedlen);
  
  if (stream->ms.suspended)
  {
    return 0;
  }
  if (stream->reformatbuf.cb_buf)
  {
	  DBG_DYNA_AUDIO_RX("CB has %d)\n", stream->reformatbuf.cb_cnt);
  }
  // try to retrieve an RTP packet on the RX path
  mp = rtp_session_recvm_with_ts(stream->ms.rtp_session,stream->ms.rxtstamp);
  if (!mp)
  {
	  stream->ms.rxts_inc += expected_decodedlen/2;
	  DBG_DYNA_AUDIO("no packet %d\n", stream->ms.rxtstamp);

	  if (stream->reformatbuf.cb_buf)
	  {
		  if (stream->reformatbuf.cb_cnt >= true_expected_decodedlen)
		  {
			  resampledlen = cb_read(&stream->reformatbuf, mbf->buf, true_expected_decodedlen);
			  mbf->next = resampledlen/2;
			  DBG_DYNA_AUDIO_RX("CB has %d after reading %d)\n", stream->reformatbuf.cb_cnt, resampledlen);
			  return resampledlen;
		  }
	  }

    return 0;
  }

  // drop RTP packet if it is not of the correct payload type
  rtp = (rtp_header_t*)mp->b_rptr;
  if ( rtp->paytype != stream->ms.payload )
  {
	DBG_DYNA_AUDIO("wrong audio payload: %d expecting %d\n", rtp->paytype, stream->ms.payload);
    freemsg(mp);
    return 0;
  }

  // drop RTP packet if stream is not in running mode
  if (!stream->ms.running)
  {
    freemsg(mp);
    return 0;
  }

  codedlen = mp->b_cont->b_wptr-mp->b_cont->b_rptr;  
//  needreformat = codedlen > encoded_framesize;
  needreformat = codedlen != encoded_framesize;
  if(needreformat)
  {
	  DBG_DYNA_AUDIO("NeedReformat:codedlen%d enc_fs %d\n", codedlen, encoded_framesize);
  }

#ifdef PH_FORCE_16KHZ
  // here, the required clockrate is always 16000
  if (codedlen)
  {
    //This is a HACK to accept g711 frames with variable sizes
    //Done in order to accept audio flow from the Wengo flash
    //application  *

    if (!strncmp(codec->mime, "PCMU", 4) || !strncmp(codec->mime, "PCMA", 4)) 
      {
		  framesize = codedlen * 2;
      }

	if (stream->clock_rate == clockrate)
    {
      if (!needreformat)
	{
	  resampledlen = decodedlen = codec->decode(stream->ms.decoder_ctx, mp->b_cont->b_rptr, codedlen, mbf->buf, framesize);
	  mbf->next = decodedlen/2;
	  stream->ms.rxts_inc += mbf->next;
	}
      else
	{
	  char *rftmp = alloca(1024*2);

	  decodedlen = codec->decode(stream->ms.decoder_ctx, mp->b_cont->b_rptr, codedlen, rftmp, 1024*2);

	  prepare_reformat_buffer(stream);

	  cb_put(&stream->reformatbuf, rftmp, decodedlen);
	  DBG_DYNA_AUDIO_RX("CB has %d after putting %d)\n", stream->reformatbuf.cb_cnt, decodedlen);
	  if (stream->reformatbuf.cb_cnt >= true_expected_decodedlen)
	  {
		  resampledlen = cb_read(&stream->reformatbuf, mbf->buf, true_expected_decodedlen);
	  }
	  else
	  {
		  resampledlen = 0;
	  }
	  DBG_DYNA_AUDIO_RX("CB has %d after reading %d)\n", stream->reformatbuf.cb_cnt, resampledlen);

	  mbf->next = resampledlen/2;
	  stream->ms.rxts_inc += decodedlen/2;
	  DBG_DYNA_AUDIO_RX("CB next %d)\n", mbf->next);
	  
	  
	}
    }
    else
    {
      char *tmp = alloca(1024*2);

      decodedlen = codec->decode(stream->ms.decoder_ctx, mp->b_cont->b_rptr, codedlen, tmp, 1024*2);
      //ph_upsample(mbf->buf, tmp, framesize, &stream->lastsample);

      if (!needreformat)
	{
	  ph_upsample(stream->resamplectx, mbf->buf, tmp, framesize);
	  mbf->next = decodedlen;
	  stream->ms.rxts_inc += decodedlen/2;
	  resampledlen = decodedlen << 1;
	}
      else
	{
	  char *rftmp = alloca(1024*4);

	  prepare_reformat_buffer(stream);

	  ph_upsample(stream->resamplectx, rftmp, tmp, decodedlen);
	  cb_put(&stream->reformatbuf, rftmp, decodedlen * 2);
	  DBG_DYNA_AUDIO_RX("CB has %d after putting %d)\n", stream->reformatbuf.cb_cnt, decodedlen*2);
	  if (stream->reformatbuf.cb_cnt >= true_expected_decodedlen)
	  {
		  resampledlen = cb_read(&stream->reformatbuf, mbf->buf, true_expected_decodedlen);
		  DBG_DYNA_AUDIO_RX("CB has %d after reading %d)\n", stream->reformatbuf.cb_cnt, resampledlen);
	  }
	  else
	  {
		  resampledlen = 0;
	  }
	  mbf->next = resampledlen/2;
	  stream->ms.rxts_inc += decodedlen/2;
	  DBG_DYNA_AUDIO_RX("CB next %d)\n", mbf->next);
	}
    }
	DBG_DYNA_AUDIO_RX("post resampling:(%d,%d,%d,%d)\n", codedlen, decodedlen, resampledlen, framesize);
  }
#else
  if (codedlen)
  {
    if (!needreformat)
      {
	resampledlen = decodedlen = codec->decode(stream->ms.decoder_ctx, mp->b_cont->b_rptr, codedlen, mbf->buf, framesize);
	mbf->next = decodedlen/2;
	stream->ms.rxts_inc += mbf->next;
      }
    else
      {
	char *rftmp = alloca(1024*2);

	prepare_reformat_buffer(stream);

	resampledlen = decodedlen = codec->decode(stream->ms.decoder_ctx, mp->b_cont->b_rptr, codedlen, rftmp, 1024*2);
	cb_put(&stream->reformatbuf, rftmp, decodedlen);

	if (stream->reformatbuf.cb_cnt >= true_expected_decodedlen)
	{
		resampledlen = cb_read(&stream->reformatbuf, mbf->buf, true_expected_decodedlen);
	}
	else
	{
		resampledlen = 0;
	}
	mbf->next = decodedlen/2;
	stream->ms.rxts_inc += decodedlen/2;

      }
  }

#endif
  DBG_DYNA_AUDIO("decodedlen%d\n", decodedlen);

  freemsg(mp);

  if (decodedlen)
  {
    struct timeval now;
    gettimeofday(&now, 0);
    stream->last_rtp_recv_time = now;
  }

  DBG_DYNA_AUDIO_RX("retrieved RX bytes: decoded(%d), resampled(%d)\n", decodedlen, resampledlen);
  return resampledlen;
}

/**
 * @brief tries to grab samples that are available for SPK playing
 */
static int
ph_audio_play_cbk(phastream_t *stream, void *playbuf, int playbufsize)
{
  int len = 0;
  phcodec_t *codec = stream->ms.codec;
  int internal_framesize = ph_astream_decoded_framesize_get(stream);
  int internal_clockrate = stream->clock_rate;
  int iter = 0;
  int played = 0;
  char *audio_drv_playbuf = playbuf;
  int audio_drv_played=0;
#ifdef PH_USE_RESAMPLE
  unsigned char resampleBuf[3000];
  int resampledSize = 0;
  char *savedPlayBuf = NULL;
  int savedBufSize = 0;
  int resampledLen = 0;
  int needResample = 0;
#endif

  DBG_DYNA_AUDIO("DYNA_AUDIO:ph_audio_play_cbk: audio drv is asking for %d (char*) casted samples\n",
				 playbufsize);
  DBG_DYNA_AUDIO("DYNA_AUDIO:ph_audio_play_cbk:internal_framesize is %d\n",
				 internal_framesize);

#ifdef PH_FORCE_16KHZ
  if (internal_clockrate == 8000)
  {
    internal_framesize *= 2;
    internal_clockrate = 16000;
  }
#endif

  // if we accept dynamic resampling, we need to retrieve the RX audio datas in a temporary buffer
#ifdef PH_USE_RESAMPLE
  savedPlayBuf = (char *) playbuf;
  savedBufSize = playbufsize;
  needResample = (internal_clockrate != stream->actual_rate);

  if (needResample) {
      playbuf = resampleBuf;
      playbufsize = sizeof(resampleBuf);
      //recordbuf = resampledBuf;
  }
#endif


  // prepare next increment for the RTP polling
  stream->ms.rxts_inc = 0;
#ifdef DO_CONF
  if (stream->to_mix)
  {
    stream->to_mix->ms.rxts_inc = 0;
  }
#endif

	// while start :
	// read packets from the network
	// the best case scenario is that one packet is ready to go
	// we do not try to read more than 4 packets in a row
	while (stream->ms.running && (playbufsize >= internal_framesize))
	{
		ph_mediabuf_t spkrbuf;
		ph_mediabuf_init(&spkrbuf, playbuf, playbufsize);

        // FLOWGRAPH STEP
        // SPIKE_AUDIO_FLOWGRAPH: pull data from the network, resample and potential conf join
        // BEGIN GRAPH NODE
#ifdef DO_CONF
		if (stream->to_mix)
		{
			// pull data for all the conf participants
			CONF_LOCK(stream);
			if (stream->to_mix && !stream->to_mix->ms.suspended)
			{
				int len2;
				len = ph_media_retrieve_decoded_frame(stream, &stream->data_in, internal_clockrate);
				len2 = ph_media_retrieve_decoded_frame(stream->to_mix, &stream->to_mix->data_in, internal_clockrate);
				ph_mediabuf_mixmedia2(&spkrbuf, &stream->data_in, &stream->to_mix->data_in, len);
				len = spkrbuf.next * 2;
			}
			CONF_UNLOCK(stream);
		}
		else
		{
			// pull data for the stream
			len = ph_media_retrieve_decoded_frame(stream, &spkrbuf, internal_clockrate);
		}
#else
		// pull data for the stream
		len = ph_media_retrieve_decoded_frame(stream, &spkrbuf, internal_clockrate);
#endif

        // if nothing could be retrieved the flowgraph stops here
        if (!len)
        {
            break;
        }

        // Here we have managed to retrieve audio data from the RX path at frequency 'internal_clockrate'

        // FLOWGRAPH STEP
        // SPIKE_AUDIO_FLOWGRAPH: half-duplex: MIC driven attenuation of SPK
        // BEGIN GRAPH NODE
        if ((stream->hdxmode == PH_HDX_MODE_MIC) && !stream->hdxsilence)
        {
            short *samples = (short *) playbuf;
            int nsamples = len >> 1;
            const int HDXSHIFT = 7;
            while(nsamples--)
            {
                *samples = *samples >> HDXSHIFT;
                samples++;
            }
        }

        // FLOWGRAPH STEP
        // SPIKE_AUDIO_FLOWGRAPH: snapshot of SPK datas for the AEC
        // BEGIN GRAPH NODE
#ifdef DO_ECHO_CAN
        if (stream->using_out_callback)
        {
            store_pcm(stream, playbuf, len);
        }
#endif

        // FLOWGRAPH STEP
        // SPIKE_AUDIO_FLOWGRAPH: half-duplex: SPK driven analysys of the SPK activity
        // BEGIN GRAPH NODE
        // SPIKE_HDX: if (mode == SPK has priority) update SPK voice activity detection
        if (stream->hdxmode == PH_HDX_MODE_SPK)
        {
            stream->spksilence = ph_vad_update0(&stream->cngo, playbuf, len);
        }

        /* if we need the last saved frame, save it */
        // note: in 28/07/2006 code it is always 0
        if (stream->lastframe)
        {
            memcpy(stream->lastframe, playbuf, len);
        }

#ifdef PH_USE_RESAMPLE
        if (needResample)
        {
            DBG_DYNA_AUDIO("RESAMPLE: ph_audio_play_cbk: need resampling with recbufsize: %d\n", len);
            resampledLen = 0;
            ph_resample_audio0(stream->resample_audiodrv_ctx_spk, playbuf, len, savedPlayBuf, &resampledLen);
			DBG_DYNA_AUDIO("RESAMPLE: ph_audio_play_cbk: after resampling with resampledSize: %d\n", resampledLen);
            savedPlayBuf += resampledLen;
            savedBufSize -= resampledLen;
            if (resampledLen!=0)
            {
                played += len;
            }
            audio_drv_played += resampledLen;
            if (savedBufSize <= 0)
            {
                break;
            }
        }
        else
#endif
        {
            // updating the placeholder for the next decode loop
            playbuf = len + (char *) playbuf;
            playbufsize -= len;
            played += len;
            audio_drv_played += len;
			DBG_DYNA_AUDIO("audio_play_cbk:len:%d playbufsize %d,played %d, internal_framesize %d\n", len, playbufsize, played, internal_framesize);
        }

        /* exit loop if we've played 4 full size packets */
        if (played >= internal_framesize * 4)
        {
            break;
        }

    } // while end : read packets from the network

	DBG_DYNA_AUDIO_RX("audio_play_cbk: endloop\n");

    // refresh rx timestamp according to what we pulled from the network
    stream->ms.rxtstamp += stream->ms.rxts_inc;
#ifdef DO_CONF
    if (stream->to_mix) 
    {
        CONF_LOCK(stream);
        if (stream->to_mix)
        {
            stream->to_mix->ms.rxtstamp += stream->to_mix->ms.rxts_inc;
        }
        CONF_UNLOCK(stream);
    }
#endif

    // what happens when nothing could be pulled from the network
    if (played == 0)
    {
        DBG_DYNA_AUDIO_RX("RX path is starving !\n");
        /* if no data for at least NOISE_START_DELAY ms, start sending noise audio data */
        if (stream->ms.running && stream->cngi.cng)
        {
            int len;
            len = ph_generate_comfort_noice(stream, playbuf);
            if (len)
            {
#ifdef DO_ECHO_CAN
                if (stream->using_out_callback)
                {
                    store_pcm(stream, playbuf, len);
                }
#endif
            }

#ifdef PH_USE_RESAMPLE
            if (needResample)
            {
                resampledLen = 0;
                ph_resample_audio0(stream->resample_audiodrv_ctx_spk, playbuf, len, savedPlayBuf, &resampledLen);
                savedPlayBuf += resampledLen;
                savedBufSize -= resampledLen;
                if (resampledLen!=0)
                {
                    played += len;
                }
                audio_drv_played += resampledLen;
            }
            else
#endif
            {
                playbuf = len + (char *) playbuf;
                playbufsize -= len;
                played += len;
                audio_drv_played += len;
            }
			DBG_DYNA_AUDIO_RX("generated %d driver len of Confort Noise\n", audio_drv_played);

        } // cng

    } // if(played == 0)


    // what happens when the required buffer is not filled totally ?
    // note: in 28/07/2006 code, this is dead code since lastframe=0
    if (stream->lastframe != 0 && playbufsize)
    {
        /* we did not fill the buffer completely */
        int morebytes = playbufsize;
        if (morebytes > ph_astream_decoded_framesize_get(stream))
        {
            morebytes = ph_astream_decoded_framesize_get(stream);
        }

#ifdef DO_ECHO_CAN
        if (stream->using_out_callback)
        {
            store_pcm(stream, playbuf, morebytes);
        }
#endif
        played += morebytes;
    }

    if (stream->record_spk_stream)
    {
        ph_media_audio_fast_recording_record(&stream->spk_stream_recorder, audio_drv_playbuf, audio_drv_played);
    }
    DBG_DYNA_AUDIO_RX("audio_driver_played: %d\n", audio_drv_played);
	return audio_drv_played;
}


void ph_encode_and_send_audio_frame(phastream_t *stream, void *recordbuf, int framesize)
{
  int silok = 0;
  int wakeup = 0;
  struct timeval diff;
  char data_out_enc[1000];
  phcodec_t *codec = stream->ms.codec;

  if (stream->ms.suspended)
  {
    return;
  }

  /* do we need to do Voice Activity Detection ? */
  if (stream->cngi.vad)
  {
    DBG_DYNA_AUDIO("VAD:ph_encode_and_send_audio_frame:stream->cngi.vad\n");
    stream->hdxsilence = silok = ph_vad_update0(&stream->cngi, recordbuf, framesize);
    if (!stream->cngi.cng && silok)
    {
      DBG_DYNA_AUDIO("VAD:ph_encode_and_send_audio_frame: resend dummy CNG packet only if CNG was not negotiated\n");
      /* resend dummy CNG packet only if CNG was not negotiated */
      ph_tvdiff(&diff, &stream->now, &stream->last_rtp_sent_time);
      wakeup = (diff.tv_sec > RTP_RETRANSMIT);
    }
  }
  // SPIKE_HDX: if MIC has priority, update MIC voice activity detection
  else if (stream->hdxmode == PH_HDX_MODE_MIC)
  {
    int hdxsil = ph_vad_update0(&stream->cngi, recordbuf, framesize);
    if (hdxsil != stream->hdxsilence) 
    {
      DBG_DYNA_AUDIO("phmedia_audio: HDXSIL=%d\n", hdxsil);
      stream->hdxsilence = hdxsil;
    }
  }

  if ((stream->dtmfi.dtmfg_phase != DTMF_IDLE) || (stream->dtmfi.dtmfq_cnt != 0))
  {
    ph_generate_out_dtmf(stream, (short *) recordbuf, framesize/2, stream->ms.txtstamp);
    silok = 0;
  }

  if (stream->mixbuf)
  {
    int n = ph_mediabuf_mixaudio(stream->mixbuf, (short *)recordbuf, framesize/2);
    if (!n)
    {
      ph_mediabuf_free(stream->mixbuf);
      stream->mixbuf =  0;
    }
    else
    {
      stream->hdxsilence = silok = 0;
    }
  }

  if (!silok)
  {
    int enclen;

#ifdef PH_FORCE_16KHZ
    if (stream->clock_rate == 16000)
    {
      enclen = codec->encode(stream->ms.encoder_ctx, recordbuf, framesize, data_out_enc, sizeof(data_out_enc));
    }
    else
    {
      DBG_DYNA_AUDIO_TX("downsampling for FORCE_16KHZ mode (%d)->(%d)\n", framesize, framesize/2);
      ph_downsample(stream->resamplectx, recordbuf, framesize);
      enclen = codec->encode(stream->ms.encoder_ctx, recordbuf, framesize/2, data_out_enc, sizeof(data_out_enc));
      framesize /= 2;
    }
#else
    DBG_DYNA_AUDIO("DYNA_AUDIO:ph_encode_and_send_audio_frame: start encoding\n");
    enclen = codec->encode(stream->ms.encoder_ctx, recordbuf, framesize, data_out_enc, sizeof(data_out_enc));
#endif

    if (stream->record_send_stream)
    {
      // record the encoded audio in a file
      ph_media_audio_fast_recording_record(&stream->send_stream_recorder, data_out_enc, enclen);
    }

    if (silok != stream->cngi.lastsil || wakeup)
    {
	  mblk_t *m;
	  int msgsize;

	  /* JULIEN WENGO
      rtp_session_set_markbit(stream->ms.rtp_session, 1);
      rtp_session_send_with_ts(stream->ms.rtp_session, data_out_enc, enclen, stream->ms.txtstamp);
      rtp_session_set_markbit(stream->ms.rtp_session, 0);
	  */

	  msgsize = enclen + RTP_FIXED_HEADER_SIZE;
	  m = rtp_session_create_packet(stream->ms.rtp_session, RTP_FIXED_HEADER_SIZE, data_out_enc, enclen);
	  if (m)
	  {
		  rtp_set_markbit(m, 1);
		  rtp_session_sendm_with_ts(stream->ms.rtp_session, m, stream->ms.txtstamp);
	  }

	  /* ***************** */
    }
    else
    {
      rtp_session_send_with_ts(stream->ms.rtp_session, data_out_enc, enclen, stream->ms.txtstamp);
    }

    stream->last_rtp_sent_time = stream->now;
  } // if(!silok)
  else if (stream->cngi.cng)
  {
    ph_tvdiff(&diff, &stream->now, &stream->cngi.last_dtx_time);
    if (diff.tv_sec >= DTX_RETRANSMIT)
    {
      ph_tvdiff(&diff, &stream->now, &stream->last_rtp_sent_time);
      if (diff.tv_sec >= DTX_RETRANSMIT)
      {
        DBG_DYNA_AUDIO("VAD:ph_encode_and_send_audio_frame: ph_send_cng\n");

        ph_send_cng(stream, stream->ms.txtstamp);
        stream->cngi.last_dtx_time = stream->now;
      }
    }

    if (wakeup)
    {
      /* send cng packet with -127dB */ 
      ph_send_cng(stream, stream->ms.txtstamp);
      stream->last_rtp_sent_time = stream->now;
    }
  }

  stream->cngi.lastsil = silok;
  stream->ms.txtstamp += framesize/2;
}

/**
 * @brief called with an available buffer of audio samples
 * @param stream ph_audio stream
 * @param recbuf recorded buffer
 * @param recbufsize available size in the recbuf buffer. not expressed in samples but in char* representation of the samples
 *
 * "rec" comes from "record", as in "the microphone has just recorded something"
 */
int ph_audio_rec_cbk(phastream_t *stream, void *buf_dataleft, int size_dataleft)
{
  // framesize must be calculated in order to be the size in char* of a 20ms period in MIC frequency unit
  int codec_tx_framesize = ph_astream_decoded_framesize_get(stream);
  int internal_framesize = 0;
  int internal_clockrate = 0;
  int rec_cbk_clockrate = 0;
  int processed = 0;
#ifdef PH_USE_RESAMPLE
  unsigned char buf_resampled[1000];
  long size_resampled = 0;
#endif

  // FLOWGRAPH STEP
  // SPIKE_AUDIO_FLOWGRAPH: record MIC source in a file
  // BEGIN GRAPH NODE
  if (stream->record_mic_stream)
  {
    ph_media_audio_fast_recording_record(&stream->mic_stream_recorder, buf_dataleft, size_dataleft);
  }
  // END GRAPH NODE

  // FLOWGRAPH STEP
  // flowgraph glue: what is the internal clockrate ?
  internal_framesize = codec_tx_framesize;
  internal_clockrate = stream->clock_rate;
#ifdef PH_FORCE_16KHZ
  // the "internal phapi clockrate" is forced to 16Khz
  if (stream->clock_rate == 8000)
  {
    internal_clockrate = 2*stream->clock_rate;
    internal_framesize = codec_tx_framesize * 2;
  }
#endif
  // flowgraph glue: what is the rec clockrate ?
  rec_cbk_clockrate = stream->actual_rate;

  // FLOWGRAPH STEP
  // SPIKE_AUDIO_FLOWGRAPH: resample the MIC source to the internal clockrate/framesize
  // NODE TYPE: local stack replacing buffer
  // BEGIN GRAPH NODE
#ifdef PH_USE_RESAMPLE
  if (internal_clockrate != rec_cbk_clockrate)
  {
    DBG_DYNA_AUDIO("RESAMPLE: ph_audio_rec_cbk: need resampling with size_dataleft: %d\n", size_dataleft);
    size_resampled = internal_framesize;
    ph_resample_audio0(stream->resample_audiodrv_ctx_mic, buf_dataleft, size_dataleft, buf_resampled, &size_resampled);
    DBG_DYNA_AUDIO("RESAMPLE: ph_audio_rec_cbk: after resampling with size_resampled: %d\n", size_resampled);
    buf_dataleft = buf_resampled;
    size_dataleft = size_resampled;
  }
#endif
  // END GRAPH NODE

  // FLOWGRAPH STEP
  // SPIKE_AUDIO_FLOWGRAPH: record resampled MIC source in a file
  // BEGIN GRAPH NODE
  if (stream->record_mic_resample_stream)
  {
    ph_media_audio_fast_recording_record(&stream->mic_resample_stream_recorder, buf_dataleft, size_dataleft);
  }
  // END GRAPH NODE

  // flowgraph glue: now we deal with audio data only tx_frame by tx_frame
  while(size_dataleft >= internal_framesize)
  {
    gettimeofday(&stream->now,0);

    // FLOWGRAPH STEP
    // SPIKE_AUDIO_FLOWGRAPH: apply half-duples SPK driven attenuator
    // NODE TYPE: in-place
    // BEGIN GRAPH NODE
    // SPIKE_HDX: if (mode = SPK has priority) and SPK is active, attenuate MIC
    if ((stream->hdxmode == PH_HDX_MODE_SPK) && !stream->spksilence)
    {
      short *samples = (short *) buf_dataleft;
      int nsamples = internal_framesize >> 1;
      const int SPKHDXSHIFT = 4;
      while(nsamples--)
      {
        *samples = *samples >> SPKHDXSHIFT;
        samples++;
      }
    }
    // END GRAPH NODE

    // FLOWGRAPH STEP
    // SPIKE_AUDIO_FLOWGRAPH: cancel acoustic echo
    // NODE TYPE: in-place
    // BEGIN GRAPH NODE
#ifdef DO_ECHO_CAN
    do_echo_update(stream, buf_dataleft, internal_framesize);
#endif
    // END GRAPH NODE

    // flowgraph glue :
    // handle conference
#ifdef DO_CONF
    if (stream->to_mix)
    {
      CONF_LOCK(stream);
      if (stream->to_mix && !stream->to_mix->ms.suspended) /* we're in conference mode */
      {
        phastream_t *other = stream->to_mix;
        other->now = stream->now;

        memcpy(stream->data_out.buf, buf_dataleft, internal_framesize);
        stream->data_out.next = internal_framesize/2;

        memcpy(other->data_out.buf, buf_dataleft, internal_framesize);
        other->data_out.next = internal_framesize/2;

        if (other->data_in.next)
        {
          ph_mediabuf_mixmedia(&stream->data_out, &other->data_in);
        }
        if (stream->data_in.next)
        {
          ph_mediabuf_mixmedia(&other->data_out, &stream->data_in);
        }

        ph_encode_and_send_audio_frame(stream, stream->data_out.buf, internal_framesize);
        ph_encode_and_send_audio_frame(other, other->data_out.buf, internal_framesize);
        //ph_handle_conference_in(stream, internal_framesize);
      }
      CONF_UNLOCK(stream);
    }
    else
#endif
	{
      ph_encode_and_send_audio_frame(stream, buf_dataleft, internal_framesize);
	}

    size_dataleft -= internal_framesize;
    processed += internal_framesize;
    buf_dataleft = internal_framesize + (char *)buf_dataleft;
  } // while end - process remaining whole tx_frames

  DBG_DYNA_AUDIO("DYNA_AUDIO:ph_audio_rec_cbk: processed %d short audio samples\n", processed/2);
  return processed;
}


/**
 * @brief callback used by the audio subsystems to communicate with the phapi audio engine
 * @param stream the concerned audio stream
 * @param recbuf buffer that has just been recorded by the subsystem
 * @param recbufsize size of the recorded buffer (maybe 0)
 * @param playbuf
 * @param playbufsize
 */
static int 
ph_audio_callback(phastream_t *stream, void *recbuf, int recbufsize, void *playbuf, int *playbufsize)
{
  int i = 0;

  if (stream->ms.running)
  {
    if (recbuf && recbufsize > 0)
    {
        ph_audio_rec_cbk(stream, recbuf, recbufsize);
    }

    if ((playbuf != 0) && playbufsize && (*playbufsize > 0))
    {
        i = ph_audio_play_cbk(stream, playbuf, *playbufsize);
        *playbufsize = i;
    }
  }

  return 0;
}


static int
ph_handle_audio_data(phastream_t *stream)
{
  char data_out[1024];
  phcodec_t *codec = stream->ms.codec;
  int internal_framesize = sizeof(data_out);
  int internal_clockrate = stream->clock_rate;
  int i;

#ifdef PH_FORCE_16KHZ
  if (internal_clockrate == 8000)
  {
    //    internal_framesize *= 2;
    internal_clockrate = 16000;
  }
#endif

  DBG_DYNA_AUDIO_TX("Reading Got %d bytes from mic\n", internal_framesize);
  i=audio_stream_read(stream, data_out, internal_framesize);
  DBG_DYNA_AUDIO_TX("Got %d bytes from mic\n", i);
  if (i>0)
  {
    i = ph_audio_rec_cbk(stream, data_out, i);
  }
  return (i>0);
}

/**
 * @brief main thread for the audio engine
 *
 * Note that it is only created when the audio subsystem driver needs it
 * It takes care of all the things that cannot be done in the subsystem threads
 */
void *
ph_audio_io_thread(void *p)
{
    phastream_t *stream = (phastream_t*)p;

    DBG_DYNA_AUDIO("audio io timer called\n");

    // if subsystem threading model does not take care of feeding SPK,
    // do it : receive packets from the RX path and play them on SPK
    if (!audio_driver_has_play_callback())
    {
        ph_handle_network_data(stream);
    }

    // if subsystem threading model does not take care of reading MIC,
    // do it : read samples from MIC and send them on the TX path
    if (!audio_driver_has_rec_callback())
    {
        ph_handle_audio_data(stream);
    }

    return NULL;
}


/*
 * mix a DTMF signal into the given signal buffer
 *
*/
void ph_generate_out_dtmf(phastream_t *stream, short *signal, int siglen, long timestamp)
{
  int mixlen, mixn;
  unsigned short  dtmfDigit;
  const int DTMF_MSECS = 240;
  int samples, rate;
  struct dtmf_info *dtmfp = &stream->dtmfi;

  rate = stream->clock_rate;
#ifdef PH_FORCE_16KHZ
  rate = 16000;
#endif

  samples = rate/1000;
  dtmf_again:
  switch (dtmfp->dtmfg_phase)
  {
  case DTMF_IDLE:
    /* if the DTMF queue is empty we do nothing */
    if (!dtmfp->dtmfq_cnt)
    {
      break;
    }

    dtmfDigit =  dtmfp->dtmfq_buf[dtmfp->dtmfq_rd++];

    /* start generating the requested tone */
    if (dtmfDigit & (PH_DTMF_MODE_INBAND << 8))
    {
      tg_dtmf_init(&dtmfp->dtmfg_ctx, dtmfDigit & 0x0ff, rate, 0);
    }

    if (dtmfDigit & (PH_DTMF_MODE_RTPPAYLOAD << 8))
    {
      rtp_session_send_dtmf2(stream->ms.rtp_session,  dtmfDigit & 0x0ff, timestamp, samples*DTMF_MSECS);
    }

    /* update queue pointers and state */
    if (!stream->to_mix)
    {
      DTMF_LOCK(stream);
    }

    if (dtmfp->dtmfq_rd >= DTMFQ_MAX)
    {
      dtmfp->dtmfq_rd = 0;
    }
    dtmfp->dtmfq_cnt--;
    if (dtmfDigit & (PH_DTMF_MODE_INBAND << 8))
    {
      dtmfp->dtmfg_phase = DTMF_GEN;
    }

    if (!stream->to_mix)
    {
      DTMF_UNLOCK(stream);
    }

    /* we're going to generate DMTMF_MSEC of DTMF tone */
    dtmfp->dtmfg_len = samples*DTMF_MSECS;

    /* if we're not using INBAND mode we're done */
    if (!(dtmfDigit & (PH_DTMF_MODE_INBAND << 8)))
    {
      return;
    }

  /* fall down */
  case DTMF_GEN:

    /* how much data we can stuff in the current signal buffer */
    mixlen = (siglen > dtmfp->dtmfg_len) ? dtmfp->dtmfg_len : siglen;

    /* mix in the generated tone */
    for( mixn = 0;  mixn < mixlen; mixn++)
    {
      signal[mixn] += tg_dtmf_next_sample(&dtmfp->dtmfg_ctx);
    }

    /* keep track of the amount of the still ungenerated samples */
    dtmfp->dtmfg_len -= mixlen;

    /* if we didn't finish with the current dtmf digit yet, we'll stay in the GEN state */
    if (dtmfp->dtmfg_len)
    {
      break;
    }

    /* we've done with the current digit, ensure we have 50msec before the next DTMF digit */
    dtmfp->dtmfg_phase = DTMF_SILENT;
    dtmfp->dtmfg_len =  50*samples; 

    /* skip past processed part of signal */
    siglen -= mixlen;
    signal += mixlen;

  /* fall down */
  case DTMF_SILENT:

    mixlen = (siglen > dtmfp->dtmfg_len) ? dtmfp->dtmfg_len : siglen;
    dtmfp->dtmfg_len -= mixlen;

    /* if we have more silence to generate, keep the SILENT state */
    if (dtmfp->dtmfg_len)
    {
      break;
    }

    /* we can handle a next DTMF digit now */
    dtmfp->dtmfg_phase = DTMF_IDLE;
    if (dtmfp->dtmfq_cnt)
    {
      signal += mixlen;
      siglen -= mixlen;
      goto dtmf_again;
    }
    break;

  }
}

/**
 * @brief init a vad (voice activity detection) structure
 */
void ph_audio_init_vad0(struct vadcng_info *cngp, int samples)
{
  cngp->pwr_size = PWR_WINDOW * samples;
  cngp->pwr_shift = calc_shift(cngp->pwr_size);
  cngp->pwr = osip_malloc(cngp->pwr_size * sizeof(int));
  if(cngp->pwr)
  {
    memset(cngp->pwr, 0, cngp->pwr_size * sizeof(int));
    DBG_DYNA_AUDIO(" DTX/VAD PWR table of %d ints allocated \n", cngp->pwr_size);
  }
  else
  {
    DBG_DYNA_AUDIO("No memory for DTX/VAD !: %d \n", cngp->pwr_size*2);
    cngp->vad = cngp->cng = 0;
  }

  cngp->pwr_pos = 0;
  cngp->sil_max = SIL_SETUP * samples;
  cngp->long_pwr_shift = calc_shift(LONG_PWR_WINDOW);
  cngp->long_pwr_pos = 0;
#ifdef TRACE_POWER
  min_pwr = 0x80000001;
  max_pwr = 0;
  max_sil = 0;
#endif
}


void ph_audio_init_ivad(phastream_t *stream)
{
  int samples = (stream->clock_rate)/1000;
  struct vadcng_info *cngp = &stream->cngi;

#ifdef PH_FORCE_16KHZ
  samples = 16000/1000;
#endif

  ph_audio_init_vad0(cngp, samples);
}


void ph_audio_init_ovad(phastream_t *stream)
{
  int samples = (stream->clock_rate)/1000;
  struct vadcng_info *cngp = &stream->cngo;

#ifdef PH_FORCE_16KHZ
  samples = 16000/1000;
#endif

  ph_audio_init_vad0(cngp, samples);
}


void ph_audio_init_cng(phastream_t *stream)
{
  struct vadcng_info *cngp = &stream->cngi;

  cngp->noise = osip_malloc(NOISE_B_LEN);
  if(!cngp->noise)
  {
    cngp->cng = 0;
    DBG_DYNA_AUDIO("No memory for NOISE ! \n");
  }
  else
  {
#if 0
    pthread_mutex_init(&cngp->cng_lock);
#endif
    cngp->nidx = 0;
  }
}


void *ph_ec_init(int framesize, int clock_rate)
{
  int frame_samples = framesize/2;
  int frame_msecs =  frame_samples / (clock_rate/1000);
  void *ctx;
  const char *len = getenv("PH_ECHO_LENGTH");

  if (!len)
  {
    len = DFLT_ECHO_LENGTH;
  }

#ifdef USE_SPXEC
  ctx = spxec_echo_state_init(frame_samples, (atoi(len)/frame_msecs) * frame_samples);
#else
  ctx = create_AEC(0, clock_rate);
#endif

  return ctx;
}


void ph_ec_cleanup(void *ctx)
{
#ifdef USE_SPXEC
  spxec_echo_state_destroy(ctx);
#else
  kill_AEC(ctx);
#endif
}

static void
setup_recording(phastream_t *stream)
{
  // recording activity
    
  ////////////////////////////////
  // recorder for the AEC activity
  // records a raw 3 channel file
  ////////////////////////////////
  stream->activate_recorder = 0;
  if (getenv("PH_USE_RECORDER"))
  {
    stream->activate_recorder = atoi(getenv("PH_USE_RECORDER"));
  }

  if (stream->activate_recorder)
  {
    char *rname = getenv("PH_RECORD_FILE");
    char fname[128];
    static int fnindex = 1;
    if (!rname)
    {
      rname = "recording%d.sw";
    }

    snprintf(fname, 128, rname, fnindex++);
    ph_media_audio_recording_init(&stream->recorder, fname, 3, 4000);
  }

  //////////////////////////////////////
  // recorder of the post-encoder stream
  //////////////////////////////////////
  stream->record_send_stream = 0;
  if (getenv("PH_RECORD_SEND_STREAM"))
  {
    stream->record_send_stream = atoi(getenv("PH_RECORD_SEND_STREAM"));
  }

  if (stream->record_send_stream)
  {
    char *rname = getenv("PH_SEND_STREAM_FILE");
    char fname[128];
    static int sfnindex = 1;
    if (!rname)
    {
       rname = DFLT_SEND_STREAM_FILE;
    }

    snprintf(fname, 128, rname, sfnindex++);
    ph_media_audio_fast_recording_init(&stream->send_stream_recorder, fname);
  }

  ////////////////////////////////////////
  // recorder of the untouched MIC stream
  // records a raw 1 channel file
  ////////////////////////////////////////
  stream->record_mic_stream = 0;
  if (getenv("PH_RECORD_MIC_STREAM"))
  {
    stream->record_mic_stream = atoi(getenv("PH_RECORD_MIC_STREAM"));
  }

  if (stream->record_mic_stream)
  {
    char *rname = NULL;
    char fname[128];
    static int mic_filename_index = 1;
    rname = "micstream%d.data";
    snprintf(fname, 128, rname, mic_filename_index++);
    ph_media_audio_fast_recording_init(&stream->mic_stream_recorder, fname);
  }

  ////////////////////////////////////////////////////
  // recorder of the MIC stream right after resampling
  // records a raw 1 channel file
  ////////////////////////////////////////////////////
  stream->record_mic_resample_stream = 0;
  if (getenv("PH_RECORD_MIC_RESAMPLE_STREAM"))
  {
    stream->record_mic_resample_stream = atoi(getenv("PH_RECORD_MIC_RESAMPLE_STREAM"));
  }

  if (stream->record_mic_resample_stream)
  {
    char *rname = NULL;
    char fname[128];
    static int mic_filename_index = 1;
    rname = "mic_resample_stream%d.data";
    snprintf(fname, 128, rname, mic_filename_index++);
    ph_media_audio_fast_recording_init(&stream->mic_resample_stream_recorder, fname);
  }

  //////////////////////////////////////////////////////////////////////
  // recorder of the SPK stream (stream as it is sent to the SPK driver)
  // records a raw 1 channel file
  //////////////////////////////////////////////////////////////////////
  stream->record_spk_stream = 0;
  if (getenv("PH_RECORD_SPK_STREAM"))
  {
    stream->record_spk_stream = atoi(getenv("PH_RECORD_SPK_STREAM"));
  }

  if (stream->record_spk_stream)
  {
    char *rname = NULL;
    char fname[128];
    static int spk_filename_index = 1;
    rname = "spk_stream%d.data";
    snprintf(fname, 128, rname, spk_filename_index++);
    ph_media_audio_fast_recording_init(&stream->spk_stream_recorder, fname);
  }

  //////////////////////////////////////////////////////////////////////
  // recorder of the flying stream (stream as it is sent to the driver)
  // records a raw 1 channel file
  //////////////////////////////////////////////////////////////////////
  stream->record_flying_mono_stream = 0;
  if (getenv("PH_RECORD_FLYING_MONO_STREAM"))
  {
    stream->record_flying_mono_stream = atoi(getenv("PH_RECORD_FLYING_MONO_STREAM"));
  }

  if (stream->record_flying_mono_stream)
  {
    char *rname = NULL;
    char fname[128];
    static int flying_mono_filename_index = 1;
    rname = "flying_mono_stream%d.data";
    snprintf(fname, 128, rname, flying_mono_filename_index++);
    ph_media_audio_fast_recording_init(&stream->flying_mono_stream_recorder, fname);
  }
  
  
}

static void
cleanup_recording(phastream_t *stream)
{
  if (stream->activate_recorder)
  {
    ph_media_audio_recording_close(&stream->recorder);
  }

  if (stream->record_mic_stream)
  {
    ph_media_audio_recording_close(&stream->mic_stream_recorder);
  }

  if (stream->record_mic_resample_stream)
  {
    ph_media_audio_recording_close(&stream->mic_resample_stream_recorder);
  }

  if (stream->record_spk_stream)
  {
    ph_media_audio_recording_close(&stream->spk_stream_recorder);
  }

  if (stream->record_flying_mono_stream)
  {
    ph_media_audio_recording_close(&stream->flying_mono_stream_recorder);
  }

}

static int 
select_audio_device(const char *deviceId)
{
	if (ph_activate_audio_driver(deviceId))
	{
		return -PH_NORESOURCES;
	}

	return 0;
}


/**
 * @brief opens a device for a given stream
 */
static int 
open_audio_device(struct ph_msession_s *s, phastream_t *stream, const char *deviceId)
{
  int fd;
  int clockrate = stream->clock_rate;
  int framesize = ph_astream_decoded_framesize_get(stream);

#ifdef PH_FORCE_16KHZ
  // devices should be opened with a 16000 Hz sampling rate
  if (clockrate == 8000)
  {
    clockrate = 16000;
    framesize *= 2;
  }
#endif

  if (s->confflags != PH_MSESSION_CONF_MEMBER)
  {
    fd = audio_stream_open(stream, (char *)deviceId, clockrate, framesize, ph_audio_callback); 

    if (fd < 0)
    {
      DBG_MEDIA_ENGINE("open_audio_device: can't open  AUDIO device\n");
      if( phcb->errorNotify )
      {
        phcb->errorNotify(PH_NOAUDIODEVICE);
      }
	  owplFireErrorEvent(OWPL_ERROR_NO_AUDIO_DEVICE, 0);
      return -1;
    }

    DBG_DYNA_AUDIO_DRV("opened i/o devices: (s->rate, s->fsize)=(%d,%d) - (rate, fsize)=(%d,%d) - (s->actual_rate)=(%d)\n",
      stream->clock_rate,
      ph_astream_decoded_framesize_get(stream),
      clockrate,
      framesize,
      stream->actual_rate);
  }
  else
  {
    stream->actual_rate = clockrate;
  }

  return 0;
}

/**
 * @brief start audio engine (read, writes)
 */
static void
start_audio_device(struct ph_msession_s *s, phastream_t *stream)
{
  stream->using_out_callback = audio_driver_has_play_callback();

  /*
     replay of last frame is only interesting when we're working in callback mode
     if activated in non callback mode it feeds the audio playback queue with one audio frame
     each 10 msecs, which is
  */

  if (0 && stream->using_out_callback)
  {
    stream->lastframe = calloc(ph_astream_decoded_framesize_get(stream), 1);
  }

  if (s->confflags != PH_MSESSION_CONF_MEMBER)
  {
    DBG_MEDIA_ENGINE("start mediastreamer: audio device threading model part\n");
    audio_stream_start(stream);
  }

  if ((!stream->ms.media_io_timer && (s->confflags != PH_MSESSION_CONF_MEMBER))
    && (!audio_driver_has_rec_callback() || !audio_driver_has_play_callback()))
  {
    DBG_MEDIA_ENGINE("initialize phapi threading model part\n");
    timer_init();
    stream->ms.media_io_timer_impl = timer_impl_getfirst();
    stream->ms.media_io_timer = stream->ms.media_io_timer_impl->timer_create();
#if defined(OS_LINUX)
    stream->ms.media_io_timer_impl->timer_set_delay(stream->ms.media_io_timer, 10);
#else
	stream->ms.media_io_timer_impl->timer_set_delay(stream->ms.media_io_timer, stream->ptime);
#endif
    stream->ms.media_io_timer_impl->timer_set_callback(stream->ms.media_io_timer,
        ph_audio_io_thread);
    stream->ms.media_io_timer_impl->timer_set_userdata(stream->ms.media_io_timer,
        stream);
    stream->ms.media_io_timer_impl->timer_start(stream->ms.media_io_timer);
  }
}

static void
setup_hdx_mode(struct ph_msession_s *s, phastream_t *stream)
{
  ph_mstream_params_t *sp = &s->streams[PH_MSTREAM_AUDIO1];

  // SPIKE_HDX: initialization for mode = MIC has priority
  if ((sp->flags & PH_MSTREAM_FLAG_MICHDX) || getenv("PH_FORCE_MICHDX"))
  {
    char*  fhdx =  getenv("PH_FORCE_MICHDX");

    stream->hdxmode = PH_HDX_MODE_MIC;
    stream->hdxsilence = 1;
    if (fhdx)
    {
      sp->vadthreshold = atoi(fhdx);
    }

    DBG_MEDIA_ENGINE("ph_mession_audio_start: MICHDX mode level=%d\n",  sp->vadthreshold);
  }

  // SPIKE_HDX: initialization for mode = SPK has priority
  if ((sp->flags & PH_MSTREAM_FLAG_SPKHDX) || getenv("PH_FORCE_SPKHDX"))
  {
    char*  spkfhdx =  getenv("PH_FORCE_SPKHDX");

    stream->hdxmode = PH_HDX_MODE_SPK;
    stream->cngo.pwr_threshold = 700;
    stream->spksilence = 1;

    if (spkfhdx)
    {
      stream->cngo.pwr_threshold = atoi(spkfhdx);
    }

    DBG_MEDIA_ENGINE("ph_mession_audio_start: SPKHDX mode level=%d\n",  stream->cngo.pwr_threshold);
  }
}

static void
setup_aec(struct ph_msession_s *s, phastream_t *stream)
{
  ph_mstream_params_t *sp = &s->streams[PH_MSTREAM_AUDIO1];

  if (!(sp->flags & PH_MSTREAM_FLAG_AEC))
  {
#ifdef DO_ECHO_CAN  
    DBG_DYNA_AUDIO_ECHO("setup_aec: Echo CAN desactivated\n");
    stream->ec = 0;
  }
  else
  {
    int framesize = ph_astream_decoded_framesize_get(stream);
    int clockrate = stream->clock_rate;

#ifdef PH_FORCE_16KHZ
    if (clockrate == 8000)
    {
      clockrate = 16000;
      framesize *= 2;
    }
#endif
    stream->ec = ph_ec_init(framesize, clockrate);
    if (stream->ec)
    {
      const char *lat = getenv("PH_ECHO_LATENCY");

#if 0
    if (!lat)
	{
      lat = DFLT_ECHO_LATENCY;
	}
#endif

      stream->audio_loop_latency = 0;
      if (lat)
      {
        stream->audio_loop_latency = atoi(lat) * 2 * clockrate/1000;
      }

      /*
        circular buffer must be able to accomodate MAX_OUTPUT_LATENCY millisecs in output direction
        and the same amount in input direction 
      */
      cb_init(&stream->pcmoutbuf, 2 * sizeof(short) * MAX_OUTPUT_LATENCY * clockrate/1000);
      stream->sent_cnt = stream->recv_cnt = 0;
      pthread_mutex_init(&stream->ecmux, NULL);

    }
    DBG_DYNA_AUDIO_ECHO("ph_msession_audio_start: Echo CAN created OK\n");
#endif
  }

}

/**
 * @brief stops a specific stream in a session
 *
 * it currently always try to stop the AUDIO1 stream
 * @param s ph_msession in which the AUDIO1 stream is found
 * @param deviceId phapi device name alias of the related device
 * @param stopdevice 0/1 should the device be stopped or kept open even if the stream is stopped
 * @param hardstop 0/1 a hardstop is a final and definitive stop upon hangup
 */
void ph_msession_audio_stream_stop(struct ph_msession_s *s, const char *deviceId, int stopdevice, int hardstop)
{
  struct ph_mstream_params_s *msp = &s->streams[PH_MSTREAM_AUDIO1];
  phastream_t *stream = (phastream_t *) msp->streamerData;
  int confflags = s->confflags;
  phastream_t *master;
  struct ph_msession_s *s2 = s->confsession;

  // we cannot stop a non-existing or non-running stream
  if (!stream || !stream->ms.running)
  {
    return;
  }

  // inform the engine that a stream stop is required
  stream->ms.running = 0;
  s->activestreams &= ~(1 << PH_MSTREAM_AUDIO1);

  // if a thread was needed in the threading model, wait and destroy it
  if (stream->ms.media_io_timer)
  {
    DBG_MEDIA_ENGINE("ph_msession_audio_stream_stop: stop mediastreamer phapi threading model part\n");
    stream->ms.media_io_timer_impl->timer_stop(stream->ms.media_io_timer);
    stream->ms.media_io_timer_impl->timer_destroy(stream->ms.media_io_timer);
    stream->ms.media_io_timer = 0;
  }

  if (stopdevice)
  {
    DBG_MEDIA_ENGINE("ph_msession_audio_stream_stop: close audio driver\n");
    audio_stream_close(stream); /* close the sound card */
  }

  // if the stream is going to disappear and a conf is taking place
  // it is necessary to unlink the members of the conf
  if (hardstop && confflags)
  {
    ph_msession_audio_stream_conf_unlink(s->confsession, s);
  }

#ifdef PH_USE_RESAMPLE
  // clean the resampling context
  if (stream->actual_rate != stream->clock_rate)
  {
    ph_resample_cleanup0(stream->resample_audiodrv_ctx_mic);
    ph_resample_cleanup0(stream->resample_audiodrv_ctx_spk);
  }
#endif

  msp->flags &= ~PH_MSTREAM_FLAG_RUNNING;

  if (stream->mixbuf)
  {
    ph_mediabuf_free(stream->mixbuf);
    stream->mixbuf = 0;
  }

  ph_mediabuf_cleanup(&stream->data_in);
  memset(&stream->data_in, 0, sizeof(stream->data_in));
  ph_mediabuf_cleanup(&stream->data_out);
  memset(&stream->data_out, 0, sizeof(stream->data_out));

#ifdef TRACE_POWER
  print_pwrstats(stream);
#endif

  //  RTP_SESSION_LOCK(stream->rtp_session);
  rtp_session_signal_disconnect_by_callback(stream->ms.rtp_session, "telephone-event",
    (RtpCallback)ph_telephone_event);
  rtp_session_signal_disconnect_by_callback(stream->ms.rtp_session, "cng_packet",
    (RtpCallback)ph_on_cng_packet);

  // WENGO
  //ortp_set_debug_file("oRTP", stdout);
  ortp_set_log_file(stdout);

  //ortp_global_stats_display();
  //ortp_session_stats_display(stream->ms.rtp_session);
  rtp_stats_display(&stream->ms.rtp_session->rtp.stats, "Session statistics");
  // WENGO
  //ortp_set_debug_file("oRTP", NULL);
   ortp_set_log_file(NULL);
  {
    /* free non default profiles */
    RtpProfile *rprofile, *sprofile;
      sprofile = rtp_session_get_send_profile(stream->ms.rtp_session);
    if (sprofile != &av_profile)
    {
      rtp_profile_destroy(sprofile);
    }

    rprofile = rtp_session_get_recv_profile(stream->ms.rtp_session);
    if (rprofile != &av_profile && rprofile != sprofile)
    {
      rtp_profile_destroy(rprofile);
    }
  }

  // Wengo Julien - Add OWSL support
  if(stream->ms.rtp_session->rtp.tr)
  {
	ph_rtp_transport_data_t *user_data = stream->ms.rtp_session->rtp.tr->data;

	if (user_data)
	{
		owsl_close(user_data->rtp_sock);
		owsl_close(user_data->rtcp_sock);

		free(user_data);
	}
  }

  rtp_session_destroy(stream->ms.rtp_session);
  stream->ms.rtp_session = 0;

  if (stream->ms.codec->encoder_cleanup)
  {
    stream->ms.codec->encoder_cleanup(stream->ms.encoder_ctx);
    stream->ms.encoder_ctx = 0;
  }
  if (stream->ms.codec->decoder_cleanup)
  {
    stream->ms.codec->decoder_cleanup(stream->ms.decoder_ctx);
    stream->ms.decoder_ctx = 0;
  }
  stream->ms.codec = 0;

  ph_audio_vad_cleanup(stream);

#ifdef DO_ECHO_CAN
  if(stream->ec)
  {
    cb_clean(&stream->pcmoutbuf);

    if(stream->ec)
    {
      ph_ec_cleanup(stream->ec);
    }
    pthread_mutex_destroy(&stream->ecmux);
    stream->ec =0;
  }

  // TODO: refactor echo canceller cleanup
  stream->sent_cnt = 0;
  stream->recv_cnt = 0;
  stream->read_cnt = 0;
  stream->mic_current_sample = 0;
  stream->spk_current_sample = 0;

  stream->ec_phase=0;
  stream->audio_loop_latency=0;
  stream->underrun=0;
#endif

  if (stream->reformatbuf.cb_buf)
    cb_clean(&stream->reformatbuf);

  stream->hdxmode = 0;

  cleanup_recording(stream);

  DBG_MEDIA_ENGINE("\naudio stream closed\n");

  if (stream->lastframe)
  {
    free(stream->lastframe);
    stream->lastframe = 0;
  }

#ifdef PH_FORCE_16KHZ
  if (stream->resamplectx)
  {
    ph_resample_cleanup(stream->resamplectx);
    stream->resamplectx = 0;
  }
#endif

  stream->ms.rxtstamp = 0;
  stream->ms.txtstamp = 0;
  stream->ms.rxts_inc = 0;

  // handover of the audio device
  if (confflags == PH_MSESSION_CONF_MASTER)
  {
    struct ph_mstream_params_s *msp2 = &s2->streams[PH_MSTREAM_AUDIO1];
    phastream_t *stream2 = (phastream_t *) msp2->streamerData;

    DBG_MEDIA_ENGINE("audio_stop: removing conf master\n");

    if (hardstop)
    {
      s2->confflags = 0;
      stream2->master = 0;
    }

    /* if the slave stream is not suspended,  start audio streaming for it */
    //if (stream2 && !stream2->ms.suspended)
    if (stream2 && (stream2->ms.mses->activestreams & (1 << PH_MSTREAM_AUDIO1)))
    {
      // we need to switch CONF roles in order to hear something
      if (s->confflags == PH_MSESSION_CONF_MASTER)
      {
        s->confflags = PH_MSESSION_CONF_MEMBER;
        s2->confflags = PH_MSESSION_CONF_MASTER;
      }

      s2->newstreams |= (1 << PH_MSTREAM_AUDIO1);
      if (!open_audio_device(s2, stream2, deviceId))
      {
        start_audio_device(s2, stream2);
      }
      DBG_MEDIA_ENGINE("audio_stop: started audio for ex-slave\n");
    }
  }

  // cleanup the phmstream structure, but remember the phmsession link
  //memset(&stream->ms, 0, sizeof(struct phmstream));
  //stream->ms.mses = s;

  // do not finish the cleanup work if it is only a suspension
  // or a restart
  if (hardstop)
  {
    pthread_mutex_destroy(&stream->dtmfi.dtmfg_lock); // for CONF_LOCK in SUSPEND mode..
    osip_free(stream);
    msp->streamerData = 0;
  }
}

/**
 * @brief used to activate what needs to be activated on an audio session
 *
 * this function is used by phapi.c through phmedia.c
 */
int ph_msession_audio_start(struct ph_msession_s *s, const char* deviceId)
{
  int stream_start_error;
  PH_MSESSION_AUDIO_LOCK();
  stream_start_error = ph_msession_audio_stream_start(s, deviceId);
  PH_MSESSION_AUDIO_UNLOCK();
  return stream_start_error;
}

/**
 * @brief hard start (or 'slow start') of a stream
 * 
 * this leads to a full ressource allocation and threading model startup for a stream
 * it is usually called when a stream has never been started or when a RE-INVITE asks
 * for a codec modification
 */
phastream_t * ph_msession_audio_stream_hardstart(struct ph_msession_s *s, int codecpt, const char* deviceId)
{
  phastream_t *stream = NULL;
  ph_mstream_params_t *sp = &s->streams[PH_MSTREAM_AUDIO1];
  phcodec_t *codec = NULL;
  RtpSession *session = NULL;
  RtpProfile *rprofile = &av_profile;
  RtpProfile *sprofile = &av_profile;
  int iPayloadIndex;
  int iPayloadPtime = 0;

  DBG_MEDIA_ENGINE("MEDIA ENGINE: ph_msession_audio_start devid=%s, confflags=%d\n", deviceId, s->confflags);
  DBG_MEDIA_ENGINE("MEDIA ENGINE: hardstart - looking for codec with payload = %d\n", codecpt);
  codec = ph_media_lookup_codec(codecpt);
  if (!codec)
  {
    DBG_MEDIA_ENGINE("hardstart: found NO codec\n");
    return NULL;
  }

  if (sp->streamerData)
  {
    DBG_MEDIA_ENGINE("reusing existing stream structure\n");

    stream = sp->streamerData;

    // reset correct linkage between potential conference streams
    // TODO(cf jwa): check if this is stricly necessary now that the stream
    // is not zeroed upon a RE-INVITE or RESUME
    if (s->confflags)
    {
      ph_msession_audio_conf_link(s, s->confsession);
    /*
      struct ph_mstream_params_s *msp1 = &s->streams[PH_MSTREAM_AUDIO1];
      phastream_t *stream1 = (phastream_t *) msp1->streamerData;
      struct ph_mstream_params_s *msp2 = &s->confsession->streams[PH_MSTREAM_AUDIO1];
      phastream_t *stream2 = (phastream_t *) msp2->streamerData;
      if (s->confflags == PH_MSESSION_CONF_MASTER)
      {
        stream1->to_mix = stream2;
        stream2->master = stream1;
      }
      if (s->confflags == PH_MSESSION_CONF_MEMBER)
      {
        stream2->to_mix = stream1;
        stream1->master = stream2;
      }
        */
    }
  }
  else
  {
    DBG_MEDIA_ENGINE("allocating a new stream structure\n");

    stream = (phastream_t *)osip_malloc(sizeof(phastream_t));
    if (!stream)
    {
      DBG_MEDIA_ENGINE("out of memory\n");
      return NULL;
    }
    memset(stream, 0, sizeof(*stream));
  }

  if (codec->encoder_init)
  {
    for (iPayloadIndex = 0; iPayloadIndex < PH_MSTREAM_PARAMS_MAX_PAYLOADS; iPayloadIndex++)
    {
      if (sp->opayloads[iPayloadIndex].number == codecpt)
	  {
        stream->ms.encoder_ctx = codec->encoder_init(&sp->opayloads[iPayloadIndex]);
	  }
    }
  }

  if (codec->decoder_init)
  {
    for (iPayloadIndex = 0; iPayloadIndex < PH_MSTREAM_PARAMS_MAX_PAYLOADS; iPayloadIndex++)
    {
      if (sp->ipayloads[iPayloadIndex].number == codecpt)
	  {
        stream->ms.decoder_ctx = codec->decoder_init(&sp->ipayloads[iPayloadIndex]);
	  }
	}
  }

  // set the correct ptime negociated during the SIP transaction
  for (iPayloadIndex = 0; iPayloadIndex < PH_MSTREAM_PARAMS_MAX_PAYLOADS; iPayloadIndex++)
  {
    if ( (sp->opayloads[iPayloadIndex].number == codecpt) && 
        (sp->opayloads[iPayloadIndex].ptime != 0) )
    {
      iPayloadPtime = sp->opayloads[iPayloadIndex].ptime;
    }
  }
  
  if (iPayloadPtime != 0)
  {
	  stream->ptime = iPayloadPtime;
  }
  else
  {
	  stream->ptime = sp->ptime;
  }

  DBG_MEDIA_ENGINE("hardstart: new audiostream = %08x\n", stream);

  // setup recorders for the stream
  setup_recording(stream);

  gettimeofday(&stream->last_rtp_sent_time, 0);
  stream->cngi.last_noise_sent_time = stream->last_rtp_recv_time = stream->cngi.last_dtx_time = stream->last_rtp_sent_time;

  setup_hdx_mode(s, stream);

  stream->ms.mses = s;
  stream->clock_rate = stream->actual_rate = codec->clockrate;
  stream->cngi.vad = (sp->flags & PH_MSTREAM_FLAG_VAD)?1:0;
  stream->cngi.pwr_threshold = sp->vadthreshold;
  stream->cngi.cng = (sp->flags & PH_MSTREAM_FLAG_CNG)?1:0;
  stream->ms.codec = codec;
  stream->ms.payload =  sp->ipayloads[0].number;
  /* FIXME: we need to handle separate directions too */
  stream->cngi.cng_pt = (stream->clock_rate > 8000) ? PH_MEDIA_CN_16000_PAYLOAD : PH_MEDIA_CN_PAYLOAD;

  DBG_MEDIA_ENGINE("ph_mession_audio_start: DTX/VAD %x\n", stream->cngi.pwr_threshold);
  DBG_MEDIA_ENGINE("ph_msession_audio_start: clock rate %d\n", stream->clock_rate);
  DBG_MEDIA_ENGINE("ph_msession_audio_start: CNG %s\n", stream->cngi.cng ? "activating" : "desactivating");
  DBG_MEDIA_ENGINE("ph_msession_audio_start: opening AUDIO device %s\n", deviceId);

  // try to open the device and negociate internal_clockrate
  if (open_audio_device(s, stream, deviceId))
  {
    // TODO: needs better cleanup
    DBG_MEDIA_ENGINE("MEDIA ENGINE:hardstart: cannot open audio driver\n");

    if (codec->encoder_init)
    {
		codec->encoder_cleanup(stream->ms.encoder_ctx);
		stream->ms.encoder_ctx = NULL;
    }

    if (codec->decoder_init)
    {
		codec->decoder_cleanup(stream->ms.decoder_ctx);
		stream->ms.decoder_ctx = NULL;
    }

    free(stream);
    return NULL;
  }

  if (stream->actual_rate == 0) {
    DBG_MEDIA_ENGINE("Fatal: no sampling rate negociated by device\n");

    if (codec->encoder_init)
    {
		codec->encoder_cleanup(stream->ms.encoder_ctx);
		stream->ms.encoder_ctx = NULL;
    }

    if (codec->decoder_init)
    {
		codec->decoder_cleanup(stream->ms.decoder_ctx);
		stream->ms.decoder_ctx = NULL;
    }

    return NULL;
  }

  // if the audio device do not support the expected clockrate
  // we need to resample audio data
#ifdef PH_USE_RESAMPLE
  if (stream->clock_rate != stream->actual_rate)
  {
    stream->resample_audiodrv_ctx_spk = ph_resample_spk_init0(stream->clock_rate, stream->actual_rate);
    stream->resample_audiodrv_ctx_mic = ph_resample_mic_init0(stream->clock_rate, stream->actual_rate);
  }
#endif


#ifdef PH_FORCE_16KHZ
  if (stream->clock_rate != stream->actual_rate)
  {
    stream->resamplectx = ph_resample_init();
  }
#endif

  // define TX - IP:port for the stream
  strcpy(stream->ms.remote_ip, sp->remoteaddr);
  stream->ms.remote_port = sp->remoteport;

  ph_mediabuf_init(&stream->data_in, malloc(2048), 2048);
  ph_mediabuf_init(&stream->data_out, malloc(2048), 2048);
  DBG_MEDIA_ENGINE("ph_msession_audio_start: opening session remoteport: %d\n", stream->ms.remote_port);

  session = rtp_session_new(RTP_SESSION_SENDRECV);

	rtp_session_set_scheduling_mode(session, SCHEDULING_MODE); /* yes */
	rtp_session_set_blocking_mode(session, BLOCKING_MODE);

  if (codecpt != stream->ms.payload)
  {
    rprofile = sprofile = rtp_profile_clone_full(sprofile);
    rtp_profile_move_payload(sprofile, codecpt, ph_speex_hook_pt);
  }
  else if (sp->ipayloads[0].number != sp->opayloads[0].number)
  {
    sprofile = rtp_profile_clone_full(sprofile);
    rtp_profile_move_payload(sprofile, sp->ipayloads[0].number,  sp->opayloads[0].number);
  }

  rtp_session_set_send_profile(session, sprofile);
  rtp_session_set_recv_profile(session, rprofile);

  rtp_session_enable_adaptive_jitter_compensation(session, 1);
  rtp_session_set_jitter_compensation(session, sp->jitter);

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
				return NULL;
			}

			raddr_rtp.sin_addr.s_addr = inet_addr(sp->remoteaddr);
			raddr_rtp.sin_port = htons((short)sp->remoteport);
			raddr_rtp.sin_family = AF_INET;

			if (owsl_connect(rtp_sock, (struct sockaddr *) &raddr_rtp, sizeof(raddr_rtp)))
			{
				owsl_close(rtp_sock);
				return NULL;
			}

			owsl_blocking_mode_set(rtp_sock, OWSL_NON_BLOCKING);
		}
		else
		{
			return NULL;
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
				return NULL;
			}

			raddr_rtcp.sin_addr.s_addr = inet_addr(sp->remoteaddr);
			raddr_rtcp.sin_port = htons((short)sp->remoteport + 1);
			raddr_rtcp.sin_family = AF_INET;

			if (owsl_connect(rtcp_sock, (struct sockaddr *) &raddr_rtcp, sizeof(raddr_rtcp)))
			{
				owsl_close(rtcp_sock);
				return NULL;
			}

			owsl_blocking_mode_set(rtcp_sock, OWSL_NON_BLOCKING);
		}
		else
		{
			return NULL;
		}

		user_data = malloc(sizeof(ph_rtp_transport_data_t));
		memset(user_data, 0, sizeof(ph_rtp_transport_data_t));
		user_data->rtp_sock = rtp_sock;
		user_data->rtcp_sock = rtcp_sock;

		// sVoIP integration 
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
		// sVoIP 

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
		
		rtp_session_set_transports(session, rtp_transport, rtcp_transport);
	}

  //rtp_session_set_payload_types(session, sp->opayloads[0].number, sp->ipayloads[0].number);
  rtp_session_set_send_payload_type(session, sp->opayloads[0].number);
  rtp_session_set_recv_payload_type(session, sp->ipayloads[0].number);
  //rtp_session_set_cng_pt(session, stream->cngi.cng_pt);
  rtp_session_signal_connect(session, "telephone-event", (RtpCallback)ph_telephone_event, s);
  rtp_session_signal_connect(session, "cng_packet", (RtpCallback)ph_on_cng_packet, s);

  stream->ms.rtp_session = session;

  // SPIKE_HDX: init the voice activity detection on the local input stream (MIC)
  if (stream->cngi.vad || stream->hdxmode == PH_HDX_MODE_MIC)
  {
    ph_audio_init_ivad(stream);
  }

  // SPIKE_HDX: init the voice activity detection on the local output stream (SPK)
  if (stream->hdxmode == PH_HDX_MODE_SPK)
  {
    ph_audio_init_ovad(stream);
  }

  if (stream->cngi.cng)
  {
    ph_audio_init_cng(stream);
  }

  setup_aec(s, stream);

  stream->dtmfCallback = s->dtmfCallback;
  pthread_mutex_init(&stream->dtmfi.dtmfg_lock, NULL);
  stream->dtmfi.dtmfq_cnt = 0;
  stream->dtmfi.dtmfg_phase = DTMF_IDLE;

  sp->flags |= PH_MSTREAM_FLAG_RUNNING;
  sp->streamerData = stream;
  s->activestreams |= (1 << PH_MSTREAM_AUDIO1);

  DBG_MEDIA_ENGINE("ph_mession_audio_start: s=%08x.stream=%08x\n", s, stream);

  stream->ms.running = 1;

  start_audio_device(s, stream);

  DBG_MEDIA_ENGINE("ph_msession_audio_start: audio stream init OK\n");

  return stream;
}

/**
 * @brief start/restart a specific stream in a session
 *
 * it currently always try to start the AUDIO1 stream
 * this is the official entry point for stream activation. It decides whether
 * a fast start or a slow start should be done
 * fast start = a stream is already active and a RE-INVITE can be done without a full de-alloc / re-alloc cycle
 * slow start (or hard start) = it is necessary to restart all the ressources
 */
int ph_msession_audio_stream_start(struct ph_msession_s *s, const char* deviceId)
{
  phastream_t *stream;
  int codecpt;
  ph_mstream_params_t *sp = &s->streams[PH_MSTREAM_AUDIO1];
  int newstreams;

  DBG_MEDIA_ENGINE("MEDIA ENGINE: ph_msession_audio_stream_start devid=%s, confflags=%d\n", deviceId, s->confflags);

  newstreams = s->newstreams;
  s->newstreams = 0;

  if (!(newstreams & (1 << PH_MSTREAM_AUDIO1)))
  {
    return 0;
  }

  if (!sp->localport || !sp->remoteport)
  {
    return 0;
  }

  if (select_audio_device(deviceId))
  {
    return -PH_NORESOURCES;
  }

  codecpt = sp->ipayloads[0].number;

  /* check if this payload should by replaced by SPEEX WB */
  if (codecpt == ph_speex_hook_pt)
  {
    codecpt = PH_MEDIA_SPEEXWB_PAYLOAD;
    DBG_MEDIA_ENGINE("ph_msession_audio_stream_start: replacing payload %d by %d\n", ph_speex_hook_pt, codecpt);
  }

  if (!sp->jitter)
  {
     sp->jitter = getenv_int( JITTER_BUFFER_ENV, DEFAULT_JITTER_BUFFER);
  }

  if (sp->flags & PH_MSTREAM_FLAG_RUNNING)
  {
    // begin branch 1 : an audio stream is already running.
    // we should process a media change if it is necessary on the (ip, port, payload)

    stream = (phastream_t*) sp->streamerData;
    assert(stream);

    DBG_MEDIA_ENGINE("ph_msession_audio_stream_start: current=%08x(rip=<%s:%u> pt=%d)=>(rip=<%s:%u> pt=%d)\n",
      stream, stream->ms.remote_ip, stream->ms.remote_port, stream->ms.payload,
      sp->remoteaddr, sp->remoteport, sp->ipayloads[0].number);

    if (stream->ms.remote_port == sp->remoteport)
    {
      if ((stream->ms.payload ==  sp->ipayloads[0].number) &&  !strcmp(stream->ms.remote_ip, sp->remoteaddr))
      {
          DBG_MEDIA_ENGINE("ph_msession_audio_stream_start: reusing current stream\n");
		  // Reset audio RTP flow and timestamps associated
		  rtp_session_reset(stream->ms.rtp_session);
		  stream->ms.rxtstamp = 0;
		  stream->ms.txtstamp = 0;
		  stream->ms.rxts_inc = 0;
          return 0;
      }
    }

    /* either IP's or payloads are different */ 
    if (stream->ms.payload ==  sp->ipayloads[0].number)
    {
      strcpy(stream->ms.remote_ip, sp->remoteaddr);
      stream->ms.remote_port = sp->remoteport;

      rtp_session_reset(stream->ms.rtp_session);

	  {
        rtp_session_set_remote_addr(stream->ms.rtp_session, stream->ms.remote_ip,	stream->ms.remote_port);
	  }

      DBG_MEDIA_ENGINE("ph_msession_audio_stream_start: audio stream reset done\n");
      return 0;
    }

    /* new payload is different from the old one */
    DBG_MEDIA_ENGINE("ph_msession_audio_stream_start: Replacing audio session\n");
    ph_msession_audio_stream_stop(s, deviceId, s->confflags != PH_MSESSION_CONF_MEMBER, 0);

  // end branch 1
  } //if (sp->flags & PH_MSTREAM_FLAG_RUNNING)

  // we need to start a lot of things : a hard start
  stream = ph_msession_audio_stream_hardstart(s, codecpt, deviceId);
  return (stream == NULL);
  // end branch 2
}


/**
 * @brief function used to hangup a call
 */
void ph_msession_audio_stop(struct ph_msession_s *s, const char *deviceId)
{ 
  struct ph_mstream_params_s *msp = &s->streams[PH_MSTREAM_AUDIO1];
  phastream_t *stream = (phastream_t *) msp->streamerData;
  int confflags = s->confflags; // save the confflags for future use
  struct ph_msession_s *s2 = s->confsession;

  PH_MSESSION_AUDIO_LOCK();
  ph_msession_audio_stream_stop(s, deviceId, confflags != PH_MSESSION_CONF_MEMBER, 1);
  PH_MSESSION_AUDIO_UNLOCK();
}


/**
 * @brief function used to suspend a call (HOLD)
 */
void ph_msession_audio_suspend(struct ph_msession_s *s, int suspendwhat, const char *deviceId)
{
  struct ph_mstream_params_s *msp = &s->streams[PH_MSTREAM_AUDIO1];
  phastream_t *stream = (phastream_t *) msp->streamerData;
  int confflags = s->confflags;
  struct ph_msession_s *s2 = s->confsession;

  DBG_MEDIA_ENGINE("audio_suspend: enter ses=%p stream=%p remoteport=%d\n", s, stream, stream->ms.remote_port);

  PH_MSESSION_AUDIO_LOCK();
  msp->traffictype &= ~suspendwhat;
  stream->ms.suspended = 1;
  ph_msession_audio_stream_stop(s, deviceId, confflags != PH_MSESSION_CONF_MEMBER, 0);
  PH_MSESSION_AUDIO_UNLOCK();

  DBG_MEDIA_ENGINE("audio_suspend: exit ses=%p stream=%p remoteport=%d\n", s, stream, stream->ms.remote_port);
}


/**
 * @brief function used to resume a call (OFF-HOLD or RESUME)
 */
int ph_msession_audio_resume(struct ph_msession_s *s, int resumewhat, const char *deviceId)
{
  int stream_start_error;
  struct ph_mstream_params_s *msp = &s->streams[PH_MSTREAM_AUDIO1];
  phastream_t *stream = (phastream_t *) msp->streamerData;

  DBG_MEDIA_ENGINE("MEDIA ENGINE:ph_msession_audio_resume:begin:ses=%p stream=%p remoteport=%d confflags=%d\n",
    s, stream, stream->ms.remote_port, s->confflags);

  PH_MSESSION_AUDIO_LOCK();

  msp->traffictype |= resumewhat;
  stream_start_error = ph_msession_audio_stream_start(s, deviceId);
  if (!stream_start_error)
  {
    stream->ms.suspended = 0;
  }

  PH_MSESSION_AUDIO_UNLOCK();

  DBG_MEDIA_ENGINE("MEDIA ENGINE:ph_msession_audio_resume:end:resume exit ses=%p stream=%p remoteport=%d confflags=%d\n",
    s, stream, stream->ms.remote_port, s->confflags);

  return stream_start_error;
}


/**
 * @brief used to join 2 sessions in a conference
 * currently it is only tested when nor s1 neither s2 is running
 */
int ph_msession_audio_conf_start(struct ph_msession_s *s1, struct ph_msession_s *s2, const char *deviceId)
{
    deviceId; // just to avoid compilation warnings
    
    // error if one of the 2 sessions is already involved in a conf
    if (s1->confflags || s2->confflags)
    {
        return -PH_NORESOURCES;
    }

    ph_msession_audio_conf_link(s1, s2);
    return 0;
}

/**
 * @brief ties 2 sessions together and set the correct MASTER/MEMBER relationship
 */
int ph_msession_audio_conf_link(struct ph_msession_s *s1, struct ph_msession_s *s2)
{
  struct ph_mstream_params_s *msp1 = &s1->streams[PH_MSTREAM_AUDIO1];
  phastream_t *stream1 = (phastream_t *) msp1->streamerData;
  struct ph_mstream_params_s *msp2 = &s2->streams[PH_MSTREAM_AUDIO1];
  phastream_t *stream2 = (phastream_t *) msp2->streamerData;

  s1->confsession = s2;
  s2->confsession = s1;

  // if S1 is running, it will be the MASTER
  if (stream1->ms.running)
  {
    DBG_MEDIA_ENGINE("ph_msession_audio_conf_start (1): MASTER (%p) MEMBER (%p)\n", s1, s2);

    CONF_LOCK(stream1);
    stream1->to_mix = stream2;
    if (stream2)
    {
      stream2->master = stream1;
    }
    s1->confflags = PH_MSESSION_CONF_MASTER;
    s2->confflags = PH_MSESSION_CONF_MEMBER;
    CONF_UNLOCK(stream1);
    return 0;
  }

  // if S2 is running, it will be the MASTER
  if (stream2->ms.running)
  {
    DBG_MEDIA_ENGINE("ph_msession_audio_conf_start (2): MASTER (%p) MEMBER (%p)\n", s2, s1);

    CONF_LOCK(stream2);
    stream2->to_mix = stream1;
    if (stream1)
    {
      stream1->master = stream2;
    }
    s2->confflags = PH_MSESSION_CONF_MASTER;
    s1->confflags = PH_MSESSION_CONF_MEMBER;
    CONF_UNLOCK(stream2);
    return 0;
  }

  DBG_MEDIA_ENGINE("ph_msession_audio_conf_start (3): MASTER (%p) MEMBER (%p)\n", s1, s2);
  
  // if S1 and S2 are not running, S1 will be the MASTER
    
  CONF_LOCK(stream1);
  stream1->to_mix = 0; // TODO: must be 0 here because !0 needs !suspended but this is false for CONF+DBL_HOLD+RESUME. should be improved
  if (stream2)
  {
      stream2->master = stream1;
  }
  s1->confflags = PH_MSESSION_CONF_MASTER;
  s2->confflags = PH_MSESSION_CONF_MEMBER;
  CONF_UNLOCK(stream1);
  //ph_msession_audio_start(s1, deviceId);
  return 0;

}
/**
 * @brief function that unties to sessions that were previously binded for a conference
 */
int ph_msession_audio_stream_conf_unlink(struct ph_msession_s *s1, struct ph_msession_s *s2)
{
  struct ph_mstream_params_s *msp1 = &s1->streams[PH_MSTREAM_AUDIO1];
  phastream_t *stream1 = (phastream_t *) msp1->streamerData;
  struct ph_mstream_params_s *msp2 = &s2->streams[PH_MSTREAM_AUDIO1];
  phastream_t *stream2 = (phastream_t *) msp2->streamerData;

  CONF_LOCK(stream1);
  if (stream1->to_mix)
  {
    stream1->to_mix->master = 0;
    stream1->to_mix = 0;
  }

  CONF_UNLOCK(stream1);

  s1->confflags = 0;
  s1->confsession = 0;

  CONF_LOCK(stream2);
  if (stream2->to_mix)
  {
    stream2->to_mix->master = 0;
    stream2->to_mix = 0;
  }
  CONF_UNLOCK(stream2);

  s2->confflags = 0;
  s2->confsession = 0;

  return 0;
}

/**
 * @brief will be used in the future to properly stop a conf
 */
int ph_msession_audio_conf_stop(struct ph_msession_s *s1, struct ph_msession_s *s2)
{
  s1;s2; // just to avoid compilation warnings
  return 0;
}

void
ph_media_audio_init()
{
  static int first_time = 1;

  void ph_pa_driver_init();
  void ph_phadfile_driver_init();

#if defined(OS_LINUX)
  void ph_oss_driver_init();
  void ph_alsa_driver_init();

#ifdef ENABLE_ARTS
  void ph_audio_arts_driver_init();
#endif

#ifdef ENABLE_ESD
  void ph_audio_esd_driver_init();
#endif

#elif defined(OS_WINDOWS)
  void ph_winmm_driver_init();
#endif

#ifdef ENABLE_OPENAL
  void ph_openal_driver_init();
#endif /* ENABLE_OPENAL */

#ifdef OS_MACOSX
  void ph_ca_driver_init();
#endif

  if (!first_time)
  {
    return;
  }

  pthread_mutex_init(&ph_audio_mux, NULL);
#ifdef OS_WINDOWS
  ph_winmm_driver_init();
#endif

#ifdef OS_MACOSX
  ph_ca_driver_init();
#endif

#ifdef OS_LINUX

#ifdef ENABLE_ALSA
  ph_alsa_driver_init();
#endif

#ifdef ENABLE_OSS
  ph_oss_driver_init();
#endif

#ifdef ENABLE_ARTS
  ph_audio_arts_driver_init();
#endif

#ifdef ENABLE_ESD
  ph_audio_esd_driver_init();
#endif

#endif

#ifdef ENABLE_PORTAUDIO
  ph_pa_driver_init();
#endif


#ifdef ENABLE_OPENAL
  ph_openal_driver_init();
#endif

  ph_phadfile_driver_init();

  tg_init_sine_table();
  ph_gen_noise();
  ph_gen_silence();

  /* retrieve payload code to be replaced by SPEEX WB */ 
  {
    char *speexhook = getenv("PH_SPEEX_HOOK");
    char *mictrace = getenv("PH_TRACE_MIC");

    if (speexhook)
    {
      ph_speex_hook_pt = atoi(speexhook);
    }

    /* enbale MIC signal level tracing */
    if (mictrace)
    {
      ph_trace_mic = atoi(mictrace);
    }
  }

  first_time = 0;
  return;
}

int ph_msession_send_dtmf(struct ph_msession_s *s, int dtmf, int mode)
{
  phastream_t *stream = (phastream_t *)(s->streams[PH_MSTREAM_AUDIO1].streamerData);

  if (!stream)
  {
    return -1;
  }

  if (!mode || mode > 3)
  {
    mode = 3;
  }

  DTMF_LOCK(stream);
  if (stream->dtmfi.dtmfq_cnt < DTMFQ_MAX)
  {
    stream->dtmfi.dtmfq_buf[stream->dtmfi.dtmfq_wr++] = (unsigned short) (dtmf | (mode << 8));
    if (stream->dtmfi.dtmfq_wr == DTMFQ_MAX)
    {
      stream->dtmfi.dtmfq_wr = 0;
    }

    stream->dtmfi.dtmfq_cnt++;
    DTMF_UNLOCK(stream);
    return 0;
  }

  DTMF_UNLOCK(stream);
  return -1;
}


int ph_msession_send_sound_file(struct ph_msession_s *s, const char *filename)
{
  phastream_t *stream = (phastream_t *)(s->streams[PH_MSTREAM_AUDIO1].streamerData);
  ph_mediabuf_t *mb;

  if (!stream)
  {
    return -PH_NOMEDIA;
  }

  /* we're already mixing a something? */
  if (stream->mixbuf)
  {
    return -PH_NORESOURCES;
  }

  mb = ph_mediabuf_load(filename, stream->actual_rate);
  if (!mb)
  {
    return -PH_NORESOURCES;
  }

  stream->mixbuf = mb;
  return 0;
}


/* we're called by rtp to announce reception of DTMF event */
void
ph_telephone_event(RtpSession *rtp_session, int event, struct ph_msession_s *s)
{
  static const char evttochar[] = "0123456789*#ABCD!";
  phastream_t *stream = (phastream_t *)s->streams[PH_MSTREAM_AUDIO1].streamerData;

  if (stream->dtmfCallback)
  {
    stream->dtmfCallback(s->cbkInfo, evttochar[event]);
  }
}
