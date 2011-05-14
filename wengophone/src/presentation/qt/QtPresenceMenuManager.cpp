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
#include "QtPresenceMenuManager.h"

#include <model/profile/UserProfile.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/connect/ConnectHandler.h>

#include <util/SafeDelete.h>
#include <presentation/qt/QtToolBar.h>
#include <presentation/qt/QtWengoPhone.h>
#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>

#include <QtGui/QMenu>


//QtPresenceMenuManager::QtPresenceMenuManager(QObject* parent, CUserProfileHandler* cUserProfileHandler, QtWengoPhone * qtWengoPhone)
//	: QObject(parent),
//	_cUserProfileHandler(cUserProfileHandler),
//	_qtWengoPhone(qtWengoPhone) {
//}

QtPresenceMenuManager::QtPresenceMenuManager(QObject* parent, CUserProfile* cUserProfile, QtWengoPhone * qtWengoPhone)
	: QObject(parent),
	_cUserProfile(cUserProfile),
	_qtWengoPhone(qtWengoPhone) {

	_actionsGroup = new QActionGroup(this);
	_actionsGroup->setExclusive(true);
}
QtPresenceMenuManager::~QtPresenceMenuManager(){
	OWSAFE_DELETE(_logOffAction);
	OWSAFE_DELETE(_actionsGroup);
}
void QtPresenceMenuManager::addPresenceActions(QMenu* menu, EnumPresenceState::PresenceState currentPresenceState) {
	addGlobalPresenceAction(
		menu, EnumPresenceState::PresenceStateOnline, currentPresenceState,
		QPixmap(":/pics/status/online.png"), tr("Online"));

	addGlobalPresenceAction(
		menu, EnumPresenceState::PresenceStateAway,currentPresenceState,
		QPixmap(":/pics/status/away.png"), tr("Away"));

	addGlobalPresenceAction(
		menu, EnumPresenceState::PresenceStateDoNotDisturb,currentPresenceState,
		QPixmap(":/pics/status/donotdisturb.png"), tr("Do Not Disturb"));

	addGlobalPresenceAction(
		menu, EnumPresenceState::PresenceStateInvisible,currentPresenceState,
		QPixmap(":/pics/status/invisible.png"), tr("Invisible"));

	//menu->addSeparator();
	_logOffAction = menu->addAction(QPixmap(":/pics/status/offline.png"),tr("Offline"), this, SLOT(logOff()) );
	_logOffAction->setCheckable ( true );//VOXOX CHANGE by Rolando - 2009.06.16 
	_actionsGroup->addAction(_logOffAction);//VOXOX CHANGE by Rolando - 2009.06.16 

	//bool connected = _cUserProfileHandler->getCUserProfile()->getUserProfile().isConnected();
	bool connected = _cUserProfile->getUserProfile().isConnected();
	_logOffAction->setEnabled(connected);//VOXOX CHANGE by Rolando - 2009.06.16 
	if(!connected){
		_logOffAction->setChecked(true);
	}

}

void QtPresenceMenuManager::addGlobalPresenceAction(QMenu* menu, EnumPresenceState::PresenceState state, EnumPresenceState::PresenceState currentPresenceState, const QPixmap& pix, const QString& text) {
	QAction* action = menu->addAction(pix, text, this, SLOT(slotGlobalPresenceActionTriggered()) );
	_actionToState[action] = state;
	action->setCheckable ( true );
	_actionsGroup->addAction(action);

	if(currentPresenceState == state){
		action->setChecked(true);
	}	
}

void QtPresenceMenuManager::slotGlobalPresenceActionTriggered() {
	EnumPresenceState::PresenceState state = _actionToState[sender()];
	//_cUserProfile->getUserProfile().setAllPresenceState(state);//VOXOX CHANGE by Rolando - 2009.06.16 - deleted to no change the state of sipaccounts when it is not online or offline presence state

	//VOXOX CHANGE by Rolando - 2009.06.16 - now we are changing the state of sipaccounts only to online or offline presence states
	IMAccountList imAccountList = _cUserProfile->getUserProfile().getIMAccountManager().getIMAccountListCopy();
	// Updates IMAccounts icons status
	IMAccountList::const_iterator it = imAccountList.begin(), end = imAccountList.end();
	for (; it!=end; ++it) 
	{
		if( EnumIMProtocol::isSip( it->second.getProtocol() ) )
		{
			if(state == EnumPresenceState::PresenceStateOnline)
			{
				_cUserProfile->getUserProfile().setPresenceState(state, it->second.getKey(), true, String::null );	//VOXOX - JRT - 2009.09.14 
			}
		}
		else
		{
			_cUserProfile->getUserProfile().setPresenceState(state, it->second.getKey(), true, String::null );	//VOXOX - JRT - 2009.09.14 
		}
	}	
}

void QtPresenceMenuManager::logOff() {

	//disconnect all the accounts
	IMAccountList imAccountList = _cUserProfile->getUserProfile().getIMAccountManager().getIMAccountListCopy();
	// Updates IMAccounts icons status
	IMAccountList::const_iterator it = imAccountList.begin(), end = imAccountList.end();
	for (; it!=end; ++it) {
		_cUserProfile->getUserProfile().getConnectHandler().disconnect(it->second.getKey());//VOXOX CHANGE by Rolando - 2009.06.16 
	}
	
}
