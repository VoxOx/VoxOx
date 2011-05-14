/*
 * phresample Audio data resampling
 *
 * Copyright (C) 2006 WENGO SAS
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

#ifndef __PHRESAMPLE_H_
#define __PHRESAMPLE_H__

/**
 * init speaker data for libsamplerate
 * @param clockrate expected clockrate
 * @param actual_clockrate current cockrate
 * @return a pointer to the context for speaker resampling
 */
void * ph_resample_spk_init0(int clockrate, int actual_clockrate);

/**
 * init microphone data for libsamplerate
 * @param clockrate expected clockrate
 * @param actual_clockrate current cockrate
 * @return a pointer to the context for microphone resampling
 */
void * ph_resample_mic_init0(int clockrate, int actual_clockrate);

/**
 * cleanup allocated memory for libsamplerate
 * @param ctx a resampling context return by ph_resample_spk_init0 or ph_resample_mic_init0
 */
void ph_resample_cleanup0(void * ctx);

/**
 * resample audio data
 * @param ctx a resampling context return by ph_resample_spk_init0 or ph_resample_mic_init0
 * @param inbuf input buffer
 * @param inbsize input buffer size
 * @param outbuf output buffer
 * @param outbsize output buffer size
 */
void ph_resample_audio0(void * ctx, void * inbuf, int inbsize, void * outbuf, int * outbsize);

#endif  //__PHRESAMPLE_H__
