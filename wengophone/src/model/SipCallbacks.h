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

#ifndef SIPCALLBACKS_H
#define SIPCALLBACKS_H

#include <sipwrapper/EnumPhoneCallState.h>
#include <sipwrapper/EnumPhoneLineState.h>

#include <util/NonCopyable.h>
#include <util/Trackable.h>

#include <string>

#include <pixertool/pixertool.h>

class UserProfile;
class SipWrapper;
class WebcamVideoFrame;

/**
 * Callbacks for SIP stacks.
 *
 * Handles events/callbacks from SipWrapper.
 *
 * phApi states: [INCOMING], [ACCEPTING], [OUTGOING], [ESTABLISHED], [ONHOLD], [CLOSED]
 * phApi call events (in the right order):
 * phDIALING, phRINGING, phNOANSWER, phCALLBUSY, phCALLREDIRECTED,
 * phCALLOK, phCALLHELD, phCALLRESUMED, phHOLDOK, phRESUMEOK,
 * phINCALL, phCALLCLOSED, phCALLERROR, phDTMF, phXFERPROGRESS,
 * phXFEROK, phXFERFAIL, phXFERREQ, phCALLREPLACED
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class SipCallbacks : NonCopyable, public Trackable {
public:

	SipCallbacks(SipWrapper & sipWrapper, UserProfile & userProfile);

	virtual ~SipCallbacks();

private:

	void phoneCallStateChangedEventHandler(SipWrapper & sender, int callId,
		EnumPhoneCallState::PhoneCallState state, const std::string & from);

	void phoneLineStateChangedEventHandler(SipWrapper & sender, int lineId,
		EnumPhoneLineState::PhoneLineState state);

	void videoFrameReceivedEventHandler(SipWrapper & sender, int callId,
		piximage* remoteVideoFrame, piximage* localVideoFrame);

	SipWrapper & _sipWrapper;

	/** WengoPhone instance in order to access the phone lines and the phone calls. */
	UserProfile & _userProfile;
};

#endif	//SIPCALLBACKS_H
