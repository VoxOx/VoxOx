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
 * along with *(PhApiSFPWrapper::getInstance()) program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "PhApiSFPCallbacks.h"
#include "PhApiSFPEvent.h"
#include "PhApiSFPWrapper.h"

#include <owpl_plugin.h>
#include <phapi-util/util.h>
#include <util/String.h>
#include <cutil/global.h>

extern "C" {

	static void inviteToTransferHandler(int cid, char * uri, char * short_filename, char * file_type, char * file_size){
		if(strfilled(uri) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::inviteToTransfer(cid, std::string((const char *) uri), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void newIncomingFileHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::newIncomingFile(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void waitingForAnswerHandler(int cid, char * uri){
		if(strfilled(uri)) {
			PhApiSFPCallbacks::waitingForAnswer(cid, std::string((const char *) uri));
		}
	}

	static void transferCancelledHandler(int cid, char * short_filename, char * file_type, char * file_size){
		if(strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferCancelled(cid, std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferCancelledByPeerHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferCancelledByPeer(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void sendingFileBeginHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::sendingFileBegin(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void receivingFileBeginHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::receivingFileBegin(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferClosedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	}

	static void transferClosedByPeerHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	}

	static void transferFromPeerFinishedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferFromPeerFinished(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferToPeerFinishedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferToPeerFinished(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferFromPeerFailedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferFromPeerFailed(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferToPeerFailedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferToPeerFailed(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferFromPeerStoppedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferFromPeerFailed(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferToPeerStoppedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferToPeerFailed(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferProgressionHandler(int cid, int percentage){
		PhApiSFPCallbacks::transferProgression(cid, percentage);
	}

	static void transferPausedByPeerHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferPausedByPeer(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferPausedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferPaused(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferResumedByPeerHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferResumedByPeer(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void transferResumedHandler(int cid, char * username, char * short_filename, char * file_type, char * file_size){
		if(strfilled(username) && strfilled(short_filename) && strfilled(file_size)) {
			PhApiSFPCallbacks::transferResumed(cid, std::string((const char *) username), std::string((const char *) short_filename), std::string((const char *) file_type), String(file_size).toInteger());
		}
	}

	static void peerNeedsUpgradeHandler(const char * username) {
		if(strfilled(username)) {
			PhApiSFPCallbacks::peerNeedsUpgrade(std::string(username));
		}
	}
	
	static void needUpgradeHandler() {
		PhApiSFPCallbacks::needUpgrade();
	}
}


void PhApiSFPCallbacks::inviteToTransfer(int callID, std::string uri, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::inviteToTransferEvent(PhApiSFPWrapper::getInstance(), callID, uri, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::newIncomingFile(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
#ifdef OS_WINDOWS
	PhApiSFPEvent::newIncomingFileEvent(PhApiSFPWrapper::getInstance(), callID, contactID, PhApiSFPCallbacks::replaceOddCharacters(fileName), fileType, fileSize);
#else
	PhApiSFPEvent::newIncomingFileEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
#endif
}

void PhApiSFPCallbacks::waitingForAnswer(int callID, std::string uri){
	PhApiSFPEvent::waitingForAnswerEvent(PhApiSFPWrapper::getInstance(), callID, uri);
}

void PhApiSFPCallbacks::transferCancelled(int callID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::transferCancelledEvent(PhApiSFPWrapper::getInstance(), callID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::transferCancelledByPeer(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::transferCancelledByPeerEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::sendingFileBegin(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::sendingFileBeginEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::receivingFileBegin(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::receivingFileBeginEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::transferFromPeerFinished(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::transferFromPeerFinishedEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::transferToPeerFinished(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::transferToPeerFinishedEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::transferFromPeerFailed(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::transferFromPeerFailedEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::transferToPeerFailed(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::transferToPeerFailedEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::transferProgression(int callID, int percentage){
	PhApiSFPEvent::transferProgressionEvent(PhApiSFPWrapper::getInstance(), callID, percentage);
}

void PhApiSFPCallbacks::transferPausedByPeer(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::transferPausedByPeerEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::transferPaused(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::transferPausedEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::transferResumedByPeer(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::transferResumedByPeerEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::transferResumed(int callID, std::string contactID, std::string fileName, std::string fileType, int fileSize){
	PhApiSFPEvent::transferResumedEvent(PhApiSFPWrapper::getInstance(), callID, contactID, fileName, fileType, fileSize);
}

void PhApiSFPCallbacks::peerNeedsUpgrade(const std::string contactID) {
	PhApiSFPEvent::peerNeedsUpgradeEvent(PhApiSFPWrapper::getInstance(), contactID);
}

void PhApiSFPCallbacks::needUpgrade() {
	PhApiSFPEvent::needUpgradeEvent(PhApiSFPWrapper::getInstance());
}

void PhApiSFPCallbacks::setCallbacks() {
	owplPluginSetCallback("SFPPlugin", "inviteToTransfer", (owplPS_CommandProc)inviteToTransferHandler);
	owplPluginSetCallback("SFPPlugin", "newIncomingFile", (owplPS_CommandProc)newIncomingFileHandler);
	owplPluginSetCallback("SFPPlugin", "waitingForAnswer", (owplPS_CommandProc)waitingForAnswerHandler);
	owplPluginSetCallback("SFPPlugin", "transferCancelled", (owplPS_CommandProc)transferCancelledHandler);
	owplPluginSetCallback("SFPPlugin", "transferCancelledByPeer", (owplPS_CommandProc)transferCancelledByPeerHandler);
	owplPluginSetCallback("SFPPlugin", "sendingFileBegin", (owplPS_CommandProc)sendingFileBeginHandler);
	owplPluginSetCallback("SFPPlugin", "receivingFileBegin", (owplPS_CommandProc)receivingFileBeginHandler);
	owplPluginSetCallback("SFPPlugin", "transferClosedByPeer", (owplPS_CommandProc)transferClosedByPeerHandler);
	owplPluginSetCallback("SFPPlugin", "transferClosed", (owplPS_CommandProc)transferClosedHandler);
	owplPluginSetCallback("SFPPlugin", "transferFromPeerFinished", (owplPS_CommandProc)transferFromPeerFinishedHandler);
	owplPluginSetCallback("SFPPlugin", "transferToPeerFinished", (owplPS_CommandProc)transferToPeerFinishedHandler);
	owplPluginSetCallback("SFPPlugin", "transferFromPeerFailed", (owplPS_CommandProc)transferFromPeerFailedHandler);
	owplPluginSetCallback("SFPPlugin", "transferToPeerFailed", (owplPS_CommandProc)transferToPeerFailedHandler);
	owplPluginSetCallback("SFPPlugin", "transferFromPeerStopped", (owplPS_CommandProc)transferFromPeerStoppedHandler);
	owplPluginSetCallback("SFPPlugin", "transferToPeerStopped", (owplPS_CommandProc)transferToPeerStoppedHandler);
	owplPluginSetCallback("SFPPlugin", "transferProgression", (owplPS_CommandProc)transferProgressionHandler);
	owplPluginSetCallback("SFPPlugin", "transferPausedByPeer", (owplPS_CommandProc)transferPausedByPeerHandler);
	owplPluginSetCallback("SFPPlugin", "transferPaused", (owplPS_CommandProc)transferPausedHandler);
	owplPluginSetCallback("SFPPlugin", "transferResumedByPeer", (owplPS_CommandProc)transferResumedByPeerHandler);
	owplPluginSetCallback("SFPPlugin", "transferResumed", (owplPS_CommandProc)transferResumedHandler);
	owplPluginSetCallback("SFPPlugin", "peerNeedUpgrade", (owplPS_CommandProc)peerNeedsUpgradeHandler);
	owplPluginSetCallback("SFPPlugin", "needUpgrade", (owplPS_CommandProc)needUpgradeHandler);
}

std::string PhApiSFPCallbacks::replaceOddCharacters(std::string str) {
	String newString(str);

	newString.replace("\\", "_", true);
	newString.replace("/", "_", true);
	newString.replace(":", "_", true);
	newString.replace("*", "_", true);
	newString.replace("?", "_", true);
	newString.replace("\"", "_", true);
	newString.replace("<", "_", true);
	newString.replace(">", "_", true);
	newString.replace("|", "_", true);

	return newString;
}
