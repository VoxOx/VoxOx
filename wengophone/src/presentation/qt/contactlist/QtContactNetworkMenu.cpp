/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* CLASS QtContactNetworkMenu 
* @author Rolando 
* @date 2009.10.23
*/

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtContactNetworkMenu.h"

#include <presentation/qt/contactlist/QtContactActionManager.h>
#include <imwrapper/QtEnumIMProtocol.h>	
#include <control/profile/CUserProfileHandler.h>
#include <control/profile/CUserProfile.h>
#include <qtutil/SafeConnect.h>

QtContactNetworkMenu::QtContactNetworkMenu(const std::string & contactId, CWengoPhone & cWengoPhone, QWidget * parent)
	: QMenu(parent),
	_cWengoPhone(cWengoPhone)
{
	_contactId = contactId;	
	buildContactMenuNetwork();
}

QtContactNetworkMenu::~QtContactNetworkMenu() {

}

std::string QtContactNetworkMenu::getContactId() const {
	return _contactId;
}

void QtContactNetworkMenu::setContactId(const std::string & contactId) {
	_contactId = contactId;
}

//VOXOX CHANGE by Rolando - 2009.10.23 
void QtContactNetworkMenu::buildContactMenuNetwork() 
{
	QtEnumIMProtocol::IMProtocol protocol = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().getContactQtProtocol(_contactId);

	QAction * chatAction	  = new QAction(tr("Chat"),   this);
	QAction * socialWebAction = new QAction(tr("Social"), this);
	
	SAFE_CONNECT(chatAction,      SIGNAL(triggered()), SLOT(chatButtonClicked()));
	SAFE_CONNECT(socialWebAction, SIGNAL(triggered()), SLOT(webProfileContactClicked()));
	
	if(protocol == QtEnumIMProtocol::IMProtocolVoxOx)
	{
		QAction * callWithVideo =  new QAction(tr("Call with Video"), this);
		SAFE_CONNECT(callWithVideo, SIGNAL(triggered()), SLOT(callButtonClicked()));

		QAction * editContact =  new QAction(tr("Edit Contact"), this);
		SAFE_CONNECT(editContact, SIGNAL(triggered()), SLOT(profileContactClicked()));

		addAction(callWithVideo);
		addAction(editContact);

		return;
		
	}
	addAction(chatAction);

	if(protocol ==QtEnumIMProtocol::IMProtocolTwitter)
	{
		chatAction->setText(tr("Reply to"));
		socialWebAction->setText(tr("Twitter Profile"));
		addAction(socialWebAction);
	}
	else if(protocol ==QtEnumIMProtocol::IMProtocolFacebook)
	{
		socialWebAction->setText(tr("Facebook Profile"));
		addAction(socialWebAction);
	}
	else if(protocol ==QtEnumIMProtocol::IMProtocolMYSPACE)
	{
		socialWebAction->setText(tr("MySpace Profile"));
		addAction(socialWebAction);
	}
}

void QtContactNetworkMenu::chatButtonClicked() 
{
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->chatContact(QString::fromStdString(_contactId));
}

void QtContactNetworkMenu::webProfileContactClicked() 
{

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->openSocialContactPage(QString::fromStdString(_contactId));
	
}

void QtContactNetworkMenu::callButtonClicked() 
{

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->callContact(QString::fromStdString(_contactId));
	
}

void QtContactNetworkMenu::profileContactClicked()
{

	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_cWengoPhone);
	contactActionManager->profileContact(QString::fromStdString(_contactId));
	
}