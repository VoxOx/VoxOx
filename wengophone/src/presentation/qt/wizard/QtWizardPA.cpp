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
#include "QtWizardPA.h"

#include "ui_WizardPA.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <cutil/global.h>
#include <qtutil/Widget.h>
#include <QtGui/QtGui>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <model/profile/UserProfile.h>

#include <model/profile/EnumSex.h>

#include <model/webservices/personalAssistant/WsPA.h>

#include <model/webservices/personalAssistant/WsPAGet.h>


QtWizardPA::QtWizardPA(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone){

	_ui = new Ui::WizardPA();
	_ui->setupUi(this);

	
	_wsPAGet = NULL;
	_wsPA	 = NULL;	//VOXOX - JRT - 2009.09.07 
	
	readConfig();
}

QtWizardPA::~QtWizardPA() { 
	if(_wsPAGet){
		_wsPAGet->paDataEvent -=
		boost::bind(&QtWizardPA::getData, this,_1);
	}
	delete _ui;
}

QString QtWizardPA::getName() const {
	return tr("WizardPA");
}

QString QtWizardPA::getTitle() const {
	return tr("Meet your Personal Assistant");
}

QString QtWizardPA::getDescription() const {
	return tr("We're giving you one ... because you're kind of a big deal!"); //VOXOX -ASV- 07-10-2009: changed the text
}


int QtWizardPA::getStepNumber() const {
	return 2;
}


void QtWizardPA::readConfig() {

	if(_ui->txtName->text().trimmed() == ""){
		UserProfile & profile = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile();

		_wsPAGet = new WsPAGet(profile.getWengoAccount(),profile);

		_wsPAGet->paDataEvent +=
		boost::bind(&QtWizardPA::getData, this,_1);

		_wsPAGet->sendInfo();
	}

}

void QtWizardPA::getData(PAData * data){


	if(_ui->txtName->text().trimmed() == ""){

		QString paGender = QString::fromStdString(data->getVoiceGender());

		QString name = QString::fromStdString(data->getFullName());

		QString gender = QString::fromStdString(data->getGender());


		//Config & config = ConfigManager::getInstance().getCurrentConfig();
		if(paGender == "2"){
			_ui->cmbAssistantVoice->setCurrentIndex(1);
		}else if(paGender == "1"){
			_ui->cmbAssistantVoice->setCurrentIndex(0);
		}


		_ui->txtName->setText(name);

		if(gender=="M"){
			_ui->cmbUserGender->setCurrentIndex(1);
		}else if(gender == "F"){
			_ui->cmbUserGender->setCurrentIndex(0);
		}

	}


}


void QtWizardPA::saveConfig() {


	Config & config = ConfigManager::getInstance().getCurrentConfig();

	UserProfile & profile = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile();
	QString name = _ui->txtName->text().trimmed();
	QString wsUserGender = "";
	int paGender;
	//profile.setFirstName(name.toStdString());

	QString sex;
	if(_ui->cmbUserGender->currentIndex()==1){
		sex = "male";
		wsUserGender = "M";
	}else if(_ui->cmbUserGender->currentIndex()==0){
		sex = "female";
		wsUserGender = "F";
	}
	//profile.setSex(EnumSex::toSex(sex.toStdString()));


	if(_ui->cmbAssistantVoice->currentIndex()==0){
	
		//config.set(Config::WIZARD_ASSISTANT_IS_MALE, false);
		paGender = 1;

	}else if(_ui->cmbAssistantVoice->currentIndex()==1){

		//config.set(Config::WIZARD_ASSISTANT_IS_MALE, true);
		paGender = 2;


	}
	if(name!=""){
		_wsPA = new WsPA(profile.getWengoAccount(),profile);

		_wsPA->sendInfo(name.toStdString(),wsUserGender.toStdString(),paGender);
	}

	
}
