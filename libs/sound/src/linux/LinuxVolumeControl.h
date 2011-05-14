/*
 * WengoPhone, a voice over Internet phone
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

#ifndef LINUXVOLUMECONTROL_H
#define LINUXVOLUMECONTROL_H

#include <sound/AudioDevice.h>
#include <sound/IVolumeControl.h>

#include <string>

/**
 * Gets and change the volume of a Linux audio device.
 *
 * @author Mathieu Stute
 */
class LinuxVolumeControl : public IVolumeControl {
public:

	LinuxVolumeControl(const AudioDevice & audioDevice);

	bool setLevel(unsigned level);

	int getLevel();

	bool setMute(bool mute);

	bool isMuted();

	bool isSettable() const;

private:

	AudioDevice _audioDevice;

	std::string _strDeviceType;
};

#endif	//LINUXVOLUMECONTROL_H
