/*
 * phrecorder -  Phone Api media recorder
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
#ifndef __PHRECORDER_H__
#define __PHRECORDER_H__

#include <stdio.h>
#include <stdlib.h>

struct recording
{
  short *samples;
  int chunksize;
  int nchannels;
  int position;
  FILE *fd;
};
typedef struct recording recording_t;

/** first version of recording functions (mainly used for audio processing debugging purpose) */
void ph_media_audio_recording_init(recording_t *recording, const char *filename, int nchannels, int chunksize);
void ph_media_payload_recording_init(recording_t *recording, const char *filename);
void ph_media_audio_recording_close(recording_t *recording);
void ph_media_audio_recording_record_one(recording_t *recording, short c1, short c2, short c3);
void ph_media_payload_record(recording_t *recording, const void *payload, int size);

void ph_media_audio_fast_recording_init(recording_t *recording, const char *filename);
void ph_media_audio_fast_recording_record(recording_t *recording, const void *payload, int size);
#endif  //__PHRECORDER_H__
