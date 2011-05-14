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

#ifndef OWWENBOX_H
#define OWWENBOX_H

#include <wenbox/IWenbox.h>

#include <util/Event.h>

/**
 * @see IWenbox
 * @author Tanguy Krotoff
 */
class Wenbox : public IWenbox {
public:

	/**
	 * A key has been pressed from the USB phone device.
	 *
	 * @param sender this class
	 * @param key key pressed by the user
	 * @see IWenbox::setKeyPressedCallback()
	 */
	Event<void (IWenbox & sender, Key key)> keyPressedEvent;

	/**
	 * Does nothing.
	 */
	void setKeyPressedCallback(KeyPressedCallback /*keyPressedCallback*/, void * /*param*/) { }

	Wenbox();

	~Wenbox();

	bool open();

	bool isOpen() const {
		return _open;
	}

	bool close();

	std::string getDeviceName();

	std::list<std::string> getAudioDeviceNameList() const;

	bool setDefaultMode(Mode mode);

	bool switchMode(Mode mode);

	bool setLCDMessage(const std::string & message);

	bool setRingingTone(int tone);

	bool setState(PhoneCallState state, const std::string & phoneNumber);

private:

	/**
	 * Callback from IWenbox.
	 *
	 * @see IWenbox::setKeyPressedCallback()
	 */
	static void keyPressedCallback(Key key, void * param);

	IWenbox * _wenboxPrivate;

	bool _open;
};

#endif	//OWWENBOX_H
