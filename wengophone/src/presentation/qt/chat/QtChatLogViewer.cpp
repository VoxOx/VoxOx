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
#include "QtChatLogViewer.h"
#include "ui_ChatLogViewer.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/contactlist/ContactProfile.h>
#include <model/contactlist/ContactList.h>
#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileHandler.h>
#include <control/contactlist/CContactList.h>
#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/QtNoWengoAlert.h>
#include <presentation/qt/QtToolBar.h>
#include <presentation/qt/contactlist/QtContactList.h>
#include <presentation/qt/contactlist/QtContactActionManager.h>
//#include <presentation/qt/contactlist/QtContactListManager.h>		//VOXOX - JRT - 2009.04.15 - no longer needed.
#include <presentation/qt/filetransfer/QtFileTransfer.h>
#include <presentation/qt/profile/QtProfileDetails.h>
#include <presentation/qt/webservices/sms/QtSms.h>

//#include <presentation/qt/QtEnumIMProtocol.h>	//VOXOX - JRT - 2009.04.21 

#include <imwrapper/IMContact.h>
#include <imwrapper/IMContactSet.h>
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.05.29 

#include <cutil/global.h>
#include <qtutil/Object.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/StringListConvert.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/StringList.h>

#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QTime>

#include <QtGui/QDialog>

QtChatLogViewer::QtChatLogViewer(QWidget * parent, QtWengoPhone & qtWengoPhone, QString log)
	: QMainWindow(parent),
	_qtWengoPhone(qtWengoPhone) {

	_ui = new Ui::ChatLogViewer();
	_ui->setupUi(this);
	setupMenuBarActions();
	setupToolBarActions();

	QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
	CContactList & cContactList = qtContactList->getCContactList();

	//VOXOX - JRT - 2009.04.15 
//	UserProfile * userProfile = _qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getUserProfileHandler().getCurrentUserProfile();
	UserProfile * userProfile = getCurrentUserProfile();
	if (userProfile) {
		// Setup user avatar
		
		
		std::string myData = userProfile->getIcon().getData();
	
		// Read history
		std::string userlogin;
		HistoryMementoCollection hmc;
		StringList cuuidList;
		userProfile->getHistory().loadChatLog(log.toStdString(), &hmc, &userlogin, &cuuidList);
		_cuuidList = StringListConvert::toQStringList(cuuidList);
		
		bool isWengoAccountConnected = userProfile->hasWengoAccount();

		QString selfContactName = QString::fromUtf8(userlogin.c_str());
		
		_ui->historyLog->init(QString(""), QString::fromStdString(cContactList.getContactDisplayName(_cuuidList.at(0).toStdString())), QString(""),QTime::currentTime(),QDate::currentDate());
		_ui->historyLog->setAvatarPixmap("self", QString::fromStdString(myData));
		
		// Set remote user avatars
		Q_FOREACH(QString uuid, _cuuidList) {
//			ContactProfile profile = cContactList.getContactProfile(uuid.toStdString());
//			std::string data = profile.getIcon().getData();
			std::string data = cContactList.getContactIconData( uuid.toStdString() );	//VOXOX - JRT - 2009.07.26 

			if (data.size() != 0) {
				_ui->historyLog->setAvatarPixmap(uuid, QString::fromStdString(data));
			}
		}
		
		// parses HistoryMementoCollection to find messages and inserts them in historyLog
		//_ui->historyLog->setProtocol (EnumIMProtocol::IMProtocolAll);
		Config & config = ConfigManager::getInstance().getCurrentConfig();
		QString wengoSuffix = "@" + QString::fromStdString( config.getWengoRealm() );
		QDate previousDate;
		for (HistoryMap::iterator it = hmc.begin(); it != hmc.end(); it++) {
		
			HistoryMemento * memento = it->second;

			// Read date and time
			Date mementoDate = memento->getDate();
			QDate date(mementoDate.getYear(), mementoDate.getMonth(), mementoDate.getDay());
			Time mementoTime = memento->getTime();
			QTime time = QTime(mementoTime.getHour(), mementoTime.getMinute());

			// Insert date line if necessary
			if (date != previousDate) {
				QString txt = QLocale::system().toString(date);
				_ui->historyLog->insertStatusMessage(txt, time);
				previousDate = date;
			}
			
			// Insert message
			QString contactName = QString::fromUtf8(memento->getPeer().c_str());
			if (isWengoAccountConnected) {
				contactName.remove(wengoSuffix);
			}
			contactName.remove("sip:");
			
			QString contactId;
			QString protocol = "";
			if (contactName == selfContactName) {
				contactId = "self";
			
			} else {
				_contactID = _cuuidList[0];
				contactId = _cuuidList[0];

				// FIXME: History format is broken: it does not store the uuid
				// associated with a peer. Using the first uid will work for
				// all one-to-one chat, but will fail for chat conference.
			}

		


			_ui->historyLog->insertMessage(
				contactId,
				contactName,
				protocol,
				QString::fromUtf8(memento->getData().c_str()),
				time,date
			);
		}
	}
	////
	
	SAFE_CONNECT(qtWengoPhone.getQtContactList(), SIGNAL(contactChangedEventSignal(QString )), SLOT(contactChangedEventSlot(QString)));
	
	updateToolBarActions();
}

QtChatLogViewer::~QtChatLogViewer() {
	OWSAFE_DELETE(_ui);
}

void QtChatLogViewer::copyQAction(QObject * actionParent, QAction * action) {
	QAction * tmp = actionParent->findChild<QAction *>(action->objectName());
	if (!tmp) {
		LOG_FATAL("QAction is null, cannot copy it");
	}
	action->setIcon(tmp->icon());
	action->setIconText(tmp->iconText());
	action->setText(tmp->text());
	action->setToolTip(tmp->toolTip());
	action->setShortcut(tmp->shortcut());
}

void QtChatLogViewer::setupMenuBarActions() {
	//QtToolBar * qtToolBar = &_qtWengoPhone.getQtToolBar();
	//QWidget * toolBar = qtToolBar->getWidget();

	//// setup "Wengo" menu
	//copyQAction(toolBar, _ui->actionShowWengoAccount);
	//SAFE_CONNECT_RECEIVER(_ui->actionShowWengoAccount, SIGNAL(triggered()), qtToolBar, SLOT(showWengoAccount()));
	//copyQAction(toolBar, _ui->actionEditMyProfile);
	//SAFE_CONNECT_RECEIVER(_ui->actionEditMyProfile, SIGNAL(triggered()), qtToolBar, SLOT(editMyProfile()));
	//copyQAction(toolBar, _ui->actionWengoServices);
	//SAFE_CONNECT_RECEIVER(_ui->actionWengoServices, SIGNAL(triggered()), qtToolBar, SLOT(showWengoServices()));
	//copyQAction(toolBar, _ui->actionClose);
	//SAFE_CONNECT(_ui->actionClose, SIGNAL(triggered()), SLOT(close()));
	//////

	//// setup "contact" menubar
	//copyQAction(toolBar, _ui->actionAddContact);
	//SAFE_CONNECT_RECEIVER(_ui->actionAddContact, SIGNAL(triggered()), qtToolBar, SLOT(addContact()));
	//copyQAction(toolBar, _ui->actionSearchWengoContact);
	//SAFE_CONNECT_RECEIVER(_ui->actionSearchWengoContact, SIGNAL(triggered()), qtToolBar, SLOT(searchWengoContact()));
	//////

	//// setup "actions" menu
	//copyQAction(toolBar, _ui->actionCreateConferenceCall);
	//SAFE_CONNECT_RECEIVER(_ui->actionCreateConferenceCall, SIGNAL(triggered()), qtToolBar, SLOT(createConferenceCall()));
	//copyQAction(toolBar, _ui->actionSendSms);
	//SAFE_CONNECT_RECEIVER(_ui->actionSendSms, SIGNAL(triggered()), qtToolBar, SLOT(sendSms()));
	//SAFE_CONNECT(_ui->actionSaveHistoryAs, SIGNAL(triggered()), SLOT(saveChatHistory()));
	//SAFE_CONNECT(_ui->actionActionRestartChat, SIGNAL(triggered()), SLOT(restartChat()));
	//////

	//// setup "tools" menu
	///*copyQAction(toolBar, _ui->actionShowFileTransfer);
	//SAFE_CONNECT_RECEIVER(_ui->actionShowFileTransfer, SIGNAL(triggered()), qtToolBar, SLOT(showFileTransferWindow()));*/
	//////

	//// setup "help" menu
	//copyQAction(toolBar, _ui->actionShowWengoForum);
	//SAFE_CONNECT_RECEIVER(_ui->actionShowWengoForum, SIGNAL(triggered()), qtToolBar, SLOT(showWengoForum()));
	//copyQAction(toolBar, _ui->actionWengoFAQ);
	//SAFE_CONNECT_RECEIVER(_ui->actionWengoFAQ, SIGNAL(triggered()), qtToolBar, SLOT(showWengoFAQ()));
	//////
}

void QtChatLogViewer::setupToolBarActions() {
	/*SAFE_CONNECT(_ui->actionCallContact, SIGNAL(triggered()), SLOT(callContact()));
	SAFE_CONNECT(_ui->actionSendSms, SIGNAL(triggered()), SLOT(sendSmsToContact()));
	SAFE_CONNECT(_ui->actionSendFile, SIGNAL(triggered()), SLOT(sendFileToContact()));
	SAFE_CONNECT(_ui->actionCreateChatConf, SIGNAL(triggered()), SLOT(createChatConference()));
	SAFE_CONNECT(_ui->actionContactInfo, SIGNAL(triggered()), SLOT(showContactInfo()));*/
}

bool QtChatLogViewer::canDoFileTransfer(const ContactProfile & contactProfile) {
	/*if (!contactProfile.getFirstWengoId().empty() && contactProfile.isAvailable()) {
		IMContact imContact = contactProfile.getFirstAvailableWengoIMContact();
		if ( (imContact.getPresenceState() != EnumPresenceState::PresenceStateOffline) &&
				(imContact.getPresenceState() != EnumPresenceState::PresenceStateUnknown) &&
				(imContact.getPresenceState() != EnumPresenceState::PresenceStateUnavailable)) {
				return true;
		}
	}
	return false;*/
	return false;
}

void QtChatLogViewer::updateToolBarActions() {

//	QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
//	ContactProfile contactProfile;
//	
//	//call'n chat
//	if (_cuuidList.size() <= 0) {
//		//no contact to call
//		_ui->actionCallContact->setEnabled(false);
//		_ui->actionActionRestartChat->setEnabled(false);
//	} else	if (_cuuidList.size() > 1) {
//		//TO DO
//		_ui->actionCallContact->setEnabled(false);
//		_ui->actionActionRestartChat->setEnabled(false);
//	} else {
//		std::string cuuid = _cuuidList[0].toStdString();
//		contactProfile = qtContactList->getCContactList().getContactProfile(cuuid);
//		_ui->actionCallContact->setEnabled(contactProfile.hasCall() && contactProfile.isAvailable());
//		
//		_ui->actionActionRestartChat->setEnabled(contactProfile.hasIM());
//	}
//	////
//	
//	// depending on sip account type ( SMS, File Transfert )
//	bool sendSMS = false;
//	bool sendFile = false;
//	//VOXOX - JRT - 2009.04.15 
////	UserProfile * userProfile = _qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getUserProfileHandler().getCurrentUserProfile();
//	UserProfile * userProfile = getCurrentUserProfile();
//	if (userProfile->hasWengoAccount()) {
//		for (QStringList::const_iterator it = _cuuidList.begin(); it != _cuuidList.end(); it++) {
//			contactProfile = qtContactList->getCContactList().getContactProfile(it->toStdString());
//			sendSMS |= !contactProfile.getMobilePhone().empty();
//			sendFile |= canDoFileTransfer(contactProfile);
//		}
//	} else if (userProfile->hasSipAccount()) {
//		sendSMS = true; // a pop up
//		//sendFile = false; //useless
//	}
//	_ui->actionSendSms->setEnabled(sendSMS);
//	_ui->actionSendFile->setEnabled(sendFile);
//	////
//
//	if (_cuuidList.size() <= 0) {
//		_ui->actionContactInfo->setEnabled(false);
//		_ui->actionCreateChatConf->setEnabled(false);
//	} else {
//		_ui->actionContactInfo->setEnabled(true);
//		_ui->actionCreateChatConf->setEnabled(false);
//	}

}

void QtChatLogViewer::sendSmsToContact() {
//	QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
//	ContactProfile contactProfile;
//	//VOXOX - JRT - 2009.04.15 
////	UserProfile * userProfile = _qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getUserProfileHandler().getCurrentUserProfile();
//	UserProfile * userProfile = getCurrentUserProfile();
//	
//	if (userProfile->hasWengoAccount()) {
//		QtSms * sms = _qtWengoPhone.getQtSms();
//		if (sms) {
//			sms->clear();
//			for (QStringList::const_iterator it = _cuuidList.begin(); it != _cuuidList.end(); it++) {
//				contactProfile = qtContactList->getCContactList().getContactProfile(it->toStdString());
//				sms->addPhoneNumber(QString::fromStdString(contactProfile.getMobilePhone()));
//			}
//			sms->getWidget()->show();
//		}
//	} else {
//		QtNoWengoAlert alert(this, _qtWengoPhone);
//		alert.getQDialog()->exec();
//	}
}

void QtChatLogViewer::sendFileToContact() {

	//QtFileTransfer * qtFileTransfer = _qtWengoPhone.getFileTransfer();
	//if (!qtFileTransfer) {
	//	return;
	//}

	//// gets a filename
	//QString filename = qtFileTransfer->getChosenFile();
	//if (filename.isEmpty()) {
	//	return;
	//}
	//QFileInfo fileInfo(filename);
	//Config & config = ConfigManager::getInstance().getCurrentConfig();
	//config.set(Config::FILETRANSFER_LASTUPLOADEDFILE_FOLDER_KEY, fileInfo.dir().absolutePath().toStdString());
	//////
	//
	//// sends this file to all available contacts
	//QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
	//CContactList & cContactList = qtContactList->getCContactList();
	//IMContactSet contactSet;
	//ContactProfile contactProfile;
	//
	//for (QStringList::const_iterator it = _cuuidList.begin(); it != _cuuidList.end(); it++) {
	//	contactProfile = qtContactList->getCContactList().getContactProfile(it->toStdString());
	//	
	//	if (contactProfile.hasAvailableWengoId()) {
	//		contactSet.insert(contactProfile.getFirstAvailableWengoIMContact());
	//	}
	//}
	//qtFileTransfer->createSendFileSession(contactSet, filename, cContactList);
	////
}

void QtChatLogViewer::callContact() {
//	if ((_cuuidList.size() > 0) && (!_cuuidList[0].isEmpty())) {
////		QtContactListManager::getInstance()->startCall(_cuuidList[0]);	//VOXOX - JRT - 2009.04.15 
//		getCUserProfile()->makeCall( _cuuidList[0].toStdString() );
//		if (_qtWengoPhone.getWidget()->isMinimized()) {
//			_qtWengoPhone.getWidget()->showNormal();
//		}
//	}
}

void QtChatLogViewer::showContactInfo() {
//
//	QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
//	CContactList & cContactList = qtContactList->getCContactList();
//	ContactProfile contactProfile;
//
//	for (QStringList::const_iterator it = _cuuidList.begin(); it != _cuuidList.end(); it++) {
//		contactProfile = cContactList.getContactProfile(it->toStdString());
//
//		QtProfileDetails qtProfileDetails(
//				*getCUserProfile(),
////				*_qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile(),	//VOXOX - JRT - 2009.04.15 
//		contactProfile, this, tr("Edit Contact"));
//		if (qtProfileDetails.show()) {
//			//VOXOX - JRT - 2009.04.15 
////			_qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile()->getCContactList().updateContact(contactProfile);
////			getCUserProfile()->getCContactList().updateContact(contactProfile);
//			getCUserProfile()->updateContact(contactProfile);	//VOXOX - JRT - 2009.05.25 
//		}
//	}
}

void QtChatLogViewer::saveChatHistory() {
	//_ui->historyLog->saveHistoryAsHtml();
}

void QtChatLogViewer::restartChat() {
	QtContactActionManager * contactActionManager = QtContactActionManager::getInstance(_qtWengoPhone.getCWengoPhone());
	contactActionManager->chatContact(QString::fromStdString(_contactID.toStdString()));
}

//VOXOX - JRT - 2009.07.27 - No longer used
//void QtChatLogViewer::createChatConference() {

	//// TO DO allow to start a multi-chat 

	//QtChatWidget * widget = getActiveTabWidget();
	//if (widget) {
	//	widget->showInviteDialog();
	//}
	
	//if (canDoMultiChat()) {
	//	QtChatRoomInviteDlg dlg(*_imChatSession,
	//		_cChatHandler.getCUserProfile().getCContactList(), this);
	//	dlg.exec();
	//}
//}

void QtChatLogViewer::contactChangedEventSlot(QString contactid) {

	bool updateNeeded = false;

	for (QStringList::const_iterator it = _cuuidList.begin(); it != _cuuidList.end(); it++) {
		if ((*it) == contactid) {
				updateNeeded = true;
		} 
	}
	
	if (updateNeeded && isVisible()) {
		updateToolBarActions();
	}
}

//VOXOX - JRT - 2009.04.15 - For readability and maintainability
CUserProfile* QtChatLogViewer::getCUserProfile()	
{
	return _qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile();
}

UserProfile* QtChatLogViewer::getCurrentUserProfile()
{
	return _qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getUserProfileHandler().getCurrentUserProfile();
}

//End VoxOx