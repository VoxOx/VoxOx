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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtIMAccountMonitor.h"

#include <control/profile/CUserProfile.h>

#include <model/connect/ConnectHandler.h>
#include <model/presence/PresenceHandler.h>
#include <model/profile/UserProfile.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <memory>


QtIMAccountMonitor::QtIMAccountMonitor(QObject* parent, CUserProfile* cUserProfile)
: QObject(parent)
, _cUserProfile(cUserProfile) {

	UserProfile& userProfile = _cUserProfile->getUserProfile();

	// imAccountManager
	IMAccountManager& imAccountManager = userProfile.getIMAccountManager();
	
	imAccountManager.imAccountAddedEvent += 
		boost::bind(&QtIMAccountMonitor::imAccountAddedEventHandler, this, _2);

	imAccountManager.imAccountRemovedEvent += 
		boost::bind(&QtIMAccountMonitor::imAccountRemovedEventHandler, this, _2);

	// connectHandler
	ConnectHandler& connectHandler = userProfile.getConnectHandler();

	connectHandler.connectedEvent +=
		boost::bind(&QtIMAccountMonitor::connectedEventHandler, this, _2);

	connectHandler.disconnectedEvent +=
		boost::bind(&QtIMAccountMonitor::disconnectedEventHandler, this, _2, _3, _4);

	connectHandler.connectionProgressEvent +=
		boost::bind(&QtIMAccountMonitor::connectionProgressEventHandler, this, _2, _3, _4, _5);

	// presenceHandler
	PresenceHandler & presenceHandler = userProfile.getPresenceHandler();
	presenceHandler.myPresenceStatusEvent +=
		boost::bind(&QtIMAccountMonitor::myPresenceStatusEventHandler, this, _2, _3);

	// Initialize the map with existing accounts
	{
		QMutexLocker lock(&_imAccountInfoMapMutex);
		IMAccountList imAccountList = userProfile.getIMAccountManager().getIMAccountListCopy();
		IMAccountList::const_iterator
			it = imAccountList.begin(),
			end = imAccountList.end();
	
		for (; it!=end; ++it) {
			IMAccountInfo info;
			//VOXOX - JRT - 2009.04.09 
			//if (it->isConnected()) {
			//	info._state = StateConnected;
			//	info._message = connectedMessageForIMAccount(*it);
			//} else {
			//	info._state = StateDisconnected;
			//	info._message = tr("Disconnected");
			//}

			IMAccount& rAcct = const_cast<IMAccount&>(it->second);
			if ( rAcct.isConnected()) {
				info._state = StateConnected;
				info._message = connectedMessageForIMAccount( rAcct);
			} else {
				info._state = StateDisconnected;
				info._message = tr("Disconnected");
			}

//			QString id = QString::fromStdString(rAcct.getUUID());
			QString id = QString::fromStdString(rAcct.getKey());	//VOXOX - JRT - 2009.04.24
			//End VoxOx

			_imAccountInfoMap[id] = info;
		}
	}
}

void QtIMAccountMonitor::imAccountAddedEventHandler(std::string imAccountId) {
	QString id = QString::fromStdString(imAccountId);

	std::auto_ptr<IMAccount> imAccount (_cUserProfile->getUserProfile().getIMAccountManager().getIMAccount(imAccountId));
	if (!imAccount.get()) {
		LOG_FATAL("No such id " + imAccountId);
	}
	
	IMAccountInfo info;
	if (imAccount->isConnected()) {
		info._state = StateConnected;
		info._message = connectedMessageForIMAccount(*imAccount);
	} else {
		info._state = StateDisconnected;
		info._message = tr("Disconnected");
	}

	{
		QMutexLocker lock(&_imAccountInfoMapMutex);
		_imAccountInfoMap[id] = info;
	}

	imAccountAdded(id);
}

void QtIMAccountMonitor::imAccountRemovedEventHandler(std::string imAccountId) {
	QString id = QString::fromStdString(imAccountId);

	{
		QMutexLocker lock(&_imAccountInfoMapMutex);
		_imAccountInfoMap.remove(id);
	}
	imAccountRemoved(id);
}

void QtIMAccountMonitor::connectedEventHandler(std::string imAccountId) {
	QString id = QString::fromStdString(imAccountId);
	{
		QMutexLocker lock(&_imAccountInfoMapMutex);
		_imAccountInfoMap[id]._state = StateConnected;
		_imAccountInfoMap[id]._message = connectedMessageForIMAccountId(imAccountId);
	}
	imAccountUpdated(id);
}

void QtIMAccountMonitor::disconnectedEventHandler(std::string imAccountId, bool connectionError, std::string reason) {
	QString id = QString::fromStdString(imAccountId);
	ConnectionState state;

	QString message;
	if (connectionError) {
		state = StateFailure;
		message = tr("Error: %1").arg(QString::fromUtf8(reason.c_str()));
	} else {
		state = StateDisconnected;
		message = tr("Disconnected");
	}
	{
		QMutexLocker lock(&_imAccountInfoMapMutex);
		_imAccountInfoMap[id]._state = state;
		_imAccountInfoMap[id]._message = message;
	}
	imAccountUpdated(id);
}

void QtIMAccountMonitor::connectionProgressEventHandler(std::string imAccountId, int currentStep, int totalSteps, std::string infoMessage) {
	QString id = QString::fromStdString(imAccountId);

	QString message = tr("%1 (%2/%3)", "%1 is message, %2 is currentStep, %3 is totalSteps")
		.arg(QString::fromStdString(infoMessage))
		.arg(currentStep + 1)
		.arg(totalSteps);

	{
		QMutexLocker lock(&_imAccountInfoMapMutex);
		_imAccountInfoMap[id]._state = StateConnecting;
		_imAccountInfoMap[id]._message = message;
	}
	imAccountUpdated(id);
}

void QtIMAccountMonitor::myPresenceStatusEventHandler(std::string imAccountId, EnumPresenceState::MyPresenceStatus status) {

	if (status == EnumPresenceState::MyPresenceStatusOk) {
		QString id = QString::fromStdString(imAccountId);
		{
			QMutexLocker lock(&_imAccountInfoMapMutex);
			_imAccountInfoMap[id]._state = StateConnected;
			_imAccountInfoMap[id]._message = connectedMessageForIMAccountId(imAccountId);
		}
		imAccountUpdated(id);
	}
}
QtIMAccountMonitor::IMAccountInfoAutoPtr QtIMAccountMonitor::getIMAccountInfo(const QString& imAccountId) const {
	QMutexLocker lock(&_imAccountInfoMapMutex);
	if (!_imAccountInfoMap.contains(imAccountId)) {
		LOG_WARN("no account found with id " + imAccountId.toStdString());
		return IMAccountInfoAutoPtr();
	}
	return IMAccountInfoAutoPtr( new IMAccountInfo(_imAccountInfoMap[imAccountId]) );
}

QString QtIMAccountMonitor::connectedMessageForIMAccountId(std::string imAccountId) {
	std::auto_ptr<IMAccount> imAccount( _cUserProfile->getUserProfile().getIMAccountManager().getIMAccount(imAccountId) );
	if (!imAccount.get()) {
		LOG_FATAL("no account found with id " + imAccountId);
	}
	QString message = connectedMessageForIMAccount(*imAccount);
	return message;
}

QString QtIMAccountMonitor::connectedMessageForIMAccount(const IMAccount& imAccount) {
	EnumPresenceState::PresenceState state = imAccount.getPresenceState();
	switch (state) {
	case EnumPresenceState::PresenceStateOnline:
		return tr("Online");
	
	case EnumPresenceState::PresenceStateInvisible:
		return tr("Invisible");
	
	case EnumPresenceState::PresenceStateAway:
		return tr("Away");

	case EnumPresenceState::PresenceStateDoNotDisturb:
		return tr("Do not disturb");

	case EnumPresenceState::PresenceStateOffline:
	case EnumPresenceState::PresenceStateUserDefined:
	case EnumPresenceState::PresenceStateUnknown:
	case EnumPresenceState::PresenceStateMulti:
	case EnumPresenceState::PresenceStateUnavailable:
		LOG_ERROR("Should not get called with a presence of " + EnumPresenceState::toString(state));
		return QString::null;
	}
	LOG_FATAL("Unhandled presence value " + EnumPresenceState::toString(state));
	return QString::null;
}
