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
#include "QtIMAccountPresenceMenuManager.h"

#include <presentation/qt/imaccount/QtIMAccountSettings.h>
#include <presentation/qt/imaccount/QtIMAccountManager.h>
#include <presentation/qt/messagebox/QtVoxMessageBox.h>		//VOXOX - CJC - 2009.06.10 

#include <control/profile/CUserProfile.h>

#include <model/connect/ConnectHandler.h>
#include <model/profile/UserProfile.h>

#include <qtutil/SafeConnect.h>
#include <util/SafeDelete.h>
#include <memory>

#include <QtGui/QApplication>
#include <QtGui/QIcon>
#include <QtGui/QMenu>




//VOXOX - JRT - 2009.09.14 - TODO: There is to much business logic concerning which protocols support
//							 which presence statuses.  This should be in the model, not here.
//							 Also, OffLine is being handled separately from other presence statuses.
//							 This should not be, since Offline is a valid presence status.  The model should handle it.

//VOXOX - JRT - 2009.09.14 - I am removing direct call to ConnectHandler::disconnect).  This needs to be handled in the model, not here.

QtIMAccountPresenceMenuManager::QtIMAccountPresenceMenuManager(QObject* parent, CUserProfile& cUserProfile, std::string imAccountId)
	: QObject(parent)
	, _cUserProfile(cUserProfile)
	, _imAccountId(imAccountId)
{
	setupDisconnectAction();

	_modifyAccountAction = new QAction(this);//VOXOX CHANGE Rolando 04-22-09
	_modifyAccountAction->setText(tr("Edit Account"));//VOXOX CHANGE Rolando 04-22-09
	SAFE_CONNECT(_modifyAccountAction, SIGNAL(triggered(bool)), SLOT(showUpdate()) );//VOXOX CHANGE Rolando 04-22-09

	_actionsGroup = new QActionGroup(this);//VOXOX CHANGE Rolando 03-20-09
	_actionsGroup->setExclusive(true);//VOXOX CHANGE Rolando 03-20-09

//	SAFE_CONNECT(_disconnectAction, SIGNAL(triggered(bool)), SLOT(disconnect()) );
	SAFE_CONNECT(_disconnectAction, SIGNAL(triggered(bool)), SLOT(slotPresenceActionTriggered()) );	//VOXOX - JRT - 2009.09.14 

	_errMsg = "";
}

QtIMAccountPresenceMenuManager::~QtIMAccountPresenceMenuManager()
{
	OWSAFE_DELETE(_disconnectAction);
	OWSAFE_DELETE(_modifyAccountAction);
	OWSAFE_DELETE(_actionsGroup);
}

//VOXOX - JRT - 2009.09.14 - This most likely should be controlled by model.
void QtIMAccountPresenceMenuManager::setupDisconnectAction()
{
	_disconnectAction = new QAction(this);//VOXOX CHANGE Rolando 03-20-09
	_disconnectAction->setText(tr("Offline"));//VOXOX CHANGE Rolando 03-20-09

	IMAccount * imAccount = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount(_imAccountId);//VOXOX CHANGE by Rolando - 2009.07.21 

	if(imAccount)
	{
		if(imAccount->getProtocol() == EnumIMProtocol::IMProtocolTwitter)
		{
			_disconnectAction->setIcon(QPixmap());//VOXOX CHANGE by Rolando - 2009.07.21 
		}
		else
		{
			_disconnectAction->setIcon(QPixmap(":/pics/status/offline.png"));//VOXOX CHANGE by Rolando - 2009.07.21 
		}

		OWSAFE_DELETE(imAccount);//VOXOX CHANGE by Rolando - 2009.07.21 
	}
	else
	{
		_disconnectAction->setIcon(QPixmap(":/pics/status/offline.png"));//VOXOX CHANGE by Rolando - 2009.07.21 
	}

	_actionToState[_disconnectAction] = EnumPresenceState::PresenceStateOffline;	//VOXOX - JRT - 2009.09.14 
}

void QtIMAccountPresenceMenuManager::addPresenceActions(QMenu* menu, EnumPresenceState::PresenceState currentPresenceState, const QString& errMsg )
{
	_errMsg = errMsg;

	bool useDefault = true;

	IMAccount * imAccount = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount(_imAccountId);//VOXOX CHANGE by Rolando - 2009.07.21 

	//VOXOX - JRT - 2009.09.14 - TODO: This should come from model
	if(imAccount && (imAccount->getProtocol() == EnumIMProtocol::IMProtocolTwitter) )
	{
		useDefault = false;
	}

	if ( useDefault )
	{
		addPresenceAction( menu, EnumPresenceState::PresenceStateOnline, currentPresenceState,
						QPixmap(":/pics/status/online.png"), tr("Online"));//VOXOX CHANGE by Rolando - 2009.07.21 
	}
	else
	{
		addPresenceAction( menu, EnumPresenceState::PresenceStateOnline, currentPresenceState,
						QPixmap(), tr("Online"));//VOXOX CHANGE by Rolando - 2009.07.21 
	}

	//TODO: VOXOX CHANGE by Rolando - 2009.05.08 fix this code to let get if the corresponding network supports each presence state
	addSpecificNetworkPresenceActions(menu, _imAccountId, currentPresenceState);	

	_disconnectAction->setCheckable ( true );//VOXOX CHANGE Rolando 03-20-09
	_actionsGroup->addAction(_disconnectAction);//VOXOX CHANGE Rolando 03-20-09
	menu->addAction(_disconnectAction);//VOXOX CHANGE Rolando 03-20-09

	//VOXOX CHANGE by Rolando - 2009.07.21 
	if(!imAccount->isConnected())
	{
		_disconnectAction->setChecked(true);
	}

	//VOXOX CHANGE Rolando 03-20-09
	EnumIMProtocol::IMProtocol protocol = imAccount->getProtocol();

	if ( !EnumIMProtocol::isSip( protocol ) && !imAccount->isVoxOxAccount() )	//VOXOX - JRT - 2009.07.02 
	{
		menu->addSeparator();
		menu->addAction(_modifyAccountAction);//VOXOX CHANGE Rolando 04-22-09
		menu->addAction(tr("Delete Account"), this, SLOT(deleteAccount()) );
	}

	//VOXOX - JRT - 2009.05.21 
	if ( !_errMsg.isEmpty() )
	{
		menu->addAction( tr("View Login Error"), this, SLOT(viewLoginError()) );
	}
	
	if(protocol == EnumIMProtocol::IMProtocolMYSPACE)
	{
		menu->addSeparator();
		menu->addAction(tr("Add friends from MySpace.com"), this, SLOT(addFriendsFromMySpace()) );
	}

	SAFE_CONNECT(menu, SIGNAL(aboutToShow()), SLOT(updateActions()) );

	OWSAFE_DELETE(imAccount);//VOXOX CHANGE by Rolando - 2009.07.21 
}


//TODO: VOXOX CHANGE by Rolando - 2009.05.08 fix this code to let get if the corresponding network supports each presence state
void QtIMAccountPresenceMenuManager::addSpecificNetworkPresenceActions(QMenu* menu, std::string imAccountId, EnumPresenceState::PresenceState currentPresenceState)
{
	IMAccount * imAccount = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount(imAccountId);//VOXOX CHANGE by Rolando - 2009.07.21 

	if(imAccount)
	{
		switch(imAccount->getProtocol())
		{
			case EnumIMProtocol::IMProtocolFacebook:
				//VOXOX CHANGE by Rolando - 2009.07.09 
				addPresenceAction(
					menu, EnumPresenceState::PresenceStateAway, currentPresenceState,
					QPixmap(":/pics/status/away.png"), tr("Away"));
				break;
			case EnumIMProtocol::IMProtocolTwitter://VOXOX - CJC - 2009.06.28 Twitter only display online/offline
				break;
			case EnumIMProtocol::IMProtocolWengo://VOXOX CHANGE by Rolando - 2009.06.12 - now we are displaying only online and offline presence states
			case EnumIMProtocol::IMProtocolSIP://VOXOX CHANGE by Rolando - 2009.06.12 - now we are displaying only online and offline presence states 
				break;
			default:
				addPresenceAction(
					menu, EnumPresenceState::PresenceStateAway, currentPresenceState,
					QPixmap(":/pics/status/away.png"), tr("Away"));

				addPresenceAction(
					menu, EnumPresenceState::PresenceStateDoNotDisturb, currentPresenceState,
					QPixmap(":/pics/status/donotdisturb.png"), tr("Do Not Disturb"));

				addPresenceAction(
					menu, EnumPresenceState::PresenceStateInvisible, currentPresenceState,
					QPixmap(":/pics/status/invisible.png"), tr("Invisible"));
		}

		OWSAFE_DELETE(imAccount);//VOXOX CHANGE by Rolando - 2009.07.21 
	}
}

void QtIMAccountPresenceMenuManager::updateActions()
{
	std::auto_ptr<IMAccount> imAccount ( _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount(_imAccountId) );
	_disconnectAction->setEnabled(imAccount->isConnected());
}

void QtIMAccountPresenceMenuManager::addPresenceAction(QMenu* menu, EnumPresenceState::PresenceState state, EnumPresenceState::PresenceState currentPresenceState, const QPixmap& pix, const QString& text) 
{
	QAction* action = menu->addAction(pix, text, this, SLOT(slotPresenceActionTriggered()) );
	action->setCheckable ( true );//VOXOX CHANGE Rolando 03-20-09
	_actionToState[action] = state;
	_actionsGroup->addAction(action);	//VOXOX CHANGE Rolando 03-20-09

	if(currentPresenceState == state)//VOXOX CHANGE Rolando 03-20-09
	{
		action->setChecked(true);//VOXOX CHANGE Rolando 03-20-09
	}
}

void QtIMAccountPresenceMenuManager::slotPresenceActionTriggered() 
{
	EnumPresenceState::PresenceState state = _actionToState[sender()];	
	
	_cUserProfile.getUserProfile().setPresenceState(state, _imAccountId, true, String::null );
}

//VOXOX - JRT - 2009.09.14 - Move this logic to UserProfile.
//void QtIMAccountPresenceMenuManager::disconnect() 
//{	
//	_cUserProfile.getUserProfile().getConnectHandler().disconnect(_imAccountId);
//}

void QtIMAccountPresenceMenuManager::viewLoginError() 
{
	std::auto_ptr<IMAccount> imAccount ( _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount(_imAccountId) );
	QString login   = QString::fromStdString(imAccount->getLogin());
	QString message = tr("Login error for account <b>%1</b>: \n\n\t%2").arg(login).arg(_errMsg);
	//VOXOX - CJC - 2009.06.10 
	QtVoxMessageBox box(QApplication::activeWindow());
	box.setWindowTitle("VoxOx");
	box.setText(message);
	box.setStandardButtons(QMessageBox::Ok);
	box.exec();
}

void QtIMAccountPresenceMenuManager::deleteAccount() 
{
	std::auto_ptr<IMAccount> imAccount ( _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount(_imAccountId) );
	QString login = QString::fromStdString(imAccount->getLogin()); //THIS IS THE ACCOUNT WHICH IS GOING TO BE REMOVE
	QString message = tr("Are sure you want to delete the account <b>%1</b>?").arg(login);

	QtVoxMessageBox box(QApplication::activeWindow());	//VOXOX - CJC - 2009.06.10 
	box.setWindowTitle("VoxOx");
	box.setText(message);
	box.addButton(tr("&Delete"), QMessageBox::AcceptRole);
	box.addButton(tr("&Cancel"), QMessageBox::RejectRole);
	//box.setStandardButtons(tr("&Delete") |  tr("Cancel"));
	int ret = box.exec();

	if (ret == QMessageBox::AcceptRole) //VOXOX - CJC - 2009.06.23
	{
		_cUserProfile.getUserProfile().removeIMAccount(*imAccount);
	}
}

//VOXOX CHANGE Rolando 04-22-09
void QtIMAccountPresenceMenuManager::showUpdate()
{
	QtIMAccountManager * imAccountManager = new QtIMAccountManager(_cUserProfile.getUserProfile(), false, 0);
		
	IMAccount * imAccount =
		_cUserProfile.getUserProfile().getIMAccountManager().getIMAccount(_imAccountId);

	if (imAccount) 
	{
		QtIMAccountSettings * imAccountSettings = new QtIMAccountSettings(_cUserProfile.getUserProfile(), *imAccount, imAccountManager->getWidget());

		OWSAFE_DELETE(imAccountSettings);
	}
}

void QtIMAccountPresenceMenuManager::addFriendsFromMySpace() 
{
	std::auto_ptr<IMAccount> imAccount ( _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount(_imAccountId) );
	_cUserProfile.getUserProfile().addFriendsFromMySpace(*imAccount);
}
