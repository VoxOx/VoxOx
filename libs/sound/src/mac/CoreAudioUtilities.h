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

#ifndef COREAUDIOUTILITIES_H
#define COREAUDIOUTILITIES_H

#include <util/NonCopyable.h>

#include <CoreAudio/CoreAudio.h>

#include <vector>
#include <string>
#include <map>

class CoreAudioUtilities : NonCopyable {
public:

	/**
	 * Gets available audio devices.
	 *
	 * @param isInput true if input device must be returned, 
	 $ false if output devices must be returned
	 * @return the device list
	 */
	static std::vector<AudioDeviceID> audioDeviceList(bool isInput);

	/**
	 * Gets an audio device name.
	 *
	 * @param id the AudioDeviceID to get the name of
	 * @param isInput true if we want the name of the input part of the device,
	 * false if we want the output part.
	 * @return the name of the device or String::null if there is no device
	 * in desired channel (input or output)
	 */
	static std::string audioDeviceName(AudioDeviceID id, bool isInput);

	/**
	 * Gets list of available data sources available on a device.
	 *
	 * @param id the device to check
	 * @param isInput true if we want to get input devices, false to get
	 * output devices
	 * @return a vector of UInt32 containing ids of data sources
	 */
	static std::vector<UInt32> dataSourceList(AudioDeviceID id, bool isInput);

	/**
	 * Gets the name of a data source.
	 *
	 * @param id the device id
 	 * @param isInput true if we want the name of the input part of the device,
	 * false if we want the output part.
	 * @param dsId id of the data source
	 * @return the name of the data source. String::null if no data source found.
	 */
	static std::string dataSourceName(AudioDeviceID id, bool isInput, UInt32 dsId);

private:

	/**
	 * Checks if a device has channel in input/output.
	 *
	 * @param true if we want to know if there is input, false 
	 * if we want to know if there is output.
	 * @return true if the device has at least one channel.
	 */
	static bool hasChannel(AudioDeviceID id, bool isInput);
	 
};

#endif //COREAUDIOUTILITIES_H
