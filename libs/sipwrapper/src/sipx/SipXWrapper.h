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

#ifndef SIPXWRAPPER_H
#define SIPXWRAPPER_H

#include "model/sipwrapper/SipWrapper.h"

#include <tapi/sipXtapi.h>
#include <tapi/sipXtapiEvents.h>

#include <string>

class SipCallbacks;

/**
 * SIP Wrapper for sipXtapi.
 *
 * http://www.sipfoundry.org/
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class SipXWrapper : public SipWrapper {
public:

	static SipXWrapper * getInstance(SipCallbacks & callbacks) {
		static SipXWrapper instance(callbacks);

		return &instance;
	}

	~SipXWrapper();

	void terminate();

	int addVirtualLine(const std::string & displayName,
					const std::string & username,
					const std::string & identity,
					const std::string & password,
					const std::string & realm,
					const std::string & proxyServer,
					const std::string & registerServer);

	void removeVirtualLine(int lineId);

	int makeCall(int lineId, const std::string & phoneNumber);

	void sendRingingNotification(int callId);

	void acceptCall(int callId);

	void rejectCall(int callId);

	void closeCall(int callId);

	void holdCall(int callId);

	void resumeCall(int callId);

	bool setCallInputAudioDevice(const std::string & deviceName);

	bool setRingerOutputAudioDevice(const std::string & deviceName);

	bool setCallOutputAudioDevice(const std::string & deviceName);

	bool enableAEC(bool enable);

private:

	SipXWrapper(SipCallbacks & callbacks);

	static SipCallbacks * _callbacks;

	/** SipX instance: generated when sipX is activated via sipxInitialize(). */
	SIPX_INST _sipxInstance;
};

#endif	//SIPXWRAPPER_H
