/*
 * The phmedia-audio-null  module implements interface to a null audio driver
 *
 * Copyright (C) 2004  Vadim Lebedev  <vadim@mbdsys.com>
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

#include <osip2/osip_mt.h>
#include <osip2/osip.h>
#ifndef T_MSVC
#include <sys/ioctl.h>
#include <sys/time.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <ortp.h>
#include <ortp-export.h>
#include <telephonyevents.h>
#include "phapi.h"
#include "phcall.h"
#include "phmedia.h"
#include "phcodec.h"
#include "tonegen.h"
#include "phmbuf.h"

#include "phmstream.h"
#include "phastream.h"
#include "phaudiodriver.h"
#include "phlog.h"

#define PH_UNREFERENCED_PARAMETER(P) (P)

/**
 * Declare the driver to phmedia-audio and initialize it.
 */
void ph_phadnull_driver_init();

/**
 * Start the stream.
 *
 * @return
 */
void phadnull_start(phastream_t *as);

/**
 * Open a device.
 *
 * @param name desired device name
 * @param rate desired audio rate
 * @param framesize
 * @param cbk
 * @return
 */
int phadnull_open(phastream_t *as, const char *name, int rate, int framesize, ph_audio_cbk cbk);

/**
 * Write sound data onto the device.
 *
 * @param buf data buffer
 * @param len length of data buffer
 */
int phadnull_write(phastream_t *as, void *buf, int len);

/**
 * Read sound data from the device.
 *
 * @param buf data buffer
 * @param len length of data buffer
 */
int phadnull_read(phastream_t *as, void *buf, int len);

/**
 * Get out data size.
 *
 * @param used
 */
int phadnull_get_out_space(phastream_t *as, int *used);

/**
 * Get available data.
 */
int phadnull_get_avail_data(phastream_t *as);

/**
 * Close the stream.
 */
void phadnull_close(phastream_t *as);

struct ph_audio_driver ph_phadnull_audio_driver = {
  "null",
  0,
  0,
  phadnull_start,
  phadnull_open,
  phadnull_write,
  phadnull_read,
  phadnull_get_out_space,
  phadnull_get_avail_data,
  phadnull_close
};

void ph_phadnull_driver_init() {
  ph_register_audio_driver(&ph_phadnull_audio_driver);
}

void phadnull_start(phastream_t *as) {
  PH_UNREFERENCED_PARAMETER(as);
  DBG_DYNA_AUDIO_DRV("phad_null: Starting audio stream\n");
}

int phadnull_open(phastream_t *as, const char *name, int rate, int framesize, ph_audio_cbk cbk) {
  PH_UNREFERENCED_PARAMETER(as);
  DBG_DYNA_AUDIO_DRV("phad_null: Opening device %s with rate: %d, framesize: %d, and callback: %p\n",
    name, rate, framesize, cbk);
  as->actual_rate = rate; // important
  return 0;
}

int phadnull_write(phastream_t *as, void *buf, int len) {
  PH_UNREFERENCED_PARAMETER(as);
  DBG_DYNA_AUDIO_DRV("phad_null: Writing %d bytes of data from buffer %p\n", len, buf);
  return 0;
}

int phadnull_read(phastream_t *as, void *buf, int len) {
  PH_UNREFERENCED_PARAMETER(as);
  DBG_DYNA_AUDIO_DRV("phad_null: Reading %d bytes of data and putting it into buffer %p\n", len, buf);
  return 0;
}

int phadnull_get_out_space(phastream_t *as, int *used) {
  *used = 0;
  PH_UNREFERENCED_PARAMETER(as);
  DBG_DYNA_AUDIO_DRV("phad_null: Out space used: 0\n", *used);
  return 0;
}

int phadnull_get_avail_data(phastream_t *as) {
  PH_UNREFERENCED_PARAMETER(as);
  DBG_DYNA_AUDIO_DRV("phad_null: Available data: 0\n");
  return 0;
}

void phadnull_close(phastream_t *as) {
  PH_UNREFERENCED_PARAMETER(as);
  DBG_DYNA_AUDIO_DRV("phad_null: Closing audio stream\n");
}
