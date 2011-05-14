/* -*- Mode: C; tab-width:8; c-basic-offset:8; -*-   */

#ifndef __PH_AUDIODRIVER_H__
#define __PH_AUDIODRIVER_H__

/*
  Copyright (C) 2004  Vadim Lebedev  <vadim@mbdsys.com>

  This module is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Phapi is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/**
 * 
 * @file phaudiodriver.h 
 *
 * The phaudiodriver module implements abstract audio device interface for 
 * phapi.
 *
 * In general, the functions in this API are not threadsafe. Exceptions are
 * documented for each method.
 *
 * The audio streams in phapi is mono mode streams with 16-bits signed data.
 * Codecs typically needs/emits data every 20 ms.
 *
 * In the playback %stream (network => speaker) phapi is generating
 * comfort noise, and jitter mgmt as required if data  is missing, 
 * network delays etc. Rebuffering is provided on a best effort basis for
 * many but all cases.
 *
 * In the %recording %stream (mic => network) phapi is responsible for echo
 * cancellation.
 *
 * @see http://dev.openwengo.com/trac/openwengo/trac.fcgi/wiki/PhApiAudio 
 *
 */
 
#ifdef __cplusplus
extern "C" {
#endif

struct ph_audio_stream;

/**
 *
 * Callback invoked from driver threads (in callback mode) to transfer
 * data between audio driver and phapi. This callback is threadsafe.
 *
 * @param recordbuf   Input data, from mic etc.
 * @param recbufsize  Length of recordbuf (bytes).
 * @param playbackbuf Output data, to speaker etc.
 * @param playsize    Length of playbackbuf (bytes), updated
 *                    to content length (bytes) on exit.
 * @return            0 
 *
 * @see phmedia-audio.c 
 */
typedef int (*ph_audio_cbk)(struct ph_audio_stream* as, 
			    void*                   recordbuf, 
			    int                     recbufsize, 
			    void*                   playbackbuf, 
			    int*                    playsize);

/**
 * Start the audio driver - possibly starts e g physical IO and threads.
 *
 * @param stream ph_audio_stream returned by ph_audio_open().
 */
typedef void ph_audio_start(struct ph_audio_stream* stream );

/**
 *
 * Open an audio %stream.
 * 
 * @param as Filled with data on normal exit.
 * @param name Name of device to open for input and output.
 * @param rate requested sample rate (samples per second).
 * @param chunk_size  # bytes transferred in each IO operation.
 * @param cbk invoked by driver to transfer data in callback mode.
 * @return 0 on success, otherwise negative ::phError error code.
 *
 */
typedef int ph_audio_open(  struct ph_audio_stream *as, 
			    char *name, 
			    int rate, 
			    int chunk_size,
			    ph_audio_cbk cbk);
/**
 *
 * Write some data to output device. This call should not block.
 *
 * @param as Started %stream.
 * @param buf Data to write.
 * @param len # bytes to write.
 * @return # bytes written, possibly 0 on errors or missing
 * buffer space.
 *
 */
typedef int ph_audio_write(struct ph_audio_stream *as, 
			   void *buf,  
			   int len);
/**
 * Read data from input device. This call should not block.
 *
 * @param as Started %stream.
 * @param buf Pointer to input buffer.
 * @param len Length of input buffer, in bytes.
 * @return # bytes read, possibly 0 if no data is available
 *         (or other errors).
 *
 */
typedef int ph_audio_read(struct ph_audio_stream *as, 
			  void *buf,  
			  int len);

/**
 * Get size of output buffer. 
 *
 * @param used On exit, used space in buffer (bytes).
 * @return Available free space in output buffer in bytes, or -1 on errors.
 *
 */
typedef int ph_audio_get_out_space( struct ph_audio_stream *as, int* used);

/**
 * Return size of buffered input data. Not used if the 
 * #PH_SNDDRVR_REC_CALLBACK flag is set.
 *
 * @return # input bytes buffered in driver, or -1 on errors.
 *
 */
typedef int ph_audio_get_avail_data( struct ph_audio_stream *as);

/**
 * Close an active %stream, deallocate all %memory used by driver.
 */
typedef void ph_audio_close( struct ph_audio_stream *as);

/**
 *
 * Get the streams file descriptors for input/output. Only implemented
 * if the #PH_SNDDRVR_FDS driver flag is set. Not used if both
 * #PH_SNDDRVR_REC_CALLBACK and #PH_SNDDRVR_PLAY_CALLBACK is set.
 * 
 * @param as Opened %stream.
 * @param fds On exit, fds[0] is the input and fds[1] the output
 *        file descriptor.
 * @return 0 on success, else an ::phError error code.
 * @see poll(3), select(2)
 *
 */
typedef int ph_audio_get_fds( struct ph_audio_stream *as, int fds[2]); 

/**
 *
 * Test if driver has a given name.
 *
 * @param as Inited audio %stream.
 * @param  name compared to the init() name argument.
 * @return PH_TRUE if the driver's name matches (is equal to) name, 
 *         else PH_FALSE.
 *
 */
typedef int ph_audio_driver_match( struct ph_audio_stream *as, 
				   const char* name); 


/** 
 *  Driver defines it's own thread using ::ph_audio_cbk 
 *  handling data phapi -> audio. 
 */
#define PH_SNDDRVR_REC_CALLBACK    1

/** 
 * Driver defines it's own thread using ::ph_audio_cbk handling 
 * data audio -> phapi. 
 */
#define PH_SNDDRVR_PLAY_CALLBACK   2

/** Driver implements ::ph_audio_get_fds. */
#define PH_SNDDRVR_FDS             4

/** The driver block defining a driver presented to phapi in init(). */
struct ph_audio_driver
{
	const char *snd_driver_kind;
	int        snd_driver_flags;
	int        snd_driver_usage;

	ph_audio_start           *snd_stream_start;
	ph_audio_open            *snd_stream_open;
	ph_audio_write           *snd_stream_write;
	ph_audio_read            *snd_stream_read;
	ph_audio_get_out_space   *snd_stream_get_out_space;
        ph_audio_get_avail_data  *snd_stream_get_avail_data;
	ph_audio_close           *snd_stream_close;
        ph_audio_get_fds         *snd_stream_get_fds;
        ph_audio_driver_match    (*snd_driver_match);
};
extern struct ph_audio_driver ph_snd_driver;

#define audio_driver_has_rec_callback()     \
	(ph_snd_driver.snd_driver_flags & PH_SNDDRVR_REC_CALLBACK)

#define audio_driver_has_play_callback()    \
	(ph_snd_driver.snd_driver_flags & PH_SNDDRVR_PLAY_CALLBACK)

#define audio_driver_has_fds()              \
	(ph_snd_driver.snd_driver_flags & PH_SNDDRVR_FDS)

#define audio_stream_open(as, name, irate, framesize, cbk) \
	ph_snd_driver.snd_stream_open(as, name, irate, framesize, cbk)

#define audio_stream_start(as)              \
	ph_snd_driver.snd_stream_start(as)

#define audio_stream_write(as, buf,  len)   \
	ph_snd_driver.snd_stream_write(as, buf,  len)

#define audio_stream_read(as, buf,  len)    \
	ph_snd_driver.snd_stream_read(as, buf,  len)

#define audio_stream_get_out_space(as,used) \
	ph_snd_driver.snd_stream_get_out_space(as, used)
	
#define audio_stream_get_avail_data(as)     \
	ph_snd_driver.snd_stream_get_avail_data(as)

#define audio_stream_close(as)              \
	ph_snd_driver.snd_stream_close(as)

#define audio_stream_get_fds(as, fds)       \
	ph_snd_driver.snd_stream_get_fds(as, fds)

#define PH_SNDDRVR_USE()                ph_snd_driver.snd_driver_usage++
#define PH_SNDDRVR_UNUSE()              ph_snd_driver.snd_driver_usage--
#define PH_SNDDRVR_USAGE                ph_snd_driver.snd_driver_usage

/**
 * Register a driver plugin that can be used later on.
 *
 * @param driver Driver definition.
 *
 * FIXME: arg should be 'const'
 */
void  ph_register_audio_driver( struct ph_audio_driver * driver);

/**  ::ph_activate_audio_driver error codes. */
enum { PH_SNDDRVR_BUSY = 1, PH_SNDDRVR_NOTFOUND = 2 };

/** Activate a driver with a given name. */
int   ph_activate_audio_driver(const char* name);

/** Locate a driver with a given name. */
struct ph_audio_driver *ph_find_audio_driver(const char *name) ;


#ifdef __cplusplus
}
#endif

#endif  //__PH_AUDIODRIVER_H__
