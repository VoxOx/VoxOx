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

#ifndef OWFILESESSIONMANAGER_H
#define OWFILESESSIONMANAGER_H

#include <filesessionmanager/ReceiveFileSession.h>

#include <util/Event.h>

#include <vector>
#include <queue>

class Account;
class IFileSessionManager;
class IReceiveFileSession;
class ISendFileSession;
class SendFileSession;
class UserProfile;

/**
 * File Session Manager.
 *
 * Used to manage classes related to File Transfer
 *
 * @author Philippe Bernery
 */
class FileSessionManager {
	friend class SendFileSession;
public:

	/**
	 * Emitted when a new FileSession is created.
	 * Can be emitted after a call to createFileSession or 
	 * because subsystem created it (surely because someone
	 * is trying to send us a file).
	 *
	 * @param sender this class
	 * @param fileSession pointer to the newly created FileSession.
	 * Here the FileSession is a pointer because the received of the event is
	 * responsible of the deleting of this object.
	 */
	Event< void (FileSessionManager & sender, ReceiveFileSession fileSession)> newReceiveFileSessionCreatedEvent;

	Event <void (FileSessionManager & sender)> needUpgradeEvent;

	Event <void (FileSessionManager & sender, const std::string contactID)> peerNeedsUpgradeEvent;

	FileSessionManager(UserProfile & userProfile);

	virtual ~FileSessionManager();

	/**
	 * Creates a SendFileSession. Caller is responsible for deleting the
	 * Session.
	 */
	SendFileSession * createSendFileSession();

	// HACK : Unique file transfer hack
	/**
	 * Queues a SendFileSession and starts it if it is the only session.
	 *
	 * @param	the session to queue
	 */
	void queueSession(SendFileSession * session);

private:

	/**
	 * @see IFileSessionManager::newIReceiveFileSessionCreatedEvent
	 */
	void newIReceiveFileSessionCreatedEventHandler(IFileSessionManager & sender, 
		IReceiveFileSession * iReceiveFileSession);

	void needUpgradeEventHandler(IFileSessionManager & sender);

	void peerNeedsUpgradeEventHandler(IFileSessionManager & sender, const std::string contactID);

	// HACK : Unique file transfer hack
	/**
	 * Handler used to start the next session in queue
	 *
	 * @param	the SendFileSession that finished, as a CoIpModule
	 */
	void moduleFinishedEventHandler(CoIpModule & sender);

	/**
	 * Create a ISendFileSession suitable for the given contact.
	 *
	 * @return a pointer to the new FileSession.
	 * Must be destroyed by the caller.
	 */
	ISendFileSession * createFileSessionForContact(const std::string & contactId);

	/** Vector of available FileSessionManager. */
	std::vector<IFileSessionManager *> _fileSessionManagerVector;

	/** Link to UserProfile. */
	UserProfile & _userProfile;

	// HACK : Unique file transfer hack
	/** Queue to ensure that only one session is active at a time */
	std::queue<SendFileSession *> _sendSessions;

};

#endif //OWFILESESSIONMANAGER_H
