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

#ifndef OWWENBOXPLUGIN_H
#define OWWENBOXPLUGIN_H

#include <wenbox/Wenbox.h>

#include <util/StringList.h>
#include <util/Trackable.h>
#include <util/NonCopyable.h>

class PhoneCall;
class Settings;
class UserProfile;

/**
 * Handles the Wenbox.
 *
 * Named WenboxPlugin rather than Wenbox since a class named Wenbox already exists.
 * Wenbox is a USB phone device loaded at runtime via a .dll
 *
 * @see IWenbox
 * @ingroup model
 * @author Tanguy Krotoff
 */
class WenboxPlugin : NonCopyable, public Trackable {
public:

	/**
	 * Phone number buffer has been updated event.
	 *
	 * Example:
	 * buffer before: 0147
	 * buffer now: 014708
	 *
	 * @param sender this class
	 * @param phoneNumberBuffer phone number buffer
	 */
	Event<void (WenboxPlugin & sender, const std::string & phoneNumberBuffer)> phoneNumberBufferUpdatedEvent;

	WenboxPlugin(UserProfile & userProfile);

	~WenboxPlugin();

	/**
	 * @see IWenbox::setState()
	 */
	void setState(Wenbox::PhoneCallState state, const std::string & phoneNumber = "");

private:

	void openWenbox();

	void closeWenbox();

	void wenboxConfigChangedEventHandler(const std::string & key);

	void keyPressedEventHandler(IWenbox & sender, IWenbox::Key key);

	/** Code factorization. */
	PhoneCall * getActivePhoneCall() const;

	StringList getWenboxAudioDeviceId(bool outputAudioDeviceId) const;

	StringList getWenboxOutputAudioDeviceId() const;

	StringList getWenboxInputAudioDeviceId() const;

	void switchCurrentAudioDeviceToWenbox();

	//Wenbox * _wenbox;

	UserProfile & _userProfile;

	std::string _phoneNumberBuffer;
};

#endif	//OWWENBOXPLUGIN_H
