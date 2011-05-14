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

#ifndef OWPHAPISFPEVENT_H
#define OWPHAPISFPEVENT_H

#include "PhApiSFPWrapper.h"

#include <util/Event.h>
#include <util/String.h>

/**
 * Class implementing the SFP events
 *
 * @author Nicolas Couturier
 */
class PhApiSFPEvent {

public :
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string uri, std::string fileName, std::string fileType, int fileSize)> inviteToTransferEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> newIncomingFileEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string uri)> waitingForAnswerEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string fileName, std::string fileType, int fileSize)> transferCancelledEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> transferCancelledByPeerEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> sendingFileBeginEvent;

	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> receivingFileBeginEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> transferFromPeerFinishedEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> transferToPeerFinishedEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> transferFromPeerFailedEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> transferToPeerFailedEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, int percentage)> transferProgressionEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> transferPausedByPeerEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> transferPausedEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> transferResumedByPeerEvent;
	
	static Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> transferResumedEvent;

	static Event<void (PhApiSFPWrapper & sender, const std::string contactID)> peerNeedsUpgradeEvent;

	static Event<void (PhApiSFPWrapper & sender)> needUpgradeEvent;
};

#endif	//OWPHAPISFPEVENT_H
