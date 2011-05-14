/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#include <sound/SoundMixerException.h>

#include <windows.h>

#include <sstream>

SoundMixerException::SoundMixerException(const std::string & soundComponent, unsigned int errorNumber) : Exception() {
	_errorNumber = errorNumber;
	_soundComponent = soundComponent + ": ";
}

SoundMixerException::~SoundMixerException() {
}

std::string SoundMixerException::what() const {
	switch (_errorNumber) {
	case MMSYSERR_NOERROR:
		return _soundComponent + "No error";

	case MMSYSERR_ERROR:
		return _soundComponent + "Unspecified error";

	case MMSYSERR_BADDEVICEID:
		return _soundComponent + "Device ID out of range";

	case MMSYSERR_NOTENABLED:
		return _soundComponent + "Driver failed enable";

	case MMSYSERR_ALLOCATED:
		return _soundComponent + "Device already allocated";

	case MMSYSERR_INVALHANDLE:
		return _soundComponent + "Device handle is invalid";

	case MMSYSERR_NODRIVER:
		return _soundComponent + "No device driver present";

	case MMSYSERR_NOMEM:
		return _soundComponent + "Memory allocation error";

	case MMSYSERR_NOTSUPPORTED:
		return _soundComponent + "Function isn't supported";

	case MMSYSERR_BADERRNUM:
		return _soundComponent + "Error value out of range";

	case MMSYSERR_INVALFLAG:
		return _soundComponent + "Invalid flag passed";

	case MMSYSERR_INVALPARAM:
		return _soundComponent + "Invalid parameter passed";

	case MMSYSERR_HANDLEBUSY:
		return _soundComponent + "Handle being used simultaneously on another thread (eg callback)";

	case MMSYSERR_INVALIDALIAS:
		return _soundComponent + "Specified alias not found";

	case MMSYSERR_BADDB:
		return _soundComponent + "Bad registry database";

	case MMSYSERR_KEYNOTFOUND:
		return _soundComponent + "Registry key not found";

	case MMSYSERR_READERROR:
		return _soundComponent + "Registry read error";

	case MMSYSERR_WRITEERROR:
		return _soundComponent + "Registry write error";

	case MMSYSERR_DELETEERROR:
		return _soundComponent + "Registry delete error";

	case MMSYSERR_VALNOTFOUND:
		return _soundComponent + "Registry value not found";

	case MMSYSERR_NODRIVERCB:
		return _soundComponent + "Driver does not call DriverCallback";

	/* Does not exist under VC6
	case MMSYSERR_MOREDATA:
		return _soundComponent + "More data to be returned";*/

	/* Compile error from VC7, case value 21 already used
	case MMSYSERR_LASTERROR:
		return _soundComponent + "Last error in range";*/

	case MIXERR_INVALLINE:
		return _soundComponent + "The audio line reference is invalid";

	case MIXERR_INVALCONTROL:
		return _soundComponent + "The control reference is invalid";

	case MIXERR_INVALVALUE:
		return _soundComponent;

	/* Compile error from VC7, case value 1026 already used
	case MIXERR_LASTERROR:
		return _soundComponent + "";*/

	default:
		std::ostringstream tmp;
		tmp << _errorNumber;
		return _soundComponent + ", error code number: " + tmp.str();
	}
}
