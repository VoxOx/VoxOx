/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
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

#ifndef OWMACVOLUMECONTROL_H
#define OWMACVOLUMECONTROL_H

#include <sound/AudioDevice.h>
#include <sound/IVolumeControl.h>

/**
 * MacOS X implementation of VolumeControl.
 *
 * @author Philippe Bernery
 */
class MacVolumeControl : public IVolumeControl {
public:

	MacVolumeControl(const AudioDevice & audioDevice);

	bool setLevel(unsigned level);

	int getLevel();

	bool setMute(bool mute);

	/**
	 * @return true if one channel of the device is muted.
	 */
	bool isMuted();

	bool isSettable() const;

private:

	/**
	 * Gets the settable/gettable channel range.
	 * If no range is found, -1 is returned as the first element of the pair.
	 */
	std::pair<int, int> getVolumeSettableChannelRange() const;

	/**
	 * Tests if a channel is settable.
	 *
	 * @param channel channel to test
	 * @return true if the given channel is settable.
	 */
	bool isChannelVolumeSettable(unsigned long channel) const;

	AudioDevice _audioDevice;
};

#endif //OWMACVOLUMECONTROL_H
