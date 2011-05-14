/*
 * ALSA mixer
 * Copyright (C) 2007  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @author Mathieu Stute <mstute@gmail.com>
 */

#ifndef _ALSA_MIXER_H
#define _ALSA_MIXER_H

#include <alsa/asoundlib.h>

typedef enum {
	MASTER_VOLUME,
	PCM_VOLUME,
	CAPTURE_VOLUME,
	MIC_CAPTURE_VOLUME,
	MIC_PLAYBACK_VOLUME,
} volume_t;

/**
 * Sets level
 *
 * @param sound_card the alsa name of (ie hw:x))
 * @param volume_type volume type
 * @param level new level
 * @return 0 on success otherwise -1
 */
int set_mixer_level(const char *sound_card, volume_t volume_type, int level);

/**
 * Gets level
 *
 * @param sound_card the alsa name of (ie hw:x))
 * @param volume_type volume type
 * @param level the current level
 * @return 0 on success otherwise -1
 */
int get_mixer_level(const char *sound_card, volume_t volume_type, int *level);

#endif  //_ALSA_MIXER_H
