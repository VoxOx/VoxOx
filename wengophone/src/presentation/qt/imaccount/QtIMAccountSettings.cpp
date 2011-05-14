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
#include "QtIMAccountSettings.h"

#include "ui_IMAccountTemplate.h"

#include "QtIMAccountPlugin.h"
#include "QtMSNSettings.h"
#include "QtAIMSettings.h"
#include "QtICQSettings.h"
#include "QtYahooSettings.h"
#include "QtMySpaceSettings.h"
#include "QtFacebookSettings.h"
#include "QtJabberSettings.h"
#include "QtTwitterSettings.h"
#include "QtGoogleTalkSettings.h"
#include "QtSkypeSettings.h"

#include <imwrapper/IMAccountParametersXMLSerializer.h>
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.05.29 

#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileFileStorage.h>
#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <util/File.h>
#include <util/Logger.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/Widget.h>

#include <QtGui/QtGui>

QtIMAccountSettings::QtIMAccountSettings(UserProfile & userProfile, IMAccount imAccount, QWidget * parent)
	: QObject(parent),
	_userProfile(userProfile) 
{
	_imAccountPlugin = NULL;
	_imAccount		 = imAccount;

//	QtEnumIMProtocol::IMProtocol protocol = QtEnumIMProtocolMap::getInstance().findByModelProtocol( imAccount.getProtocol() )->getQtProtocol();
//	createIMProtocolWidget(parent, protocol );
	createIMProtocolWidget(parent, _imAccount.getQtProtocol() );	//VOXOX - JRT - 2009.07.01 
}

QtIMAccountSettings::QtIMAccountSettings(UserProfile & userProfile, QtEnumIMProtocol::IMProtocol imProtocol, QWidget * parent)
	: QObject(parent),
	_userProfile(userProfile) 
{
	_imAccountPlugin = NULL;
	EnumIMProtocol::IMProtocol modelProtocol = QtEnumIMProtocolMap::getInstance().toModelIMProtocol( imProtocol );
	_imAccount = IMAccount( modelProtocol );

	// Init _imAccount parameters
	std::string imAccountParametersData = UserProfileFileStorage::loadSystemIMAccountParametersData();
	IMAccountParametersXMLSerializer serializer(_imAccount.getIMAccountParameters());
	serializer.unserializeSystemSettings(imAccountParametersData);

	createIMProtocolWidget(parent, imProtocol);
}

void QtIMAccountSettings::createIMProtocolWidget(QWidget* parent, QtEnumIMProtocol::IMProtocol imProtocol) 
{
	QDialog * imAccountTemplateWindow = new QDialog(parent);

	_ui = new Ui::IMAccountTemplate();
	_ui->setupUi(imAccountTemplateWindow);

	switch (imProtocol) {
	case QtEnumIMProtocol::IMProtocolMSN: {
		_imAccountPlugin = new QtMSNSettings(_userProfile, _imAccount, imAccountTemplateWindow);
		break;
	}
	case QtEnumIMProtocol::IMProtocolMYSPACE: {
		_imAccountPlugin = new QtMySpaceSettings(_userProfile, _imAccount, imAccountTemplateWindow);
		break;
	}

	case QtEnumIMProtocol::IMProtocolFacebook: {
		_imAccountPlugin = new QtFacebookSettings(_userProfile, _imAccount, imAccountTemplateWindow);
		break;
	}

   	case QtEnumIMProtocol::IMProtocolTwitter: {
		_imAccountPlugin = new QtTwitterSettings(_userProfile, _imAccount, imAccountTemplateWindow);
		break;
	}

    case QtEnumIMProtocol::IMProtocolSkype: {
		_imAccountPlugin = new QtSkypeSettings(_userProfile, _imAccount, imAccountTemplateWindow);
		break;
	}

	case QtEnumIMProtocol::IMProtocolYahoo: {
		_imAccountPlugin = new QtYahooSettings(_userProfile, _imAccount, imAccountTemplateWindow);
		break;
	}

	case QtEnumIMProtocol::IMProtocolAIM: {
		_imAccountPlugin = new QtAIMSettings(_userProfile, _imAccount, imAccountTemplateWindow);
		break;
	}

	case QtEnumIMProtocol::IMProtocolICQ: {
		_imAccountPlugin = new QtICQSettings(_userProfile, _imAccount, imAccountTemplateWindow);
		break;
	}

	case QtEnumIMProtocol::IMProtocolJabber: {
		_imAccountPlugin = new QtJabberSettings(_userProfile, _imAccount, imAccountTemplateWindow);
		break;
	}

	case QtEnumIMProtocol::IMProtocolGoogleTalk: {
		_imAccountPlugin = new QtGoogleTalkSettings(_userProfile, _imAccount, imAccountTemplateWindow);
		break;
	}

	case QtEnumIMProtocol::IMProtocolVoxOx:
		assert(false);								//VOXOX - JRT - 2009.07.01 - if we got here, then some one changed QtEnumIMProtocol.
		break;

	default:
		LOG_FATAL("unknown IM protocol=" + String::fromNumber(imProtocol));
	}

	SAFE_CONNECT_RECEIVER(_ui->saveButton,   SIGNAL(clicked()), _imAccountPlugin,		 SLOT(checkAndSave()));
	SAFE_CONNECT_RECEIVER(_ui->cancelButton, SIGNAL(clicked()), imAccountTemplateWindow, SLOT(reject()));

	QWidget * imProtocolWidget = _imAccountPlugin->getWidget();
	QString styleSheet("QLabel,QCheckBox{color:#343434;}");
	imProtocolWidget->setStyleSheet(styleSheet);
	Widget::createLayout(_ui->settingsGroupBox)->addWidget(imProtocolWidget);
	_ui->settingsGroupBox->setTitle(imProtocolWidget->windowTitle());

	imAccountTemplateWindow->setWindowTitle(imProtocolWidget->windowTitle());
	imAccountTemplateWindow->exec();
}
