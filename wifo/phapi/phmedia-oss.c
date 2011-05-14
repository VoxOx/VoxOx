/*
 * The phmedia-oss  module implements interface to OSS audio devices for phapi
 *
 * Copyright (C) 2004  Vadim Lebedev  <vadim@mbdsys.com>
 * Copyright (C) 2004-2007 WENGO SAS
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

#include "phmedia.h"
#include "phastream.h"
#include "phaudiodriver.h"
#include "phlog.h"

#include <sys/soundcard.h>
#include <strings.h>
#include <sys/ioctl.h>

#define ADEV(x) ((int)(x->drvinfo))

void oss_stream_start(phastream_t *as);
int  oss_stream_open(phastream_t *as, char *name, int rate, int framesize, ph_audio_cbk cbk);
int  oss_stream_write(phastream_t *as, void *buf,  int len);
int  oss_stream_read(phastream_t *as, void *buf,  int len);
int  oss_stream_get_out_space(phastream_t *as, int *used);
int  oss_stream_get_avail_data(phastream_t *as);
void oss_stream_close(phastream_t *as);
int oss_stream_get_fds(phastream_t *as, int fds[2]);

void ph_oss_driver_init(void);

struct ph_audio_driver ph_oss_driver = {
  "oss",
  PH_SNDDRVR_FDS,
  0,
  oss_stream_start,
  oss_stream_open,
  oss_stream_write,
  oss_stream_read,
  oss_stream_get_out_space,
  oss_stream_get_avail_data,
  oss_stream_close,
  oss_stream_get_fds,
  NULL
};

void ph_oss_driver_init(void)
{
  ph_register_audio_driver(&ph_oss_driver);
}

int oss_stream_open(phastream_t *as, char *name, int rate, int framesize, ph_audio_cbk cbk)
{
  int fd, p;
  int blocksize = 512;
  int min_size;
  int cond;
  struct audio_buf_info info;
  int bufsize;

  DBG_DYNA_AUDIO_DRV("OSS: oss_stream_open: %s\n", name);
  if (!strncasecmp(name, "oss:", 4))
  {
    name += 4;
  }

  fd=open(name, O_RDWR|O_NONBLOCK);

  if (fd < 0)
  {
    DBG_DYNA_AUDIO_DRV("OSS: error opening opening AUDIO device (fd = %d)\n", fd);
    exit(1);
  }

  ioctl(fd, SNDCTL_DSP_RESET, 0);

  p = 16;  /* 16 bits */
  ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &p);

  p = 1;
  ioctl(fd, SNDCTL_DSP_CHANNELS, &p);

  p = AFMT_S16_NE; /* choose LE or BE (endian) */
  ioctl(fd, SNDCTL_DSP_SETFMT, &p);

  p = rate;  /* rate in hz*/
  ioctl(fd, SNDCTL_DSP_SPEED, &p);

  ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &min_size);
  if (min_size > blocksize)
  {
    cond = 1;
    p = min_size / blocksize;
    while(cond)
    {
      int i = ioctl(fd, SNDCTL_DSP_SUBDIVIDE, &p);
      if (i != 0)
      {
        DBG_DYNA_AUDIO_DRV( "SUB_DIVIDE %d said error=%i,errno=%i\n", p, i, errno);
      }
      if ((i == 0) || (p == 1))
      {
        cond = 0;
      }
      else
      {
        p = p / 2;
      }
    }
  }
  ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &min_size);
  if (min_size > blocksize)
  {
    blocksize = min_size;
  }
  else
  {
    /* no need to access the card with less latency than needed*/
    min_size=blocksize;
  }

  if( 0 > ioctl(fd, SNDCTL_DSP_GETISPACE, &info))
  {
    DBG_DYNA_AUDIO_DRV("OSS: AUDIO DEV IOCTL error\n");
    close(fd);
    exit(1);
  }
  else
  {
    DBG_DYNA_AUDIO_DRV("Audio in buffer %d %d\n",info.fragstotal,info.fragsize);
  }

  bufsize =  info.fragstotal*info.fragsize;

  if( 0 > ioctl(fd, SNDCTL_DSP_GETOSPACE, &info))
  {
    DBG_DYNA_AUDIO_DRV("OSS: AUDIO DEV IOCTL error\n");
    close(fd);
    exit(1);
  }
  else
  {
    DBG_DYNA_AUDIO_DRV("Audio out buffer %d %d\n", info.fragstotal, info.fragsize);
  }

  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)&~O_NONBLOCK);

  // HACK: assume that the rate is always good
  as->actual_rate = rate;

  as->drvinfo = (void*)fd;

  PH_SNDDRVR_USE();

  return 0;
}


/* start reading from the device */
void oss_stream_start(phastream_t *as)
{

  int enable_bits  = PCM_ENABLE_INPUT|PCM_ENABLE_OUTPUT;
  int i;

  i = ioctl(ADEV(as), SNDCTL_DSP_SETTRIGGER, &enable_bits);
  if (i < 0)
  {
    DBG_DYNA_AUDIO_DRV( "Can't start audio intput\n");
  }
}

int oss_stream_write(phastream_t *as, void *buf,  int len)
{
  return write(ADEV(as), buf, len);
}

int oss_stream_read(phastream_t *as, void *buf,  int len)
{
  return read(ADEV(as), buf, len);
}

void oss_stream_close(phastream_t *as)
{
  if (ADEV(as) == -1)
  {
    return; 
  }

  close(ADEV(as));

  as->drvinfo = (void *)-1;

  PH_SNDDRVR_UNUSE();
}

int oss_stream_get_out_space(phastream_t *as, int *used)
{
  struct audio_buf_info info;

  if( 0 > ioctl(ADEV(as), SNDCTL_DSP_GETOSPACE, &info))
  {
    return -1;
  }

  *used = info.fragsize * info.fragstotal - info.bytes;
  return info.bytes;
}

int oss_stream_get_avail_data(phastream_t *as)
{
  struct audio_buf_info info;

  if( 0 > ioctl(ADEV(as), SNDCTL_DSP_GETISPACE, &info))
  {
    return -1;
  }

  return info.bytes;
}

int oss_stream_get_fds(phastream_t *as, int fds[2])
{
  fds[0] = fds[1] = ADEV(as);
  return 0;
}
