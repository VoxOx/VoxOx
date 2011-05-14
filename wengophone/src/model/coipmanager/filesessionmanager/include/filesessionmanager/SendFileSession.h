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

#ifndef OWSENDFILESESSION_H
#define OWSENDFILESESSION_H

#include <coipmanager/Session.h>
#include <filesessionmanager/IFileSession.h>
#include <imwrapper/EnumIMProtocol.h>
#include <util/File.h>

class FileSessionManager;
class ISendFileSession;
class UserProfile;

/**
 * File Session.
 *
 * Interface to File transfer services.
 *
 *  - Send mode: 'start' starts sending files by creating a FileSession per
 * Contact and per file. 'stop' stops the transfer.
 *
 * @see TestFileSession for an example
 *
 * @author Philippe Bernery
 */
class SendFileSession : public Session {
public:

	/**
	 * Emitted when something happened in this SendFileSession.
	 *
	 * @param sender this class
	 * @param imContact the peer that send or receive the file
	 * @param sentFile the sent File
	 */
	Event< void (SendFileSession & sender, IFileSession::IFileSessionEvent event,
		IMContact imContact, File sentFile) > fileTransferEvent;

	/**
	 * Information about progression of a download.
	 *
	 * @param sender this class
	 * @param imContact the peer that send or receive the file
	 * @param sentFile the sent File
	 * @param percentage completion of the transfer in percentage
	 */
	Event< void (SendFileSession & sender, IMContact imContact, 
		File sentFile, int percentage) > fileTransferProgressionEvent;

	/**
	 * Creates a SendFileSession.
	 *
	 * @param fileSessionManager reference to the main FileSessionManager.
	 * @param userProfile link to UserProfile
	 */
	SendFileSession(FileSessionManager & fileSessionManager, UserProfile & userProfile);

	SendFileSession(const SendFileSession & sendFileSession);

	virtual ~SendFileSession();

	// Inherited from Session
	virtual void start();

	virtual void pause();

	virtual void resume();

	virtual void stop();
	////

	/**
	 * Adds a file to the transfer queue.
	 *
	 * Thread safe.
	 *
	 * @param file the file to add. The File object is copied.
	 */
	void addFile(const File & file);

	/**
	 * Gets the File list.
	 */
	std::vector<File> getFileList() const { return _fileVector; }

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

	/**
	 * @see Module::moduleFinishedEvent
	 */
	void moduleFinishedEventHandler(CoIpModule & sender);

	/** List of Files to send. */
	std::vector<File> _fileVector;

	/** Pointer to running IFileSession. */
	ISendFileSession * _currentFileSessionImp;

	/** Link to FileSessionManager. */
	FileSessionManager & _fileSessionManager;
};

#endif //OWSENDFILESESSION_H
