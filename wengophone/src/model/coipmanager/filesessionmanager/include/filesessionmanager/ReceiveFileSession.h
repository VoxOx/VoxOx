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

#ifndef OWRECEIVEFILESESSION_H
#define OWRECEIVEFILESESSION_H

#include <coipmanager/Session.h>
#include <filesessionmanager/IFileSession.h>
#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMContact.h>
#include <util/File.h>

class FileSessionManager;
class IFileSession;
class IReceiveFileSession;
class UserProfile;

/**
 * File Session.
 *
 * Interface to File transfer services.
 *
 *  - Receive mode: 'start' accepts the transfer and 'stop' refuses it.
 *
 * @see TestFileSession for an example
 *
 * @author Philippe Bernery
 */
class ReceiveFileSession : public Session {
public:

	/**
	 * Emitted when something happened in this ReceiveFileSession.
	 *
	 * @param sender this class
	 * @param imContact the peer that send or receive the file
	 * @param sentFile the sent File
	 */
	Event< void (ReceiveFileSession & sender, IFileSession::IFileSessionEvent event,
		IMContact imContact, File sentFile) > fileTransferEvent;

	/**
	 * Information about progression of a download.
	 *
	 * @param sender this class
	 * @param imContact the peer that send or receive the file
	 * @param sentFile the sent File
	 * @param percentage completion of the transfer in percentage
	 */
	Event< void (ReceiveFileSession & sender, IMContact imContact, 
		File sentFile, int percentage) > fileTransferProgressionEvent;

	/**
	 * Creates a ReceiveFileSession.
	 *
	 * @param userProfile link to UserProfile
	 * @param fileSessionImp the IFileSession to use for receiving the file
	 */
	ReceiveFileSession(UserProfile & userProfile, IReceiveFileSession * fileSessionImp);

	ReceiveFileSession(const ReceiveFileSession & receiveFileSession);

	virtual ~ReceiveFileSession();

	// Inherited from Session
	virtual void start();

	virtual void pause();

	virtual void resume();

	virtual void stop();
	////

	/**
	 * Gets the IMContact who sends the File.
	 */
	IMContact getIMContact() const;

	/**
	 * Sets the path for saving the File.
	 */
	void setFilePath(const std::string & path);

	/**
	 * Gets the path to the File to be saved.
	 */
	std::string getFilePath() const;

	/**
	 * Gets the name of the file to receive.
	 */
	std::string getFileName() const;

	/**
	 * Gets the size of the file to receive.
	 */
	unsigned getFileSize() const;

	/**
	 * Gets the last event.
	 */
	IFileSession::IFileSessionEvent getLastEvent() const { return _lastEvent; };

private:

	/**
	 * @see IFileSession::fileTransferEvent
	 */
	void fileTransferEventHandler(IFileSession & sender, 
		IFileSession::IFileSessionEvent event, IMContact imContact, File sentFile);

	/**
	 * @see IFileSession::fileTransferProgressionEvent
	 */
	void fileTransferProgressionEventHandler(IFileSession & sender, 
		IMContact imContact, File sentFile, int percentage);

	// Inherited from Session
	virtual void addContact(const std::string & /*contactId*/) { }

	virtual void removeContact(const std::string & /*contactId*/) { }
	////

	/**
	 * @see Module::moduleFinishedEvent
	 */
	void moduleFinishedEventHandler(CoIpModule & sender);

	/** Last occured event. */
	IFileSession::IFileSessionEvent _lastEvent;

	/** Pointer to running IFileSession. */
	IReceiveFileSession * _currentFileSessionImp;
};

#endif //OWRECEIVEFILESESSION_H
