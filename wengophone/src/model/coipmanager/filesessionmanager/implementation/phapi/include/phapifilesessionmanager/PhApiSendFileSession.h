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

#ifndef OWPHAPISENDFILESESSION_H
#define OWPHAPISENDFILESESSION_H

#include <filesessionmanager/ISendFileSession.h>

class UserProfile;
class PhApiSFPWrapper;

/**
 * TODO
 *
 * @author Nicolas Couturier
 */
class PhApiSendFileSession : public ISendFileSession {

public:

	PhApiSendFileSession(UserProfile & userProfile);

	PhApiSendFileSession(const PhApiSendFileSession & phApiSendFileSession);

	virtual PhApiSendFileSession * clone() const;

	virtual ~PhApiSendFileSession();

	void start();

	void pause();

	void resume();

	void stop();

private :

	void inviteToTransferEventHandler(PhApiSFPWrapper & sender, int callID, std::string uri, std::string fileName, std::string fileType, size_t fileSize);

	void waitingForAnswerEventHandler(PhApiSFPWrapper & sender, int callID, std::string uri);

	void transferCancelledEventHandler(PhApiSFPWrapper & sender, int callID, std::string fileName, std::string fileType, size_t fileSize);

	void transferCancelledByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize);

	void sendingFileBeginEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize);

	void transferFromPeerFinishedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize);

	void transferToPeerFinishedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize);

	void transferFromPeerFailedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize);

	void transferToPeerFailedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize);

	void transferProgressionEventHandler(PhApiSFPWrapper & sender, int callID, int percentage);

	void transferPausedByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize);

	void transferPausedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize);

	void transferResumedByPeerEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize);

	void transferResumedEventHandler(PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, size_t fileSize);
	
	std::string extractContactIDFromURI(std::string uri) const;

	/**
	* The file that is being sent / received currently
	*/
	std::vector<File>::iterator _currentFile;

	/**
	* The contact that is curently sending / receiving the file
	*/
	IMContactSet::iterator _currentContact;

	/**
	* The call id of the file current file transfer
	*/
	int _currentCallID;

	bool _initialized;

	UserProfile & _userProfile;

};

#endif	//OWPHAPISENDFILESESSION_H
