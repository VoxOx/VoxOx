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

#ifndef OWENUMDEVICETYPE_H
#define OWENUMDEVICETYPE_H

#include <util/NonCopyable.h>

#include <string>

/**
 * Kind of audio mixer device to deal with.
 *
 * @author Tanguy Krotoff
 */
class EnumDeviceType : NonCopyable {
public:

	enum DeviceType {
		//MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
		DeviceTypeMasterVolume,

		//MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT
		DeviceTypeWaveOut,

		//MIXERLINE_COMPONENTTYPE_DST_WAVEIN
		DeviceTypeWaveIn,

		//MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC
		DeviceTypeCDOut,

		//MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE
		DeviceTypeMicrophoneOut,

		//MIXERLINE_COMPONENTTYPE_DST_WAVEIN + MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE
		DeviceTypeMicrophoneIn
	};

	static std::string toString(DeviceType deviceType);

	static DeviceType toDeviceType(const std::string & deviceType);
};

#endif	//OWENUMDEVICETYPE_H
