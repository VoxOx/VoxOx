/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#ifndef OWPHONECALL_H
#define OWPHONECALL_H

#include "SipAddress.h"

#include <sipwrapper/EnumTone.h>
#include <sipwrapper/EnumPhoneCallState.h>
#include <sipwrapper/CodecList.h>

#include <util/Event.h>
#include <util/List.h>
#include <util/NonCopyable.h>

#include <string>

#include <pixertool/pixertool.h>

class WebcamVideoFrame;
class PhoneCallState;
class IPhoneLine;
class WenboxPlugin;
class ConferenceCall;

/**
 * Class that holds informations about a current phone call.
 *
 * A phone call is associated with a PhoneLine.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class PhoneCall : NonCopyable {
public:

	/**
	 * The state of the PhoneCall has changed.
	 *
	 * @param sender this class
	 * @param status new status
	 */
	Event<void (PhoneCall & sender, EnumPhoneCallState::PhoneCallState status)> stateChangedEvent;

	/**
	 * A video frame has been received from the network.
	 *
	 * @param sender this class
	 * @param remoteVideoFrame remote video frame
	 * @param localVideoFrame local video frame from the webcam
	 */
	Event<void (PhoneCall & sender, piximage * remoteVideoFrame, piximage * localVideoFrame)> videoFrameReceivedEvent;

	/**
	 * Creates a new PhoneCall given a PhoneLine.
	 *
	 * @param phoneLine PhoneLine associated with this PhoneCall
	 * @param sipAddress caller/callee/peer SIP address
	 */
	PhoneCall(IPhoneLine & phoneLine, const SipAddress & sipAddress);

	~PhoneCall();

	/**
	 * @see IPhoneLine::acceptCall()
	 */
	void accept();

	/**
	 * @see IPhoneLine::resumeCall()
	 */
	void resume();

	/**
	 * @see IPhoneLine::holdCall()
	 */
	void hold();

	/**
	 * @see IPhoneLine::closeCall()
	 */
	void close();

	/**
	 * @see IPhoneLine::blindTransfer()
	 */
	void blindTransfer(const std::string & sipAddress);

	/**
	 * @see IPhoneLine::playSoundFile()
	 */
	void playSoundFile(const std::string & soundFile);

	/**
	 * @see IPhoneLine::getAudioCodecUsed()
	 */
	CodecList::AudioCodec getAudioCodecUsed();

	/**
	 * @see IPhoneLine::getVideoCodecUsed()
	 */
	CodecList::VideoCodec getVideoCodecUsed();

	/**
	 * Video frame received.
	 *
	 * @param remoteVideoFrame received
	 * @param localWebcam local webcam
	 */
	void videoFrameReceived(piximage * remoteVideoFrame, piximage * localVideoFrame);

	/**
	 * Changes the state of this PhoneCall.
	 *
	 * @param state state code corresponding to the new PhoneCall state
	 */
	void setState(EnumPhoneCallState::PhoneCallState state);

	/**
	 * Gets the current state of this PhoneCall.
	 *
	 * @return state of this PhoneCall
	 */
	EnumPhoneCallState::PhoneCallState getState() const;

	/**
	 * Sets the call id associated with this PhoneCall.
	 *
	 * Should only be called by PhoneLine.
	 *
	 * @param callId call id of this PhoneCall
	 */
	void setCallId(int callId) {
		_callId = callId;
	}

	/**
	 * Gets the call id of this PhoneCall.
	 *
	 * @return call id of this PhoneCall
	 */
	int getCallId() const {
		return _callId;
	}

	/**
	 * Gets the caller/callee/peer SIP address associated with this PhoneCall.
	 *
	 * @return caller/callee/peer SIP address
	 */
	const SipAddress & getPeerSipAddress() const {
		return _sipAddress;
	}

	/**
	 * Gets the PhoneLine associated with this PhoneCall.
	 *
	 * @return phone line
	 */
	IPhoneLine & getPhoneLine() const {
		return _phoneLine;
	}

	/**
	 * Gets the Wenbox.
	 *
	 * Used by PhoneCallState (code factorization).
	 *
	 * @return Wenbox
	 */
	WenboxPlugin & getWenboxPlugin() const;

	/**
	 * Gets the duration of the call in seconds
	 *
	 * @return duration of the call or -1 if not call duration
	 */
	int getDuration() const {
		return _duration;
	}

	/**
	 * Gets the conference associated with this call.
	 *
	 * @return ConferenceCall or NULL if this call is not associated with a conference
	 */
	ConferenceCall * getConferenceCall() const {
		return _conferenceCall;
	}

	/**
	 * Gets the phone call type (audio or video).
	 *
	 * @return true if this call is a video call; false otherwise
	 */
	bool isVideoEnabled() const {
		return _videoEnabled;
	}

	/**
	 * @return True if the call is encrypted
	 */
	bool isCallEncrypted() const;

	/**
	 * Sets the conference associated with this call.
	 *
	 * Internal method, used only by ConferenceCall and PhoneLine.
	 *
	 * @param conferenceCall conference
	 */
	void setConferenceCall(ConferenceCall * conferenceCall) {
		_conferenceCall = conferenceCall;
	}

private:

	void applyState(EnumPhoneCallState::PhoneCallState state);

	/** PhoneLine associated with this PhoneCall. */
	IPhoneLine & _phoneLine;

	/** Call id of this PhoneCall. */
	int _callId;

	/** Caller/callee/peer SIP address. */
	SipAddress _sipAddress;

	/** Current state of this PhoneCall. */
	PhoneCallState * _state;

	/** Defines the vector of PhoneCallState. */
	typedef List < PhoneCallState * > PhoneCallStates;

	/** List of PhoneCallState. */
	PhoneCallStates _phoneCallStateList;

	/** Call duration in seconds. */
	int _duration;

	/** If the PhoneCall should be held. */
	bool _holdRequest;

	/** If the PhoneCall should be resumed. */
	bool _resumeRequest;

	/** Conference associated with this call. */
	ConferenceCall * _conferenceCall;

	/** If this PhoneCall has been rejected as an incoming call. */
	bool _callRejected;

	/** Computes the PhoneCall duration. */
	int _timeStart;

	/** If this call is a video call or an audio call. */
	bool _videoEnabled;
};

#endif	//OWPHONECALL_H
