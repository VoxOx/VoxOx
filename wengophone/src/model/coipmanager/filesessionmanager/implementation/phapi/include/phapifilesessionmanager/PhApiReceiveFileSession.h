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

#ifndef OWPHAPIRECEIVEFILESESSION_H
#define OWPHAPIRECEIVEFILESESSION_H

#include <filesessionmanager/IReceiveFileSession.h>

#include <PhApiSFPWrapper.h>

/**
 * TODO
 *
 * @author Nicolas Couturier
 */
class PhApiReceiveFileSession : public IReceiveFileSession {

public:

	PhApiReceiveFileSession();

	PhApiReceiveFileSession(const PhApiReceiveFileSession & phApiReceiveFileSession);

	virtual PhApiReceiveFileSession * clone() const;

	PhApiReceiveFileSession(int callID, std::string contactID, std::string fileName, unsigned int fileSize);

	virtual ~PhApiReceiveFileSession();

	void start();

	void pause();

	void resume();

	void stop();

private :

	void transferCancelledEventHandler(PhApiSFPWrapper & sender, int callID, std::string fileName, std::string fileType, int fileSize);

	void transferCancelledByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize);

	void sendingFileBeginEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize);

	void transferFromPeerFinishedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize);

	void transferToPeerFinishedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize);

	void transferFromPeerFailedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize);

	void transferToPeerFailedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize);

	void transferProgressionEventHandler(PhApiSFPWrapper & sender, int callID, int percentage);

	void transferPausedByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize);

	void transferPausedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize);

	void transferResumedByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize);

	void transferResumedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize);

	int _currentCallID;

};

#endif	//OWPHAPIRECEIVEFILESESSION_H
