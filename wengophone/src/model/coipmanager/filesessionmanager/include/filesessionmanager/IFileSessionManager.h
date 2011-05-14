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

#ifndef OWIFILESESSIONMANAGER_H
#define OWIFILESESSIONMANAGER_H

#include <imwrapper/EnumIMProtocol.h>
#include <util/Event.h>
#include <util/File.h>
#include <util/Trackable.h>

class IReceiveFileSession;
class ISendFileSession;
class UserProfile;

/**
 * File Session Manager.
 *
 * Used to manage classes related to File Transfer
 *
 * @author Philippe Bernery
 */
class IFileSessionManager : public Trackable {
public:

	/**
	 * Emitted when a new IReceiveFileSession is created.
	 *
	 * @param sender this class
	 * @param fileSession pointer to the newly created ISendFileSession.
	 * Here the ISendFileSession is a pointer because the received of the event is
	 * responsible of deleting of this object.
	 */
	Event< void (IFileSessionManager & sender, IReceiveFileSession * iReceiveFileSession)> 
		newIReceiveFileSessionCreatedEvent;

	Event <void (IFileSessionManager & sender)> needUpgradeEvent;

	Event <void (IFileSessionManager & sender, const std::string contactID)> peerNeedsUpgradeEvent;

	IFileSessionManager(UserProfile & userProfile);

	virtual ~IFileSessionManager();

	/**
	 * @return true if the protocol is supported by this FileSessionManager.
	 */
	virtual bool isProtocolSupported(EnumIMProtocol::IMProtocol protocol) const = 0;

	/**
	 * Creates an ISendFileSession in FileSessionModeSend mode.
	 */
	virtual ISendFileSession * createSendFileSession() = 0;

protected:

	UserProfile & _userProfile;

};

#endif //OWIFILESESSIONMANAGER_H
