#ifndef __PHASTREAM_H__
#define __PHASTREAM_H__ 1

#include "tonegen.h"
#include "phmbuf.h"
#include "phrecorder.h"

#include <ortp.h>
#include "phcodec.h"
#include "phmstream.h"

#include <pthread.h>

# define MAX_IN_BUFFERS 4
# define USED_IN_BUFFERS 4
# define MAX_OUT_BUFFERS 4
# define USED_OUT_BUFFERS 4
# define AUDIO_INBUF_SIZE  4096
# define AUDIO_OUTBUF_SIZE 4096

struct circbuf
{
  int  cb_rdx;
  int  cb_wrx;
  int  cb_cnt;
  int  cb_siz;
  char *cb_buf;
};

#define DTMF_LOCK(s) pthread_mutex_lock(&s->dtmfi.dtmfg_lock)
#define DTMF_UNLOCK(s) pthread_mutex_unlock(&s->dtmfi.dtmfg_lock)

struct dtmf_info
{
#define DTMFQ_MAX 32
  unsigned short   dtmfq_buf[DTMFQ_MAX];
  int   dtmfq_wr;
  int   dtmfq_rd;
  int   dtmfq_cnt;

  enum { DTMF_IDLE, DTMF_GEN, DTMF_SILENT } dtmfg_phase;
  int   dtmfg_len;
  struct dtmfgen dtmfg_ctx;
  pthread_mutex_t dtmfg_lock;
};

/*
  DTX decision is made up after SIL_SETUP ms of low mean magnitude. The mean magnitude is calculated using sliding window of PWR_WINDOW ms
  PWR_WINDOW should be short - like 32ms
  The long mean magnitude used to generate the CNG frame is the sum of LONG_PWR_WINDOW mean magnitudes,
  calculated using also the sliding window
  LONG_PWR_WINDOW is set to 64 according to dynamic range (0-127dB) of signal to be sent in CNG frame
  Each PWR_WINDOW ms this sum of mean magnitudes is updated.
*/
struct vadcng_info
{

#define NOISE_START_DELAY   4000 /* in ms, the delay to start noise generation */  
#define SIL_SETUP       500   /* max time in ms  before stopping transfer */
#define PWR_WINDOW      128    //32    /* power calculate window in ms, for 8kHz il will be 32*8 samples, to optimize should be power of 2 */
#define LONG_PWR_WINDOW 64    /* long mean power window, to optimize should be power of 2 */
#define DTX_RETRANSMIT  10  /* timeout in seconds to retransmit cng packet */
#define RTP_RETRANSMIT  3  /* timeout in seconds to retransmit rtp or dummy CNG  packet if VAD activated */
  unsigned int vad;         /* VAD/DTX are activated or not, if b31 = 1,  the lower 31 bits indicates the wanted level */
  int cng;         /* CNG is activated or not for this stream */
  int cng_pt;       /* CNG payload type */
  struct timeval last_dtx_time;      /* time of last cng packet transfer */
  char got_cng;
  char lastsil;

  unsigned int long_pwr[LONG_PWR_WINDOW];     /* long power table */
  unsigned int long_mean_pwr;
  int long_pwr_shift;     /* shift  to optimize long mean magnitude calculation */
  int long_pwr_pos;      /* current power index */

  unsigned int *pwr;         /* power table */
  unsigned int pwr_threshold;   /* power threshold */
  unsigned int mean_pwr;     /* mean power of PWR_WINDOW ms*/
  int pwr_pos;      /* current power index */
  int pwr_size;     /* size of magnitude window */
  int pwr_shift;     /* shift  to optimize mean magnitude calculation */
  int sil_cnt;      /* current number of silence samples */
  int sil_max;      /* number of silence samples before stopping transfer */ 
  char *noise;      /* pointer to noise pattern */
  unsigned int noise_max;
  int nidx;      /* index for noise pattern */

#if 0
  phtread_mutex_t cng_lock;

#define CNG_LOCK(s) pthread_mutex_lock(&s->cng_lock)
#define CNG_UNLOCK(s) pthread_mutex_unlock(&s->cng_lock)
#else
#define CNG_LOCK(s)
#define CNG_UNLOCK(s)
#endif

  struct timeval last_noise_sent_time;  /* time of last sent noise packet */
};

#define MAX_FRAME_SIZE 160
#define PCM_TRACE_LEN (50*MAX_FRAME_SIZE)   /* data are stored for 50 frames of 10ms */ 
#define MAX_OUTPUT_LATENCY 250  /* (milliseconds) */

enum EC_PHASE { EC_PRETRAINING=1, EC_TRAINING };

struct ph_audio_stream
{
  struct phmstream ms;
  void   *drvinfo;

  // ptime negociated during the SIP transaction
  int ptime;

  /*
  	valid when this stream belongs to MASTER session during a conf.
  	it is a pointer to the stream that needs to be mixed
  */
  struct ph_audio_stream *to_mix;
  /*
  	valid when this stream belongs to MEMBER session during a conf.
  	it is a pointer to the stream that is the MASTER stream
  */
  struct ph_audio_stream *master;
  ph_mediabuf_t   data_in;
  ph_mediabuf_t   data_out;

  ph_mediabuf_t      *mixbuf;
  struct dtmf_info   dtmfi;
  struct vadcng_info cngi;
  struct vadcng_info cngo;

  // SPIKE_HDX: declaration of possible hdx state values on an audio stream
  int    hdxmode;
  int    hdxsilence; /* 0: MIC is busy, 1: MIC is silent */
  int    spksilence; /* 0: SPK is busy, 1: SPK is silent */

  struct circbuf reformatbuf;   /*  This is used if input stream has unexepectedly big payload */ 
#ifdef DO_ECHO_CAN
  void  *ec;
  struct circbuf pcmoutbuf;
  unsigned long sent_cnt; /* total char* sent to the echo canceller circbuf */
  unsigned long recv_cnt; /* total char* asked from the echo canceller circbuf */
  unsigned long read_cnt; /* total char* actually read out of the echo canceller circbuf */
  unsigned long mic_current_sample; /* sample number of the last sample that the microphone recorded. 0 is based on the first chunk received from the network */
  unsigned long spk_current_sample; /* sample number of the last sample that the speaker played. 0 is based on the first chunk received from the network */

  int      ec_phase;
  int      audio_loop_latency;
  int      underrun;
  pthread_mutex_t   ecmux;
#endif /* DO_ECHO_CAN */

  void   (*dtmfCallback)(void *ca, int dtmf);
  int clock_rate;
  int    actual_rate;                       /* actual sampling rate */

  struct timeval last_rtp_recv_time;  /* time of last received data packet */
  struct timeval last_rtp_sent_time;  /* time of last sent data packet */
  struct timeval now;                /* time of thread's current execution */

  int activate_recorder; /** 0/1 will init a recorder on "recording.raw" */
  int record_send_stream; /** 0/1 will init a recorder on "recording.raw" */
  int record_recv_stream; /** 0/1 will init a recorder on "recording.raw" */
  int record_mic_stream; /** 0/1 will init a recorder of the pure MIC recorded sound */
  int record_mic_resample_stream; /** 0/1 will init a recorder of the pure MIC recorded sound */
  int record_spk_stream; /** 0/1 will init a recorder of the pure SPK played sound */
  int record_flying_mono_stream; /** 0/1 will init a recorder that is flying (temporary) */

  recording_t recorder; /** recording structure associated with the activate_recorder int */
  recording_t send_stream_recorder;
  recording_t recv_stream_recorder;
  recording_t mic_stream_recorder;
  recording_t mic_resample_stream_recorder;
  recording_t spk_stream_recorder;
  recording_t flying_mono_stream_recorder;

  /** used to choose the location where network data is recorded in the echo canceller circular buffer */
  int using_out_callback;
  /** last played frame to be reused in the case of underflow */
  void *lastframe;
  /**  used in upsampling algo */
  short lastsample;
  /** resampling context when we want to simulate a 16 KHz codec negociation (useful for easy multi-codec conference) */
  void *resamplectx;
  /** resampling context at the audio driver interface (MIC and SPK) */
  void  *resample_audiodrv_ctx_mic;
  void  *resample_audiodrv_ctx_spk;
};
typedef struct ph_audio_stream phastream_t;

void ph_gen_noice();
void pg_gen_silence();

void ph_telephone_event(RtpSession *rtp_session, int event, struct ph_msession_s *s);
void ph_generate_out_dtmf(phastream_t *stream, short *signal, int siglen, long timestamp);

int ph_astream_decoded_framesize_get(phastream_t *stream);

#endif
