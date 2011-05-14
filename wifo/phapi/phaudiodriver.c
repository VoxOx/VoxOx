/*
  The phaudiodriver module implements abstract audio device interface for phapi
  Copyright (C) 2004  Vadim Lebedev  <vadim@mbdsys.com>
  Copyright (C) 2006  Wengo SAS

  this module is free software; you can redistribute it and/or modify
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


#include <stdlib.h>
#include <string.h>
#include "phlog.h"
#include "phaudiodriver.h"

#ifdef WIN32
#define strncasecmp strnicmp
#endif

#define MAX_SOUND_DRIVERS  8

struct ph_audio_driver *ph_snd_driver_map[MAX_SOUND_DRIVERS];
struct ph_audio_driver ph_snd_driver;

void ph_register_audio_driver(struct ph_audio_driver *d)
{
  struct ph_audio_driver **map = ph_snd_driver_map;

  DBG_DYNA_AUDIO_DRV("registering audio driver of kind \"%s\" ...\n", d->snd_driver_kind);

  if (!d)
  {
    return;
  }

  for( ; map  < &ph_snd_driver_map[MAX_SOUND_DRIVERS]; map++)
  {

    if (*map == d)
    {
      DBG_DYNA_AUDIO_DRV("...already registered\n");
      return;
    }

    if (!*map)
    {
      *map = d;
      DBG_DYNA_AUDIO_DRV("...registration ok\n");
      return;
    }
  }
}

struct ph_audio_driver *ph_find_audio_driver(const char *name)
{
  struct ph_audio_driver **map = ph_snd_driver_map;

  for( ; map  < &ph_snd_driver_map[MAX_SOUND_DRIVERS]; map++)
  {
    int l;

    if (!*map)
    {
        continue;
    }

    if ((*map)->snd_driver_match && !(*map)->snd_driver_match(*map, name))
    {
        return *map;
    }

    l = strlen((*map)->snd_driver_kind);

    if (!strncasecmp((*map)->snd_driver_kind, name, l))
    {
      return *map;
    }
  }

  return 0;
}

int ph_activate_audio_driver(const char *name)
{
  struct ph_audio_driver *d;

  if (!name || !name[0])
  {
    name = getenv("PH_AUDIO_DEVICE");
  }

  if (!name)
  {
    name = "alsa:default";
  }

  d = ph_find_audio_driver(name);

  if (!d)
  {
    return -PH_SNDDRVR_NOTFOUND;
  }

  if (ph_snd_driver.snd_driver_kind && !strcmp(ph_snd_driver.snd_driver_kind, d->snd_driver_kind))
  {
    return 0;
  }

  if (PH_SNDDRVR_USAGE > 0)
  {
    return -PH_SNDDRVR_BUSY;
  }

  ph_snd_driver = *d;

  return 0;
}
