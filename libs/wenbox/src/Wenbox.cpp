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

#include <wenbox/Wenbox.h>

#define LOGGER_COMPONENT "Wenbox"
#include <util/Logger.h>
#include <util/Path.h>
#include <util/String.h>
#include <util/StringList.h>

#include <cutil/global.h>

#include <shlibloader/SharedLibLoader.h>

IWenbox * getInstance() {
	return NULL;
}

Wenbox::Wenbox() {
	//Plugin name hardcoded, there is only one Wenbox at a time
	String wenboxDll = "yealinkwenbox";

	_open = false;
	_wenboxPrivate = NULL;
#ifndef OS_MACOSX
	typedef IWenbox * (*GetInstanceFunction)();

	std::string dllPath = Path::getApplicationDirPath() + wenboxDll;

	GetInstanceFunction getInstance = (GetInstanceFunction) SharedLibLoader::resolve(dllPath, "getInstance");

	if (getInstance) {
		LOG_DEBUG("Wenbox dll loaded");
		_wenboxPrivate = getInstance();
		_wenboxPrivate->setKeyPressedCallback(Wenbox::keyPressedCallback, this);
	} else {
		LOG_DEBUG("Wenbox dll not loaded");
	}
#endif
}

Wenbox::~Wenbox() {
	close();
	delete _wenboxPrivate;
}

bool Wenbox::open() {
	if (_wenboxPrivate) {
		if (_open) {
			//Avoid to open the Wenbox a second time
			return _open;
		}

		LOG_DEBUG("open device");
		_open = _wenboxPrivate->open();
		return _open;
	}
	return false;
}

bool Wenbox::close() {
	if (_wenboxPrivate && _open) {
		_open = false;
		LOG_DEBUG("close device");
		return _wenboxPrivate->close();
	}
	return false;
}

std::string Wenbox::getDeviceName() {
	if (_wenboxPrivate && _open) {
		return _wenboxPrivate->getDeviceName();
	}
	return String::null;
}

std::list<std::string> Wenbox::getAudioDeviceNameList() const {
	StringList strList;

	if (_wenboxPrivate && _open) {
		return _wenboxPrivate->getAudioDeviceNameList();
	}

	return strList;
}

bool Wenbox::setDefaultMode(Mode mode) {
	if (_wenboxPrivate && _open) {
		return _wenboxPrivate->setDefaultMode(mode);
	}
	return false;
}

bool Wenbox::switchMode(Mode mode) {
	if (_wenboxPrivate && _open) {
		return _wenboxPrivate->switchMode(mode);
	}
	return false;
}

bool Wenbox::setLCDMessage(const std::string & message) {
	if (_wenboxPrivate && _open) {
		LOG_DEBUG("LCD message changed");
		return _wenboxPrivate->setLCDMessage(message);
	}
	return false;
}

bool Wenbox::setRingingTone(int tone) {
	if (_wenboxPrivate && _open) {
		LOG_DEBUG("ringing tone changed");
		return _wenboxPrivate->setRingingTone(tone);
	}
	return false;
}

bool Wenbox::setState(PhoneCallState state, const std::string & phoneNumber) {
	if (_wenboxPrivate && _open) {
		return _wenboxPrivate->setState(state, phoneNumber);
	}
	return false;
}

void Wenbox::keyPressedCallback(Key key, void * param) {
	Wenbox * wenbox = (Wenbox *) param;
	if (wenbox) {
		wenbox->keyPressedEvent(*wenbox, key);
	} else {
		LOG_FATAL("wenbox instance is null, check void * param for IWenbox::keyPressedCallback()");
	}
}
