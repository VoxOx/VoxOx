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

#ifndef OWIFILESESSION_H
#define OWIFILESESSION_H

#include <coipmanager/CoIpModule.h>
#include <imwrapper/IMContact.h>
#include <util/File.h>

class Account;

/**
 * Must be inherited to be used.
 *
 * @author Philippe Bernery
 */
class IFileSession : public CoIpModule {
public:

	enum IFileSessionEvent {

		/** Emitted when a ask for transferring a file has been sent. */
		IFileSessionEventInviteToTransfer,

		/** Emitted when the invite to transfer has been received by the peer but
		 * he has not yet accept the tranfer. */
		IFileSessionEventWaitingForAnswer,

		/** Emitted when a file transfer is finished. */
		IFileSessionEventFileTransferFinished,

		/** Emitted when a file transfer failed. */
		IFileSessionEventFileTransferFailed,

		/** Emitted when a File transfer is paused by the user. */
		IFileSessionEventFileTransferPaused,

		/** Emitted when a File transfer is paused by peer. */
		IFileSessionEventFileTransferPausedByPeer,
		
		/** Emitted when a File transfer is resumed by the user. */
		IFileSessionEventFileTransferResumed,

		/** Emitted when a File transfer is resumed by peer. */
		IFileSessionEventFileTransferResumedByPeer,
		
		/** Emitted when a File transfer has been cancelled by the user. */
		IFileSessionEventFileTransferCancelled,

		/** Emitted when a File transfer has been cancelled by peer. */
		IFileSessionEventFileTransferCancelledByPeer,

		/** Emitted when the transfer of a File starts. */
		IFileSessionEventFileTransferBegan,

	};

	/**
	 * Emitted when something happened in this IFileSession.
	 *
	 * @param sender this class
	 * @param imContact the peer that send or receive the file
	 * @param sentFile the sent File
	 */
	Event< void (IFileSession & sender, IFileSessionEvent event,
		IMContact imContact, File sentFile) > fileTransferEvent;

	/**
	 * Information about progression of a download.
	 *
	 * @param sender this class
	 * @param imContact the peer that send or receive the file
	 * @param sentFile the sent File
	 * @param percentage completion of the transfer in percentage
	 */
	Event< void (IFileSession & sender, IMContact imContact,
		File sentFile, int percentage) > fileTransferProgressionEvent;

	IFileSession();

	IFileSession(const IFileSession & iFileSession);

	virtual IFileSession * clone() const = 0;

	virtual ~IFileSession();

	/**
	 * Sets the Account to use.
	 */
	void setAccount(const Account * account);

protected:

	/** The Account to use for File transfer. */
	Account * _account;

};

#endif //OWIFILESESSION_H
