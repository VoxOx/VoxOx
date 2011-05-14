/* -*- Mode: C; tab-width:8; c-basic-offset:8; -*-
 *
 * Copyright (C) 2007	 Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307	 USA
 *
 */

/**
 * Handles fixed sounds, plays audio files from disk files to alsa sink.
 *
 * @author Mathieu Stute <mstute@gmail.com>
 *
 * 2007-09-10 Alec leamas
 *	 - Refactored alsa_open to smaller funcs.
 *	 - Handling of devices which cannot handle float data
 *	 - Handling of devices which cannot set # channels
 *	 - Centralized error handling with longjmp.
 *	 - Comments update.
 */

#include "alsa_sndfile.h"

#include <setjmp.h>
#include <sndfile.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <assert.h>
#include <stdarg.h>

#include <util/String.h>
#include <util/Logger.h>

/**
 * @name Internal code only used within this file
 *
 * @{
 */

/** Size of output buffer, in bytes. */
#define BUFFER_LEN (2048)

#define ALSA_PERIOD_SIZE 1024
#define ALSA_BUFFER_FRAMES (4 * 1024)

#ifndef TRUE
#define TRUE	1
#define FALSE 0
#endif

typedef struct play_context;	 // forward

/**
 *
 * A function used to retrieve data from the sound file
 *
 * @param ctx The function reads data from ctx->sndfile and
 *			updates ctx->buffer and ctx->samples.
 * @param t		# samples requested from source
 * @return		# samples actually read, possibly 0.
 *
 */
typedef sf_count_t wp_read_func( play_context* ctx, sf_count_t t );


/** The overall context while playing the sound. */
struct play_context{
		snd_pcm_t*		device;
		const char*		device_name;
		SNDFILE*		sndfile;
		const char*		filename;
		size_t			sample_size;   /**< Size of each sample in bytes, 1-8. */
		unsigned		sample_rate;
		unsigned		file_channels; /**< # input channels, 1-2 (mono/stereo)*/
		unsigned		alsa_channels; /**< # output cahnnels, 1-2. */
		/**
		 * Used buffer is 0.\ .\ BUFFER_LEN-1,
		 * create_stereo buffer expands it.
		 */
		char			buffer[ BUFFER_LEN * 2];
		unsigned		samples;	   /**< # samples in buffer. */
		wp_read_func	*read_func;	   /**< Method to read data from sndfile. */

		snd_pcm_hw_params_t*	hw_params;
		snd_pcm_sw_params_t*	sw_params;
};


/**
 *
 * Data for a given format.
 *
 */
typedef struct format_def {
		snd_pcm_format_t alsa_fmt;
		unsigned int	 sf_fmt;	   /**< One of the SF_* subformats constants.*/
		wp_read_func	 *read_func;
		size_t			 size;		   /**< bytes per sample, 0..8, 0 = unused. */
} format_def;

/**
 *
 * Open an alsa device for write
 *
 * @param devname Name of alsa device.
 *
 * @param ctx On normal exit is device, device_name, sample_size,
 *			hw_params, sw_params and alsa_channels updated.
 * @param exc longjmp target for error returns. Has alsa error code or -1
 *			in the argument to setjmp/longjmp.
 */
static	void
alsa_open( const char* devname, play_context* ctx, jmp_buf exc );

/**
 *
 *	Write some data to an alsa device.
 *
 *	@param ctx	The method writes <i>samples</i> from <i>buffer</i>
 *		to the <i>device </i>.
 *	@param exc	longjmp target for error conditions. Returns the
 *		negative ALSA return code to setjmp.
 *
 */
static int alsa_write_data(	 play_context* ctx, jmp_buf exc );

/** Initiate a fresh context. */
static void init_context( play_context* ctx );

/** Release all resources used by context. */
static void free_context( play_context* ctx );

/**
 *
 * Open the libsndfile-handled audio infile.
 *
 * @param  file Path to open
 * @return fields in *ctx: sndfile, file_channels, sample_rate
 *
 */
static void sndfile_open( const char* file, play_context* ctx, jmp_buf exc );


/**
 *		Logs an error message and performs a longjmp.
 *
 *		@param	message printf-style messsage format
 *		@retval return value returend through siglongjmp/setjmp.
 *		@param	siglongjmp(3) target for return.
 *
 *		@see setjmp(3)
 */
static void raise( jmp_buf retbuff, int retval, char* message, ... ) {
	va_list ap;
	char msg[ 256 ];

	va_start( ap, message );
	vsnprintf( msg, sizeof( msg ), message, ap );
	va_end( ap );
	LOG_WARN( msg );
	siglongjmp( retbuff, retval );
}


/**
 *	wp_read_func implementation reading byte-oriented data from file.
 */
static sf_count_t wp_read_raw( play_context* ctx, sf_count_t t ) {
	return sf_read_raw( ctx->sndfile, ctx->buffer, t );
};

/**
 *	Unimplemented wp_read_func: throws an assert()
 */
static sf_count_t wp_read_unimpl(	 play_context* ctx, sf_count_t t ) {
	assert( SF_FALSE ); // TBD error message...
};


/**
 *	wp_read_func implementation reading word-oriented data from file.
 */
static sf_count_t wp_read_short( play_context* ctx, sf_count_t t ) {
	return sf_read_short( ctx->sndfile,
			      (short*)ctx->buffer,
			      t / sizeof(short));
};

/**
 * wp_read_func implementation reading 32-bit integer data from file.
 */
static sf_count_t wp_read_int( play_context* ctx, sf_count_t t ) {
	return sf_read_int( ctx->sndfile, (int*)ctx->buffer, t	/ sizeof( int ) );
};

/**
 * wp_read_func implementation reading 32-bit float data from file.
 */
static sf_count_t wp_read_float( play_context* ctx, sf_count_t t ) {
	return sf_read_float( ctx->sndfile,
			      (float*)ctx->buffer ,
			      t	 / sizeof( float ) );
};

/**
 * wp_read_func implementation reading 64-bit float data from file.
 */
static sf_count_t wp_read_double( play_context* ctx, sf_count_t t ) {
	return sf_read_double( ctx->sndfile,
			       (double*)ctx->buffer ,
			       t / sizeof( double ) );
};

/**
 *
 *	  Mappings between the libsndfile SF_FORMAT format code and ALSA's
 *	  snd_pcm_format_t. The list is sorted in descending priority, first
 *	  usable match will be used.
 *
 *	  @see http://www.mega-nerd.com/libsndfile/api.html
 *	  @see http://www.alsa-project.org/alsa-doc/alsa-lib
 *
 */
static format_def FORMAT_DEFS[] = {
     {SND_PCM_FORMAT_FLOAT64,         SF_FORMAT_DOUBLE,       wp_read_double, 8      },
     {SND_PCM_FORMAT_FLOAT,           SF_FORMAT_FLOAT,        wp_read_float,  4      },
     {SND_PCM_FORMAT_S32,             SF_FORMAT_PCM_32,       wp_read_int,    4      },
     {SND_PCM_FORMAT_S24,             SF_FORMAT_PCM_24,       wp_read_unimpl, 0      },
     {SND_PCM_FORMAT_S16,             SF_FORMAT_PCM_16,       wp_read_short,  2      },
     {SND_PCM_FORMAT_GSM,             SF_FORMAT_GSM610,       wp_read_raw,    0      },
     {SND_PCM_FORMAT_A_LAW,           SF_FORMAT_ALAW,         wp_read_raw,    1      },
     {SND_PCM_FORMAT_MU_LAW,          SF_FORMAT_ULAW,         wp_read_raw,    1      },
     {SND_PCM_FORMAT_U8,              SF_FORMAT_PCM_U8,       wp_read_raw,    1      },
     {SND_PCM_FORMAT_S8,              SF_FORMAT_PCM_S8,       wp_read_raw,    1      },
     {SND_PCM_FORMAT_IMA_ADPCM,       SF_FORMAT_IMA_ADPCM,    wp_read_unimpl, 0      }
};

/** # of items in FORMAT_DEFS. */
static const int FORMAT_CNT = sizeof( FORMAT_DEFS ) / sizeof( format_def );

static void init_context( play_context* ctx ) {
	memset( ctx, 0, sizeof( play_context ));
	ctx->alsa_channels = -1;
	ctx->file_channels = -1;
}

static void free_context( play_context* ctx ) {
	if( ctx->device != 0 ) {
		snd_pcm_close( ctx->device );
	}
	if( ctx->sndfile != 0 ) {
		sf_close( ctx->sndfile );
	}
	if( ctx->hw_params != 0 ) {
		snd_pcm_hw_params_free (ctx->hw_params);
	}
	if( ctx->sw_params != 0 ) {
		snd_pcm_sw_params_free (ctx->sw_params);
	}
}

static void
sndfile_open( const char* file, play_context* ctx, jmp_buf exc ) {
	int			 subformat;
	SF_INFO		 sf_info;

	LOG_DEBUG( "playing " + String( file ? file : "NULL" ));

	ctx->filename = file;
	memset( &sf_info, 0, sizeof( SF_INFO  ));
	ctx->sndfile = sf_open( ctx->filename, SFM_READ, &sf_info);
	if( ctx->sndfile == 0 ) {
		raise( exc, -1,
		       "Can't open file %s, %s",
		       ctx->filename, sf_strerror(0) );
	}
	if(sf_info.channels < 1 || sf_info.channels > 2) {
		raise( exc, -1, "Error : channels	= %d", sf_info.channels );
	}
	ctx->file_channels = sf_info.channels;
	ctx->sample_rate = sf_info.samplerate;

	subformat = sf_info.format & SF_FORMAT_SUBMASK;
	if( subformat == SF_FORMAT_FLOAT || subformat == SF_FORMAT_DOUBLE) {
		sf_command( ctx->sndfile, SFC_SET_NORM_FLOAT, 0, SF_TRUE);
	}
}


/**
 *
 * If errcode != 0, log a message + snd_strerror and make a longjmp exit.
 *
 */
static void check_alsa( int errcode, jmp_buf retbuff, char* message ) {
	if( errcode != 0) {
		raise( retbuff, errcode,
		       "%s: %s", message, snd_strerror( errcode));
	}
}

/**
 *
 *	Return best format usable for libsndfile and an open stream.
 *
 *	@param	ctx Active, loaded context.
 *	@return A format_def with the best usable format accepted by
 *		both libsndfile and the alsa device, or null if no such found.
 *
 */
static format_def* get_best_format(	 play_context* ctx ) {
	int rc;

	for( int i = 0; i < FORMAT_CNT; i += 1 ) {
		if( FORMAT_DEFS[i].size == 0 ) {
			continue;
		}
		rc = snd_pcm_hw_params_test_format( ctx->device,
				ctx->hw_params,
				FORMAT_DEFS[i].alsa_fmt);
		if( rc >= 0 ) {
			return &FORMAT_DEFS[i];
		}
	}
	return 0;
}

/**
 *
 * Set the hw_params, alsa_channels, sample_size and readfunc in *ctx.
 *
 */
static void set_hw_params( play_context* ctx, jmp_buf exc ) {
	snd_pcm_uframes_t	alsa_period_size, alsa_buffer_frames;
	int					err;
	snd_pcm_uframes_t	buffer_size;
	format_def*			format;

	alsa_period_size =	ALSA_PERIOD_SIZE;
	alsa_buffer_frames = ALSA_BUFFER_FRAMES;

	err = snd_pcm_hw_params_malloc( &(ctx->hw_params) );
	check_alsa( err, exc, "Cannot allocate hardware parameter structure" );

	err = snd_pcm_hw_params_any(ctx->device, ctx->hw_params);
	check_alsa( err, exc, "Cannot initialize hardware parameter structure");

	err = snd_pcm_hw_params_set_access (ctx->device,
					    ctx->hw_params,
					    SND_PCM_ACCESS_RW_INTERLEAVED);
	check_alsa( err, exc, "Cannot set access type" );

	format = get_best_format( ctx );
	err = snd_pcm_hw_params_set_format (ctx->device,
					    ctx->hw_params,
					    format->alsa_fmt);
	check_alsa( err,	exc, "Cannot set sample format" );
	ctx->sample_size = format->size;
	ctx->read_func = format->read_func;

	err = snd_pcm_hw_params_set_rate_near(ctx->device,
					      ctx->hw_params,
					      &(ctx->sample_rate),
					      0);
	check_alsa( err,	exc, "Cannot set sample rate" );

	ctx->alsa_channels = ctx->file_channels;
	err = snd_pcm_hw_params_set_channels( ctx->device,
					      ctx->hw_params,
					      ctx->alsa_channels );
	if( err != 0 ) {
		if( ctx->alsa_channels == 2 ) {
			LOG_WARN( "Cannot set stereo sound, trying mono instead." );
			ctx->alsa_channels = 1;
		} else if ( ctx->alsa_channels == 1 ) {
			ctx->alsa_channels = 2;
		}
	}
	err = snd_pcm_hw_params_set_channels( ctx->device,
					      ctx->hw_params,
					      ctx->alsa_channels );
	check_alsa( err, exc, "Cannot set channel count" );

	err = snd_pcm_hw_params_set_buffer_size_near (ctx->device,
						      ctx->hw_params,
						      &alsa_buffer_frames);
	check_alsa( err, exc, "Cannot set buffer size" );

	err = snd_pcm_hw_params_set_period_size_near (ctx->device,
						      ctx->hw_params,
						      &alsa_period_size,
						      0);
	check_alsa( err, exc, "Cannot set period size" );

	err = snd_pcm_hw_params (ctx->device, ctx->hw_params);
	check_alsa( err, exc, "Cannot set parameters" );

	/* extra check: if we have only one period, this code won't work */
	snd_pcm_hw_params_get_period_size(ctx->hw_params, &alsa_period_size, 0 );
	snd_pcm_hw_params_get_buffer_size(ctx->hw_params, &buffer_size );
	if( alsa_period_size == buffer_size) {
		raise( exc, -1, "Can't use period equal to buffer size" );
	}
}

/**
 * set ctx->sw_params, update the alsa device.
 */
static void set_sw_params( play_context* ctx, jmp_buf exc ) {
	snd_pcm_uframes_t buffer_size, xfer_align, start_threshold;
	int err;

	err = snd_pcm_sw_params_malloc( &(ctx->sw_params) );
	check_alsa( err, exc, "snd_pcm_sw_params_malloc" );

	err = snd_pcm_sw_params_current (ctx->device, ctx->sw_params);
	check_alsa( err, exc, "snd_pcm_sw_params_current" );

	/* note: set start threshold to delay start until the
         *ring buffer is full
	 */
	err = snd_pcm_sw_params_get_xfer_align (ctx->sw_params, &xfer_align);
	check_alsa( err, exc, "cannot get xfer align" );

	/* round up to closest transfer boundary */
	start_threshold = (buffer_size / xfer_align) * xfer_align;
	if (start_threshold < 1) {
		start_threshold = 1;
	}

	err = snd_pcm_sw_params_set_start_threshold (ctx->device,
						     ctx->sw_params,
						     start_threshold);
	check_alsa( err, exc, "cannot set start threshold" );

	err = snd_pcm_sw_params (ctx->device, ctx->sw_params);
	check_alsa( err, exc, "snd_pcm_sw_params" );
}

static void
alsa_open( const char* devname, play_context* ctx, jmp_buf exc ) {
	int   err = 0;

	ctx->device_name = devname;
	err = snd_pcm_open( &(ctx->device),
			    ctx->device_name,
			    SND_PCM_STREAM_PLAYBACK,
			    0);
	if( err != 0) {
		raise( exc, err,
		       "Cannot open audio device %s", ctx->device_name );
	}

	snd_pcm_nonblock(ctx->device, 0);

	set_hw_params( ctx, exc );
	set_sw_params( ctx, exc);

	snd_pcm_reset( ctx->device);
}

/**
 *
 * Create an interleaved stereo buffer with the same
 * contents in both channels from a mono source buffer. The alghoritm
 * works also for expanding one buffer in_place.
 *
 * @param ctx The ctx->buffer will expanded to a stereo buffer, twice
 *			the size of the original. Uses buffer, samples and sample_size
 *			to determine size of old and new buffer. The buffer must have
 *			space to double it's contents.
 * @return	Updated values in buffer, ctx->samples is unchanged.
 *
 */
static void create_stereo_buff( play_context*	 ctx ) {
	const unsigned datasize = ctx->samples * ctx->sample_size;
	const char* last_in_buffer = ctx->buffer + datasize - ctx->sample_size;
	const char* src;
	char* dest;

	dest = ctx->buffer + (datasize	* 2) - ctx->sample_size;
	for( src = last_in_buffer; src >= ctx->buffer; src -= ctx->sample_size) {
		memcpy( dest, src,	ctx->sample_size ) ;
		dest -=	 ctx->sample_size;
		memcpy( dest, src,	ctx->sample_size ) ;
		dest -=	 ctx->sample_size;
	}
}

/**
 *
 * Handle ALSA error messages while writing.
 *
 * @param device Alsa device used for writing.
 * @param error Error code obtained from alsa calls.
 * @param exc Error return for unrecoverable errors, has the
 *			ALSA error code as argument to setjmp/longjmp.
 *
 */
static void handle_pcm_errors( snd_pcm_t* device, int error, jmp_buf exc ) {
	int res;

	switch ( error ) {
		case -EAGAIN:
			LOG_WARN("must wait" + String(snd_strerror(res)));
			res = (snd_pcm_wait( device , 1000));
			if( res <= 0 ) {
				raise( exc, res, "snd_pcm_wait failed" );
			}
			return;

		case	-EPIPE:
			LOG_WARN("overrun" + String(snd_strerror(res)));
			// Overrun, restart device.
			res = snd_pcm_prepare(device);
			if( res < 0) {
				raise( exc, res, "snd_pcm_prepare failed" );
			}
			return;

		case -ESTRPIPE:
			LOG_WARN("Attempting resume after suspend" );

			// wait until suspend flag is released
			while ((res = snd_pcm_resume(device)) == -EAGAIN) {
				sleep(1);
			}
			if (res >=0 ) {
				return;
			}

			// failed to restart stream, let's try to repair
			LOG_WARN("Failed resuming stream: "
				 + String( snd_strerror( res)));
			if ((res = snd_pcm_prepare(device)) >= 0) {
				return;
			}
			raise( exc, res,
			       "Resume failed, giving up" );


		case -ENODEV:
		case -ENOTTY:
			raise( exc, error,
			       "The sound device is gone (USB unit unplugged?)" );

		default:
			raise( exc, error,
			       "Unexpected error in write_data %d", error );
	}
}

static int alsa_write_data(play_context* ctx, jmp_buf exc ) {
	size_t		 total = 0;
	int			 res;
	void*		 next_chunk;
        const int    framesize = ctx->alsa_channels * ctx->sample_size;

	if(	 ctx->samples == 0 ) {
		return 0;
	}

	assert( ctx->samples * ctx->sample_size <= BUFFER_LEN );

	if( ctx->file_channels == 2 && ctx->alsa_channels == 1 ) {
		raise( exc, -1, "Attempt to play stereo stream on mono device" );
	}

	if( ctx->file_channels == 1 && ctx->alsa_channels == 2 ) {
		create_stereo_buff( ctx );
	}

	while( total < ctx->samples ) {
		next_chunk =  ctx->buffer + total * framesize;
		res = snd_pcm_writei( ctx->device,
				      next_chunk,
				      ctx->samples - total);
		if( res < 0 ) {
			handle_pcm_errors( ctx->device, res, exc );
			continue;
		} else {
			total += res;
		}
	}
	return total;
}
/**@}*/

/* See alsa_sndfile.h. */
void alsa_play_file(const char* filename, const char* device, int* stopper) {
	play_context	 ctx;
	jmp_buf		 exc;

	init_context( &ctx );
	if( setjmp( exc) != 0) {
		free_context( &ctx );
		return ;
	}

	LOG_DEBUG("playing " + String(filename));

	sndfile_open( filename, &ctx, exc );
	alsa_open( device, &ctx, exc );

	assert( BUFFER_LEN % (ctx.sample_size * ctx.file_channels) == 0 );
	do {
		ctx.samples = ctx.read_func( &ctx,
					     BUFFER_LEN / ctx.file_channels );
		alsa_write_data( &ctx, exc );
	} while ( ctx.samples > 0  && !(*stopper) );

	snd_pcm_drain( ctx.device );
	free_context( &ctx );
}

