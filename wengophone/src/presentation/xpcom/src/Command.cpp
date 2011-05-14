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

#include <Command.h>
#include <Listener.h>

#include "XPCOMWengoPhone.h"
#include "XPCOMPhoneLine.h"
#include "XPCOMPhoneCall.h"
#include "XPCOMFactory.h"
#include "XPCOMIMHandler.h"
#include "XPCOMSms.h"
#include "ListenerList.h"

#include <control/CWengoPhone.h>

#include <model/WengoPhone.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <imwrapper/IMWrapperFactory.h>
#include <sipwrapper/SipWrapperFactory.h>
#include <WengoPhoneBuildId.h>

#include <sound/VolumeControl.h>

#if defined(CC_MSVC)
	#include <memorydump/MemoryDump.h>
#endif

#ifdef PHAPIWRAPPER
	#include <PhApiFactory.h>
#elif defined(SIPXWRAPPER)
	#include <SipXFactory.h>
	#include <NullIMFactory.h>
#elif defined (MULTIIMWRAPPER)
	#include <PhApiFactory.h>
	#include <multiim/MultiIMFactory.h>
	#include <GaimIMFactory.h>
#else
	#include <NullSipFactory.h>
	#include <NullIMFactory.h>
#endif

Command::Command(const std::string & configFilesPath, const std::string & configPath) {

#ifdef CC_MSVC
	new MemoryDump("VoxOxNG", String::fromUnsignedLongLong(WengoPhoneBuildId::getSvnRevision().c_str()));
#endif

	//Graphical interface implementation
	PFactory::setFactory(new XPCOMFactory());

	//SIP implementation
	SipWrapperFactory * sipFactory = NULL;

	//IM implementation
	IMWrapperFactory * imFactory = NULL;

#ifdef SIPXWRAPPER
	sipFactory = new SipXFactory();
	imFactory = new NullIMFactory();
#elif defined(PHAPIWRAPPER)
	PhApiFactory * phApiFactory = new PhApiFactory();
	sipFactory = phApiFactory;
	imFactory = phApiFactory;
#elif defined(MULTIIMWRAPPER)
	PhApiFactory * phApiFactory = new PhApiFactory();
	GaimIMFactory * gaimIMFactory = new GaimIMFactory();
	sipFactory = phApiFactory;
	imFactory = new MultiIMFactory(*phApiFactory, *gaimIMFactory);
#else
	sipFactory = new NullSipFactory();
	imFactory = new NullIMFactory();
#endif
	SipWrapperFactory::setFactory(sipFactory);
	IMWrapperFactory::setFactory(imFactory);

	/*WengoPhone::CONFIG_FILES_PATH = configFilesPath;

	//Codec plugin path (phspeexplugin and phamrplugin)
	ConfigManager::getInstance().getCurrentConfig().set(Config::CODEC_PLUGIN_PATH,
		WengoPhone::getConfigFilesPath() + "../extensions/{debaffee-a972-4d8a-b426-8029170f2a89}/libraries/");*/

	_wengoPhone = new WengoPhone();
	_cWengoPhone = new CWengoPhone(*_wengoPhone);
}

Command::~Command() {
	delete _wengoPhone;
	delete _cWengoPhone;
}

void Command::start() {
	//Starts the model component thread
	_cWengoPhone->start();
}

void Command::setHttpProxySettings(const std::string & hostname, unsigned port,
				const std::string & login, const std::string & password) {

	/*WengoPhone::HTTP_PROXY_HOSTNAME = hostname;
	WengoPhone::HTTP_PROXY_PORT = port;*/
}

void Command::terminate() {
	_cWengoPhone->terminate();
}

void Command::addWengoAccount(const std::string & login, const std::string & password, bool autoLogin) {
	//_cWengoPhone->addWengoAccount(login, password, autoLogin);
}

void Command::addListener(Listener * listener) {
	ListenerList & listenerList = ListenerList::getInstance();
	listenerList += listener;
}

bool Command::removeListener(Listener * listener) {
	ListenerList & listenerList = ListenerList::getInstance();
	return listenerList -= listener;
}

void Command::removeAllListeners() {
	ListenerList & listenerList = ListenerList::getInstance();
	listenerList.clear();
}

int Command::makeCall(const std::string & phoneNumber, int lineId) {
	PhoneLineList & phoneLineList = PhoneLineList::getInstance();
	for (unsigned i = 0; i < phoneLineList.size(); i++) {
		if (i == (lineId - 1)) {
			XPCOMPhoneLine * phoneLine = phoneLineList[i];
			return phoneLine->makeCall(phoneNumber);
		}
	}

	return CallIdError;
}

void Command::hangUp(int callId) {
	PhoneCallMap & phoneCallMap = PhoneCallMap::getInstance();
	XPCOMPhoneCall * phoneCall = phoneCallMap[callId];
	if (phoneCall) {
		phoneCall->hangUp();
	}
}

bool Command::canHangUp(int callId) const {
	PhoneCallMap & phoneCallMap = PhoneCallMap::getInstance();
	XPCOMPhoneCall * phoneCall = phoneCallMap[callId];
	if (phoneCall) {
		return phoneCall->canHangUp();
	}

	return false;
}

void Command::pickUp(int callId) {
	PhoneCallMap & phoneCallMap = PhoneCallMap::getInstance();
	XPCOMPhoneCall * phoneCall = phoneCallMap[callId];
	if (phoneCall) {
		phoneCall->pickUp();
	}
}

bool Command::canPickUp(int callId) const {
	PhoneCallMap & phoneCallMap = PhoneCallMap::getInstance();
	XPCOMPhoneCall * phoneCall = phoneCallMap[callId];
	if (phoneCall) {
		return phoneCall->canPickUp();
	}

	return false;
}

void Command::subscribeToPresenceOf(const std::string & contactId) {
	XPCOMIMHandler & imHandler = XPCOMIMHandler::getInstance();
	imHandler.subscribeToPresenceOf(contactId);
}

void Command::publishMyPresence(EnumPresenceState::PresenceState state, const std::string & note) {
	XPCOMIMHandler & imHandler = XPCOMIMHandler::getInstance();
	imHandler.publishMyPresence(state, note);
}

int Command::sendChatMessage(const std::string & sipAddress, const std::string & message) {
	XPCOMIMHandler & imHandler = XPCOMIMHandler::getInstance();
	return imHandler.sendChatMessage(sipAddress, message);
}

void Command::setInputVolume(int volume) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	VolumeControl volumeControl(config.getAudioInputDeviceName(), VolumeControl::DeviceTypeInput);
	volumeControl.setLevel(volume);
}

void Command::setOutputVolume(int volume) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	VolumeControl volumeControl(config.getAudioOutputDeviceName(), VolumeControl::DeviceTypeOutput);
	volumeControl.setLevel(volume);
}

int Command::getInputVolume() const {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	try {
		VolumeControl volumeControl(config.getAudioInputDeviceName(), VolumeControl::DeviceTypeInput);
		return volumeControl.getLevel();
	} catch (Exception & e) {
		return -1;
	}
}

int Command::getOutputVolume() const {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	try {
		VolumeControl volumeControl(config.getAudioOutputDeviceName(), VolumeControl::DeviceTypeOutput);
		return volumeControl.getLevel();
	} catch (Exception & e) {
		return -1;
	}
}

int Command::sendSMS(const std::string & phoneNumber, const std::string & message) {
	if (XPCOMSms::sms) {
		return XPCOMSms::sms->sendSMS(phoneNumber, message);
	}
	return -1;
}

void Command::playTone(int callId, EnumTone::Tone tone) {
	PhoneCallMap & phoneCallMap = PhoneCallMap::getInstance();
	XPCOMPhoneCall * phoneCall = phoneCallMap[callId];
	if (phoneCall) {
		phoneCall->playTone(tone);
	}
}
