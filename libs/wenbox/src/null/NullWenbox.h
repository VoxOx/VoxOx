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

#ifndef OWNULLWENBOX_H
#define OWNULLWENBOX_H

#include <wenbox/IWenbox.h>

/**
 * Wenbox stub (null) backend.
 *
 * @author Tanguy Krotoff
 */
class NullWenbox : public IWenbox {
public:

	NullWenbox();

	~NullWenbox();

	void setKeyPressedCallback(KeyPressedCallback keyPressedCallback, void * param);

	bool open();

	bool close();

	std::string getDeviceName();

	std::list<std::string> getAudioDeviceNameList() const;

	bool setDefaultMode(Mode mode);

	bool switchMode(Mode mode);

	bool setLCDMessage(const std::string & message);

	bool setRingingTone(int tone);

	bool setState(PhoneCallState state, const std::string & phoneNumber);
};

#endif	//OWNULLWENBOX_H
