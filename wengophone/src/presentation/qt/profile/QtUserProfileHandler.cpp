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
#include "QtUserProfileHandler.h"

#include <control/profile/CUserProfileHandler.h>

#include <presentation/PFactory.h>
#include <presentation/qt/QtLanguage.h>
#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/login/QtLoginDialog.h>
#include <presentation/qt/QtSystray.h>
#include <presentation/qt/statusbar/QtStatusBar.h>
#include <presentation/qt/login/QtVoxOxLoginMessage.h>
#include <presentation/qt/login/QtVoxOxLogin.h>

#include <qtutil/QtThreadEvent.h>//VOXOX CHANGE by Rolando - 2009.09.11 
#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtCore/QTimer>

QtUserProfileHandler::QtUserProfileHandler(CUserProfileHandler & cUserProfileHandler,
	QtWengoPhone & qtWengoPhone)
	: QObject(&qtWengoPhone),
	_cUserProfileHandler(cUserProfileHandler),
	_qtWengoPhone(qtWengoPhone),_qtLoginDialog(0) {

	//Connection for UserProfile change
	SAFE_CONNECT_TYPE(this, SIGNAL(setCurrentUserProfileEventHandlerSignal()),     SLOT(setCurrentUserProfileEventHandlerSlot()),	  Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(noCurrentUserProfileSetEventHandlerSignal()),   SLOT(noCurrentUserProfileSetEventHandlerSlot()),   Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(currentUserProfileWillDieEventHandlerSignal()), SLOT(currentUserProfileWillDieEventHandlerSlot()), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(userProfileInitializedEventHandlerSignal()),    SLOT(userProfileInitializedEventHandlerSlot()),    Qt::QueuedConnection);

	qRegisterMetaType<SipAccount>("SipAccount");
	qRegisterMetaType<WengoAccount>("WengoAccount");
	qRegisterMetaType<EnumSipLoginState::SipLoginState>("EnumSipLoginState::SipLoginState");

	//VOXOX CHANGE by Rolando - 2009.09.10 
	//SAFE_CONNECT_TYPE(this, SIGNAL(sipAccountConnectionFailedEventHandlerSignal(SipAccount*, EnumSipLoginState::SipLoginState, std::string)), SLOT(sipAccountConnectionFailedEventHandlerSlot(SipAccount*, EnumSipLoginState::SipLoginState, std::string)), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(profileLoadedFromBackupsEventHandlerSignal(QString)), 	SLOT(profileLoadedFromBackupsEventHandlerSlot(QString)), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(profileCannotBeLoadedEventHandlerSignal(QString)),		SLOT(profileCannotBeLoadedEventHandlerSlot(QString)),	 Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(sipNetworkErrorEventHandlerSignal(SipAccount*)),			SLOT(sipNetworkErrorEventHandlerSlot(SipAccount*)),		 Qt::QueuedConnection);
}

void QtUserProfileHandler::updatePresentation() {
}

void QtUserProfileHandler::noCurrentUserProfileSetEventHandler() {
	noCurrentUserProfileSetEventHandlerSignal();
}

void QtUserProfileHandler::currentUserProfileWillDieEventHandler() {
	currentUserProfileWillDieEventHandlerSignal();
}

void QtUserProfileHandler::userProfileInitializedEventHandler() {
	userProfileInitializedEventHandlerSignal();
}

//VOXOX CHANGE by Rolando - 2009.09.10 
void QtUserProfileHandler::sipAccountConnectionFailedEventHandler(const SipAccount & sipAccount, EnumSipLoginState::SipLoginState state, std::string loginErrorMessage) {
	// We need to clone the SipAccount because we don't want to pass a
	// reference of the original, and doing a simple copy would produce a
	// SipAccount instance, even if the reference is in fact a WengoAccount.
	//VOXOX CHANGE by Rolando - 2009.09.11 sipAccountConnectionFailedEventHandlerSignal(sipAccount.clone(), state, loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
	//VOXOX CHANGE by Rolando - 2009.09.11 sipAccountConnectionFailedEventHandlerSlot(sipAccount.clone(), state, loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.11 

	typedef ThreadEvent3<void (SipAccount *, EnumSipLoginState::SipLoginState, std::string), SipAccount *, EnumSipLoginState::SipLoginState, std::string> MyThreadEvent;
		MyThreadEvent * event = new MyThreadEvent(boost::bind(&QtUserProfileHandler::sipAccountConnectionFailedEventHandlerThreadSafe, this, _1, _2, _3), sipAccount.clone(), state, loginErrorMessage);
		PFactory::postEvent(event);
}

void QtUserProfileHandler::profileLoadedFromBackupsEventHandler(std::string profileName) {
	profileLoadedFromBackupsEventHandlerSignal(QString::fromStdString(profileName));
}

void QtUserProfileHandler::profileCannotBeLoadedEventHandler(std::string profileName) {
	profileCannotBeLoadedEventHandlerSignal(QString::fromStdString(profileName));
}

void QtUserProfileHandler::networkErrorEventHandler(const SipAccount & sipAccount) {
	sipNetworkErrorEventHandlerSignal(sipAccount.clone());
}

void QtUserProfileHandler::noCurrentUserProfileSetEventHandlerSlot() {
	_qtWengoPhone.uninstallQtBrowserWidget();
	showLoginWindow();
}

void QtUserProfileHandler::setCurrentUserProfileEventHandler() {
	setCurrentUserProfileEventHandlerSignal();
}

void QtUserProfileHandler::setCurrentUserProfileEventHandlerSlot() {
	_qtWengoPhone.setCurrentUserProfileEventHandlerSlot();
}

void QtUserProfileHandler::currentUserProfileWillDieEventHandlerSlot() {
	//if(!_qtLoginDialog)//VOXOX CHANGE by Rolando 2009.05.05, commented this sentences because we don't use qtLoginDialog now
	//{
	//	_qtWengoPhone.currentUserProfileWillDieEventHandlerSlot();
	//	_cUserProfileHandler.currentUserProfileReleased();

	//// For some reason, if we call showLoginWindow directly, the CPU is at 100%
	//// on MacOS X. Delaying showLoginWindow with a timer solves it.
	//	QTimer::singleShot(0, this, SLOT(showLoginWindow()));
	//}

	_qtWengoPhone.currentUserProfileWillDieEventHandlerSlot();
	_cUserProfileHandler.currentUserProfileReleased();

	// For some reason, if we call showLoginWindow directly, the CPU is at 100%
	// on MacOS X. Delaying showLoginWindow with a timer solves it.
	QTimer::singleShot(0, this, SLOT(showLoginWindow()));
	
}

void QtUserProfileHandler::userProfileInitializedEventHandlerSlot() {
	_qtWengoPhone.userProfileInitializedEventHandlerSlot();
}

void QtUserProfileHandler::profileLoadedFromBackupsEventHandlerSlot(QString profileName) {
	//VOXOX CHANGE by Rolando 04-06-09, deactivated message 

	/*QMessageBox::warning(_qtWengoPhone.getWidget(), tr("@product@"),
		tr("A problem occured while loading your profile.\n"
			"The last backuped profile has been loaded: \n"
			"you may have lost last changes made"),
		QMessageBox::Ok, QMessageBox::NoButton);*/

	//VOXOX CHANGE by Rolando 04-06-09, added to log file
	LOG_DEBUG(QString("error: A problem occured while loading your profile: %1").arg(profileName).toStdString());
}

void QtUserProfileHandler::profileCannotBeLoadedEventHandlerSlot(QString profileName) {
	//VOXOX CHANGE by Rolando 04-06-09, message deactivated

	/*QMessageBox::warning(_qtWengoPhone.getWidget(), tr("@product@"),
		tr("Your profile could not be loaded.\n"
			"Choose another profile or create a new one."),
		QMessageBox::Ok, QMessageBox::NoButton);*/

	//VOXOX CHANGE by Rolando 04-06-09, added to log file
	LOG_DEBUG(QString("error: Your profile: %1, could not be loaded.").arg(profileName).toStdString());
}

void QtUserProfileHandler::showLoginWindow() {

	//if(!_qtLoginDialog)//VOXOX CHANGE by Rolando 2009.05.05, commented this sentences because we don't use qtLoginDialog now
	//{
	//	if(!_qtLoginDialog)
	//	_qtLoginDialog = new QtLoginDialog(&_qtWengoPhone, _cUserProfileHandler);
	//	if (_qtLoginDialog->exec() == QDialog::Accepted) {
	//		_qtWengoPhone.installQtBrowserWidget();
	//		//VOXOX CHANGE CJC WE DONT NEED THIS ANYMORE FOR STATUS BAR
	//		//_qtWengoPhone.getQtStatusBar().updatePhoneLineState(EnumPhoneLineState::PhoneLineStateProgress);
	//		_qtWengoPhone.getQtSystray().phoneLineStateChanged(EnumPhoneLineState::PhoneLineStateProgress);
	//	}
	//	delete _qtLoginDialog;
	//	_qtLoginDialog = 0;
	//}

	_qtWengoPhone.showLoginWindow();//VOXOX CHANGE by Rolando 2009.05.05, we now show Login Window inside qtwengophone window
	_qtWengoPhone.installQtBrowserWidget();
	//VOXOX CHANGE CJC WE DONT NEED THIS ANYMORE FOR STATUS BAR
	//_qtWengoPhone.getQtStatusBar().updatePhoneLineState(EnumPhoneLineState::PhoneLineStateProgress);
	_qtWengoPhone.getQtSystray().phoneLineStateChanged(EnumPhoneLineState::PhoneLineStateProgress);
	

}

void QtUserProfileHandler::showLoginWindowWithWengoAccount(const WengoAccount & wengoAccount) {

	//if(!_qtLoginDialog )//VOXOX CHANGE by Rolando 2009.05.05, commented this sentences because we don't use qtLoginDialog now
	//{
	//	_qtLoginDialog = new QtLoginDialog(&_qtWengoPhone, _cUserProfileHandler);

	//	_qtLoginDialog->setValidAccount(wengoAccount);
	//	if (_qtLoginDialog->exec() == QDialog::Accepted) {
	//		_qtWengoPhone.installQtBrowserWidget();
	//		//VOXOX CHANGE CJC WE DONT NEED THIS ANYMORE FOR STATUS BAR
	//		//_qtWengoPhone.getQtStatusBar().updatePhoneLineState(EnumPhoneLineState::PhoneLineStateProgress);
	//		_qtWengoPhone.getQtSystray().phoneLineStateChanged(EnumPhoneLineState::PhoneLineStateProgress);
	//	}
	//	delete _qtLoginDialog;
	//	_qtLoginDialog = 0;
	//}
	_qtWengoPhone.showLoginWindow();//VOXOX CHANGE by Rolando 2009.05.05, we now show Login Window inside qtwengophone window
	_qtWengoPhone.getLoginWindow()->load(wengoAccount);

	_qtWengoPhone.installQtBrowserWidget();
	//VOXOX CHANGE CJC WE DONT NEED THIS ANYMORE FOR STATUS BAR
	//_qtWengoPhone.getQtStatusBar().updatePhoneLineState(EnumPhoneLineState::PhoneLineStateProgress);
	_qtWengoPhone.getQtSystray().phoneLineStateChanged(EnumPhoneLineState::PhoneLineStateProgress);
		

}

void QtUserProfileHandler::sipNetworkErrorEventHandlerSlot(SipAccount* sipAccount) {
	/*_qtWengoPhone.uninstallQtBrowserWidget();

	QtLoginDialog dlg(&_qtWengoPhone, _cUserProfileHandler);
	dlg.setInvalidAccount(*sipAccount);
	dlg.setErrorMessage(tr("Network error."));
	if (dlg.exec() == QDialog::Accepted) {
		_qtWengoPhone.installQtBrowserWidget();
	}
	delete sipAccount;*/
}

//VOXOX CHANGE by Rolando - 2009.09.10 
//void QtUserProfileHandler::sipAccountConnectionFailedEventHandlerSlot(SipAccount* sipAccount, EnumSipLoginState::SipLoginState state, std::string loginErrorMessage) {
//	LOG_DEBUG("error: %d", state);
//	// FIXME: Why do we do different things depending on the account type?
//	if (sipAccount->getType() == SipAccount::SipAccountTypeWengo) {
//		_qtWengoPhone.uninstallQtBrowserWidget();
//	} else {
//		_qtWengoPhone.currentUserProfileWillDieEventHandlerSlot();
//		_cUserProfileHandler.currentUserProfileReleased();
//	}
//
//	const WengoAccount * wengoAccount = dynamic_cast<const WengoAccount *>(sipAccount);
//	_qtWengoPhone.showLoginMessageWindow();
//	_qtWengoPhone.getLoginMessageWindow()->setProfileName(QString::fromStdString(wengoAccount->getWengoLogin()));
//
//	QString message;
//	if (state == EnumSipLoginState::SipLoginStatePasswordError) {
//		
//		message = tr("Sorry, the login you entered is incorrect.");//VOXOX CHANGE by Rolando 03-02-09, new Message
//		_qtWengoPhone.getLoginMessageWindow()->setAdviceMessage(tr("Please make sure your login information is correct\nand then sign in again."));//VOXOX CHANGE by Rolando 03-02-09, new Message
//
//	} else if (state == EnumSipLoginState::SipLoginStateNetworkError) {
//		message = tr("Connection error");
//		_qtWengoPhone.getLoginMessageWindow()->setAdviceMessage(tr("Please check your network settings.\nThen Try Again"));//VOXOX CHANGE by Rolando 03-02-09, new Message
//		
//	} else if (state == EnumSipLoginState::SipLoginStateUnActivatedError) {//VOXOX CHANGE by Rolando - 2009.05.22 - added SipLoginStateUnActivatedError state to handle error when user has an account not activated
//		message = tr("Sorry, you must verify your email address first.");
//		_qtWengoPhone.getLoginMessageWindow()->setAdviceMessage(tr("Please check your inbox and click the verification link\nthat was sent to you after you completed the sign up form."));//VOXOX CHANGE by Rolando - 2009.05.22, new Message
//		
//	} else if (state == EnumSipLoginState::SipLoginStateOtherError) {//VOXOX CHANGE by Rolando - 2009.05.22 - added SipLoginStateUnActivatedError state to handle error when user has an account not activated
//		message = QString::fromStdString(loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.11 - handling custom error messages
//		_qtWengoPhone.getLoginMessageWindow()->setAdviceMessage(tr("VoxOx Message:"));//VOXOX CHANGE by Rolando - 2009.09.11 
//		
//	} else{
//		message = tr("Unexpected error (code: %1)").arg(int(state));
//		_qtWengoPhone.getLoginMessageWindow()->setAdviceMessage(QString("Please check your network settings.\nThen Try Again"));
//	}
//	//VOXOX CHANGE by Rolando 2009.05.05, we now show Login Message Window inside qtwengophone window
//	_qtWengoPhone.getLoginMessageWindow()->setErrorMessage(message);
//	_qtWengoPhone.installQtBrowserWidget();
//	_qtWengoPhone.getQtSystray().phoneLineStateChanged(EnumPhoneLineState::PhoneLineStateProgress);
//	
//}

void QtUserProfileHandler::sipAccountConnectionFailedEventHandlerThreadSafe(SipAccount* sipAccount, EnumSipLoginState::SipLoginState state, std::string loginErrorMessage) {
	LOG_DEBUG("error: %d", state);
	// FIXME: Why do we do different things depending on the account type?
	if (sipAccount->getType() == SipAccount::SipAccountTypeWengo) {
		_qtWengoPhone.uninstallQtBrowserWidget();
	} else {
		_qtWengoPhone.currentUserProfileWillDieEventHandlerSlot();
		_cUserProfileHandler.currentUserProfileReleased();
	}

	const WengoAccount * wengoAccount = dynamic_cast<const WengoAccount *>(sipAccount);
	_qtWengoPhone.showLoginMessageWindow();
	_qtWengoPhone.getLoginMessageWindow()->setProfileName(QString::fromStdString(wengoAccount->getWengoLogin()));

	QString message;
	if (state == EnumSipLoginState::SipLoginStatePasswordError) {
		
		message = tr("Sorry, the login you entered is incorrect.");//VOXOX CHANGE by Rolando 03-02-09, new Message
		_qtWengoPhone.getLoginMessageWindow()->setAdviceMessage(tr("Please make sure your login information is correct\nand then sign in again."));//VOXOX CHANGE by Rolando 03-02-09, new Message

	} else if (state == EnumSipLoginState::SipLoginStateNetworkError) {
		message = tr("Connection error");
		_qtWengoPhone.getLoginMessageWindow()->setAdviceMessage(tr("Troubleshoot this issue by checking your Internet connections.  Make sure that VoxOx is not being blocked by a local firewall or network device.\nIf the problem persists, please contact customer support."));//VOXOX CHANGE by Rolando 03-02-09, new Message
		
	} else if (state == EnumSipLoginState::SipLoginStateUnActivatedError) {//VOXOX CHANGE by Rolando - 2009.05.22 - added SipLoginStateUnActivatedError state to handle error when user has an account not activated
		message = tr("Sorry, you must verify your email address first.");
		_qtWengoPhone.getLoginMessageWindow()->setAdviceMessage(tr("Please check your inbox and click the verification link\nthat was sent to you after you completed the sign up form."));//VOXOX CHANGE by Rolando - 2009.05.22, new Message
		
	} else if (state == EnumSipLoginState::SipLoginStateOtherError) {//VOXOX CHANGE by Rolando - 2009.05.22 - added SipLoginStateUnActivatedError state to handle error when user has an account not activated
		message = "VoxOx Error Message";//VOXOX CHANGE by Rolando - 2009.09.11 - handling custom error messages
		_qtWengoPhone.getLoginMessageWindow()->setAdviceMessage(QString::fromStdString(loginErrorMessage));//VOXOX CHANGE by Rolando - 2009.09.11 
		
	} else{
		message = tr("Unexpected error (code: %1)").arg(int(state));
		_qtWengoPhone.getLoginMessageWindow()->setAdviceMessage(QString::fromStdString(loginErrorMessage));//VOXOX CHANGE by Rolando - 2009.09.23 
	}
	//VOXOX CHANGE by Rolando 2009.05.05, we now show Login Message Window inside qtwengophone window
	_qtWengoPhone.getLoginMessageWindow()->setErrorMessage(message);
	_qtWengoPhone.installQtBrowserWidget();
	_qtWengoPhone.getQtSystray().phoneLineStateChanged(EnumPhoneLineState::PhoneLineStateProgress);
	
}