/* -*- Mode: C; tab-width:8; c-basic-offset:8; -*-
 *
 * Copyright (C) 2004  Vadim Lebedev  <vadim@mbdsys.com>
 * Copyright (C) 2006-2007 WENGO SAS
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
 * License along with wengophone; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  2007-09-27 Alec Leamas  <nospam>
 *     - Documentation added
 *     - Hardware buffer limited to 3 periods 
 *     - Support for silence_threshold and silence_size added (not used).
 *     - Now uses implicit (start_threshold) start instead of explicit.
 *     - Changed threading model, now uses internal threads.
 *     - Added handling of stereo devices which can't be opened in mono.
 *     - Added rebuffering support in playback_thread().
 *     - Fixed bug in snd_pcm_open, which was done in blocking mode
 *       (potential hang if device can't be opened).
 *     - Refactored alsa_dev_open fixed memory leaks.
 *     - Added function prototypes to avoid compiler warnings.
 *     - Removed DBG_DYNA_DRV references, uses std owplDebug calls instead.
 *     - Added statistics support.
 *     - Made lot's of literals to symbolic constants.
 *
 *  BUGS
 *     - Since the device is held open by this driver, playing sounds
 *       through alsa_sndfile.cpp does not work on physical alsa devices.
 *     - Marked as FIXME  here and there...
 */

/**
 *
 * @file phmedia-alsa.c
 *
 * The phmedia-alsa  module implements interface to ALSA devices for phapi
 *
 * @see http://www.linuxjournal.com/article/6735 for an introduction to ALSA  
 * @see http://dev.openwengo.com/trac/openwengo/trac.fcgi/wiki/PhApiAudio [PHAPI]
 * @see http://www.alsa-project.org/  [ALSA]
 *
 * The data transferred to/from the ALSA driver is organized in three levels:
 *
 * - The first level is the samplesize, the size of each sample. [PHAPI] 
 *   defines each sample as a 16-bit, signed and little endian entity.
 *
 * - [ALSA] defines the frame, which is defined as a sample from each channel.
 *   Thus, in mono mode framesize == samplesize, in stereo 
 *   framesize == 2 * samplesize.
 *
 * - Data is transferred to/from the driver in chunks, typically a 100-2000
 *   bytes.  These chunks are described as 'frames' in [PHAPI] and as 
 *   'periods' in [ALSA].  For now, these  chunks are  mono streams of 
 *   samples.
 *
 *  [ALSA] defines the basic setup parameters:
 *  - The size of the hardware ring buffer. This is tradeoff between not
 *    not losing packets, and packets becoming to old to be
 *    usable in an oversized buffer.
 *  - The 'period' size, effectively the same as a 'chunk'.
 *  - The relations between audio format, channels and framesize.
 *  - The start threshold - a stream starts if the amount of buffered 
 *    (output) or requested(input) data is equal to or exceeds this value. 
 *    This drivers setup starts streams in this way.
 *  - The silence threshold and silence length: See ::SILENCE_THRESHOLD
 * 
 *  The fundamental requirement for the flow comes from the codecs, which emits
 *  and requires data at a specific rate. Wengophone's codecs has a sampling
 *  rate of 16 000 frames per second. This requirement is embodied in the
 *  parameters chunk_size and sample_rate to the open() function.
 *
 *  Basic dependencies, with some real example data (see [ALSA], API 
 *  reference, PCM section)
 *  - Sample rate = 16000 samples/sec.
 *  - Time for 1 sample = 1/rate s = 1000/rate ms:  1000/16000 ms. 
 *  - Chunk(bytes, mono) = 640 bytes
 *  - Chunk(bytes, stereo) = 1280 bytes
 *  - Chunk(frames) = chunk(bytes)/sample_size/channels: 1280/2/2 = 320
 *  - Period size(frames) = 1 chunk = 320 frames.
 *  - Period time = chunk(frames)/sample rate: 320/16000 = 20 ms
 *  - Buffer size = periods per buffer * period size: 3 * 320  = 960 frames
 *  - Buffer time = buffer size(frames)/sample rate: 960/16000 = 60 ms
 *
 *  Debugging tip: <i>cat /proc/asound/card0/pcm0p/sub0/hw_params</i>, 
 *  <i>cat /proc/asound/card0/pcm0c/sub0/sw_params</i>. I bet your 
 *  filenames are different.
 *
 */

//FIXME: split in header  + implementation file.

#ifdef ENABLE_ALSA

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <fcntl.h>  
#include <libgen.h>
#include <pthread.h>

#include "phapi.h"
#include "phastream.h"
#include "phaudiodriver.h"
#include "phlog.h"
#include "owpl_log.h"

/** Preferred # channels (1-2). Wengophone always uses mono internally. */
const int DEFAULT_CHANNELS   = 1; 

/** Size of hw buffer (periods). */
const unsigned  BUFFER_PERIODS  = 3;

/** 
 *
 * Iff I've understood this right: When the number of outgoing,
 * buffered frames drops below this value, SILENZE_SIZE number
 * of silence frames is added to the output - all this to make the
 * expected underrun less noisy.
 * 
 * SILENZE_SIZE should be <= SILENCE_THRESHOLD,
 * or >= boundary (INT_MAX works fine). The latter case fills the 
 * buffer with silence.
 *
 * Beware: The [ALSA] docs for this are horrible, and I might have got
 * all this wrong. 
 *
 * Using internal threads and rebuffering support, this is not used.
 *
 * @see [ALSA], set_silence_threshold().
 *
 */
const snd_pcm_uframes_t   SILENCE_THRESHOLD = 0;

/** Time (ms) we wait for a device in snd_pcm_wait(). */
const unsigned            SND_PCM_WAIT_MS   = 500;

/** @see SILENCE_THRESHOLD. */
const snd_pcm_uframes_t   SILENCE_SIZE      = 0;

/** Used format. */ 
const snd_pcm_format_t    PCM_FORMAT        = SND_PCM_FORMAT_S16_LE;

/** Suppress ALSA diagnostics on stdout/stderr (0/1). */
const int                 ALSA_SILENT       = 1;

#define   OUTPUT_LATENCY         100  /**< Buffering before stream start
					   as percent of a period.   */
#define   INPUT_LATENCY          50   /**< Min size of requested data 
					   which starts stream.    */

#define   MONO_AVERAGE           1    /**< stereo2mono takes the average
					 of the two stereo channels. */

#define   PH_TRUE                1
#define   PH_FALSE               0
typedef   short                  ph_bool; 

/** Returns the alsa_drv_info* ptr stored as drvinfo in phastream_t. */
#define   DRV_INFO(x) ((alsa_drv_info *)((x)->drvinfo))

/** Kludge to fix incompatibility between gcc 4.2 and alsa. */
#define GCC_VERSION (__GNUC__ * 10    + __GNUC_MINOR__  )

#if GCC_VERSION > 41

#undef snd_pcm_sw_params_alloca
#define snd_pcm_sw_params_alloca(ptr) \
do { \
    *ptr = (snd_pcm_sw_params_t*) alloca(snd_pcm_sw_params_sizeof()); \
    memset(*ptr, 0, snd_pcm_sw_params_sizeof()); \
} while (0)

#undef snd_pcm_hw_params_alloca
#define snd_pcm_hw_params_alloca(ptr) \
do { \
    *ptr = (snd_pcm_hw_params_t*) alloca(snd_pcm_hw_params_sizeof()); \
    memset(*ptr, 0, snd_pcm_hw_params_sizeof()); \
} while (0)

#undef snd_pcm_status_alloca
#define snd_pcm_status_alloca(ptr) \
do  { \
    *ptr = (snd_pcm_status_t *) alloca(snd_pcm_status_sizeof()); \
    memset(*ptr, 0, snd_pcm_status_sizeof()); \
} while (0)

#endif  //GCC_VERSION


/**
 * @name Driver entry points
 *
 * @{
 *
 */

/**
 *
 * Implements ::ph_audio_open.
 *
 * @BUG  No check if ain/aout has same channel count
 *
 */
static int alsa_stream_open(phastream_t* as,
			    char* name, 
			    int wished_rate, 
			    int chunk_size, 
			    ph_audio_cbk cbk);

/**
 * Implements ::ph_audio_close
 */
static void alsa_stream_close(phastream_t *as);

/**
 * Implements ::ph_audio_start
 */
static void alsa_stream_start(phastream_t *as);

/**
 * Implements ::ph_audio_write 
 *
 *  Timing: On a AMD X2 4200+ each invocation is about 150 us.
 *  
*/
static int alsa_stream_write(phastream_t *as, void *buf,  int len);

/**
 * Implements ::ph_audio_read
 */
static int alsa_stream_read(phastream_t *as, void *buf,  int len);

/**
 * Implements ::ph_audio_get_avail_data
 */
static int alsa_stream_get_avail_data(phastream_t *as);

/**
 * Implements ::ph_audio_get_out_space.
 */
static int alsa_stream_get_out_space(phastream_t *as, int *used);

/**
 * Implements ::ph_audio_get_fds
 */
static int alsa_stream_get_fds(phastream_t *as, int fds[2]);
/** Register the driver. */
void ph_alsa_driver_init(void);

/*@}*/


/** Description of one device (input or output). */
struct _alsa_dev {
		snd_pcm_t*     pcm;
		char*          id;           /**< id string e g "input".  */
		unsigned long  bytes;        /**< # bytes read/written.   */
		unsigned long  rebuffered;   /**< Resent (no input) bytes.*/
		unsigned       soft_errors;  /**< recoverable errors.     */
		unsigned       again_errors; /**< EAGAIN soft error.      */
		unsigned       hard_errors;  /**< Non-recoverable errors. */
		pthread_t      thread;       /**< Thread in async mode.   */
		unsigned       stop_thread;  /**< Initially 0, set to 1   */
		                             /**  of main thread to stop  */
		                             /**  child, set to 2 by      */
		                             /**  child when child exits. */
};

typedef struct _alsa_dev alsa_dev;

/** The driver info kept in the phastream->drv_info. */
struct _alsa_drv_info {
		alsa_dev  input;
		alsa_dev  output;
		unsigned  channels;         /**< Mono/stereo (1, 2).      */
		unsigned  sample_size;      /**< In bytes, 1.\ .\ 8.      */
		size_t    chunk_size;       /**< As to alsa_stream_open().*/
		ph_audio_cbk  callback;     /**< Invoked by threads.      */
};

typedef struct _alsa_drv_info alsa_drv_info;

/** Argument when opening device. */
struct _pcm_data {
		snd_pcm_t* device;    
		unsigned channels;   
                unsigned rate;
                ssize_t  chunk_size;  /**< # bytes (sic!) in each transfer. */
		unsigned  threshold;   
                                      /**< Delay before stream starts(frames)*/
                unsigned buffer_periods; /**< Size of hw buffer, in periods. */
		unsigned silence_threshold;
	        unsigned silence_size;
};

typedef struct _pcm_data pcm_data;

/** 
 * The driver info registered by init()
 */
struct ph_audio_driver ph_alsa_driver = {
	"alsa",
	PH_SNDDRVR_PLAY_CALLBACK | PH_SNDDRVR_REC_CALLBACK,
	0,
	alsa_stream_start,
	alsa_stream_open,
	alsa_stream_write,
	alsa_stream_read,
	alsa_stream_get_out_space,
	alsa_stream_get_avail_data,
	alsa_stream_close,
	alsa_stream_get_fds,
	NULL
};

/**
 * nanosleep(2) arg to relinquish CPU while waiting for IO, 5 ms.
 */
const struct timespec io_wait = { 0, 5 * 1000000 };

/** Dump lot's of information about a pcm device. */
static void dump_pcm_data( snd_pcm_t* handle, snd_pcm_hw_params_t *params);  

/**
 * Open an ALSA device for input or output
 *
 * @param handle On exit, handle to device in state SND_PCM_STATE_OPEN.
 *
 * @param name Name of device. Might be the plain device name, or environment
 *        setting on the form "alsa:IN=inputdev OUT=outputdev".
 * @param type indicates a playback or a recorder device.
 *
 * @return 0 on success, else ALSA error code.
 *
 */
static int open_device( snd_pcm_t**      handle, 
			const char*      name, 
			snd_pcm_format_t type);

/**
 *
 * Open and setup an ALSA device for input or output.
 * 
 * @param name Plain ALSA device name e g "hw:0,0" or a string as present
 * in environment e g "alsa:IN=hw:0,0 OUT=hw:1,0".
 * @param pcm Device and device data. On successful exit, pcm->pcm is 
 * in state SND_PCM_STATE_PREPARE.
 *
 * @return 0 on success, else negative alsa error code.
 *
 */
static int alsa_dev_open( const char*      name, 
			  snd_pcm_stream_t type,
			  pcm_data*        pcm);

/** Return size of hw buffer, in frames.  */
static snd_pcm_uframes_t 
pcm_data_get_buffer_size( snd_pcm_hw_params_t* params, pcm_data* pcm);

/** Return size of one period, in frames. */

static snd_pcm_sframes_t 
pcm_data_get_period_size( snd_pcm_hw_params_t* params, pcm_data* pcm);

/** Return size of input/output threshold, in frames. */
static snd_pcm_uframes_t pcm_data_get_threshold( pcm_data* pcm );

static snd_pcm_uframes_t pcm_data_get_silence_threshold( pcm_data* pcm);

static snd_pcm_uframes_t pcm_data_get_silence_size( pcm_data* pcm);

/** printf-style sibling to LOG_ERROR. */
#define ALSA_ERROR( fmt, ... )		  \
	alsa_log(OWPL_LOG_LEVEL_ERROR,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/** printf-style sibling to LOG_DEBUG. */
#define ALSA_DEBUG( fmt, ... )		  \
	alsa_log(OWPL_LOG_LEVEL_DEBUG,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/** Non-blocking handling of ALSA error code, reset dev as required. */
static void ph_handle_pcm_error( alsa_dev* device, int err );

static snd_pcm_uframes_t 
wp_bytes_to_frames( snd_pcm_hw_params_t* params, size_t bytes);

/** Set hw parameters for an alsa device. */
static int set_hw_params( snd_pcm_stream_t     type,  
			  pcm_data*            pcm, 
			  snd_pcm_hw_params_t* hw_params);

/** Set sw parameters for an alsa device. */
static int set_sw_params( snd_pcm_stream_t     type,  
			  pcm_data*            pcm, 
			  snd_pcm_hw_params_t* hw_params );

/** 
 * Initiate a new pcm_data instance.
 *
 * @see snd_pcm_sw_params_set_start_threshold.
 * @param pcm Filled with data on exit.
 * @param rate Sample rate (frames/sec).
 * @param chunk_size Size of blocks read or written (bytes).
 * @param latency Start threshold value as percent of a period.
 */
static void pcm_data_new( pcm_data* pcm,  
			  unsigned  rate, 
			  unsigned  chunk_size,  
			  unsigned  latency);

/** 
 * Create a stereo buffer from a mono source. 
 *
 * @param dest   The destination stereo buffer, 2 * size big.
 * @param src    The source mono buffer
 * @param size   Size of source mono buffer (bytes).
 * @param sample_size Size in bytes of each sample 1.\ .\ 8
 *
 */
static  void mono2stereo( char*    dest, 
			  char*    src, 
			  size_t   size, 
			  size_t   sample_size); 

/**
 *
 * Read data from ALSA, support for alsa_stream_read.
 *
 * @param stream Data source (mic).
 * @param buf    Buffer for data, stereo or mono as required.
 * @param len    Buffer length = requested read size (bytes).
 * @return       # bytes read, <= len.
 *
 */
static int stream_read( alsa_dev* input, void* buff, int len);

/**
 *
 * Write data to ALSA, support for alsa_stream_write.
 *
 * @param output Where to send data.
 * @param buff Stereo or mono data.
 * @len   length of buff (bytes).
 * @return #  bytes written.
 *
 */
static int stream_write( alsa_dev* output, void *buf, int len);

/** 
 *
 * Create a mono buffer from a stereo source. 
 *
 * @param src    The stereo input buffer with signed 16-bit samples.
 * @param dest   The mono destination buffer.
 * @param size   Size of source stereo buffer (bytes).
 * @param sample_size Size in bytes of each sample 1.\ .\ 8.
 * @return       Size of returned mono buffer (i e, size / 2 ).
 *
 */
static size_t  stereo2mono( char*       dest,
			    char*       src,
			    size_t      size, 
			    size_t      sample_size); 

/** Shuffles data from ALSA to phapi. */
static void* recorder_thread( void* arg) ;

/** Shuffles data from phapi to ALSA. */
static void* playback_thread( void* arg);

static void alsa_log( OWPL_LOG_LEVEL level, 
		      const char*    file, 
		      unsigned       line, 
		      const char*    fmt, ...) {

	char filename[ 256 ];
	char msg[256];

	strncpy( filename, file, sizeof( filename));
	snprintf( msg, sizeof( msg), 
		  "In %s, line %d:", basename( filename ), line);

	va_list ap;
	va_start( ap, fmt);
	vsnprintf( 
		msg + strlen( msg), sizeof( msg) - strlen( msg), fmt, ap);
	va_end( ap);
	switch ( level) {
		case OWPL_LOG_LEVEL_DEBUG:
			owplLogDebug( msg );
			break;

		case OWPL_LOG_LEVEL_INFO:
			owplLogInfo( msg );
			break;

		case OWPL_LOG_LEVEL_WARN:
			owplLogWarn( msg );
			break;

		case OWPL_LOG_LEVEL_ERROR:
			owplLogError( msg );
			break;
	}
}

static void dump_pcm_data( snd_pcm_t*          handle, 
			   snd_pcm_hw_params_t *params) {

	int dir;
	unsigned int val2;
	unsigned int val;
	snd_pcm_access_t access;
	snd_pcm_subformat_t format;
	snd_pcm_uframes_t uframe;
	snd_pcm_uframes_t frames;

	/* Display information about the PCM interface */
	ALSA_DEBUG("PCM handle name = '%s'", snd_pcm_name(handle));
	ALSA_DEBUG("PCM state = %s", 
			   snd_pcm_state_name(snd_pcm_state(handle)));
	snd_pcm_hw_params_get_access(params, &access);
	ALSA_DEBUG("access type = %s", snd_pcm_access_name(access));
	snd_pcm_hw_params_get_format(params, &dir);
	ALSA_DEBUG("format = '%s' (%s)", 
			   snd_pcm_format_name((snd_pcm_format_t)dir), 
			   snd_pcm_format_description((snd_pcm_format_t)dir));
	snd_pcm_hw_params_get_subformat(params, &format);
	ALSA_DEBUG("subformat = '%s' (%s)",
			   snd_pcm_subformat_name(format),
			   snd_pcm_subformat_description(format));
	snd_pcm_hw_params_get_channels(params, &val);
	ALSA_DEBUG("channels = %d", val);
	snd_pcm_hw_params_get_rate(params, &val, &dir);
	ALSA_DEBUG("rate = %d bps", val);
	snd_pcm_hw_params_get_period_time(params, &val, &dir);
	ALSA_DEBUG("period time = %d us", val);
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	ALSA_DEBUG("period size = %d frames", (int)frames);
	snd_pcm_hw_params_get_buffer_time(params, &val, &dir);
	ALSA_DEBUG("buffer time = %d us", val);
	snd_pcm_hw_params_get_buffer_size(params, &uframe);
	ALSA_DEBUG("buffer size = %d frames", (int)uframe);
	snd_pcm_hw_params_get_periods(params, &val, &dir);
	ALSA_DEBUG("periods per buffer = %d frames", val);
	snd_pcm_hw_params_get_rate_numden(params, &val, &val2);
	ALSA_DEBUG("exact rate = %d/%d bps", val, val2);
	val = snd_pcm_hw_params_get_sbits(params);
	ALSA_DEBUG("significant bits = %d", val);
	snd_pcm_hw_params_get_tick_time(params, &val, &dir);
	ALSA_DEBUG("tick time = %d us", val);
	val = snd_pcm_hw_params_is_batch(params);
	ALSA_DEBUG("is batch = %d", val);
	val = snd_pcm_hw_params_is_block_transfer(params);
	ALSA_DEBUG("is block transfer = %d", val);
	val = snd_pcm_hw_params_is_double(params);
	ALSA_DEBUG("is double = %d", val);
	val = snd_pcm_hw_params_is_half_duplex(params);
	ALSA_DEBUG("is half duplex = %d", val);
	val = snd_pcm_hw_params_is_joint_duplex(params);
	ALSA_DEBUG("is joint duplex = %d", val);
	val = snd_pcm_hw_params_can_overrange(params);
	ALSA_DEBUG("can overrange = %d", val);
	val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
	ALSA_DEBUG("can mmap = %d", val);
	val = snd_pcm_hw_params_can_pause(params);
	ALSA_DEBUG("can pause = %d", val);
	val = snd_pcm_hw_params_can_resume(params);
	ALSA_DEBUG("can resume = %d", val);
	val = snd_pcm_hw_params_can_sync_start(params);
	ALSA_DEBUG("can sync start = %d", val);
}

/** Register this driver. */
void ph_alsa_driver_init( void) {
	ph_register_audio_driver( &ph_alsa_driver);
}

static void pcm_data_new( pcm_data* pcm, 
			  unsigned rate,
			  unsigned chunk_size, 
			  unsigned latency) {

	memset( pcm, 0, sizeof( pcm_data));
	pcm->channels = DEFAULT_CHANNELS;
	pcm->rate = rate;
	pcm->threshold = latency;
        pcm->chunk_size = chunk_size;
	pcm->buffer_periods = BUFFER_PERIODS;
	pcm->silence_threshold = SILENCE_THRESHOLD;
	pcm->silence_size = SILENCE_SIZE;
}

static snd_pcm_sframes_t 
pcm_data_get_period_size( snd_pcm_hw_params_t* params, pcm_data* pcm) {

	return wp_bytes_to_frames( params, pcm->chunk_size);
}

static snd_pcm_uframes_t 
pcm_data_get_buffer_size( snd_pcm_hw_params_t* params, pcm_data* pcm) {

	return  pcm_data_get_period_size( params, pcm) * pcm->buffer_periods;
}

static snd_pcm_uframes_t pcm_data_get_threshold( pcm_data* pcm){

        snd_pcm_uframes_t period = 
		snd_pcm_bytes_to_frames( pcm->device, pcm->chunk_size);
	return ( pcm->threshold * period) / 100;
}

static snd_pcm_uframes_t pcm_data_get_silence_size( pcm_data* pcm){

	return ( pcm->silence_size );
}

static snd_pcm_uframes_t pcm_data_get_silence_threshold( pcm_data* pcm){

	return ( pcm->silence_threshold );
}


static int open_device( snd_pcm_t** handle, 
			const char* name, 
			snd_pcm_format_t type) {

        const ph_bool   input   = ( type == SND_PCM_STREAM_CAPTURE);
        const char*     keyword = ( input ? "in" : "out");
	
	char       string[128];
	char*      strtok_ptr;
	char*      device = NULL;
	int        rc;

	if ( name == NULL) {
		ALSA_ERROR( "alsa_dev_open: name == NULL!");
		return -1;
	}
	strncpy( string, name, sizeof( string));
        device = strtok_r( string, ":", &strtok_ptr);	
        if( strcasecmp( device, "alsa") == 0) {
		while( strcasecmp( device, keyword) != 0 && device != NULL) {
			device = strtok_r( NULL, " =", &strtok_ptr);
		}
		if( device != NULL) {
			device = strtok_r( NULL, " =", &strtok_ptr);
		}
		if( device == NULL) {
			ALSA_ERROR( "Illegal device string: %s", name);
			return -1;
		}
	}
   
	rc = snd_pcm_open( handle, device, type, SND_PCM_NONBLOCK);
	if (rc < 0){
		ALSA_ERROR( "Cannot open pcm device %s: %s", 
			    name, snd_strerror( rc));
		return rc;
	}
	ALSA_DEBUG( "open pcm device %s for %s: OK",
		    name, input ? "input" : "output");
	return 0;
}

static int set_hw_params( snd_pcm_stream_t     type, 
			  pcm_data*            pcm,
			  snd_pcm_hw_params_t* params) {

        snd_pcm_uframes_t   frames;
	int                 rc;

	/* Fill it in with default values. */
	snd_pcm_hw_params_any( pcm->device, params);

	/* Set the desired hardware parameters. */
	rc = snd_pcm_hw_params_set_channels_near( pcm->device, 
						  params, 
						  &pcm->channels);
	if ( rc < 0) {
		ALSA_ERROR("Unable to set hw parameters(channel): %s", 
			   snd_strerror( rc));		
		return( rc );
	}

        if( pcm->channels == 2) {
		// We will expand the mono chunk to a stereo chunk.
		pcm->chunk_size *= 2;
	}

	rc = snd_pcm_hw_params_set_access( pcm->device, 
					   params, 
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	if( rc < 0) {
		ALSA_ERROR(
			"Set access SND_PCM_ACCESS_RW_INTERLEAVED: %s",
			snd_strerror(rc));
		return( rc );
	}

	/* Signed 16-bit little-endian format. */
	rc = snd_pcm_hw_params_set_format( pcm->device, 
					   params, 
					   PCM_FORMAT);
	if ( rc < 0) {
		ALSA_ERROR( "Unable to set format %s: %s",
			    "SND_PCM_FORMAT_S16_LE",
			    snd_strerror( rc));
		return( rc );
	}
	rc = snd_pcm_hw_params_set_rate_near( pcm->device, 
					      params, 
					      &pcm->rate, 
					      0);
	if ( rc < 0) {
		ALSA_ERROR( "Unable to set hw parameters: %s", 
			    snd_strerror( rc));
		return( rc );
	}

	frames = pcm_data_get_period_size( params, pcm);
	rc = snd_pcm_hw_params_set_period_size_near( pcm->device, 
						     params, 
						     &frames, 
						     0);
	if ( rc < 0) {
		ALSA_ERROR(
			"Unable to set hw params (period_size=%d): %s", 
			(int) frames, snd_strerror( rc));
		return( rc );
	}

	frames = pcm_data_get_buffer_size( params, pcm);
        rc = snd_pcm_hw_params_set_buffer_size_near( pcm->device,
						     params,
						     &frames);
        if( rc < 0) {
		ALSA_ERROR( "Unable to set hw buffer size: %s", 
			    snd_strerror( rc));
		return( rc );
	}
	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params( pcm->device, params);
	if ( rc < 0) {
		ALSA_ERROR( "Unable to set hw params: %s", 
			    snd_strerror( rc));
		return( rc );
	}

	return( 0 );

}

static int set_sw_params( snd_pcm_stream_t     type, 
			  pcm_data*            pcm, 
			  snd_pcm_hw_params_t* hw_params )
{
	int                 rc;
	snd_pcm_sw_params_t *sparams = 0;
        snd_pcm_uframes_t   frames;

	snd_pcm_sw_params_alloca( &sparams);

	/* retrieve the parameters from the driver */
	rc = snd_pcm_sw_params_current( pcm->device, sparams);
	assert( rc == 0 );

        frames = pcm_data_get_threshold( pcm);
	rc = snd_pcm_sw_params_set_start_threshold( pcm->device, 
						    sparams, 
						    frames);
	assert( rc == 0);

	frames = pcm_data_get_period_size( hw_params, pcm);
	rc = snd_pcm_sw_params_set_avail_min( pcm->device,
					      sparams,
					      frames); 
	assert( rc == 0);

	if( type == SND_PCM_STREAM_PLAYBACK) {
		frames = pcm_data_get_silence_threshold( pcm);
		rc = snd_pcm_sw_params_set_silence_threshold( pcm->device,
							      sparams,
							      frames); 
		assert( rc == 0);

		frames = pcm_data_get_silence_size( pcm);
		rc = snd_pcm_sw_params_set_silence_size( pcm->device,
							 sparams,
							 frames); 
		assert( rc == 0);
	}

	/* Write the parameters to the driver */
	rc = snd_pcm_sw_params( pcm->device, sparams);
	if( rc < 0) {
		ALSA_ERROR( "Unable to set sw parameters: %s", 
			    snd_strerror( rc));
		return( rc);
	}
	return( 0);

}

static snd_pcm_uframes_t 
wp_bytes_to_frames( snd_pcm_hw_params_t* params, size_t bytes) {
       
	unsigned             channels;
	int                  res;
	snd_pcm_format_t     format;
        int                  samplesize;

        res = snd_pcm_hw_params_get_channels( params, &channels);
	assert( res >= 0  && channels >= 1);
	res = snd_pcm_hw_params_get_format( params, &format);
	assert( res >= 0);
	samplesize = snd_pcm_format_size( format, 1);
	assert( samplesize > 0);
	return bytes / (samplesize * channels);
}

static int alsa_dev_open( const char*       name, 
			  snd_pcm_stream_t  type, 
			  pcm_data*         pcm) {

	int                  rc;
	snd_pcm_hw_params_t* hw_params;

	snd_pcm_hw_params_alloca( &hw_params);

	ALSA_DEBUG( "open_device:  %s (name: %s, rate: %d, chunk: %d)",
		    type == SND_PCM_STREAM_CAPTURE ? "input" : "output",
		    name, pcm->rate, pcm->chunk_size);

	rc  = open_device( &pcm->device, name, type);
        if( rc < 0 ) {
		return ( rc);
        };
	rc = set_hw_params( type, pcm, hw_params );
	snd_pcm_nonblock( pcm->device, 1);
	dump_pcm_data( pcm->device, hw_params);
	if( rc == 0) {
		rc = set_sw_params( type, pcm, hw_params);
	}

	if( rc < 0 && pcm->device != 0) {
		snd_pcm_close( pcm->device );
	}
	return rc;
}


static int alsa_stream_open( phastream_t* as, 
			     char*        name, 
			     int          wished_rate, 
			     int          chunk_size, 
			     ph_audio_cbk cbk) {

	alsa_drv_info*  ad = 0;
        pcm_data        input_dev;
	pcm_data        output_dev;

	ALSA_DEBUG(
		"alsa_stream_open: (name: %s, rate: %d, chunk: %d)", 
		name, wished_rate, chunk_size);
	ad = calloc( sizeof( alsa_drv_info), 1);
	if ( !ad) {
		return -PH_NORESOURCES;
	}
        
        pcm_data_new( &output_dev, wished_rate, chunk_size, OUTPUT_LATENCY);
	alsa_dev_open(name,  SND_PCM_STREAM_PLAYBACK,  &output_dev); 
        ad->output.pcm = output_dev.device;
	if ( !ad->output.pcm) {
		free( ad);
		return -PH_NOAUDIODEVICE;
	}
        ad->output.id = "output";

        pcm_data_new( &input_dev, wished_rate, chunk_size, INPUT_LATENCY); 
	alsa_dev_open( name, SND_PCM_STREAM_CAPTURE, &input_dev);
	ad->input.pcm = input_dev.device;
	if ( !ad->input.pcm) {
		// close the playback device if we fail to open capture device
		snd_pcm_close( ad->output.pcm);
		free( ad);
		return -PH_NOAUDIODEVICE;
	}
	ad->input.id = "input";

	if( input_dev.rate != output_dev.rate)
  	{
  		ALSA_ERROR( "Mic speed %d differs from speaker rate %d",   
  			    input_dev.rate, output_dev.rate);
  		ALSA_ERROR( "Lets try anyway, but this is scary...");
  	}
  
        // TODO: refactor the actual_rate across all phaudio_driver backends
	//       it should be an OUT parameter in the dev_open prototype
	// here, hopefully, both MIC and SPK devices have been opened with 
        // the same rate. We use only the MIC rate.

	as->actual_rate = input_dev.rate;
        ad->channels = input_dev.channels;
	ad->sample_size = snd_pcm_format_size( PCM_FORMAT, 1);
	ad->chunk_size = chunk_size;
	ad->callback = cbk;
	ALSA_DEBUG( "alsa_stream_open: chosen rate (freq)=(%d)",
			   as->actual_rate);
	as->drvinfo = ad;
	PH_SNDDRVR_USE();

	return 0;
 }

static void alsa_stream_close( phastream_t *as) {

	alsa_dev* input = &( DRV_INFO(as)->input);
	alsa_dev* output = &( DRV_INFO(as)->output);

	ALSA_DEBUG( "alsa_stream_close");
	if ( !as->drvinfo){
		ALSA_DEBUG( "ALSA streams already closed");
		return;
	}

	ALSA_DEBUG( "Stop playback thread");
	output->stop_thread = 1;
	pthread_join( output->thread, NULL);
	snd_pcm_drop( output->pcm);
	snd_pcm_close( output->pcm);
	ALSA_DEBUG( 
		"Output: (sent,rebuffered,again,soft,hard): %ld %ld %d, %d, %d",
		output->bytes, 
		output->rebuffered,
		output->again_errors, 
		output->soft_errors, 
		output->hard_errors); 

	ALSA_DEBUG( "Stop recorder thread");
	input->stop_thread = 1;
	pthread_join( input->thread, NULL);
	snd_pcm_drop( input->pcm );
	snd_pcm_close( input->pcm);
	ALSA_DEBUG( 
		"Input: received %ld, errors(again, soft,hard) : %d, %d, %d",
		input->bytes, 
		input->again_errors, 
		input->soft_errors, 
		input->hard_errors); 

	free( as->drvinfo);
	as->drvinfo = 0;

	PH_SNDDRVR_UNUSE();
}

static void alsa_stream_start( phastream_t *as) {

	alsa_dev*       output = &( DRV_INFO( as)->output);
	alsa_dev*       input = &( DRV_INFO( as)->input);
        int             rc;

	// Output start automagically when the buffer exceeds threshold.
	ALSA_DEBUG( "Streams ready to start");

	output->stop_thread = 0;
	rc = pthread_create( &output->thread,
			     NULL,
			     playback_thread,
			     (void*) as);
	assert( rc == 0);
	ALSA_DEBUG( "Playback thread started" );

	input->stop_thread = 0;
	rc = pthread_create( &input->thread,
			     NULL,
			     recorder_thread,
			     (void*) as);
	assert( rc == 0);
	ALSA_DEBUG( "Recorder thread started");
}

static void mono2stereo( char*       dest,
			 char*       src,
			 size_t      size, 
			 size_t      sample_size) {

	char*  last_in_buffer = src + size - sample_size;
	char*  stereo         = dest + 2 * size - sample_size;
	char*  mono;

	for( mono = last_in_buffer; mono >= src; mono -= sample_size) {
		memcpy( stereo, mono, sample_size) ;
		stereo -= sample_size;
		memcpy( stereo, mono, sample_size) ;
		stereo -= sample_size;
	}

}

static void ph_handle_pcm_error( alsa_dev* device, int err )
{

	assert( err < 0 );
	if( err == -EAGAIN){
		device->again_errors += 1;
	}
	else {
		device->soft_errors += 1;
		err = snd_pcm_recover( device->pcm, err, ALSA_SILENT);
		if( err < 0) {
			ALSA_ERROR( "Can't restore ALSA %s: %s",
				    device->id, snd_strerror( err));
			device->hard_errors += 1;
			snd_pcm_prepare( device->pcm);
		}
	}
}

static int stream_write( alsa_dev* output, void *buf,  int len){

	snd_pcm_uframes_t frames;
	int               res = 0;
	ssize_t           bytes;
        

	frames = snd_pcm_bytes_to_frames( output->pcm, len);
	res = snd_pcm_writei( output->pcm, buf, frames);
        if( res < 0 ){
		ph_handle_pcm_error( output, res );
		bytes = 0;
	}
	else{
		bytes = snd_pcm_frames_to_bytes( output->pcm, res);
	}
	return bytes;
}

static int alsa_stream_write( phastream_t *as, void *buf,  int len) {

	char*      ph_buf;
	ssize_t    bytes;
	alsa_dev*  output = &( DRV_INFO( as)->output);
        
	assert( len >=0 && len <= (int) DRV_INFO( as)->chunk_size);

        if( DRV_INFO( as)->channels == 1) {
		bytes = stream_write( output, buf, len);
	}
	else {
		ph_buf = alloca( len * 2);  
		mono2stereo( ph_buf, buf, len, DRV_INFO(as)->sample_size);
		bytes = stream_write( output, ph_buf, len * 2) / 2;
	}
	output->bytes += bytes;

	assert( bytes >= 0 && 
		bytes <= (ssize_t) DRV_INFO( as)->chunk_size); 
	return bytes;
}

static size_t stereo2mono( char*    dest_arg, 
			   char*    src_arg,
			   size_t   size, 
			   size_t   sample_size)
{
	long     val;
	short*   src = (short*) src_arg;
	short*   dest= (short*) dest_arg;

	assert( sample_size == 2);  // This only works for 16bit signed data.
	assert( sizeof( short ) == 2);

#ifdef  MONO_AVERAGE
	while( (char*) src < src_arg + size) {
		val = *src++;
		val += *src++;

		*dest = (short)(val / 2);
		dest++;
	}
#else
        while( (char*) src < buf + size) { 
		*dest = *(src + (src % 4) / 2);
  	        src += 2;
                dest += 1;
        }
#endif
	return size / 2;
}

static int stream_read(alsa_dev* input, void *buf,  int len) {

	snd_pcm_uframes_t frames;
	int               res = 0;
	unsigned          bytes;

	frames = snd_pcm_bytes_to_frames( input->pcm, len); 
	res = snd_pcm_readi( input->pcm, buf, frames);
	if ( res == 0) {
		ALSA_DEBUG( "Empty read!");
		bytes = 0;
	}		
	else if ( res < 0) {
		ph_handle_pcm_error( input, res );
		bytes = 0;
	}
	else{
		bytes = snd_pcm_frames_to_bytes( input->pcm, res);
	}
        return bytes;
}

static int alsa_stream_read( phastream_t *as, void *buf, int len) {
	
	char*      bigger_buf;
	size_t     bytes = len;
	alsa_dev*  input = &( DRV_INFO( as)->input);

	assert( len <= (int) DRV_INFO( as)->chunk_size);
	
        if( DRV_INFO( as)->channels == 1) {
		bytes = stream_read( input, buf, len);
	}
	else {
		assert( DRV_INFO( as)->channels == 2);
		bigger_buf = alloca( len * 2); 
		bytes = stream_read( input, bigger_buf, len * 2);
		bytes = stereo2mono( buf, bigger_buf, bytes, 2);
	}
	input->bytes  += bytes; 
	assert( (int) bytes <= len);
	return( bytes);
}

static int alsa_stream_get_out_space( phastream_t *as, int *used) {
	snd_pcm_status_t  *st;
	snd_pcm_uframes_t available;
        snd_pcm_t* output = DRV_INFO( as)->output.pcm;

        snd_pcm_status_alloca( &st);

        if ( snd_pcm_status( output, st) < 0) {
                *used = 0;
                return 320;  //FIXME! Whats this?!
        }
       
        *used = snd_pcm_frames_to_bytes( output,
                                         snd_pcm_status_get_delay( st));
       
        available = snd_pcm_status_get_avail( st);
        ALSA_DEBUG( "Get_out_space, used: %d, available:%d",
                    *used, available);
        return snd_pcm_frames_to_bytes( output, available);
}


static int alsa_stream_get_avail_data( phastream_t *as) {
	
	ALSA_ERROR( "alsa_stream_get_avail_data(): not implemented" );
	return -1;
}

static int alsa_stream_get_fds( phastream_t *as, int fds[2]) {

	ALSA_ERROR( "alsa_stream_get_fds(): not implemented" );
	return -1;
}

//
// Timing: on a lightly loaded AMD x2 4200 I've measured 
// invocation of routines within +-3ms, without looking for
// exceptions which sure are there.
static void* playback_thread( void* arg) {

	phastream_t*   ps         = (phastream_t*) arg;
	alsa_dev*      output     = &( DRV_INFO( ps)->output);
	const unsigned chunk_size = DRV_INFO( ps)->chunk_size;
	int            size;
	unsigned       sent; 
	char*          buffer;
	char*          nxt_buff   = buffer;
	unsigned short prev_size  = chunk_size;
	int            odd        = 0;
      
        buffer = alloca( 2 * chunk_size);
	snd_pcm_format_set_silence( PCM_FORMAT, buffer, chunk_size);	

	while( !output->stop_thread) {
		size = chunk_size;
		odd = ( odd + 1) % 2;
		nxt_buff =  buffer + odd * chunk_size;
		DRV_INFO( ps)->callback( ps, NULL, 0, nxt_buff, &size);
		if( size == 0) {
			size = prev_size;
			odd = ( odd + 1) % 2;
			nxt_buff =  buffer + odd * chunk_size;
			output->rebuffered += size;
		}
		else {
			prev_size = size;
		}
		for( sent = 0; sent < (unsigned) size; ) {
			sent += alsa_stream_write( ps,
						   nxt_buff + sent,
						   size - sent);
			snd_pcm_wait( output->pcm, SND_PCM_WAIT_MS); 
		}
	}
	output->stop_thread++;	
	return( NULL);  
}

static void* recorder_thread( void* arg) {	
	
	phastream_t*     ps = (phastream_t*) arg;
	alsa_dev*        input = &( DRV_INFO(ps)->input);
	int              res;
	char*            buffer;

	buffer = alloca( DRV_INFO(ps)->chunk_size);

	while( !input->stop_thread) {
		snd_pcm_wait( input->pcm, SND_PCM_WAIT_MS);

		res = alsa_stream_read( ps, 
					buffer, 
					DRV_INFO(ps)->chunk_size);
		if( res <= 0) {
			// Don't monopolize CPU if continous errors.
			nanosleep( &io_wait, NULL);
			continue;  
		}
		DRV_INFO( ps)->callback(ps, buffer, res, NULL, 0);
	}
	input->stop_thread++;
	return( NULL);  
}
		
#endif //ENABLE_ALSA
