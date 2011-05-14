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

#include "ui_ContactCallListWidget.h"
#include "QtContactCallListWidget.h"

#include <control/CWengoPhone.h>
#include <control/phonecall/CPhoneCall.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <model/webservices/info/WsInfo.h>//VOXOX CHANGE by Rolando - 2009.10.16 
#include <model/phonecall/ConferenceCall.h>
#include <model/phonecall/PhoneCall.h>
#include <model/profile/UserProfile.h>
#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <presentation/qt/phonecall/QtPhoneCall.h>

//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>

#include <QtGui/QtGui>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

QtContactCallListWidget::QtContactCallListWidget(CWengoPhone & cWengoPhone, QWidget * parent)
	: QObject(NULL),
	_cWengoPhone(cWengoPhone) {

	_contactCallListWidget = new QWidget(parent);
	_ui = new Ui::ContactCallListWidget();
	_ui->setupUi(_contactCallListWidget);

	QHBoxLayout* layout = new QHBoxLayout(_ui->callFrame);

	//VOXOX CHANGE by Rolando 04-07-09
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QPalette pal;
	QPixmap pixmap(QString::fromStdString(config.getKeypadBackgroundPath()));	//VOXOX - SEMR - 2009.07.31 change keypad background
    pal.setBrush(_contactCallListWidget->backgroundRole(), pixmap);
	_contactCallListWidget->setPalette(pal);
	_contactCallListWidget->setAutoFillBackground(true);	

	//Leads to a crash in conference call when
	//everybody hangs up
	//setAttribute(Qt::WA_DeleteOnClose, true);

	_conferenceState = CS_CallingFirst;

	_key = "";
}

QtContactCallListWidget::~QtContactCallListWidget() {	
	OWSAFE_DELETE(_contactCallListWidget);
	OWSAFE_DELETE(_ui);
}

void QtContactCallListWidget::addPhoneCall(QtPhoneCall * qtPhoneCall) {
	_phoneCallList.append(qtPhoneCall);

	_ui->callFrame->layout()->addWidget(qtPhoneCall->getWidget());
	SAFE_CONNECT(qtPhoneCall, SIGNAL(destroyed(QObject*)),
		SLOT(slotQtPhoneCallDestroyed(QObject*)));

	if (qtPhoneCall->getCPhoneCall().getPhoneCall().getConferenceCall()) {
		SAFE_CONNECT(qtPhoneCall, SIGNAL(startedTalking(QtPhoneCall*)),
			SLOT(slotStartedTalking(QtPhoneCall*)) );
	}
}

void QtContactCallListWidget::slotStartedTalking(QtPhoneCall * qtPhoneCall) {
	//QMutexLocker locker(&_mutex);

	//Hack
	//Fix a crash in conference mode when QtPhoneCall comes NULL while
	//this function is not ended
	_qtPhoneCall = qtPhoneCall;
	//VOXOX - CJC - 2009.06.10 
	QtVoxMessageBox box(_contactCallListWidget);
	switch (_conferenceState) {
	case CS_CallingFirst:

		
		box.setWindowTitle("VoxOx – Conference");
		box.setText( tr("<b>Tell your first contact that they are going to be part of a conference call.</b> Hold first contact and call second one"));
		box.setStandardButtons(QMessageBox::Ok );
		box.exec();

		/*QMessageBox::information(
			_contactCallListWidget,
			 tr("VoxOx – Conference"),
             tr("<b>Tell your first contact that they are going to be part of a conference call.</b>"),
             tr("Hold first contact and call second one")
             );*/

		if (_qtPhoneCall) {
			_qtPhoneCall->holdOrResume();
			_conferenceState = CS_FirstOnHold;
		}
		return;

	case CS_FirstOnHold:


		
		box.setWindowTitle("VoxOx – Conference");
		box.setText( tr("<b>Tell your second contact that they are going to be part of a conference call.</b> Start Conference"));
		box.setStandardButtons(QMessageBox::Ok );
		box.exec();

		//QMessageBox::information(
		//	_contactCallListWidget,
		//	tr("VoxOx – Conference"),
  //          tr("<b>Tell your second contact that they are going to be part of a conference call.</b>"),
  //          tr("Start Conference")
  //          );

		if (_qtPhoneCall) {
			_qtPhoneCall->holdOrResume();
			_conferenceState = CS_Started;
		}
		return;

	case CS_Started:
		// Nothing to do
		break;
	}
}

bool QtContactCallListWidget::hasConferenceStarted(){
	if(_conferenceState == CS_Started){
		return true;
	}else{
		return false;
	}
}

void QtContactCallListWidget::hangup() {
	QMutexLocker locker(&_mutex);

	QtPhoneCall * phone;

	QtPhoneCallList::iterator iter;

	for (iter = _phoneCallList.begin(); iter != _phoneCallList.end(); iter++) {
		phone = (*iter);

		phone->rejectCall();
	}
	aboutToHangUp();//VOXOX CHANGE Rolando 03-25-09
	_phoneCallList.clear();
}

bool QtContactCallListWidget::hasPhoneCall(PhoneCall * phoneCall) {
	QMutexLocker locker(&_mutex);

	QtPhoneCallList::iterator iter;
	QtPhoneCall * tmp = NULL;
	for (iter = _phoneCallList.begin(); iter != _phoneCallList.end(); iter++) {
		tmp = (*iter);
		if (tmp->getCPhoneCall().getPhoneCall().getCallId() == phoneCall->getCallId()) {
			return true;
		}
	}
	return false;
}

QtPhoneCall * QtContactCallListWidget::takeQtPhoneCall(PhoneCall * phoneCall) {
	QMutexLocker locker(&_mutex);

	QtPhoneCallList::iterator iter;
	for (iter = _phoneCallList.begin(); iter != _phoneCallList.end(); iter++) {
		QtPhoneCall * qtphoneCall = *iter;
		if (qtphoneCall->getCPhoneCall().getPhoneCall().getPeerSipAddress().getUserName() ==
				phoneCall->getPeerSipAddress().getUserName()) {
			_ui->callFrame->layout()->removeWidget(qtphoneCall->getWidget());
			_phoneCallList.erase(iter);
			return qtphoneCall;
		}
	}
	return NULL;
}

QtPhoneCall * QtContactCallListWidget::getFirstQtPhoneCall() {
	if (_phoneCallList.count() == 0) {
		return NULL;
	}
	return _phoneCallList[0];
}

bool QtContactCallListWidget::hasActivePhoneCall() {
	QMutexLocker locker(&_mutex);

	QtPhoneCallList::iterator iter;
	for (iter = _phoneCallList.begin(); iter != _phoneCallList.end(); iter++) {
		if ((*iter)->getCPhoneCall().getState() == EnumPhoneCallState::PhoneCallStateTalking) {
			return true;
		}
	}
	return false;
}

void QtContactCallListWidget::slotQtPhoneCallDestroyed(QObject * qtPhoneCall) {
	QMutexLocker locker(&_mutex);
	QtPhoneCallList::iterator iter;
	QtPhoneCall * tmp = NULL;
	for (iter = _phoneCallList.begin(); iter != _phoneCallList.end(); iter++) {
		tmp = (*iter);
		if (tmp == qtPhoneCall) {
			_phoneCallList.erase(iter);
			break;
		}
	}
	if (_phoneCallList.size() == 0) {
		aboutToClose();
		isClosing(getKey());
		//deleteLater();
		//close();
		//QTabWidget * tabWidget = qobject_cast<QTabWidget *>(parentWidget()->parentWidget());
		//if (!tabWidget) {
		//	LOG_FATAL("parent QWidget is not a QTabWidget");
		//}
		//int index = tabWidget->indexOf(this);
		//tabWidget->removeTab(index);
	}

	//VOXOX CHANGE by Rolando - 2009.10.16 
	//Update User Credits
	WsInfo * wsInfo = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getWsInfo();//VOXOX CHANGE by Rolando - 2009.10.16 
	if (wsInfo) {//VOXOX CHANGE by Rolando - 2009.10.16 
		wsInfo->getWengosCount(true);//VOXOX CHANGE by Rolando - 2009.10.16 
		wsInfo->execute();//VOXOX CHANGE by Rolando - 2009.10.16 
	}
	_qtPhoneCall = NULL;
}
