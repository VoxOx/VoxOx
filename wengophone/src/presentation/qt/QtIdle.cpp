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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtIdle.h"

#include <model/profile/UserProfile.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#define LOGGER_COMPONENT "QtIdle"
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <psiidle/idle.h>

QtIdle::QtIdle(UserProfile & userProfile, QObject * parent)
	: QObjectThreadSafe(parent),
	_userProfile(userProfile) {

	_idle = NULL;
	_autoAway = false;

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	config.valueChangedEvent += boost::bind(&QtIdle::configChangedEventHandler, this, _1);

	//Init the idle class
	configChangedEventHandler(Config::GENERAL_AWAY_TIMER_KEY);
}

QtIdle::~QtIdle() {
}

void QtIdle::setOnlineIMAccountsAway() {
	_onlineIMAccountIdList.clear();
	IMAccountManager& imAccountManager = _userProfile.getIMAccountManager();
	IMAccountList	  imAccountList    = imAccountManager.getIMAccountListCopy();

	IMAccountList::const_iterator it = imAccountList.begin(), end = imAccountList.end();
	for (; it!=end; ++it) 
	{
		//VOXOX - JRT - 2009.07.04 - TODO: This logic should be in IMAccount.  
		//							I suggest a method name shouldChangePresenceStatus( EnumPresenceState::PresenceStateAway );
		if ( !EnumIMProtocol::isSip( it->second.getProtocol() ) )	//VOXOX - JRT - 2009.07.04 - SIP accounts do not support AWAY.
		{
			if (it->second.isOnline() && it->second.isConnected()) // VOXOX - MARIN This is for accounts are disconnected, so don't be connected
			{
				std::string id = it->second.getKey();
				_onlineIMAccountIdList.append(id);

				_userProfile.setPresenceState(EnumPresenceState::PresenceStateAway, id, false, String::null );	//VOXOX - JRT - 2009.09.14 
			}
		}
	}
}

void QtIdle::restoreOnlineIMAccounts() {
	Q_FOREACH(std::string id, _onlineIMAccountIdList) 
	{
		_userProfile.setPresenceState(EnumPresenceState::PresenceStateOnline, id, false, String::null );
	}
}

void QtIdle::slotSecondsIdle(int seconds) {
	LOG_DEBUG("seconds=%d", seconds);
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	int autoAwaySeconds = config.getGeneralAwayTimer() * 60;

	if (_autoAway) {
		if (seconds < autoAwaySeconds) {
			LOG_DEBUG("No longer away");
			restoreOnlineIMAccounts();
			_autoAway = false;
		}
	} else {
		if (seconds > autoAwaySeconds) {
			LOG_DEBUG("Auto away");
			setOnlineIMAccountsAway();
			_autoAway = true;
		}
	}
}

void QtIdle::configChangedEventHandler(const std::string & key) {
	typedef PostEvent1<void (std::string), std::string> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtIdle::configChangedEventHandlerThreadSafe, this, _1), key);
	postEvent(event);
}

void QtIdle::configChangedEventHandlerThreadSafe(std::string key) {
	if (key != Config::GENERAL_AWAY_TIMER_KEY) {
		return;
	}

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	int interval = config.getGeneralAwayTimer();
	if (interval != Config::NO_AWAY_TIMER) {
		if (!_idle) {
			_idle = new Idle(this);
			SAFE_CONNECT(_idle, SIGNAL(secondsIdle(int)), SLOT(slotSecondsIdle(int)) );
		}
		_idle->start();
	} else {
		OWSAFE_DELETE(_idle);
	}
}
