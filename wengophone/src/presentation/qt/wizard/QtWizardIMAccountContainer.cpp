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
#include "QtWizardIMAccountContainer.h"

#include "ui_WizardIMAccountContainer.h"

#include <presentation/qt/imaccount/QtIMAccountPlugin.h>
#include <presentation/qt/imaccount/QtMSNSettings.h>
#include <presentation/qt/imaccount/QtAIMSettings.h>
#include <presentation/qt/imaccount/QtICQSettings.h>
#include <presentation/qt/imaccount/QtYahooSettings.h>
#include <presentation/qt/imaccount/QtMySpaceSettings.h>
#include <presentation/qt/imaccount/QtFacebookSettings.h>
#include <presentation/qt/imaccount/QtJabberSettings.h>
#include <presentation/qt/imaccount/QtTwitterSettings.h>
#include <presentation/qt/imaccount/QtGoogleTalkSettings.h>
#include <presentation/qt/imaccount/QtSkypeSettings.h>

#include <imwrapper/IMAccountParametersXMLSerializer.h>
#include <imwrapper/QtEnumIMProtocol.h>	

#include <model/profile/UserProfile.h>
#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <model/profile/UserProfileFileStorage.h>
#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <util/File.h>
#include <util/Logger.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/Widget.h>

#include <QtGui/QtGui>

//#include <QtEnumIMProtocol.h>	//VOXOX - JRT - 2009.04.21 


QtWizardIMAccountContainer::QtWizardIMAccountContainer(CWengoPhone & cWengoPhone,QtEnumIMProtocol::IMProtocol imProtocol, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone) {

	_imAccountPlugin = NULL;
	EnumIMProtocol::IMProtocol modelProtocol = QtEnumIMProtocolMap::getInstance().toModelIMProtocol( imProtocol );
	_imAccount = IMAccount( modelProtocol );
	//End VoxOx

	// Init _imAccount parameters
	std::string imAccountParametersData = UserProfileFileStorage::loadSystemIMAccountParametersData();
	IMAccountParametersXMLSerializer serializer(_imAccount.getIMAccountParameters());
	serializer.unserializeSystemSettings(imAccountParametersData);

	createIMProtocolWidget(imProtocol);
}

void QtWizardIMAccountContainer::createIMProtocolWidget(
	QtEnumIMProtocol::IMProtocol imProtocol) {
	_createdIMProtocol = imProtocol;
	UserProfile & userProfile  = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile();
	_ui = new Ui::WizardIMAccountContainer();
	_ui->setupUi(this);

	switch (imProtocol) {
	case QtEnumIMProtocol::IMProtocolMSN: {
		_imAccountPlugin = new QtMSNSettings(userProfile, _imAccount, NULL);
		break;
	}
	case QtEnumIMProtocol::IMProtocolMYSPACE: {
		_imAccountPlugin = new QtMySpaceSettings(userProfile, _imAccount, NULL);
		break;
	}

	case QtEnumIMProtocol::IMProtocolFacebook: {
		_imAccountPlugin = new QtFacebookSettings(userProfile, _imAccount, NULL);
		break;
	}

   	case QtEnumIMProtocol::IMProtocolTwitter: {
		_imAccountPlugin = new QtTwitterSettings(userProfile, _imAccount, NULL);
		break;
	}

    case QtEnumIMProtocol::IMProtocolSkype: {
		_imAccountPlugin = new QtSkypeSettings(userProfile, _imAccount, NULL);
		break;
	}

	case QtEnumIMProtocol::IMProtocolYahoo: {
		_imAccountPlugin = new QtYahooSettings(userProfile, _imAccount, NULL);
		break;
	}

	case QtEnumIMProtocol::IMProtocolAIM: {
		_imAccountPlugin = new QtAIMSettings(userProfile, _imAccount, NULL);
		break;
	}

	case QtEnumIMProtocol::IMProtocolICQ: {
		_imAccountPlugin = new QtICQSettings(userProfile, _imAccount, NULL);
		break;
	}

	case QtEnumIMProtocol::IMProtocolJabber: {
		_imAccountPlugin = new QtJabberSettings(userProfile, _imAccount, NULL);
		break;
	}

	case QtEnumIMProtocol::IMProtocolGoogleTalk: {
		_imAccountPlugin = new QtGoogleTalkSettings(userProfile, _imAccount, NULL);
		break;
	}

	default:
		LOG_FATAL("unknown IM protocol=" + String::fromNumber(imProtocol));
	}

	//saveButton
	//*SAFE_CONNECT_RECEIVER(_ui->saveButton, SIGNAL(clicked()),
	//	_imAccountPlugin, SLOT(checkAndSave()));*/

	////cancelButton
	//*SAFE_CONNECT_RECEIVER(_ui->cancelButton, SIGNAL(clicked()),
	//	imAccountTemplateWindow, SLOT(reject()));*/

	QWidget * imProtocolWidget = _imAccountPlugin->getWidget();
	QString styleSheet("QLabel,QCheckBox{color:#ffffff;}");
	imProtocolWidget->setStyleSheet(styleSheet);
	Widget::createLayout(_ui->frame)->addWidget(imProtocolWidget);

}

QtWizardIMAccountContainer::~QtWizardIMAccountContainer() {
	delete _ui;
}

QString QtWizardIMAccountContainer::getName() const {

	return "IMAccount";
}

QString QtWizardIMAccountContainer::getTitle() const {
	
	
	return QString::fromStdString(QtEnumIMProtocolMap::getInstance().toString(_createdIMProtocol));
	
}

QString QtWizardIMAccountContainer::getDescription() const {
	return tr("");
}

int QtWizardIMAccountContainer::getStepNumber() const {
	return 1;
}


QString QtWizardIMAccountContainer::getPixmapPath() const {

	QString imagePath = "";
	switch (_createdIMProtocol) {
	case QtEnumIMProtocol::IMProtocolMSN: {
		imagePath = ":pics/wizard/protocols/msn.png";
		break;
	}
	case QtEnumIMProtocol::IMProtocolMYSPACE: {
		imagePath = ":pics/wizard/protocols/myspace.png";
		break;
	}

	case QtEnumIMProtocol::IMProtocolFacebook: {
		imagePath = ":pics/wizard/protocols/facebook.png";
		break;
	}

   	case QtEnumIMProtocol::IMProtocolTwitter: {
		imagePath = ":pics/wizard/protocols/twitter.png";
		break;
	}

    case QtEnumIMProtocol::IMProtocolSkype: {
		imagePath = ":pics/wizard/protocols/skype.png";
		break;
	}

	case QtEnumIMProtocol::IMProtocolYahoo: {
		imagePath = ":pics/wizard/protocols/yahoo.png";
		break;
	}

	case QtEnumIMProtocol::IMProtocolAIM: {
		imagePath = ":pics/wizard/protocols/aim.png";
		break;
	}

	case QtEnumIMProtocol::IMProtocolICQ: {
		imagePath = ":pics/wizard/protocols/icq.png";
		break;
	}

	case QtEnumIMProtocol::IMProtocolJabber: {
		imagePath = ":pics/wizard/protocols/jabber.png";
		break;
	}

	case QtEnumIMProtocol::IMProtocolGoogleTalk: {
		imagePath = ":pics/wizard/protocols/gtalk.png";
		break;
	}
	}

	return imagePath;
}

void QtWizardIMAccountContainer::readConfig() {
	
}

void QtWizardIMAccountContainer::saveConfig() {

	_imAccountPlugin->checkAndSave();
}

