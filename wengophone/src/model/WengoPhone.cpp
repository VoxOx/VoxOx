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
#include "WengoPhone.h"

#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileHandler.h>

#include "commandserver/CommandServer.h"
#include "config/Config.h"
#include "config/ConfigManager.h"
#include "config/ConfigManagerFileStorage.h"
#include "dtmf/DtmfThemeManager.h"
#include "network/NetworkObserver.h"
#include "network/NetworkProxyDiscovery.h"

#include "WengoPhoneBuildId.h"

#include <cutil/global.h>
#include <http/HttpRequest.h>
#include <thread/Timer.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <webcam/WebcamDriver.h>

#include <sstream>

WengoPhone::WengoPhone() {

	_enableSerialization = true;
	//VOXOXCHANGE CJC ADD VOXOX TO USER AGENT
	//set HttpRequest User Agent
	std::stringstream ss;
	ss << "VoxOx";
	ss << "-";
	ss << WengoPhoneBuildId::getVersion();
	ss << "-";
	/*ss << WengoPhoneBuildId::getBuildId();
	ss << "-";*/
	ss << WengoPhoneBuildId::getSvnRevision();
	HttpRequest::setUserAgent(ss.str());
	////

	//Creating instance of NetworkObserver
	NetworkObserver::getInstance();
	////

	//Creating instance of NetworkProxyDiscovery
	NetworkProxyDiscovery::getInstance();
	////

	//Initialize libwebcam for the model thread.
	WebcamDriver::apiInitialize();
	////

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	//Binding events
	config.valueChangedEvent += boost::bind(&WengoPhone::valueChangedEventHandler, this, _1);
	////

	//Creating the UserProfileHandler instance
	_userProfileHandler = new UserProfileHandler();
	////

	//Creating instance of CommandServer
	CommandServer::getInstance(*this);
	////
}

void WengoPhone::exitAfterTimeout() {
	exitEvent(*this);
}

WengoPhone::~WengoPhone() {
	//Deleting created objects
	OWSAFE_DELETE(_userProfileHandler);
	OWSAFE_DELETE(_dtmfThemeManager);
	////
	saveConfiguration();
}

void WengoPhone::init() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	//DTMFThemeManager
	_dtmfThemeManager = new DtmfThemeManager(*this, config.getAudioSmileysDir());
	//Sends the DtmfThemeManager creation event
	dtmfThemeManagerCreatedEvent(*this, *_dtmfThemeManager);

	_userProfileHandler->init();

	//initFinishedEvent
	initFinishedEvent(*this);
}

void WengoPhone::run() {
	init();

	LOG_DEBUG("model thread is ready for events");

	runEvents();
}

void WengoPhone::terminate() {
	Thread::terminate();
}

void WengoPhone::prepareToTerminate() {
	/**
	 * Set up a timeout triggered if SIP registering is too long
	 * so that closing WengoPhone is not too long.
	 */
	static Timer shutdownTimeout;
	shutdownTimeout.timeoutEvent += boost::bind(&WengoPhone::exitAfterTimeout, this);
	shutdownTimeout.start(3000, 3000);
}

void WengoPhone::saveConfiguration() {

	if (_enableSerialization) {
		ConfigManagerFileStorage configManagerStorage(ConfigManager::getInstance());
		configManagerStorage.save();
	}
}

void WengoPhone::valueChangedEventHandler(const std::string & key) {
	saveConfiguration();
}

CoIpManager * WengoPhone::getCoIpManager() const {
	CoIpManager *result = NULL;
	UserProfile *userProfile = _userProfileHandler->getCurrentUserProfile();

	if (userProfile) {
		result = &userProfile->getCoIpManager();
	}

	return result;
}

void WengoPhone::enableSerialization(bool enable) {
	_enableSerialization = enable;
	_userProfileHandler->enableAutoSave(enable);
}

void WengoPhone::setStartupCall(const std::string& call) {
	_startupCall = call;
}

std::string WengoPhone::getStartupCall() const {
	return _startupCall;
}
