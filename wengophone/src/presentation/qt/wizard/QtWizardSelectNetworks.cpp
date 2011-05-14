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
#include "QtWizardSelectNetworks.h"

#include "ui_WizardSelectNetworks.h"
#include <control/CWengoPhone.h>
#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/profile/UserProfile.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <imwrapper/QtEnumIMProtocol.h>
#include <cutil/global.h>

#include <QtGui/QtGui>

QtWizardSelectNetworks::QtWizardSelectNetworks(CWengoPhone & cWengoPhone, QWidget * parent)
	: _cWengoPhone(cWengoPhone),
	QWidget(parent) {

	_ui = new Ui::WizardSelectNetworks();
	_ui->setupUi(this);

	readConfig();

}

QtWizardSelectNetworks::~QtWizardSelectNetworks() {
	delete _ui;
}

QString QtWizardSelectNetworks::getName() const {
	return tr("SelectNetworks");
}

QString QtWizardSelectNetworks::getTitle() const {
	return tr("Networks");
}

QString QtWizardSelectNetworks::getDescription() const {
	return tr("Take Control of your networks.");
}


int QtWizardSelectNetworks::getStepNumber() const {
	return 1;
}



void QtWizardSelectNetworks::readConfig() {
	 _aimAdded		= false;
	 _facebookAdded	= false;
	 _googleAdded	= false;
	 _icqAdded		= false;
	 _jabberAdded	= false;
	 _msnAdded		= false;
	 _myspaceAdded	= false;
	 _skypeAdded	= false;
	 _twitterAdded	= false;
	 _yahooAdded	= false;

	IMAccountList imAccountList =  _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getIMAccountManager().getIMAccountListCopy();
	for (IMAccountList::iterator it = imAccountList.begin(); it != imAccountList.end(); it++) {

		IMAccount imAccount = it->second;
		if(!imAccount.isVoxOxAccount())
		{
			QtEnumIMProtocol::IMProtocol imQtProtocol = imAccount.getQtProtocol();

			if(!EnumIMProtocol::isSip(imAccount.getProtocol())){//VOXOX CHANGE by Rolando - 2009.07.05 - this fixes a crash when tries to evaluate a sip account going to default case in switch

				switch( imQtProtocol )
				{
				case QtEnumIMProtocol::IMProtocolGoogleTalk:
					_googleAdded = true;
					_ui->chkGoogleTalk->setChecked(true);
					break;
			
				case QtEnumIMProtocol::IMProtocolAIM:
					_aimAdded = true;
					_ui->chkAIM->setChecked(true);
					break;

				case QtEnumIMProtocol::IMProtocolFacebook:
					_facebookAdded = true;
					_ui->chkFacebook->setChecked(true);
					break;

				case QtEnumIMProtocol::IMProtocolICQ:
					_icqAdded = true;
					_ui->chkICQ->setChecked(true);
					break;

				case QtEnumIMProtocol::IMProtocolJabber:
					_jabberAdded = true;
					_ui->chkJabber->setChecked(true);
					break;

				case QtEnumIMProtocol::IMProtocolMSN:
					_msnAdded = true;
					_ui->chkMSN->setChecked(true);
					break;

				case QtEnumIMProtocol::IMProtocolMYSPACE:
					_myspaceAdded = true;
					_ui->chkMySpace->setChecked(true);
					break;

				case QtEnumIMProtocol::IMProtocolSkype:
					_skypeAdded = true;
					_ui->chkSkype->setChecked(true);
					break;

				case QtEnumIMProtocol::IMProtocolTwitter:
					_twitterAdded = true;
					_ui->chkTwitter->setChecked(true);
					break;

				case QtEnumIMProtocol::IMProtocolYahoo:
					_yahooAdded = true;
					_ui->chkYahoo->setChecked(true);
					break;

				case QtEnumIMProtocol::IMProtocolVoxOx:
					break;						//Should not be here?

				default:
					assert(false);				//new QtProtocol?
				}
			}
		}
	}
}

void QtWizardSelectNetworks::saveConfig() {
}

bool QtWizardSelectNetworks::getAddAIM(){

	if(!_aimAdded && _ui->chkAIM->isChecked()){
		return true;
	}else{
		return false;
	}
}

bool QtWizardSelectNetworks::getAddFacebook(){

	if(!_facebookAdded && _ui->chkFacebook->isChecked()){
		return true;
	}else{
		return false;
	}
}

bool QtWizardSelectNetworks::getAddGoogleTalk(){

	if(!_googleAdded && _ui->chkGoogleTalk->isChecked()){
		return true;
	}else{
		return false;
	}
}

bool QtWizardSelectNetworks::getAddICQ(){

	if(!_icqAdded && _ui->chkICQ->isChecked()){
		return true;
	}else{
		return false;
	}
}

bool QtWizardSelectNetworks::getAddJabber(){

	if(!_jabberAdded && _ui->chkJabber->isChecked()){
		return true;
	}else{
		return false;
	}
}

bool QtWizardSelectNetworks::getAddMSN(){

	if(!_msnAdded && _ui->chkMSN->isChecked()){
		return true;
	}else{
		return false;
	}
}


bool QtWizardSelectNetworks::getAddMySpace(){

	if(!_myspaceAdded && _ui->chkMySpace->isChecked()){
		return true;
	}else{
		return false;
	}
}

bool QtWizardSelectNetworks::getAddSkype(){

	if(!_skypeAdded && _ui->chkSkype->isChecked()){
		return true;
	}else{
		return false;
	}
}

bool QtWizardSelectNetworks::getAddTwitter(){

	if(!_twitterAdded && _ui->chkTwitter->isChecked()){
		return true;
	}else{
		return false;
	}
}

bool QtWizardSelectNetworks::getAddYahoo(){

	if(!_yahooAdded && _ui->chkYahoo->isChecked()){
		return true;
	}else{
		return false;
	}
}

