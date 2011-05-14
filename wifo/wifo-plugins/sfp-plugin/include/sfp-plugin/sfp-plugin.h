/*
* WengoPhone, a voice over Internet phone
* Copyright (C) 2004-2005  Wengo
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

#ifndef __SFP_PLUGIN_H__
#define __SFP_PLUGIN_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


	/* ----- CALLBACKS ----- */

	/**
	 * An invitation to transfer a file has been sent to peer
	 */
	typedef int (*inviteToTransferCb_T)(int cid, char * uri, char * short_filename, char * file_type, char * file_size);

	/**
	 * An invitation to transfer a file has been received from a peer
	 */
	typedef int (*newIncomingFileCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The peer received the invitation and the user is waiting for him to accept / refuse
	 */
	typedef int (*waitingForAnswerCb_T)(int cid, char * uri);

	/**
	 * The transfer has been cancelled by the user
	 */
	typedef int (*transferCancelledCb_T)(int cid, char * short_filename, char * file_type, char * file_size);

	/**
	 * The transfer has been cancelled by peer
	 */
	typedef int (*transferCancelledByPeerCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The transfer is starting
	 */
	typedef int (*sendingFileBeginCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The transfer is starting
	 */
	typedef int (*receivingFileBeginCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The peer closed the SIP call
	 */
	typedef int (*transferClosedByPeerCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The user closed the SIP call
	 */
	typedef int (*transferClosedCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * Transfer complete and OK
	 */
	typedef int (*transferFromPeerFinishedCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * Transfer complete and OK
	 */
	typedef int (*transferToPeerFinishedCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * A SIP failure occured
	 */
	typedef int (*transferFromPeerFailedCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * A SIP failure occured
	 */
	typedef int (*transferToPeerFailedCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The transfer has been closed before being complete (socket closed, transfer incomplete)
	 */
	typedef int (*transferFromPeerStoppedCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The transfer has been closed before being complete (socket closed, transfer incomplete)
	 */
	typedef int (*transferToPeerStoppedCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * Progression of the file transfer
	 */
	typedef int (*transferProgressionCb_T)(int cid, int percentage);

	/**
	 * The peer paused the transfer
	 */
	typedef int (*transferPausedByPeerCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The user paused the transfer
	 */
	typedef int (*transferPausedCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The peer resumed the transfer
	 */
	typedef int (*transferResumedByPeerCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The user resumed the transfer
	 */
	typedef int (*transferResumedCb_T)(int cid, char * username, char * short_filename, char * file_type, char * file_size);

	/**
	 * The peer need to upgrade his version
	 */
	typedef int (*peerNeedUpgradeCb_T)(void);

	/**
	 * The user need to upgrade his version
	 */
	typedef int (*needUpgradeCb_T)(void);


	extern inviteToTransferCb_T inviteToTransfer;
	extern newIncomingFileCb_T newIncomingFile;
	extern waitingForAnswerCb_T waitingForAnswer;
	extern transferCancelledCb_T transferCancelled;
	extern transferCancelledByPeerCb_T transferCancelledByPeer;
	extern sendingFileBeginCb_T sendingFileBegin;
	extern receivingFileBeginCb_T receivingFileBegin;
	extern transferClosedByPeerCb_T transferClosedByPeer;
	extern transferClosedCb_T transferClosed;
	extern transferFromPeerFinishedCb_T transferFromPeerFinished;
	extern transferToPeerFinishedCb_T transferToPeerFinished;
	extern transferFromPeerFailedCb_T transferFromPeerFailed;
	extern transferToPeerFailedCb_T transferToPeerFailed;
	extern transferFromPeerStoppedCb_T transferFromPeerStopped;
	extern transferToPeerStoppedCb_T transferToPeerStopped;
	extern transferProgressionCb_T transferProgression;
	extern transferPausedByPeerCb_T transferPausedByPeer;
	extern transferPausedCb_T transferPaused;
	extern transferResumedByPeerCb_T transferResumedByPeer;
	extern transferResumedCb_T transferResumed;
	extern peerNeedUpgradeCb_T peerNeedUpgrade;
	extern needUpgradeCb_T needUpgrade;


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __SFP_PLUGIN_H__ */
