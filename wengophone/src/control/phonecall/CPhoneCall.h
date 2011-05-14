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

#ifndef OWCPHONECALL_H
#define OWCPHONECALL_H

#include <control/Control.h>

#include <sipwrapper/EnumPhoneCallState.h>
#include <sipwrapper/CodecList.h>

#include <pixertool/pixertool.h>

#include <string>

class Presentation;
class CWengoPhone;
class PPhoneCall;
class PhoneCall;
class WengoPhone;

/**
 * Control layer for a phone call.
 *
 * @see PhoneCall
 * @ingroup control
 * @author Tanguy Krotoff
 */
class CPhoneCall : public Control {
public:

	CPhoneCall(PhoneCall & phoneCall, CWengoPhone & cWengoPhone);

	~CPhoneCall();

	Presentation * getPresentation() const;

	CWengoPhone & getCWengoPhone() const;

	/**
	 * FIXME this breaks the control layer.
	 * Used inside QtContactCallListWidget in order to create a ConferenceCall.
	 */
	PhoneCall & getPhoneCall() const {
		return _phoneCall;
	}

	/**
	 * @see PhoneCall::getState()
	 */
	EnumPhoneCallState::PhoneCallState getState() const;

	/**
	 * @see PhoneCall::close()
	 * @see PhoneCall::reject()
	 */
	void hangUp();

	/**
	 * @see PhoneCall::accept()
	 */
	void accept();

	/**
	 * @see PhoneCall::mute()
	 */
	void mute();

	/**
	 * @see PhoneCall::hold()
	 */
	void hold();

	/**
	 * @see PhoneCall::resume()
	 */
	void resume();

	/**
	 * @see PhoneCall::blindTransfer()
	 */
	void blindTransfer(const std::string & phoneNumber);

	/**
	 * @see PhoneCall::getAudioCodecUsed()
	 */
	CodecList::AudioCodec getAudioCodecUsed();

	/**
	 * @see PhoneCall::getVideoCodecUsed()
	 */
	CodecList::VideoCodec getVideoCodecUsed();

	/**
	 * Gets the peer SIP address (phone number) of the current call.
	 *
	 * @return SIP address
	 */
	std::string getPeerSipAddress() const;

	/**
	 * Gets the peer username part of the SIP address.
	 *
	 * @return peer username
	 */
	std::string getPeerUserName() const;

	/**
	 * Gets the peer display name part of the SIP address.
	 *
	 * @return peer display name
	 */
	std::string getPeerDisplayName() const;

	/**
	 * @see PhoneCall::getDuration()
	 */
	int getDuration() const;

	/**
	 * @see PhoneCall::isCallEncrypted
	 */
	bool isCallEncrypted() const;

private:

	void unbindAndClose();

	void initPresentationThreadSafe();

	void stateChangedEventHandler(PhoneCall & sender, EnumPhoneCallState::PhoneCallState state);

	void videoFrameReceivedEventHandler(PhoneCall & sender, piximage * remoteVideoFrame, piximage * localVideoFrame);

	void stateChangedEventHandlerThreadSafe(EnumPhoneCallState::PhoneCallState state);

	void videoFrameReceivedEventHandlerThreadSafe(piximage * remoteVideoFrame, piximage * localVideoFrame);

	void hangUpThreadSafe();

	void acceptThreadSafe();

	void holdThreadSafe();

	void resumeThreadSafe();

	void blindTransferThreadSafe(std::string phoneNumber);

	PhoneCall & _phoneCall;

	PPhoneCall * _pPhoneCall;

	CWengoPhone & _cWengoPhone;
};

#endif	//OWCPHONECALL_H
