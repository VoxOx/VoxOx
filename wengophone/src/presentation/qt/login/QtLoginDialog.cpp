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
#include "QtLoginDialog.h"

#include "QtAddAccount.h"
#include "QtAddWengoAccount.h"
#include "QtAddVoxOxAccount.h"
#include "QtAddSIPAccount.h"
#include "QtLogAccount.h"
#include "QtLoginMessage.h"

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/QtToolBar.h>

#include <control/profile/CUserProfileHandler.h>

#include <model/account/SipAccount.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <qtutil/SafeConnect.h>

#include <util/Logger.h>


#include <util/String.h>

#include <QtGui/QtGui>

QtLoginDialog::QtLoginDialog(QtWengoPhone * qtWengoPhone, CUserProfileHandler & cUserProfileHandler)
	:QDialog(qtWengoPhone->getWidget()),
	_qtWengoPhone(qtWengoPhone) {

	_centralWidget = new QStackedWidget(this);
	//_loginPage.push_back(new QtAddAccount(this, _centralWidget, cUserProfileHandler));//VOXOX CHANGE by Rolando 02-25-09
	//_loginPage.push_back(new QtAddWengoAccount(this, _centralWidget, cUserProfileHandler));//VOXOX CHANGE by Rolando 02-25-09
	_loginPage.push_back(new QtAddVoxOxAccount(this, _centralWidget, cUserProfileHandler));
	_loginPage.push_back(new QtLoginMessage(this, _centralWidget, cUserProfileHandler));
	
	//_loginPage.push_back(new QtAddSIPAccount(this, _centralWidget, cUserProfileHandler));//VOXOX CHANGE by Rolando 02-25-09
	//_loginPage.push_back(new QtLogAccount(this, _centralWidget, cUserProfileHandler));//VOXOX CHANGE by Rolando 02-25-09
    SAFE_CONNECT(this, SIGNAL(rejected()), SLOT(loginRejected()));

    //_centralWidget->addWidget(_loginPage[AddAccount]);//VOXOX CHANGE by Rolando 02-25-09
    //_centralWidget->addWidget(_loginPage[AddWengoAccount]);//VOXOX CHANGE by Rolando 02-25-09
	_centralWidget->addWidget(_loginPage[AddVoxOxAccount]);
	_centralWidget->addWidget(_loginPage[LoginMessage]);
    //_centralWidget->addWidget(_loginPage[AddSIPAccount]);//VOXOX CHANGE by Rolando 02-25-09
	//_centralWidget->addWidget(_loginPage[LogAccount]);//VOXOX CHANGE by Rolando 02-25-09

	 
	
	//changePage(LogAccount);//VOXOX CHANGE by Rolando 02-25-09
	changePage(AddVoxOxAccount);//VOXOX CHANGE by Rolando 02-25-09
}

QtLoginDialog::~QtLoginDialog() {
}

void QtLoginDialog::changePage(int newPage, std::string sipAccountName) {
	if ( (newPage >= 0) && (newPage < _centralWidget->count())) 
	{
		int page = newPage ;
		
	/*#ifndef CUSTOM_ACCOUNT
		if( page == 0)
		{
			page = 2;
		}
	#endif*/
		
		_centralWidget->setCurrentIndex(page);
		_loginPage[page]->initPage();
		if (!sipAccountName.empty()) {
			_loginPage[page]->load(sipAccountName);
		}
	}
}

ILogin* QtLoginDialog::currentPage() const {
	return _loginPage[_centralWidget->currentIndex()];
}

void QtLoginDialog::loginRejected() {
    _qtWengoPhone->getQtToolBar()->enableMenuBar();
}

void QtLoginDialog::setInvalidAccount(const SipAccount & sipAccount) {
	if (sipAccount.getType() ==  SipAccount::SipAccountTypeWengo) {
		//wengo account
		/*changePage(AddWengoAccount);//VOXOX CHANGE by Rolando 02-25-09
		_loginPage[AddWengoAccount]->load(sipAccount);*/		
		changePage(LoginMessage);//VOXOX CHANGE by Rolando 02-25-09
		_loginPage[LoginMessage]->load(sipAccount);//VOXOX CHANGE by Rolando 02-25-09
	} else {
		//"generic" sip account
		/*changePage(AddSIPAccount);//VOXOX CHANGE by Rolando 02-25-09
		_loginPage[AddSIPAccount]->load(sipAccount);//VOXOX CHANGE by Rolando 02-25-09*/
		changePage(LoginMessage);//VOXOX CHANGE by Rolando 02-25-09
		_loginPage[LoginMessage]->load(sipAccount);//VOXOX CHANGE by Rolando 02-25-09	
	}
}

void QtLoginDialog::setProfileName(const QString & profileName){	
	_loginPage[_centralWidget->currentIndex()]->setProfileName(profileName);//VOXOX CHANGE by Rolando 02-25-09
	
}

void QtLoginDialog::setValidAccount(SipAccount sipAccount) {
	/*changePage(LogAccount);//VOXOX CHANGE by Rolando 02-25-09
	_loginPage[LogAccount]->load(sipAccount);
	_loginPage[LogAccount]->setInfoMessage();*/
	changePage(AddVoxOxAccount);//VOXOX CHANGE by Rolando 02-25-09
	_loginPage[AddVoxOxAccount]->load(sipAccount);//VOXOX CHANGE by Rolando 02-25-09
	_loginPage[AddVoxOxAccount]->setInfoMessage();//VOXOX CHANGE by Rolando 02-25-09
}

void QtLoginDialog::setInfoMessage() {
	_loginPage[_centralWidget->currentIndex()]->setInfoMessage();
}

void QtLoginDialog::setErrorMessage(const QString & message) {
	_loginPage[_centralWidget->currentIndex()]->setErrorMessage(message);
}

void QtLoginDialog::setAdviceMessage(const QString & message) {//VOXOX CHANGE by Rolando 03-02-09, new Message
	_loginPage[_centralWidget->currentIndex()]->setAdviceMessage(message);
}