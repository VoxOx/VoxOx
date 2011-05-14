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
#include "QtConferenceCallWidget.h"

#include "ui_ConferenceCallWidget.h"

#include <presentation/qt/contactlist/QtContactMenu.h>
#include <presentation/qt/QtWengoPhone.h>//VOXOX CHANGE by Rolando - 2009.06.03 
#include <model/phoneline/IPhoneLine.h>
#include <model/phonecall/ConferenceCall.h>
#include <control/contactlist/CContactList.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <model/contactlist/ContactProfile.h>
#include <control/CWengoPhone.h>
#include <util/SafeDelete.h>
#include <util/Logger.h>
#include <qtutil/WidgetBackgroundImage.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/VoxOxToolTipLineEdit.h>

#include <presentation/qt/callbar/customCallBar/QtVoxOxCallBarFrame.h>

#include <presentation/PFactory.h>

#include <qtutil/Widget.h>
#include <util/StringList.h>

#include <QtGui/QtGui>

const QString DEFAULT_CONFERENCE_MESSAGE = QString("Name or Number");

QtConferenceCallWidget::QtConferenceCallWidget(QWidget * parent, CWengoPhone & cWengoPhone, IPhoneLine * phoneLine , bool startConference)
	: QWidget(parent), _phoneLine(phoneLine), _cWengoPhone(cWengoPhone),_startConference(startConference)  {

	_ui = new Ui::ConferenceCallWidget();
	_ui->setupUi(this);

	setStyleSheet( QString("QLabel#headerLabel{color: #dce602; font-size: 12px; font-family: \"Tahoma\"; } ")
				  + QString("QLabel#phoneNumber1Label{color: #ffffff; font-weight: bold; font-size: 11px; font-family: \"Tahoma\"; } ")
				  + QString("QLabel#phoneNumber2Label{color: #ffffff; font-weight: bold; font-size: 11px; font-family: \"Tahoma\"; } "));
	
	CUserProfile * currentCUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();

	_qtVoxOxCallBarFrame1 = new QtVoxOxCallBarFrame();
	_qtVoxOxCallBarFrame1->getVoxOxToolTipLineEdit()->setToolTipDefaultText(DEFAULT_CONFERENCE_MESSAGE);
	/*TODO: VOXOX CHANGE by Rolando, uncomment this code when SQLite be integrated, this method will get phonenumbers of all contacts
	allowing call to contacts according their nicknames, by the moment just could be filled with numbers this component*/
	/*if(currentCUserProfile){
		_qtVoxOxCallBarFrame1->setCUserProfile(currentCUserProfile);
	}*/
	Widget::createLayout(_ui->phoneNumber1Frame)->addWidget(_qtVoxOxCallBarFrame1);
	
	_qtVoxOxCallBarFrame2 = new QtVoxOxCallBarFrame();
	_qtVoxOxCallBarFrame2->getVoxOxToolTipLineEdit()->setToolTipDefaultText(DEFAULT_CONFERENCE_MESSAGE);
	/*TODO: VOXOX CHANGE by Rolando, uncomment this code when SQLite be integrated, this method will get phonenumbers of all contacts
	allowing call to contacts according their nicknames, by the moment just could be filled with numbers this component*/
	/*if(currentCUserProfile){
		_qtVoxOxCallBarFrame2->setCUserProfile(currentCUserProfile);
	}*/
	Widget::createLayout(_ui->phoneNumber2Frame)->addWidget(_qtVoxOxCallBarFrame2);

#ifdef OS_WINDOWS//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->phoneNumber1Frame->setMinimumHeight(20);//VOXOX CHANGE by Rolando - 2009.07.13	
	_ui->phoneNumber1Frame->setMaximumHeight(25);//VOXOX CHANGE by Rolando - 2009.07.13
	_ui->phoneNumber2Frame->setMinimumHeight(20);//VOXOX CHANGE by Rolando - 2009.07.13	
	_ui->phoneNumber2Frame->setMaximumHeight(25);//VOXOX CHANGE by Rolando - 2009.07.13 
#endif

	populateVoxOxContactsMap();

	//VOXOX CHANGE by Rolando 04-07-09	
	_ui->startButton->setImages(QString(":/pics/phonecall/btn_lrg_call.png"),QString(":/pics/phonecall/btn_lrg_call_press.png"));
	_ui->cancelButton->setImages(QString(":/pics/phonecall/btn_back_arrow_up.png"),QString(":/pics/phonecall/btn_back_arrow_down.png"));//VOXOX CHANGE by Rolando - 2009.07.08 
	
	SAFE_CONNECT(_ui->startButton, SIGNAL(clicked()), SLOT(startClicked()));
	SAFE_CONNECT(_ui->cancelButton, SIGNAL(clicked()), SLOT(cancel()));

}

QtConferenceCallWidget::~QtConferenceCallWidget() {
	OWSAFE_DELETE(_ui);

	OWSAFE_DELETE(_qtVoxOxCallBarFrame1);

	OWSAFE_DELETE(_qtVoxOxCallBarFrame2);
}

void QtConferenceCallWidget::startClicked() {

	QString phoneNumber1 = _qtVoxOxCallBarFrame1->getFullPhoneNumberText();
	QString phoneNumber2 = _qtVoxOxCallBarFrame2->getFullPhoneNumberText();

	if ((!phoneNumber1.isEmpty()) && (!phoneNumber2.isEmpty()))
	{

		if(_startConference)
		{
			if(!_qtVoxOxCallBarFrame1->textIsPhoneNumber()){
				phoneNumber1 = QString::fromStdString(_voxOxContactsMap[phoneNumber1.toStdString()]);
				if(phoneNumber1 == ""){//TODO: fix this, this should not return empty string, fix it when we integrate SQLite
					phoneNumber1 = _qtVoxOxCallBarFrame1->getFullPhoneNumberText();
				}
			}

			if(!_qtVoxOxCallBarFrame2->textIsPhoneNumber()){
				phoneNumber2 = QString::fromStdString(_voxOxContactsMap[phoneNumber2.toStdString()]);
				if(phoneNumber2 == ""){//TODO: fix this, this should not return empty string, fix it when we integrate SQLite
					phoneNumber2 = _qtVoxOxCallBarFrame1->getFullPhoneNumberText();
				}
			}
			
			ConferenceCall * confCall = new ConferenceCall(*_phoneLine);
			//VOXOX CHANGE by Rolando - 2009.06.03 
			confCall->makeConferenceCallErrorEvent += boost::bind(&QtConferenceCallWidget::makeConferenceCallErrorEventHandler, this, _2, _3);//VOXOX CHANGE by Rolando - 2009.06.02 - as we are using chat to insert calls we are handling errors in QtChatWindow

			confCall->addPhoneNumber(phoneNumber1.toStdString());
			confCall->addPhoneNumber(phoneNumber2.toStdString());
		}
		startClickedSignal();

	} else {
		if (phoneNumber1.isEmpty()) {
			_qtVoxOxCallBarFrame1->getVoxOxToolTipLineEdit()->setFocus();
		} else {
			_qtVoxOxCallBarFrame2->getVoxOxToolTipLineEdit()->setFocus();
		}
	}
}

void QtConferenceCallWidget::cancel(){
	cancelClicked();
}


//TODO: VOXOX CHANGE by Rolando 04-29-09, fix this method to be able to get all phonenumbers of contacts when SQLite is integrated
void QtConferenceCallWidget::populateVoxOxContactsMap()
{
	//VOXOX - JRT - 2009.07.27 - TODO: this _voxOxContactsMap seems like overkill and includes ALL contacts.
	CUserProfile * currentCUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();

	if (currentCUserProfile) 
	{
		CContactList& currentCContactList = currentCUserProfile->getCContactList();
		StringList    currentContactsIds  = currentCContactList.getContactIds();

		for (StringList::const_iterator it = currentContactsIds.begin(); it != currentContactsIds.end(); ++it) 
		{
			ContactProfile	tmpContactProfile = currentCContactList.getContactProfile(*it);
			QString			displayName		  = QString::fromUtf8(tmpContactProfile.getDisplayName().c_str());			
			std::string		freePhoneNumber   = tmpContactProfile.getVoxOxPhone();
			std::string		displayNameStr	  = displayName.toStdString();

			_voxOxContactsMap[displayNameStr] = freePhoneNumber;
		}
	}
}

//VOXOX CHANGE by Rolando - 2009.05.29 - added parameter phoneNumber to check what call failed
void QtConferenceCallWidget::makeConferenceCallErrorEventHandler(EnumMakeCallError::MakeCallError error, std::string phoneNumber) {
	typedef ThreadEvent2<void (EnumMakeCallError::MakeCallError error, std::string phoneNumber), EnumMakeCallError::MakeCallError, std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&QtConferenceCallWidget::makeConferenceCallErrorEventHandlerThreadSafe, this, _1, _2), error, phoneNumber);
	PFactory::postEvent(event);
}

//VOXOX CHANGE by Rolando - 2009.05.29 - added parameter phoneNumber to check what call failed
void QtConferenceCallWidget::makeConferenceCallErrorEventHandlerThreadSafe(EnumMakeCallError::MakeCallError error, std::string phoneNumber) {

	//VOXOX CHANGE by Rolando - 2009.05.29 - if no error happened
	if(error != EnumMakeCallError::NoError && error != EnumMakeCallError::CallNotHeld){//VOXOX CHANGE by Rolando - 2009.05.29 - when we make a conference call we have to ignore error by call on hold
		QtWengoPhone * qtWengoPhone = dynamic_cast<QtWengoPhone*>(_cWengoPhone.getPresentation());
		qtWengoPhone->showCallErrorMessage(error,phoneNumber);
		cancelConferenceCall(phoneNumber);//VOXOX CHANGE by Rolando - 2009.05.29 - we send this signal when an error happened
	}	
}