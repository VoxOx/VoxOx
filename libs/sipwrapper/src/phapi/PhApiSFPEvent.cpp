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

#include "PhApiSFPEvent.h"

Event<void (PhApiSFPWrapper & sender, int callID, std::string uri, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::inviteToTransferEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::newIncomingFileEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string uri)> PhApiSFPEvent::waitingForAnswerEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::transferCancelledEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::transferCancelledByPeerEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::sendingFileBeginEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::receivingFileBeginEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::transferFromPeerFinishedEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::transferToPeerFinishedEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::transferFromPeerFailedEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::transferToPeerFailedEvent;

Event<void (PhApiSFPWrapper & sender, int callID, int percentage)> PhApiSFPEvent::transferProgressionEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::transferPausedByPeerEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::transferPausedEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::transferResumedByPeerEvent;

Event<void (PhApiSFPWrapper & sender, int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize)> PhApiSFPEvent::transferResumedEvent;

Event<void (PhApiSFPWrapper & sender, const std::string contactID)> PhApiSFPEvent::peerNeedsUpgradeEvent;

Event<void (PhApiSFPWrapper & sender)> PhApiSFPEvent::needUpgradeEvent;
