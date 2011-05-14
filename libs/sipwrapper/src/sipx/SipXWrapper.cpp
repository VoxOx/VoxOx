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

#include "SipXWrapper.h"

#include "SipXCallbacks.h"
#include "model/WengoPhoneLogger.h"
#include "model/account/SipAccount.h"
#include "model/sipwrapper/SipCallbacks.h"

#include <StringList.h>

SipCallbacks * SipXWrapper::_callbacks = NULL;

SipXWrapper::SipXWrapper(SipCallbacks & callbacks) {
	_callbacks = &callbacks;

	LOG_DEBUG("sipX initialization");

	SIPX_RESULT ret = sipxInitialize(&_sipxInstance,
					DEFAULT_UDP_PORT,
					DEFAULT_TCP_PORT,
					DEFAULT_RTP_START_PORT,
					DEFAULT_CONNECTIONS,
					/*DEFAULT_IDENTITY,*/
					"tanguy-krotoff",
					"0.0.0.0");
	if (ret != SIPX_RESULT_SUCCESS) {
		LOG_FATAL("cannot initialize sipX");
	}

	ret = sipxConfigEnableRport(_sipxInstance, true);
	if (ret != SIPX_RESULT_SUCCESS) {
		LOG_FATAL("cannot initialize sipX");
	}

	ret = sipxListenerAdd(_sipxInstance, SipXCallbacks::sipCallbackProc, _callbacks);
	if (ret != SIPX_RESULT_SUCCESS) {
		LOG_FATAL("cannot initialize sipX");
	}

	ret = sipxLineListenerAdd(_sipxInstance, SipXCallbacks::sipLineCallbackProc, _callbacks);
	if (ret != SIPX_RESULT_SUCCESS) {
		LOG_FATAL("cannot initialize sipX");
	}
}

SipXWrapper::~SipXWrapper() {
}

void SipXWrapper::terminate() {
}

int SipXWrapper::addVirtualLine(const std::string & displayName,
				const std::string & username,
				const std::string & identity,
				const std::string & password,
				const std::string & realm,
				const std::string & proxyServer,
				const std::string & registerServer) {

	unsigned int lineId;
	std::string lineUrl = displayName + " sip:" + identity + "@" + registerServer;
	int ret = sipxLineAdd(_sipxInstance, lineUrl.c_str(), true, &lineId);
	if (ret != SIPX_RESULT_SUCCESS) {
		return VirtualLineIdError;
	}

	ret = sipxConfigSetOutboundProxy(_sipxInstance, proxyServer.c_str());
	if (ret != SIPX_RESULT_SUCCESS) {
		return VirtualLineIdError;
	}

	ret = sipxLineAddCredential(lineId, identity.c_str(), password.c_str(), realm.c_str());
	if (ret != SIPX_RESULT_SUCCESS) {
		return VirtualLineIdError;
	}

	return lineId;
}

void SipXWrapper::removeVirtualLine(int lineId) {
	sipxLineRemove(lineId);
}

int SipXWrapper::makeCall(int lineId, const std::string & phoneNumber) {
	unsigned callId;

	int ret = sipxCallCreate(_sipxInstance, lineId, &callId);
	if (ret != SIPX_RESULT_SUCCESS) {
		return CallIdError;
	}

	LOG_DEBUG("call " + phoneNumber);
	ret = sipxCallConnect(callId, phoneNumber.c_str());
	if (ret != SIPX_RESULT_SUCCESS) {
		return CallIdError;
	}

	return callId;
}

void SipXWrapper::sendRingingNotification(int callId) {
	//FIXME Not sure about that
	//sipxCallAnswer(callId);
}

void SipXWrapper::acceptCall(int callId) {
	LOG_DEBUG("call accepted=" + String::fromNumber(callId));
	//sipxCallAccept(callId);
	sipxCallAnswer(callId);
}

void SipXWrapper::rejectCall(int callId) {
	sipxCallReject(callId);
}

void SipXWrapper::closeCall(int callId) {
	unsigned int tmp = callId;
	sipxCallDestroy(tmp);
}

void SipXWrapper::holdCall(int callId) {
	sipxCallHold(callId);
}

void SipXWrapper::resumeCall(int callId) {
	sipxCallUnhold(callId);
}

bool SipXWrapper::setCallInputAudioDevice(const std::string & deviceName) {
	int ret = sipxAudioSetCallInputDevice(_sipxInstance, deviceName.c_str());
	if (ret != SIPX_RESULT_SUCCESS) {
		return false;
	}
	return true;
}

bool SipXWrapper::setRingerOutputAudioDevice(const std::string & deviceName) {
	int ret = sipxAudioSetRingerOutputDevice(_sipxInstance, deviceName.c_str());
	if (ret != SIPX_RESULT_SUCCESS) {
		return false;
	}
	return true;
}

bool SipXWrapper::setCallOutputAudioDevice(const std::string & deviceName) {
	int ret = sipxAudioSetCallOutputDevice(_sipxInstance, deviceName.c_str());
	if (ret != SIPX_RESULT_SUCCESS) {
		return false;
	}
	return true;
}

bool SipXWrapper::enableAEC(bool enable) {
	int ret = sipxAudioEnableAEC(_sipxInstance, enable);
	if (ret != SIPX_RESULT_SUCCESS) {
		return false;
	}
	return true;
}
