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
#include "QtUserProfile.h"

#include <presentation/qt/QtBrowserWidget.h>
#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/login/QtLoginDialog.h>
#include <presentation/qt/profile/QtProfileDetails.h>
#include <presentation/qt/profile/QtUserProfileHandler.h>

#include <presentation/qt/QtSystray.h>
#include <presentation/qt/statusbar/QtStatusBar.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/network/NetworkObserver.h>//VOXOX CHANGE by Rolando - 2009.07.28 
#include <model/contactlist/ContactProfile.h>
#include <model/presence/PresenceHandler.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/IMContact.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <presentation/qt/messagebox/QtVoxMessageBox.h>	//VOXOX - CJC - 2009.06.10 

#include <QtGui/QtGui>

QtUserProfile::QtUserProfile(CUserProfile & cUserProfile, QtWengoPhone & qtWengoPhone)
	: QObjectThreadSafe(NULL),
	_qtWengoPhone(qtWengoPhone),
	_cUserProfile(cUserProfile) 
{
	typedef PostEvent0<void ()> MyPostEvent;
	MyPostEvent* event = new MyPostEvent(boost::bind(&QtUserProfile::initThreadSafe, this));
	postEvent(event);
}

void QtUserProfile::initThreadSafe() 
{
	qRegisterMetaType<IMContact>("IMContact");

	SAFE_CONNECT_TYPE(this, SIGNAL(loginStateChangedEventHandlerSignal(SipAccount *, int)),            SLOT(loginStateChangedEventHandlerSlot(SipAccount *, int)), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(networkDiscoveryStateChangedEventHandlerSignal(SipAccount *, int)), SLOT(networkDiscoveryStateChangedEventHandlerSlot(SipAccount *, int)), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(authorizationRequestEventHandlerSignal(PresenceHandler *, IMContact, QString)), SLOT(authorizationRequestEventHandlerSlot(PresenceHandler *, IMContact, QString)), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(incomingSubscribeEventHandlerSignal(PresenceHandler *,const QString&,int,const QString&, const QString&)), SLOT(incomingSubscribeEventHandlerSlot(PresenceHandler *,const QString&,int,const QString&,const QString&)), Qt::QueuedConnection);

	QtBrowserWidget* browserWidget = _qtWengoPhone.getQtBrowserWidget();
	if (browserWidget) 
	{
		browserWidget->loadAccountURL();
	}

	//VOXOX CHANGE by Rolando - 2009.07.28 - added to handle connection and disconnection events to show offline and online webpages in apps tab
	//Check Internet connection status
	NetworkObserver::getInstance().connectionIsDownEvent += boost::bind(&QtUserProfile::connectionIsDownEventHandler, this);
	NetworkObserver::getInstance().connectionIsUpEvent   += boost::bind(&QtUserProfile::connectionIsUpEventHandler,   this);
}

QtUserProfile::~QtUserProfile() 
{
	LOG_DEBUG("QtUserProfile");
}

void QtUserProfile::updatePresentation() 
{
}

void QtUserProfile::updatePresentationThreadSafe() 
{
}

void QtUserProfile::loginStateChangedEventHandler(SipAccount & sender, EnumSipLoginState::SipLoginState state) 
{
	loginStateChangedEventHandlerSignal(&sender, (int) state);
}

void QtUserProfile::networkDiscoveryStateChangedEventHandler(SipAccount & sender, SipAccount::NetworkDiscoveryState state) 
{
	networkDiscoveryStateChangedEventHandlerSignal(&sender, (int) state);
}

void QtUserProfile::authorizationRequestEventHandler(PresenceHandler & sender, const IMContact & imContact, const std::string & message) 
{
	authorizationRequestEventHandlerSignal(&sender, imContact, QString::fromStdString(message));
}

void QtUserProfile::incomingSubscribeEventHandler(PresenceHandler & sender,const std::string & imaccountId,int sid,const std::string & from,const std::string & evtType) 
{
	incomingSubscribeEventHandlerSignal(&sender, QString::fromStdString(imaccountId), sid,QString::fromStdString(from), QString::fromStdString(evtType));
}

void QtUserProfile::loginStateChangedEventHandlerSlot(SipAccount * sender, int iState) 
{
	EnumSipLoginState::SipLoginState state = (EnumSipLoginState::SipLoginState) iState;

	switch (state) 
	{
	case EnumSipLoginState::SipLoginStateReady:
		_qtWengoPhone.getQtBrowserWidget()->loadDefaultURL();
		break;

	case EnumSipLoginState::SipLoginStateConnected:
		_qtWengoPhone.getQtBrowserWidget()->loadAccountURL();
		break;

	case EnumSipLoginState::SipLoginStateDisconnected:
		//_qtWengoPhone.getQtBrowserWidget()->loadDefaultURL();//VOXOX CHANGE by Rolando - 2009.07.24
		_qtWengoPhone.getQtBrowserWidget()->loadOfflineURL();//VOXOX CHANGE by Rolando - 2009.07.24 
		break;

	case EnumSipLoginState::SipLoginStatePasswordError:
		LOG_DEBUG("SipLoginStatePasswordError");
		break;

	case EnumSipLoginState::SipLoginStateNetworkError:
	{
		CUserProfileHandler & handler = _qtWengoPhone.getCWengoPhone().getCUserProfileHandler();
		QtLoginDialog dlg(&_qtWengoPhone, handler);
		dlg.setValidAccount((WengoAccount &) *sender);
		dlg.setErrorMessage(tr("Network error."));
		dlg.exec();
	}
		break;
	case EnumSipLoginState::SipLoginStateProgress:
		//VOXOX CHANGE CJC WE DONT NEED THIS ANY MOER INT HE STATUS BAR
		//_qtWengoPhone.getQtStatusBar().updatePhoneLineState(EnumPhoneLineState::PhoneLineStateProgress);
		_qtWengoPhone.getQtSystray().phoneLineStateChanged(EnumPhoneLineState::PhoneLineStateProgress);
		break;
	
	default:
		LOG_FATAL("unknown state=" + String::fromNumber(state));
	}

	_qtWengoPhone.updatePresentation();
}

//VOXOX CHANGE by Rolando - 2009.07.27 
void QtUserProfile::connectionIsDownEventHandler() 
{
	_qtWengoPhone.showHomeTab();//VOXOX CHANGE by Rolando - 2009.07.27 
	_qtWengoPhone.getQtBrowserWidget()->loadOfflineURL();//VOXOX CHANGE by Rolando - 2009.07.27 
}

//VOXOX CHANGE by Rolando - 2009.07.27 
void QtUserProfile::connectionIsUpEventHandler() 
{
	_qtWengoPhone.showHomeTab();		//VOXOX CHANGE by Rolando - 2009.07.27 
	_qtWengoPhone.getQtBrowserWidget()->loadAccountURL();//VOXOX CHANGE by Rolando - 2009.07.27 
}

void QtUserProfile::networkDiscoveryStateChangedEventHandlerSlot(SipAccount * /*sender*/, int /*iState*/) {

	//SipAccount::NetworkDiscoveryState state = (SipAccount::NetworkDiscoveryState) iState;
}

void QtUserProfile::incomingSubscribeEventHandlerSlot(PresenceHandler * sender,const QString & imaccountId,int sid,const QString & from, const QString& evtType) 
{
	QString request = QString("%1 would like to add you as a buddy.").arg(from);

	//VOXOX - CJC - 2009.06.10 
	QtVoxMessageBox box(_qtWengoPhone.getWidget());
	box.setWindowTitle("VoxOx - Authorization Request");
	box.setText(request);
	box.addButton(tr("&Accept"), QMessageBox::AcceptRole);
	box.addButton(tr("&Deny"), QMessageBox::RejectRole);
	int buttonClicked = box.exec();


//	int buttonClicked = QMessageBox::question(_qtWengoPhone.getWidget(),
//		tr("@product@ - Authorization Request"), request,
//		tr("&Authorize"), tr("&Block"));

	if (buttonClicked == QMessageBox::AcceptRole) 
	{
		//TODO: give a personal message

//		if (_cUserProfile.getCContactList().findContactThatOwns(imContact).empty()) 
//		{
//			//If the contact is not in our ContactList
//			ContactProfile contactProfile;
//			contactProfile.addIMContact(imContact);
//			QtProfileDetails qtProfileDetails(_cUserProfile, contactProfile, _qtWengoPhone.getWidget(), tr("Edit Contact"));
//			if (qtProfileDetails.show()) 
//			{
//				_cUserProfile.getCContactList().addContact(contactProfile);
//			}
//		}

		sender->acceptSubscription(sid,imaccountId.toStdString());
	} 
	else 
	{
		sender->rejectSubscription(sid,imaccountId.toStdString());
		//TODO: give a personal message
		//TODO: avoid direct access to model (as we are in the GUI thread)
		//sender->authorizeContact(imContact, false, String::null);
	}
}

void QtUserProfile::authorizationRequestEventHandlerSlot(PresenceHandler * sender, IMContact imContact, QString message) 
{
	IMAccount * imAccount = _cUserProfile.getUserProfile().getIMAccountManager().getIMAccount(imContact.getIMAccountId());
	if (imAccount) 
	{
		QString protocolName( QtEnumIMProtocolMap::getQtProtocolName( imContact.getQtProtocol() ).c_str() );	//VOXOX - JRT - 2009.07.02 
		QString request = QString("%1 (from %2) would like to add you as a buddy.")//VOXOX - CJC - 2009.06.23 
			.arg(QString::fromStdString(imContact.getContactId()))
//			.arg(QString::fromStdString(EnumIMProtocol::toString(imContact.getProtocol())))
			.arg( protocolName )								//VOXOX - JRT - 2009.07.02 
			.arg(QString::fromStdString(imAccount->getLogin()));//VOXOX - CJC - 2009.06.23 

		if (!message.isEmpty()) 
		{
			request += QString("%1\n").arg(message);//VOXOX - CJC - 2009.06.23 
		}

		QString lowerIMContactId = QString::fromStdString(imContact.getContactId()).toLower(); 
		IMContact lowerContact(*imContact.getIMAccount(),lowerIMContactId.toStdString());

		if (_cUserProfile.getCContactList().findContactThatOwns(imContact).empty() && _cUserProfile.getCContactList().findContactThatOwns(lowerContact).empty())//VOXOX - CJC - 2009.07.30 Validate that the message only show if the contact is not present, if we allready have the contact, lets just autorized it
		{
			//VOXOX - CJC - 2009.06.10 
			QtVoxMessageBox box(_qtWengoPhone.getWidget());//VOXOX - CJC - 2009.06.23 
			box.setWindowTitle("VoxOx - Authorization Request");
			box.setText(request);
			box.addButton(tr("&Accept"), QMessageBox::AcceptRole);
			box.addButton(tr("&Deny"), QMessageBox::RejectRole);
			int buttonClicked = box.exec();

			if (buttonClicked == QMessageBox::AcceptRole) 
			{
					//TODO: give a personal message

				{
					//If the contact is not in our ContactList
					ContactProfile contactProfile;
					contactProfile.addIMContact(imContact);
					/*QtProfileDetails qtProfileDetails(_cUserProfile, contactProfile, _qtWengoPhone.getWidget(), tr("Edit Contact"));
					if (qtProfileDetails.show()) {*/
						_cUserProfile.getCContactList().addContact(contactProfile);
					//}
				}

				sender->authorizeContact(imContact, true, String::null);
			} 
			else 
			{
				//TODO: give a personal message
				//TODO: avoid direct access to model (as we are in the GUI thread)
				sender->authorizeContact(imContact, false, String::null);
			}
		}
		else	//VOXOX - CJC - 2009.07.30 If contact is already on the list, then it should be accepted already.  Just authorize (Jabber authorization method)
		{
			sender->authorizeContact(imContact, true, String::null);	
		}

		OWSAFE_DELETE(imAccount);
	} 
	else 
	{
		LOG_ERROR("IMAccount not found!");
	}
}
