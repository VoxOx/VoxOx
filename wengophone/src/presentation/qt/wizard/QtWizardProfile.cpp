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
#include "QtWizardProfile.h"

#include "ui_WizardProfile.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <model/profile/UserProfile.h>

#include <model/profile/EnumSex.h>

#include <cutil/global.h>
#include <qtutil/PixmapMerging.h>
#include <qtutil/SafeConnect.h>
#include <QtGui/QtGui>
#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/profilebar/QtProfileBar.h>
#include <presentation/qt/profilebar/QtIMProfileWidget.h>

QtWizardProfile::QtWizardProfile(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone){

	_ui = new Ui::WizardProfile();
	_ui->setupUi(this);
	
	SAFE_CONNECT(_ui->avatarButton, SIGNAL(clicked()), SLOT(changeAvatar()));

		//VOXOX - CJC - 2009.07.13 Relationship is missing on the sync
	_ui->cmbRelationship->setVisible(false);
	_ui->lblRelationship->setVisible(false);

	readConfig();
}

QtWizardProfile::~QtWizardProfile() {
	delete _ui;
}

QString QtWizardProfile::getName() const {
	return tr("WizardProfile");
}

QString QtWizardProfile::getTitle() const {
	return tr("Profile");
}

QString QtWizardProfile::getDescription() const {
	return tr("Enter your profile information to help us customize VoxOx for you.");
}


int QtWizardProfile::getStepNumber() const {
	return 5;
}



void QtWizardProfile::readConfig() {

	UserProfile & profile = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile();

	QString firstName = QString::fromStdString(profile.getFirstName());

	QString lastName = QString::fromStdString(profile.getLastName());

	StreetAddress address = profile.getStreetAddress();
	QString location = QString::fromStdString(address.getStreet1());
	//VOXOX - CJC - 2009.06.24 Missing gender from integration
	QString gender = QString::fromStdString(EnumSex::toString(profile.getSex()));
	
	int year = profile.getBirthdate().getYear();
	int month = profile.getBirthdate().getMonth();
	int day = profile.getBirthdate().getDay();
	QDate birthday(year,month,day);
	//VOXOX - CJC - 2009.06.24 Missing relationship
	QString interest = QString::fromStdString(profile.getInterests());

	bool searchable = profile.isSearchable();

	_ui->txtFirstName->setText(firstName);
	_ui->txtLastName->setText(lastName);

	if(gender=="male"){
		_ui->cmbGender->setCurrentIndex(0);
	}else if(gender == "female"){
		_ui->cmbGender->setCurrentIndex(1);
	}

	_ui->txtLocation->setText(location);
	_ui->dtBirthday->setDate(birthday);
	_ui->txtInterest->setText(interest);
	_ui->chkShareProfile->setChecked(searchable);

	updateAvatar();


}

void QtWizardProfile::saveConfig() {

	UserProfile & profile = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile();
	profile.setFirstName(_ui->txtFirstName->text().toStdString());
	profile.setLastName(_ui->txtLastName->text().toStdString());

	StreetAddress address = profile.getStreetAddress();
	address.setStreet1(_ui->txtLocation->text().toStdString());
	profile.setStreetAddress(address);

	Date date(_ui->dtBirthday->date().day(),_ui->dtBirthday->date().month(),_ui->dtBirthday->date().year());

	profile.setBirthdate(date);
	profile.setInterests(_ui->txtInterest->toPlainText().toStdString());
	QString sex;
	if(_ui->cmbGender->currentIndex()==0){
		sex = "male";
	}else if(_ui->cmbGender->currentIndex()==1){
		sex = "female";
	}
	profile.setSex(EnumSex::toSex(sex.toStdString()));
	profile.setIsSearchable(_ui->chkShareProfile->isChecked());

	//VOXOX - JRT - 2009.08.20 - TODO: detect if there were changes to minimize server hits.
	_cWengoPhone.getCUserProfileHandler().updateUserProfile();
}


void QtWizardProfile::changeAvatar(){

	dynamic_cast<QtWengoPhone*>(_cWengoPhone.getPresentation())->getQtProfileBar()->getQtImProfileWidget()->changeAvatarClicked();
	updateAvatar();

}

void QtWizardProfile::updateAvatar(){

	std::string backgroundPixmapFilename = ":/pics/profilebar/avatar_background.png";
	
	std::string foregroundPixmapData = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getIcon().getData();
	_ui->avatarButton->setIcon(PixmapMerging::merge(foregroundPixmapData, backgroundPixmapFilename));//merge two images


}
