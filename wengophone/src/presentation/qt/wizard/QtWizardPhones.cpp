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
* Wizard Progress
* @author Chris Jimenez C 
* @date 2009.06.01
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtWizardPhones.h"

#include "ui_WizardPhones.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <presentation/qt/callbar/customCallBar/QtVoxOxCallBarFrame.h>
#include <qtutil/VoxOxToolTipLineEdit.h>
#include <cutil/global.h>
#include <qtutil/Widget.h>
#include <QtGui/QtGui>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <model/profile/UserProfile.h>

#include <model/profile/EnumSex.h>

QtWizardPhones::QtWizardPhones(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone){

	_ui = new Ui::WizardPhones();
	_ui->setupUi(this);

	_callBarFrameMobile = new QtVoxOxCallBarFrame();
	_callBarFrameMobile->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->framePhone1)->addWidget(_callBarFrameMobile);

	_callBarFrameHome = new QtVoxOxCallBarFrame();
	_callBarFrameHome->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->framePhone2)->addWidget(_callBarFrameHome);

	_callBarFrameWork = new QtVoxOxCallBarFrame();
	_callBarFrameWork->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->framePhone3)->addWidget(_callBarFrameWork);

	_callBarFrameFax = new QtVoxOxCallBarFrame();
	_callBarFrameFax->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->framePhone4)->addWidget(_callBarFrameFax);

	_callBarFrameOther = new QtVoxOxCallBarFrame();
	_callBarFrameOther->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->framePhone5)->addWidget(_callBarFrameOther);

#ifdef OS_WINDOWS//VOXOX CHANGE by Rolando - 2009.07.13 
	_callBarFrameMobile->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13
	_callBarFrameHome->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13
	_callBarFrameWork->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13
	_callBarFrameFax->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13
	_callBarFrameOther->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13
#endif
	
	readConfig();
}

QtWizardPhones::~QtWizardPhones() {
	delete _callBarFrameMobile;
	delete _callBarFrameHome;
	delete _callBarFrameWork;
	delete _callBarFrameFax;
	delete _callBarFrameOther;
	delete _ui;
}

QString QtWizardPhones::getName() const {
	return tr("WizardPhones");
}

QString QtWizardPhones::getTitle() const {
	return tr("Phones");
}

QString QtWizardPhones::getDescription() const {
	return tr("Take control of your phones.");
}


int QtWizardPhones::getStepNumber() const {
	return 2;
}


void QtWizardPhones::readConfig() {

	UserProfile & profile = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile();

	
	QString mobile = QString::fromStdString(profile.getMobilePhone());
	QString home = QString::fromStdString(profile.getHomePhone());
	QString work = QString::fromStdString(profile.getWorkPhone());
	QString fax = QString::fromStdString(profile.getFax());
	QString other = QString::fromStdString(profile.getOtherPhone());
	
	if(mobile!=""){
		_callBarFrameMobile->setFullPhoneNumber(mobile);
	}
	if(home!=""){
	_callBarFrameHome->setFullPhoneNumber(home);
	}
	if(work!=""){
	_callBarFrameWork->setFullPhoneNumber(work);
	}
	if(fax!=""){
	_callBarFrameFax->setFullPhoneNumber(fax);
	}
	if(other!=""){
	_callBarFrameOther->setFullPhoneNumber(other);
	}



	//QString home = _callBarFrameHome->getFullPhoneNumberText();
	//QString work = _callBarFrameWork->getFullPhoneNumberText();
	//QString fax = _callBarFrameFax->getFullPhoneNumberText();
	//QString other = _callBarFrameOther->getFullPhoneNumberText();


	//_callBarFrameMobile->getVoxOxToolTipLineEdit()->set

	/*Config & config = ConfigManager::getInstance().getCurrentConfig();
	int wizardStep = config.getWizardStepsComplete();
	switch(wizardStep){
		case 1:
			_ui->lblChkImage1->setVisible(true);
			break;
		case 2:
			_ui->lblChkImage2->setVisible(true);
			break;
		case 3:
			_ui->lblChkImage3->setVisible(true);
			break;
		case 4:
			_ui->lblChkImage4->setVisible(true);
			break;
		case 5:
			_ui->lblChkImage5->setVisible(true);
			break;
	}*/
}

void QtWizardPhones::saveConfig() {

	UserProfile & profile = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile();

	QString mobile = _callBarFrameMobile->getFullPhoneNumberText();
	QString home = _callBarFrameHome->getFullPhoneNumberText();
	QString work = _callBarFrameWork->getFullPhoneNumberText();
	QString fax = _callBarFrameFax->getFullPhoneNumberText();
	QString other = _callBarFrameOther->getFullPhoneNumberText();

	profile.setMobilePhone(mobile.toStdString());
	profile.setHomePhone(home.toStdString());
	profile.setWorkPhone(work.toStdString());
	profile.setFax(fax.toStdString());
	profile.setOtherPhone(other.toStdString());

	//VOXOX - JRT - 2009.08.20 - TODO: detect if there were changes to minimize server hits.
	_cWengoPhone.getCUserProfileHandler().updateUserProfile();	//VOXOX - JRT - 2009.08.20 
}
