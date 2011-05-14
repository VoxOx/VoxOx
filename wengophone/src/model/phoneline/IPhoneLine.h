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

#ifndef OWIPHONELINE_H
#define OWIPHONELINE_H

#include <util/Event.h>
#include <util/Interface.h>
#include <util/List.h>

#include "EnumMakeCallError.h"

#include <sipwrapper/EnumTone.h>
#include <sipwrapper/EnumPhoneCallState.h>
#include <sipwrapper/EnumPhoneLineState.h>
#include <sipwrapper/CodecList.h>

#include <string>

class PhoneCall;
class ConferenceCall;
class PhoneLineState;
class SipAccount;
class SipWrapper;
class SipAddress;
class SipPresenceState;
class UserProfile;

/**
 * Interface representing a phone line.
 *
 * A phone line creates/deletes/contains a list of PhoneCall.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class IPhoneLine : Interface {
public:

	/**
	 * @see SipWrapper::phoneLineStateChangedEvent
	 */
	Event<void (IPhoneLine & sender, EnumPhoneLineState::PhoneLineState state)> stateChangedEvent;

	/**
	 * A PhoneCall has been created.
	 *
	 * @param sender this class
	 * @param phoneCall phoneCall created
	 */
	Event<void (IPhoneLine & sender, PhoneCall & phoneCall)> phoneCallCreatedEvent;

	/**
	 * A PhoneCall has been closed.
	 *
	 * @param sender this class
	 * @param phoneCall phoneCall closed
	 */
	Event<void (IPhoneLine & sender, PhoneCall & phoneCall)> phoneCallClosedEvent;

	virtual ~IPhoneLine() {
	}

	virtual std::string getMySipAddress() const = 0;

	virtual void redial() = 0;//VOXOX CHANGE BY ROLANDO 04-07-09

	/**
	 * @see SipWrapper::makeCall()
	 */
	virtual EnumMakeCallError::MakeCallError makeCall(const std::string & phoneNumber, ConferenceCall * conferenceCall = NULL) = 0;

	/**
	 * @see SipWrapper::acceptCall()
	 */
	virtual void acceptCall(int callId) = 0;

	/**
	 * @see SipWrapper::rejectCall()
	 */
	virtual void rejectCall(int callId) = 0;

	/**
	 * @see SipWrapper::closeCall()
	 */
	virtual void closeCall(int callId) = 0;

	/**
	 * @see SipWrapper::holdCall()
	 */
	virtual void holdCall(int callId) = 0;

	/**
	 * @see SipWrapper::resumeCall()
	 */
	virtual void resumeCall(int callId) = 0;

	/**
	 * @see SipWrapper::blindTransfer()
	 */
	virtual void blindTransfer(int callId, const std::string & sipAddress) = 0;

	/**
	 * @see SipWrapper::playDtmf()
	 */
	virtual void playDtmf(int callId, char dtmf) = 0;

	/**
	 * @see SipWrapper::playSoundFile()
	 */
	virtual void playSoundFile(int callId, const std::string & soundFile) = 0;

	/**
	 * @see SipWrapper::getAudioCodecUsed()
	 */
	virtual CodecList::AudioCodec getAudioCodecUsed(int callId) = 0;

	/**
	 * @see SipWrapper::getVideoCodecUsed()
	 */
	virtual CodecList::VideoCodec getVideoCodecUsed(int callId) = 0;

	/**
	 * Gets the SIP account associated with this PhoneLine.
	 *
	 * @return SIP account associated with this PhoneLine or NULL
	 */
	virtual SipAccount & getSipAccount() = 0;

	/**
	 * Establishes a connection with the SIP server.
	 */
	virtual bool connect() = 0;

	/**
	 * Finishes the connection with the SIP server.
	 *
	 * @param force if true, disconnect without doing any transactions
	 */
	virtual void disconnect(bool force = false) = 0;

	/**
	 * @return true if connected, otherwise false
	 */
	virtual bool isConnected() const = 0;

	/**
	 * Sets the state of a PhoneCall.
	 *
	 * @param callId PhoneCall id
	 * @param state the state (e.g event)
	 * @param sipAddress from who we get a phone call
	 */
	virtual void setPhoneCallState(int callId, EnumPhoneCallState::PhoneCallState state, const SipAddress & sipAddress) = 0;

	/**
	 * Gets the SIP implementation wrapper.
	 *
	 * @return the SIP implementation wrapper
	 */
	virtual SipWrapper & getSipWrapper() const = 0;

	/**
	 * Gets the PhoneLine id.
	 *
	 * @return the id associated with this PhoneLine
	 */
	virtual int getLineId() const = 0;

	/**
	 * Changes the state of this PhoneLine.
	 *
	 * @param state state code corresponding to the new PhoneLine state
	 */
	virtual void setState(EnumPhoneLineState::PhoneLineState state) = 0;

	/**
	 * Gets the current state of this PhoneLine.
	 *
	 * @return state of this PhoneLine
	 */
	virtual const PhoneLineState & getState() const = 0;

	/**
	 * Gets the UserProfile instance.
	 *
	 * @return UserProfile instance
	 */
	virtual UserProfile & getUserProfile() const = 0;

	/**
	 * Gets the PhoneCall associated with a given callId.
	 *
	 * @return PhoneCall given its callId or NULL if no PhoneCall corresponds to the callId
	 */
	virtual PhoneCall * getPhoneCall(int callId) /*const*/ = 0;

	/**
	 * Gets the active PhoneCall associated with this PhoneLine.
	 *
	 * @return active PhoneCall or NULL
	 */
	virtual PhoneCall * getActivePhoneCall() const = 0;

	typedef List<PhoneCall *> PhoneCallList;

	/**
	 * Gets the list of PhoneCall associated with this PhoneLine.
	 *
	 * @return the list of PhoneCall
	 */
	virtual PhoneCallList getPhoneCallList() const = 0;

	/**
	 * Set flip video parameter.
	 */
	virtual void flipVideoImage(bool flip) = 0;

	/**
	 * @see SipWrapper::setCallsEncryption
	 */
	virtual void setCallsEncryption(bool enable) = 0;

	/**
	 * @see SipWrapper::areCallsEncrypted
	 */
	virtual bool isCallEncrypted(int callId) = 0;

	/**
	 * @return True if there's pending call(s)
	 */
	virtual bool hasPendingCalls() const = 0;

	/**
	 * Close active call(s).
	 */
	virtual void closePendingCalls() = 0;

};

#endif	//OWIPHONELINE_H
