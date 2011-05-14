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

#include "NullWenbox.h"

#include <util/StringList.h>

IWenbox * getInstance() {
	NullWenbox * wenbox = new NullWenbox();
	return wenbox;
}

NullWenbox::NullWenbox() {
}

NullWenbox::~NullWenbox() {
}

void NullWenbox::setKeyPressedCallback(KeyPressedCallback keyPressedCallback, void * param) {
}

bool NullWenbox::open() {
	return false;
}

bool NullWenbox::close() {
	return false;
}

std::string NullWenbox::getDeviceName() {
	return "NullWenbox";
}

std::list<std::string> NullWenbox::getAudioDeviceNameList() const {
	StringList strList;
	strList += "NullWenbox";
	return strList;
}

bool NullWenbox::setDefaultMode(Mode mode) {
	return false;
}

bool NullWenbox::switchMode(Mode mode) {
	return false;
}

bool NullWenbox::setLCDMessage(const std::string & message) {
	return false;
}

bool NullWenbox::setRingingTone(int tone) {
	return false;
}

bool NullWenbox::setState(PhoneCallState state, const std::string & phoneNumber) {
	return false;
}
