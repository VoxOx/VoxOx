/*
 This module is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 eXosip is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#include <esd.h>

/*
 * @author David Ferlier <david.ferlier@wengo.fr>
 *
 * esd driver for phapi
 *
 */

/**
 * Declare the driver to phmedia-audio and initialize it.
 */
void ph_ph_audio_esd_driver_init();

/**
 * Start the stream.
 *
 * @return 
 */
void ph_audio_esd_start(phastream_t *as);

/**
 * Open a device.
 *
 * @param name desired device name
 * @param rate desired audio rate
 * @param framesize 
 * @param cbk 
 * @return 
 */
int ph_audio_esd_open(phastream_t *as, const char *name, int rate, int framesize, ph_audio_cbk cbk);

/**
 * Write sound data onto the device.
 *
 * @param buf data buffer
 * @param len length of data buffer
 */
int ph_audio_esd_write(phastream_t *as, void *buf, int len);

/**
 * Read sound data from the device.
 *
 * @param buf data buffer
 * @param len length of data buffer
 */
int ph_audio_esd_read(phastream_t *as, void *buf, int len);

/**
 * Get out data size.
 *
 * @param used
 */
int ph_audio_esd_get_out_space(phastream_t *as, int *used);

/**
 * Get available data.
 */
int ph_audio_esd_get_avail_data(phastream_t *as);

/**
 * Close the stream.
 */
void ph_audio_esd_close(phastream_t *as);

struct esd_priv_data {
    esd_format_t rec_format;
    esd_format_t out_format;
    int rec_sock;
    int out_sock;
    char *host;
};

struct ph_audio_driver ph_audio_esd_driver = {
	"esd",
	0,
	0,
	ph_audio_esd_start,
	ph_audio_esd_open,
	ph_audio_esd_write,
	ph_audio_esd_read,
	ph_audio_esd_get_out_space,
	ph_audio_esd_get_avail_data,
	ph_audio_esd_close
};


void ph_audio_esd_driver_init() {
	ph_register_audio_driver(&ph_audio_esd_driver);
}

void ph_audio_esd_start(phastream_t *as) {
}

int ph_audio_esd_open(phastream_t *as, const char *name, int rate, int framesize, ph_audio_cbk cbk) {
    struct esd_priv_data *ad;
    int errcode;

    ad = (struct esd_priv_data *) malloc (sizeof(struct esd_priv_data));

    ad->rec_format = ESD_BITS16 | ESD_STEREO | ESD_STREAM | ESD_RECORD;
    ad->out_format = ESD_BITS16 | ESD_STEREO | ESD_STREAM | ESD_PLAY;
    ad->host = NULL;

    ad->rec_sock = esd_record_stream(ad->rec_format, rate,
            ad->host, NULL);

    if (ad->rec_sock <= 0)
	{
        return -PH_NORESOURCES;
    }

    ad->out_sock = esd_play_stream(ad->out_format, rate,
            ad->host, NULL);

    if (ad->out_sock <= 0)
	{
        return -PH_NORESOURCES;
    }

    as->drvinfo = ad;
    return 0;
}

int ph_audio_esd_write(phastream_t *as, void *buf, int len) {
    struct esd_priv_data *ad = as->drvinfo;
    int written_bytes;

    written_bytes = write(ad->out_sock, buf, len);
    return written_bytes;
}

int ph_audio_esd_read(phastream_t *as, void *buf, int len) {
    struct esd_priv_data *ad = as->drvinfo;
    int read_bytes;

    read_bytes = read(ad->rec_sock, buf, len);
    return read_bytes;
}

int ph_audio_esd_get_out_space(phastream_t *as, int *used) {
    struct esd_priv_data *ad = as->drvinfo;
	*used = 2 * ESD_BUF_SIZE;
	return *used;
}

int ph_audio_esd_get_avail_data(phastream_t *as) {
	return 0;
}

void ph_audio_esd_close(phastream_t *as) {
    struct esd_priv_data *ad = as->drvinfo;
    close(ad->rec_sock);
    close(ad->out_sock);
}
