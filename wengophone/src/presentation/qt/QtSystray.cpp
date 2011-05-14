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
#include "QtSystray.h"

#include "QtToolBar.h"
#include "QtWengoPhone.h"
#include "QtPresenceMenuManager.h"
#include "QtPresencePixmapHelper.h"
#include "QtIMAccountMonitor.h"
#include "QtNoWengoAlert.h"
//#include "contactlist/QtContactListManager.h"	//VOXOX - JRT - 2009.04.15 - no longer needed.
#include "contactlist/QtContactMenu.h"
#include "webservices/sms/QtSms.h"
#include "history/QtHistoryWidget.h"


#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/network/NetworkObserver.h>
#include <model/profile/UserProfile.h>

#include <cutil/global.h>
#include <util/Logger.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/LanguageChangeEventFilter.h>

#include <QtGui/QtGui>

#include <trayicon.h>

#if defined(OS_MACOSX)
	#include "macosx/QtMacApplication.h"
#endif

#ifdef OS_WINDOWS
	#include <windows.h>
#endif

static const char* SYSTRAY_ICON_TEMPLATE=":/pics/systray/%1.png";
static const int SYSTRAY_ICON_CENTERX=8;
static const int SYSTRAY_ICON_CENTERY=9;
static const int SYSTRAY_ICON_RADIUS=16;

QtSystray::QtSystray(QObject * parent)
	: QObjectThreadSafe(NULL) {

//	_qtWengoPhone = (QtWengoPhone *) parent;	//VOXOX - JRT - 2009.04.15 
	_qtWengoPhone = dynamic_cast<QtWengoPhone*>(parent);
	_qtToolBar = dynamic_cast<QtToolBar*>(_qtWengoPhone->getQtToolBar());

	_recentContacts = NULL;
	_startNew = NULL;

	//Check Internet connection status
	NetworkObserver::getInstance().connectionIsDownEvent +=
		boost::bind(&QtSystray::connectionIsDownEventHandler, this);

	NetworkObserver::getInstance().connectionIsUpEvent +=
		boost::bind(&QtSystray::connectionIsUpEventHandler, this);

	//trayMenu
	_trayMenu = new QMenu(_qtWengoPhone->getWidget());
	SAFE_CONNECT(_trayMenu, SIGNAL(aboutToShow()), SLOT(setTrayMenu()));

	//trayIcon
	_trayIcon = new TrayIcon(QPixmap(":pics/systray/voxox.png"), tr("VoxOx"), _trayMenu, _qtWengoPhone->getWidget());

#if defined(OS_LINUX)
	// On Linux, the main window is shown using only one click
	SAFE_CONNECT(_trayIcon, SIGNAL(clicked( const QPoint&, int)), SLOT(toggleMainWindow()));
#else
	SAFE_CONNECT(_trayIcon, SIGNAL(doubleClicked(const QPoint &)), SLOT(toggleMainWindow()));
#endif
	phoneLineStateChanged(EnumPhoneLineState::PhoneLineStateProgress);
	_trayIcon->show();

	//systray re-initialization
	SAFE_CONNECT(_qtWengoPhone, SIGNAL(userProfileDeleted()), SLOT(userProfileDeleted()));

	setTrayMenu();

	LANGUAGE_CHANGE(_trayIcon);
}

QtSystray::~QtSystray() {
}

void QtSystray::setTrayMenu() {
	_trayMenu->clear();

	//openAction
#if defined(OS_MACOSX)
	// On Mac OS X, clicking the Dock icon should show the application thus the
	// 'Open WengoPhone' entry is not necessary. We get the Dock Icon click event
	// from our QtMacApplication class.
	QtMacApplication * qMacApp = dynamic_cast<QtMacApplication *>(QApplication::instance());
	SAFE_CONNECT_RECEIVER(qMacApp, SIGNAL(applicationMustShow()), _qtWengoPhone->getWidget(), SLOT(show()));
#endif
	_startNew = new QMenu(tr("Start New"));
	
	_newCall = new QAction(tr("Call"), _startNew);
	_newConference = new QAction(tr("Conference Call"), _startNew);
	//_newInstantMessage = new QMenu(tr("Instant Message"));
	_newInstantMessage = new QAction(tr("Instant Message"), _startNew);
	_newGroupChat = new QAction(tr("Group Chat"), _startNew);
	_newTextMessage = new QAction(tr("Text Message"), _startNew);
	_newGroupTextMessage = new QAction(tr("Group Text Message"), _startNew);
	_newEmail = new QAction(tr("Email"), _startNew);
	_newGroupEmail = new QAction(tr("Group Email"), _startNew);
	_newSendFile = new QAction(tr("Send File"), _startNew);
	_newSendFax = new QAction(tr("Send Fax"), _startNew);

	SAFE_CONNECT_RECEIVER(_newCall, SIGNAL(triggered()), _qtToolBar, SLOT(showKeyTab()));
	SAFE_CONNECT_RECEIVER(_newConference, SIGNAL(triggered()), _qtToolBar, SLOT(createConferenceCall()));
	/*_newInstantMessage->clear();
	SAFE_CONNECT(_newInstantMessage, SIGNAL(triggered(QAction *)), SLOT(startChat(QAction *)));
	QtContactMenu::populateChatMenu(_newInstantMessage, _qtWengoPhone->getCWengoPhone());*/
	_newInstantMessage->setDisabled(true);
	_newGroupChat->setDisabled(true);
	_newTextMessage->setDisabled(true);
	_newGroupTextMessage->setDisabled(true);
	_newEmail->setDisabled(true);
	_newGroupEmail->setDisabled(true);
	_newSendFile->setDisabled(true);
	_newSendFax->setDisabled(true);
	
	_startNew->addAction(_newCall);
	_startNew->addAction(_newConference);
	_startNew->addAction(_newInstantMessage);
	//_startNew->addMenu(_newInstantMessage);
	_startNew->addAction(_newGroupChat);
	_startNew->addAction(_newTextMessage);
	_startNew->addAction(_newGroupTextMessage);
	_startNew->addAction(_newEmail);
	_startNew->addAction(_newGroupEmail);
	_startNew->addAction(_newSendFile);
	_startNew->addAction(_newSendFax);	

	_recentContacts = new QMenu(tr("Recent Contacts"));
	_viewConversations = new QAction(tr("View Conversations"), _trayMenu);
	_showContactList = new QAction(tr("Show Contact List"), _trayMenu);
	_settings = new QAction(tr("Settings"), _trayMenu);
	_signOut = new QAction(tr("Sign Out"), _trayMenu);
	_quit = new QAction(tr("Quit @product@"), _trayMenu);
	
	_recentContacts->setDisabled(true);
	SAFE_CONNECT_RECEIVER(_viewConversations, SIGNAL(triggered()), _qtToolBar, SLOT(showHistory()));
	SAFE_CONNECT_RECEIVER(_showContactList, SIGNAL(triggered()), _qtToolBar, SLOT(showContactsTab()));
	SAFE_CONNECT_RECEIVER(_settings, SIGNAL(triggered()), _qtToolBar, SLOT(showConfig()));
//	SAFE_CONNECT_RECEIVER(_signOut, SIGNAL(triggered()), _qtToolBar, SLOT(logOff()));
//	SAFE_CONNECT_RECEIVER(_quit, SIGNAL(triggered()), _qtWengoPhone, SLOT(prepareToExitApplication()));	
	SAFE_CONNECT_RECEIVER(_signOut, SIGNAL(triggered()), _qtWengoPhone, SLOT(logOff()));					//VOXOX - JRT - 2009.07.13 
	SAFE_CONNECT_RECEIVER(_quit,	SIGNAL(triggered()), _qtWengoPhone, SLOT(exitApplication()));			//VOXOX - JRT - 2009.07.13 
	
	if(!_qtToolBar->getUserIsLogged()){
		_startNew->setDisabled(true);
		_viewConversations->setDisabled(true);
		_showContactList->setDisabled(true);
		_settings->setDisabled(true);
		_signOut->setDisabled(true);
	}

	_trayMenu->addMenu(_startNew);
	_trayMenu->addMenu(_recentContacts);
	_trayMenu->addAction(_viewConversations);
	_trayMenu->addAction(_showContactList);
	if ( getCUserProfile() ) {
		_trayMenu->addMenu(createStatusMenu());
	}
	_trayMenu->addAction(_settings);
	_trayMenu->addAction(_signOut);
	_trayMenu->addAction(_quit);

	_trayIcon->setPopup(_trayMenu);
}

QMenu * QtSystray::createStatusMenu() {
	CWengoPhone & cWengoPhone = _qtWengoPhone->getCWengoPhone();
	CUserProfile * cUserProfile = getCUserProfile();

	QMenu * menu = new QMenu(_qtWengoPhone->getWidget());
	menu->setTitle(tr("Global Status"));

	QtPresenceMenuManager * manager = new QtPresenceMenuManager(menu, cUserProfile,_qtWengoPhone);
	manager->addPresenceActions(menu);

	return menu;
}

void QtSystray::updateMenu() {
	CWengoPhone & cWengoPhone = _qtWengoPhone->getCWengoPhone();

#ifndef DISABLE_SMS
	//sendSmsMenu
	QMenu * sendSmsMenu = new QMenu(_qtWengoPhone->getWidget());
	sendSmsMenu->setTitle(tr("Send a SMS"));
	sendSmsMenu->setIcon(QIcon(":/pics/contact/sms.png"));
	SAFE_CONNECT(sendSmsMenu, SIGNAL(triggered(QAction *)), SLOT(sendSms(QAction *)));
	sendSmsMenu->clear();

	QAction * sendSmsBlankAction = sendSmsMenu->addAction(QIcon(":/pics/contact/sms.png"), tr("Send SMS"));
	sendSmsBlankAction->setData(String::null);
	_trayMenu->addMenu(sendSmsMenu);
	QtContactMenu::populateMobilePhoneMenu(sendSmsMenu, cWengoPhone);
	///
#endif
	////startChatMenu
	//QMenu * startChatMenu = new QMenu(_qtWengoPhone->getWidget());
	//startChatMenu->setIcon(QIcon(":/pics/contact/chat.png"));
	//SAFE_CONNECT(startChatMenu, SIGNAL(triggered(QAction *)), SLOT(startChat(QAction *)));
	//startChatMenu->clear();
	//startChatMenu->setTitle(tr("Start a chat"));
	//_trayMenu->addMenu(startChatMenu);
	//QtContactMenu::populateChatMenu(startChatMenu, cWengoPhone);
	/////

	////call menu
	//QAction * placeCallBlankAction = _callMenu->addAction(QIcon(":/pics/contact/call.png"), tr("Place Call"));
	//placeCallBlankAction->setData(String::null);

	////callWengoMenu
	//QMenu * callWengoMenu = new QMenu(_qtWengoPhone->getWidget());
	//callWengoMenu->setTitle(tr("Call @company@"));
	//SAFE_CONNECT(callWengoMenu, SIGNAL(triggered(QAction *)), SLOT(makeCall(QAction *)));
	//callWengoMenu->clear();
	//_callMenu->addMenu(callWengoMenu);
	//QtContactMenu::populateWengoCallMenu(callWengoMenu, cWengoPhone);
	/////

	////callMobileMenu
	//QMenu * callMobileMenu = new QMenu(_qtWengoPhone->getWidget());
	//callMobileMenu->setTitle(tr("Call Mobile"));
	//SAFE_CONNECT(callMobileMenu, SIGNAL(triggered(QAction *)), SLOT(makeCall(QAction *)));
	//callMobileMenu->clear();
	//_callMenu->addMenu(callMobileMenu);
	//QtContactMenu::populateMobilePhoneMenu(callMobileMenu, cWengoPhone);
	/////

	////callLandlineMenu
	//QMenu * callLandlineMenu = new QMenu(_qtWengoPhone->getWidget());
	//callLandlineMenu->setTitle(tr("Call land line"));
	//SAFE_CONNECT(callLandlineMenu, SIGNAL(triggered(QAction *)), SLOT(makeCall(QAction *)));
	//callLandlineMenu->clear();
	//_callMenu->addMenu(callLandlineMenu);
	//QtContactMenu::populateHomePhoneMenu(callLandlineMenu, cWengoPhone);
	/////

	////callSIPMenu
	//QMenu * callSIPMenu = new QMenu(_qtWengoPhone->getWidget());
	//callSIPMenu->setTitle(tr("Call SIP"));
	//SAFE_CONNECT(callSIPMenu, SIGNAL(triggered(QAction *)), SLOT(makeCall(QAction *)));
	//callSIPMenu->clear();
	//_callMenu->addMenu(callSIPMenu);
	//QtContactMenu::populateSipCallMenu(callSIPMenu, cWengoPhone);
	/////
	//
	////sendFileMenu
	//QMenu * sendFileMenu = new QMenu(_qtWengoPhone->getWidget());
	//sendFileMenu->setTitle(tr("Send File"));
	//sendFileMenu->setIcon(QIcon(":/pics/actions/send_file.png"));
	//SAFE_CONNECT(sendFileMenu, SIGNAL(triggered(QAction *)), SLOT(sendFile(QAction *)));
	//sendFileMenu->clear();
	//_trayMenu->addMenu(sendFileMenu);
	////TODO: call a new method populateFreeCallMenu with contactId
	//QtContactMenu::populateWengoUsersContactId(sendFileMenu, cWengoPhone);
	/////
}

void QtSystray::updateSystrayIcon() {
	//VOXOX - JRT - 2009.04.15 
//	CWengoPhone & cWengoPhone = _qtWengoPhone->getCWengoPhone();
//	CUserProfile * cUserProfile = cWengoPhone.getCUserProfileHandler().getCUserProfile();
	//CUserProfile * cUserProfile = getCUserProfile();

	//if(cUserProfile)
	//{
	//	//VOXOXO CHANGE CJC WE DONT NEED THIS ANY MORE
	//	/*IMAccountList accountList = cUserProfile->getUserProfile().getIMAccountManager().getIMAccountListCopy();
	//	QtPresencePixmapHelper helper(accountList);
	//	QPixmap icon = helper.createPixmap(SYSTRAY_ICON_TEMPLATE, 
	//		SYSTRAY_ICON_CENTERX, 
	//		SYSTRAY_ICON_CENTERY,
	//		SYSTRAY_ICON_RADIUS);*/
	//	

	//	EnumPresenceState::PresenceState presenceState = cUserProfile->getUserProfile().getPresenceState();

	//	switch (presenceState) {
	//	case EnumPresenceState::PresenceStateAway:
	//		_trayIcon->setToolTip(tr("@product@") + " - " + tr("Away"));
	//		_trayIcon->setIcon(QPixmap(":/pics/systray/away.png"));
	//		break;
	//	case EnumPresenceState::PresenceStateOnline:
	//		_trayIcon->setToolTip(tr("@product@") + " - " + tr("Online"));
	//		_trayIcon->setIcon(QPixmap(":/pics/systray/online.png"));
	//		break;
	//	case EnumPresenceState::PresenceStateOffline:
	//		_trayIcon->setToolTip(tr("@product@") + " - " + tr("Offline"));
	//		_trayIcon->setIcon(QPixmap(":/pics/systray/offline.png"));
	//		break;
	//	case EnumPresenceState::PresenceStateInvisible:
	//		_trayIcon->setToolTip(tr("@product@") + " - " + tr("Invisible"));
	//		_trayIcon->setIcon(QPixmap(":/pics/systray/invisible.png"));
	//		break;
	//	case EnumPresenceState::PresenceStateDoNotDisturb:
	//		_trayIcon->setToolTip(tr("@product@") + " - " + tr("Do Not Disturb"));
	//		_trayIcon->setIcon(QPixmap(":/pics/systray/donotdisturb.png"));
	//		break;
	//	case EnumPresenceState::PresenceStateMulti:
	//		_trayIcon->setToolTip(tr("@product@") + " - " + tr("Multiple Status"));
	//		//VOXOX CHANGE CJC TODO: I DONT KNOW WHAT TO DO WITH MULTIPLE STATUS SO USING ONLINE IMAGE
	//		_trayIcon->setIcon(QPixmap(":/pics/systray/online.png"));
	//		break;
	//	case EnumPresenceState::PresenceStateUnknown:
	//		_trayIcon->setToolTip(tr("@product@") + " - " + tr("Unknown Status"));
	//		break;
	//	default:
	//		LOG_FATAL("unknown presenceState=" + String::fromNumber(presenceState));
	//		break;
	//	}
	//}
}

void QtSystray::userProfileDeleted() {
	connectionStateEventHandlerThreadSafe(false);
}

void QtSystray::phoneLineStateChanged(EnumPhoneLineState::PhoneLineState state) {
	/*bool connected = false;

	switch (state) {
	case EnumPhoneLineState::PhoneLineStateUnknown:
		break;
	case EnumPhoneLineState::PhoneLineStateServerError:
		break;
	case EnumPhoneLineState::PhoneLineStateTimeout:
		break;
	case EnumPhoneLineState::PhoneLineStateOk:
		connected = true;
		break;
	case EnumPhoneLineState::PhoneLineStateClosed:
		break;
	case EnumPhoneLineState::PhoneLineStateProgress:
		_trayIcon->setIcon(QPixmap(":/pics/systray/connecting.png"));
		_trayIcon->setToolTip(tr("@product@") + " - " + tr("Connecting..."));
		return;
	default:
		LOG_FATAL("unknown state=" + EnumPhoneLineState::toString(state));
	};*/

	//connectionStateEventHandlerThreadSafe(connected);
}

void QtSystray::connectionIsDownEventHandler() {
	//typedef PostEvent1<void (bool), bool> MyPostEvent;
	//MyPostEvent * event = new MyPostEvent(boost::bind(&QtSystray::connectionStateEventHandlerThreadSafe, this, _1), false);
	//FIXME Replaced by phoneLineStateChanged()
	//postEvent(event);
}

void QtSystray::connectionIsUpEventHandler() {
	//typedef PostEvent1<void (bool), bool> MyPostEvent;
	//MyPostEvent * event = new MyPostEvent(boost::bind(&QtSystray::connectionStateEventHandlerThreadSafe, this, _1), true);
	//FIXME Replaced by phoneLineStateChanged()
	//postEvent(event);
}

void QtSystray::connectionStateEventHandlerThreadSafe(bool connected) {
	//if (connected) 
	//{
	//	//_trayIcon->setIcon(QPixmap(":/pics/systray/connecting.png"));
	//	//_trayIcon->setToolTip(tr("@product@") + " - " + tr("Connecting..."));
	//	//updateSystrayIcon();
	//} 
	//else {
	//	_trayIcon->setIcon(QPixmap(":/pics/systray/disconnected.png"));
	//	_trayIcon->setToolTip(tr("@product@") + " - " + tr("Internet Connection Error"));
	//}
}

void QtSystray::toggleMainWindow() {
	// If the wengophone is hidden or minimized, show & raise it, otherwise hide it
	//if (_qtWengoPhone->getWidget()->isHidden() || _qtWengoPhone->getWidget()->isMinimized()) {
	//	_qtWengoPhone->getWidget()->showNormal();
	//	_qtWengoPhone->getWidget()->activateWindow();//VOXOX - CJC - 2009.07.08 
	//	_qtWengoPhone->getWidget()->raise();
	//}else if(!_qtWengoPhone->getWidget()->isHidden() && !_qtWengoPhone->getWidget()->isMinimized() && !_qtWengoPhone->getWidget()->isActiveWindow()){
	//	_qtWengoPhone->getWidget()->showNormal();
	//	_qtWengoPhone->getWidget()->activateWindow();//VOXOX - CJC - 2009.07.08 
	//	_qtWengoPhone->getWidget()->raise();
	//}
	/*if(!_qtWengoPhone->getWidget()->isHidden() || !_qtWengoPhone->getWidget()->isMinimized()) {
		_qtWengoPhone->getWidget()->hide();
	}else{*/
		_qtWengoPhone->getWidget()->showNormal();
		_qtWengoPhone->getWidget()->activateWindow();//VOXOX - CJC - 2009.07.08 
		_qtWengoPhone->getWidget()->raise();
	//}
}

void QtSystray::hide() {
	_trayIcon->hide();
}

void QtSystray::makeCall(QAction * action) {
	CWengoPhone & cWengoPhone = _qtWengoPhone->getCWengoPhone();
	if (action) {
		LOG_DEBUG("call=" + action->data().toString().toStdString());
//		cWengoPhone.getCUserProfileHandler().getCUserProfile()->makeCall(action->data().toString().toStdString());	//VOXOX - JRT - 2009.04.15 
		getCUserProfile()->makeCall(action->data().toString().toStdString());
	} else {
		LOG_FATAL("QAction cannot be NULL");
	}
}

void QtSystray::startChat(QAction * action) {
	if (action) {
//		QtContactListManager * contactListManager = QtContactListManager::getInstance();	//VOXOX - JRT - 2009.04.15 
//		if (contactListManager) {
			LOG_DEBUG("chat=" + action->data().toString().toStdString());
//			contactListManager->startChat(action->data().toString());	//VOXOX - JRT - 2009.04.15 
			getCUserProfile()->startIM( action->data().toString().toStdString() );
//		} else {
//			LOG_FATAL("QtContactListManager cannot be NULL");
//		}
	} else {
		LOG_FATAL("QAction cannot be NULL");
	}
}

void QtSystray::sendSms(QAction * action) {
////	CWengoPhone & cWengoPhone = _qtWengoPhone->getCWengoPhone();		//VOXOX - JRT - 2009.04.15 
//	if (action) {
////		CUserProfile * cuserprofile = cWengoPhone.getCUserProfileHandler().getCUserProfile();	//VOXOX - JRT - 2009.04.15 
//		CUserProfile * cuserprofile = getCUserProfile();
//		if (cuserprofile) {
//			if (_qtWengoPhone->getQtSms() &&
//				cuserprofile->getUserProfile().hasWengoAccount() &&
//				cuserprofile->getUserProfile().getWengoAccount()->isConnected()) {
//
//				_qtWengoPhone->getQtSms()->getWidget()->show();
//				_qtWengoPhone->getWidget()->show();
//				_qtWengoPhone->getWidget()->setWindowState(_qtWengoPhone->getWidget()->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
//				_qtWengoPhone->getQtSms()->setPhoneNumber(action->data().toString());
//			} else {
//				QtNoWengoAlert noWengoAlert(NULL, *_qtWengoPhone);
//				noWengoAlert.getQDialog()->exec();
//			}
//		}
//	} else {
//		LOG_FATAL("QAction cannot be NULL");
//	}
}

void QtSystray::sendFile(QAction * action) {
//	if (action) {
////		QtContactListManager * contactListManager = QtContactListManager::getInstance();	//VOXOX - JRT - 2009.04.15 
////		if (contactListManager) {
//			LOG_DEBUG("sendfile=" + action->data().toString().toStdString());
////			contactListManager->sendFile(action->data().toString());	//VOXOX - JRT - 2009.04.15 
//			getCUserProfile()->sendFile( action->data().toString().toStdString() );
////		}
//	} else {
//		LOG_FATAL("QAction cannot be NULL");
//	}
}

void QtSystray::languageChanged() {
	updateMenu();
}

//VOXOX - JRT - 2009.04.15 - Shortcut to access business logic. Provides easier maintenance and readability.
CUserProfile* QtSystray::getCUserProfile()
{
	return _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
	//return cWengoPhone.getCUserProfileHandler().getCUserProfile();
}
