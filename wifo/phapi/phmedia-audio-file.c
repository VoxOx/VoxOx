/*
 * The phmedia-audio-file  module implements interface to a file audio driver
 * It is especially usefull to mimic phapi's networking behaviour on a machine that has no soundcard
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

#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "phlog.h"
#include "phapi.h"

#include "phmstream.h"
#include "phastream.h"
#include "phrecorder.h"
#include "phaudiodriver.h"

const char* DFLT_AD_FILE_MIC_FILE = "phadfile_mic.data";
const char* DFLT_AD_FILE_SPK_FILE = "phadfile_spk.data";

#define PH_UNREFERENCED_PARAMETER(P) (P)

struct phadfile_dev {
	/* virtual MIC parameters */
	char mic_filename[128];
	FILE *mic_fd;
	/* virtual SPK parameters */
	char spk_filename[128];
	recording_t spk_recorder;
};

// NOTE: phadfile stands for ph_audio_driver "file"

/**
 * Declare the driver to phmedia-audio and initialize it.
 */
void ph_phadfile_driver_init();

/**
 * Open a device.
 * pay attention to the as->actual_rate that is the agreed sample rate
 * @param name desired device name
 * @param rate desired audio rate
 * @param framesize
 * @param cbk
 * @return
 */
int phadfile_open(phastream_t *as, const char *name, int rate, int framesize, ph_audio_cbk cbk);

/**
 * Start the stream.
 *
 * @return
 */
void phadfile_start(phastream_t *as);

/**
 * Write sound data onto the device.
 *
 * @param buf data buffer
 * @param len length of data buffer
 */
int phadfile_write(phastream_t *as, void *buf, int len);

/**
 * Read sound data from the device.
 *
 * @param buf data buffer
 * @param len length of data buffer
 */
int phadfile_read(phastream_t *as, void *buf, int len);

/**
 * Get out data size.
 *
 * @param used
 */
int phadfile_get_out_space(phastream_t *as, int *used);

/**
 * Get available data.
 */
int phadfile_get_avail_data(phastream_t *as);

/**
 * Close the stream.
 */
void phadfile_close(phastream_t *as);

struct ph_audio_driver ph_phadfile_audio_driver = {
  "file",
  0, // the threading model is fully managed by phapi
  0,
  phadfile_start,
  phadfile_open,
  phadfile_write,
  phadfile_read,
  phadfile_get_out_space,
  phadfile_get_avail_data,
  phadfile_close
};

void ph_phadfile_driver_init() {
  ph_register_audio_driver(&ph_phadfile_audio_driver);
}

int phadfile_open(phastream_t *as, const char *name, int rate, int framesize, ph_audio_cbk cbk) {
  struct phadfile_dev *pd;
  char* mic_tmpfilename = NULL;
  char* spk_tmpfilename = NULL;

  DBG_DYNA_AUDIO_DRV("phad_file: Opening device %s with rate: %d, framesize: %d, and callback: %p\n",
    name, rate, framesize, cbk);

  pd = calloc(1, sizeof(*pd));
  if (!pd)
  {
    return -PH_NORESOURCES;
  }
  memset(pd, 0, sizeof(*pd));

  mic_tmpfilename = getenv("PH_FILE_MIC_FILE");
  spk_tmpfilename = getenv("PH_FILE_SPK_FILE");

  if (!mic_tmpfilename)
  {
    mic_tmpfilename =  DFLT_AD_FILE_MIC_FILE;
  }

  if (!spk_tmpfilename)
  {
    spk_tmpfilename = DFLT_AD_FILE_SPK_FILE;
  }

  snprintf(pd->mic_filename, 128, mic_tmpfilename);
  snprintf(pd->spk_filename, 128, spk_tmpfilename);
  
  pd->mic_fd = fopen(pd->mic_filename,"rb");
  if (!pd->mic_fd)
  {
    return -PH_NORESOURCES;
  }

  ph_media_audio_fast_recording_init(&pd->spk_recorder, pd->spk_filename);
  as->drvinfo = pd;
  as->actual_rate = rate; // important - here we suppose FORCE_16KHZ
  return 0;
}

void phadfile_start(phastream_t *as) {
  PH_UNREFERENCED_PARAMETER(as);
  DBG_DYNA_AUDIO_DRV("phad_file: Starting audio stream\n");
}

int phadfile_write(phastream_t *as, void *buf, int len) {
  struct phadfile_dev *pd = as->drvinfo;
  DBG_DYNA_AUDIO_DRV("phad_file: Writing %d bytes of data from buffer %p\n", len, buf);

  ph_media_audio_fast_recording_record(&pd->spk_recorder, buf, len);
  
  return len;
}

int phadfile_read(phastream_t *as, void *buf, int len) {
  struct phadfile_dev *pd = as->drvinfo;
  int could_read = 0;
  DBG_DYNA_AUDIO_DRV("phad_file: Reading %d bytes of data and putting it into buffer %p\n", len, buf);
  
  // for a quick hack, we loop by dropping the last unaligned chunk
  could_read = fread(buf, 1, len, pd->mic_fd);
  DBG_DYNA_AUDIO_DRV("phad_file: just read %d bytes from file\n", could_read);
  if (could_read < len)
  {
	  rewind(pd->mic_fd);
	  fread(buf, 1, len, pd->mic_fd);
  }
  
  return len;
}

int phadfile_get_out_space(phastream_t *as, int *used) {
  *used = 0;
  PH_UNREFERENCED_PARAMETER(as);
  DBG_DYNA_AUDIO_DRV("phad_file: Out space used: 0\n", *used);
  return 0;
}

int phadfile_get_avail_data(phastream_t *as) {
  PH_UNREFERENCED_PARAMETER(as);
  DBG_DYNA_AUDIO_DRV("phad_file: Available data: 0\n");
  return 0;
}

void phadfile_close(phastream_t *as) {
  struct phadfile_dev *pd = as->drvinfo;
  DBG_DYNA_AUDIO_DRV("phad_file: Closing audio stream\n");

  ph_media_audio_recording_close(&pd->spk_recorder);
  
  if(pd->mic_fd)
  {
    fclose(pd->mic_fd);
  }
  
  free(pd);
  
  as->drvinfo = NULL;
}
