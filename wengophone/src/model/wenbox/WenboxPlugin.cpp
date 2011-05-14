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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "WenboxPlugin.h"

#include "EnumWenboxStatus.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/phonecall/PhoneCall.h>
#include <model/phoneline/IPhoneLine.h>
#include <model/profile/UserProfile.h>

#include <sound/AudioDeviceManager.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

using namespace std;

static Wenbox * _wenbox = 0;

WenboxPlugin::WenboxPlugin(UserProfile & userProfile)
	: _userProfile(userProfile) {

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	if (!_wenbox) {
		_wenbox = new Wenbox();
	}
	EnumWenboxStatus::WenboxStatus wenboxStatus = EnumWenboxStatus::toWenboxStatus(config.getWenboxEnable());
	if (_wenbox->open()) {
		if (wenboxStatus == EnumWenboxStatus::WenboxStatusNotConnected) {
			wenboxStatus = EnumWenboxStatus::WenboxStatusEnable;
		}
	} else {
		wenboxStatus = EnumWenboxStatus::WenboxStatusNotConnected;
	}

	config.valueChangedEvent += boost::bind(&WenboxPlugin::wenboxConfigChangedEventHandler, this, _1);

	config.set(Config::WENBOX_ENABLE_KEY, EnumWenboxStatus::toString(wenboxStatus));
}

WenboxPlugin::~WenboxPlugin() {
	//FIXME hack: initializes only once the Wenbox
	//OWSAFE_DELETE(_wenbox);
}

void WenboxPlugin::openWenbox() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	EnumWenboxStatus::WenboxStatus wenboxStatus = EnumWenboxStatus::toWenboxStatus(config.getWenboxEnable());

	if (_wenbox->open()) {

		if (wenboxStatus == EnumWenboxStatus::WenboxStatusEnable) {
			//Disable Half-duplex mode
			config.set(Config::AUDIO_HALFDUPLEX_KEY, false);
			//Enable AEC
			config.set(Config::AUDIO_AEC_KEY, true);

			switchCurrentAudioDeviceToWenbox();
			_wenbox->setDefaultMode(Wenbox::ModeUSB);
			_wenbox->switchMode(Wenbox::ModeUSB);
			_wenbox->keyPressedEvent += boost::bind(&WenboxPlugin::keyPressedEventHandler, this, _1, _2);
		} else {
			_wenbox->close();
		}
	}
}

void WenboxPlugin::closeWenbox() {
	if (_wenbox->isOpen()) {
		_wenbox->close();
	}
}

void WenboxPlugin::wenboxConfigChangedEventHandler(const std::string & key) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if (key == Config::WENBOX_ENABLE_KEY) {
		if (EnumWenboxStatus::toWenboxStatus(config.getWenboxEnable()) == EnumWenboxStatus::WenboxStatusEnable) {
			openWenbox();
		} else {
			closeWenbox();
		}
	}
}

void WenboxPlugin::keyPressedEventHandler(IWenbox & sender, IWenbox::Key key) {
	PhoneCall * phoneCall = getActivePhoneCall();

	switch (key) {
	case IWenbox::KeyPickUp:
		if (phoneCall) {
			phoneCall->accept();
		}
		break;

	case IWenbox::KeyHangUp:
		_phoneNumberBuffer = "";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		if (phoneCall) {
			phoneCall->close();
		}
		break;

	case Wenbox::Key0:
		_phoneNumberBuffer += "0";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	case Wenbox::Key1:
		_phoneNumberBuffer += "1";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	case Wenbox::Key2:
		_phoneNumberBuffer += "2";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	case Wenbox::Key3:
		_phoneNumberBuffer += "3";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	case Wenbox::Key4:
		_phoneNumberBuffer += "4";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	case Wenbox::Key5:
		_phoneNumberBuffer += "5";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	case Wenbox::Key6:
		_phoneNumberBuffer += "6";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	case Wenbox::Key7:
		_phoneNumberBuffer += "7";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	case Wenbox::Key8:
		_phoneNumberBuffer += "8";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	case Wenbox::Key9:
		_phoneNumberBuffer += "9";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	case Wenbox::KeyStar:
		_phoneNumberBuffer += "*";
		phoneNumberBufferUpdatedEvent(*this, _phoneNumberBuffer);
		break;

	default:
		LOG_FATAL("unknown key pressed=" + String::fromNumber(key));
	}
}

PhoneCall * WenboxPlugin::getActivePhoneCall() const {
	IPhoneLine * phoneLine = _userProfile.getActivePhoneLine();
	PhoneCall * phoneCall = NULL;
	if (phoneLine) {
		phoneCall = phoneLine->getActivePhoneCall();
	}
	return phoneCall;
}

void WenboxPlugin::setState(Wenbox::PhoneCallState state, const std::string & phoneNumber) {
	_wenbox->setState(state, phoneNumber);
}

StringList WenboxPlugin::getWenboxAudioDeviceId(bool outputAudioDeviceId) const {
	/*
	 * First looks in the Wenbox audio device list, then
	 * it looks into the audio device list and try to find a matching
	 * string pattern.
	 */

	StringList wenboxAudioDeviceList = _wenbox->getAudioDeviceNameList();
	for (unsigned i = 0; i < wenboxAudioDeviceList.size(); i++) {
		string wenboxAudioDeviceName = wenboxAudioDeviceList[i];

		std::list<AudioDevice> audioDeviceList;
		if (outputAudioDeviceId) {
			audioDeviceList = AudioDeviceManager::getInstance().getOutputDeviceList();
		} else {
			audioDeviceList = AudioDeviceManager::getInstance().getInputDeviceList();
		}
		for (std::list<AudioDevice>::const_iterator it = audioDeviceList.begin();
			it != audioDeviceList.end(); it++) {
			StringList audioDevice = (*it).getData();

			if (String(audioDevice[0]).contains(wenboxAudioDeviceName)) {
				//We found the real name of the Wenbox audio device
				LOG_DEBUG("wenbox audio device name=" + audioDevice[0]);
				return audioDevice;
			}
		}
	}

	//We didn't find the real name of the Wenbox audio device
	LOG_DEBUG("Wenbox audio device not found");
	//Empty string
	StringList tmp;
	return tmp;
}

StringList WenboxPlugin::getWenboxOutputAudioDeviceId() const {
	return getWenboxAudioDeviceId(true);
}

StringList WenboxPlugin::getWenboxInputAudioDeviceId() const {
	return getWenboxAudioDeviceId(false);
}

void WenboxPlugin::switchCurrentAudioDeviceToWenbox() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	//Looks for the Wenbox audio device from the list of devices from the OS
	StringList wenboxAudioDeviceId = getWenboxOutputAudioDeviceId();
	if (!wenboxAudioDeviceId.empty()) {
		//Changes audio settings
		config.set(Config::WENBOX_AUDIO_OUTPUT_DEVICEID_KEY, wenboxAudioDeviceId);
		config.set(Config::WENBOX_AUDIO_RINGER_DEVICEID_KEY, wenboxAudioDeviceId);
	}

	wenboxAudioDeviceId = getWenboxInputAudioDeviceId();
	if (!wenboxAudioDeviceId.empty()) {
		//Changes audio settings
		config.set(Config::WENBOX_AUDIO_INPUT_DEVICEID_KEY, wenboxAudioDeviceId);
	}
}
