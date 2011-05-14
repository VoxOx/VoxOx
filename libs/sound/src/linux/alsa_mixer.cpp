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

#include "alsa_mixer.h"

#include <util/String.h>
#include <util/Logger.h>

typedef enum {
	PLAYBACK,
	CAPTURE,
} mixer_element_type;

/**
 * opens a sound mixer for the given sound card.
 *
 * @param sound_card the alsa name of (ie hw:x))
 * @return if success a valid pointer to a mixer otherwise NULL
 */
static snd_mixer_t * open_mixer(const char *sound_card) {
	snd_mixer_t *mixer_handle = NULL;
	int err;

	// open an empty mixer
	err = snd_mixer_open(&mixer_handle, 0);
	if (err < 0) {
		LOG_WARN("snd_mixer_open failed: " + String(snd_strerror(err)));
		return NULL;
	}
	////

	// attach a sound card to a mixer element
	err = snd_mixer_attach(mixer_handle, sound_card);
	if (err < 0) {
		LOG_WARN("failed to attach mixer to card " + String(sound_card) + " " +
		String(snd_strerror(err)));
		snd_mixer_close(mixer_handle);
		return NULL;
	}
	////

	// register mixer simple element class
	err = snd_mixer_selem_register(mixer_handle, NULL, NULL);
	if (err < 0) {
		LOG_WARN("snd_mixer_selem_register failed: " + String(snd_strerror(err)));
		snd_mixer_close(mixer_handle);
		return NULL;
	}
	////

	// loads mixer element
	err = snd_mixer_load(mixer_handle);
	if (err < 0) {
		LOG_WARN("snd_mixer_load failed: " + String(snd_strerror(err)));
		snd_mixer_close(mixer_handle);
	}
	////

	return mixer_handle;
}

/**
 * closes the given mixer.
 *
 * @param mixer_handle pointer to a snd_mixer_t
 * @return 0 on success otherwise -1
 */
static int close_mixer(snd_mixer_t *mixer_handle) {
	int err = snd_mixer_close(mixer_handle);
	if (err < 0) {
		LOG_WARN("failed to close mixer: " + String(snd_strerror(err)));
		return -1;
	}

	return 0;
}

/**
 * finds a mixer element.
 * @param mixer_handle pointer to a snd_mixer_t
 * @param element_name the searched snd_mixer_elem_t
 * @return if success a valid pointer to a snd_mixer_elem_t otherwise NULL
 */
static snd_mixer_elem_t * find_mixer_element(snd_mixer_t *mixer_handle, const char *element_name) {
	snd_mixer_elem_t *mixer_element;
	const char *name;

	if (mixer_handle == NULL) {
		LOG_WARN("mixer_handle is NULL!");
	}

	mixer_element = snd_mixer_first_elem(mixer_handle);
	while (mixer_element != NULL) {
		name = snd_mixer_selem_get_name(mixer_element);
		if (strcmp(name, element_name) == 0) {
			return mixer_element;
		}
		mixer_element = snd_mixer_elem_next(mixer_element);
	}

	return mixer_element;
}

/**
 * sets volume
 * @param mixer_handle a valid pointer to a snd_mixer_t
 * @param name name of the snd_mixer_elem_t
 * @param type mixer_element_type
 * @param level new level
 * @return 0 if success otherwise -1
 */
static int set_mixer_level_private(snd_mixer_t *mixer_handle, const char *name, mixer_element_type type, int level) {
	int err;
	long volume_min;
	long volume_max;
	long new_level;
	snd_mixer_elem_t *mixer_element = NULL;

	mixer_element = find_mixer_element(mixer_handle, name);
	if (mixer_element == NULL) {
		LOG_WARN("failed to find snd_mixer_elem_t: " + String(name));
		return -1;
	}

	switch (type) {
	case PLAYBACK:
		if (snd_mixer_selem_has_playback_volume(mixer_element)) {
			snd_mixer_selem_get_playback_volume_range(mixer_element, &volume_min, &volume_max);
			new_level = (((volume_max- volume_min) * level) / 100) + volume_min;

			/* unmute just in case */
			if (new_level != 0) {
				snd_mixer_selem_set_playback_switch_all (mixer_element, 1);
			}

			err = snd_mixer_selem_set_playback_volume_all(mixer_element, new_level);
			if (err < 0) {
				LOG_WARN("failed to set " + String(name) + " volume: " + String(snd_strerror(err)));
				return -1;
			}
		}
		break;
	case CAPTURE:
		if (snd_mixer_selem_has_capture_volume(mixer_element)) {
			snd_mixer_selem_get_capture_volume_range(mixer_element, &volume_min, &volume_max);
			new_level = (((volume_max- volume_min) * level) / 100) + volume_min;
			err = snd_mixer_selem_set_capture_volume_all(mixer_element, new_level);
			if (err < 0) {
				LOG_WARN("failed to set " + String(name) + " volume: " + String(snd_strerror(err)));
				return -1;
			}
		}
		break;
	}

	return 0;
}

/**
 * gets volume
 * @param mixer_handle a valid pointer to a snd_mixer_t
 * @param name name of the snd_mixer_elem_t
 * @param type mixer_element_type
 * @return if success the volume otherwise -1
 */
static int
get_mixer_level_private(snd_mixer_t *mixer_handle, const char *name, mixer_element_type type) {
	int err;
	long volume_min;
	long volume_max;
	long level;
	snd_mixer_elem_t *mixer_element = NULL;

	mixer_element = find_mixer_element(mixer_handle, name);
	if (mixer_element == NULL) {
		LOG_WARN("failed to find snd_mixer_elem_t: " + String(name));
		return -1;
	}

	switch (type) {
	case PLAYBACK:
		if (snd_mixer_selem_has_playback_volume(mixer_element)) {
			snd_mixer_selem_get_playback_volume_range(mixer_element, &volume_min, &volume_max);
			err = snd_mixer_selem_get_playback_volume(mixer_element, SND_MIXER_SCHN_MONO, &level);
			if (err < 0) {
				LOG_WARN("failed to get" + String(name) + " volume: " +String(snd_strerror(err)));
				return -1;
			}
			return (100 * (level - volume_min)) / (volume_max -volume_min);
		}
		break;
	case CAPTURE:
		if (snd_mixer_selem_has_capture_volume(mixer_element)) {
			snd_mixer_selem_get_capture_volume_range(mixer_element, &volume_min, &volume_max);
			err = snd_mixer_selem_get_capture_volume(mixer_element, SND_MIXER_SCHN_MONO, &level);
			if (err < 0) {
				LOG_WARN("failed to get" + String(name) + " volume: " +String(snd_strerror(err)));
				return -1;
			}
			return (100 * (level - volume_min)) / (volume_max -volume_min);
		}
		break;
	}

	return -1;
}

int set_mixer_level(const char *sound_card, volume_t volume_type, int level) {
	snd_mixer_t *mixer_handle;
	int err = -1;

	mixer_handle = open_mixer(sound_card);
	if (mixer_handle == NULL) {
		return -1;
	}

	switch(volume_type) {
	case MASTER_VOLUME:
		err = set_mixer_level_private(mixer_handle, "Master", PLAYBACK, level);
		break;
	case PCM_VOLUME:
		err = set_mixer_level_private(mixer_handle, "PCM", PLAYBACK, level);
		break;
	case MIC_PLAYBACK_VOLUME:
		err = set_mixer_level_private(mixer_handle, "Mic", PLAYBACK, level);
		break;
	case CAPTURE_VOLUME:
		err = set_mixer_level_private(mixer_handle, "Capture", CAPTURE, level);
		break;
	case MIC_CAPTURE_VOLUME:
		err = set_mixer_level_private(mixer_handle, "Mic", CAPTURE, level);
		break;
	default:
		LOG_WARN("unknown volume_t: " + String::fromNumber(volume_type));
		close_mixer(mixer_handle);
		return -1;
	}

	close_mixer(mixer_handle);
	return err;
}

int get_mixer_level(const char *sound_card, volume_t volume_type, int *level) {
	snd_mixer_t *mixer_handle;
	int err = -1;

	mixer_handle = open_mixer(sound_card);
	if (mixer_handle == NULL) {
		return -1;
	}

	switch(volume_type) {
	case MASTER_VOLUME:
		*level = get_mixer_level_private(mixer_handle, "Master", PLAYBACK);
		break;
	case PCM_VOLUME:
		*level = get_mixer_level_private(mixer_handle, "PCM", PLAYBACK);
		break;
	case MIC_PLAYBACK_VOLUME:
		*level = get_mixer_level_private(mixer_handle, "Mic", PLAYBACK);
		break;
	case CAPTURE_VOLUME:
		*level = get_mixer_level_private(mixer_handle, "Capture", CAPTURE);
		break;
	case MIC_CAPTURE_VOLUME:
		*level = get_mixer_level_private(mixer_handle, "Mic", CAPTURE);
		break;
	default:
		LOG_WARN("unknown volume_t: " + String::fromNumber(volume_type));
		close_mixer(mixer_handle);
		return -1;
	}

	close_mixer(mixer_handle);
	return err;
}
