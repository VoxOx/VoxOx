/*
 * phmedia -  Phone Api media streamer
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

#include "phrecorder.h"

static void ph_media_audio_recording_dump(recording_t *recording);

void
ph_media_audio_recording_init(recording_t *recording, const char *filename, int nchannels, int chunksize) 
{
  recording->samples = (short *) malloc(nchannels * chunksize * sizeof(short));
  recording->chunksize = chunksize;
  recording->nchannels = nchannels;
  recording->position = 0;
  recording->fd = fopen(filename,"wb");
}

void
ph_media_audio_recording_record_one(recording_t *recording, short c1, short c2, short c3) 
{
  short *samples = recording->samples+recording->nchannels*recording->position;

  *samples++ = c1;
  if (recording->nchannels > 1)
  {
    *samples++ = c2;
  }
  if (recording->nchannels > 2)
  {
    *samples++ = c3;
  }

  recording->position++;
  if (recording->position == recording->chunksize) 
  {
    ph_media_audio_recording_dump(recording);
    recording->position = 0;
  }
}

static void
ph_media_audio_recording_dump(recording_t *recording) 
{
  if (recording->position > 0) 
  {
    fwrite(recording->samples, recording->nchannels*sizeof(short), recording->position, recording->fd);
  }
}

void
ph_media_audio_recording_close(recording_t *recording) 
{
  ph_media_audio_recording_dump(recording);
  if(recording->fd)
  {
    fclose(recording->fd);
  }

  if (recording->samples)
  {
    free(recording->samples);
  }
}

void
ph_media_audio_fast_recording_init(recording_t *recording, const char *filename)
{
  recording->samples = 0;
  recording->chunksize = 0;
  recording->nchannels = 0;
  recording->position = 0;
  recording->fd = fopen(filename,"wb");
}

void
ph_media_audio_fast_recording_record(recording_t *recording, const void *payload, int size)
{
  if(recording->fd)
  {
    fwrite(payload, 1, size, recording->fd);
  }
}
