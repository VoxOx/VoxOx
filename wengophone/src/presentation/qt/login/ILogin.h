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

#ifndef ILOGIN_H
#define ILOGIN_H

#include "QtLoginDialog.h"

#include <model/account/SipAccount.h>

#include <control/profile/CUserProfileHandler.h>

#include <QtGui/QWidget>
#include <QtCore/QString>

#include <string>

/**
 * Qt login window.
 *
 * @author Xavier Desjardins
 */
class ILogin : public QWidget {
	Q_OBJECT
public:

	ILogin(QtLoginDialog * qtLoginDialog, CUserProfileHandler & cUserProfileHandler) ;

	virtual ~ILogin();

	/**
	 * loads and uses a sipaccount to initialize widget value
	 *
	 * @param sipAccountName name of the SipAccount to be loaded
	 */
	virtual void load(std::string sipAccountName) = 0;
	
	/**
	 * loads and uses a sipaccount to initialize widget value
	 *
	 * @param sipAccount SipAccount to be loaded
	 */
	virtual void load(const SipAccount & sipAccount);

	/**
	 * Initialze the page. Called when the page is shown.
	 */
	virtual void initPage() = 0;

	/**
	 * Sets the text of infoLabel to 'message' and colorize it to normal color.
	 */
	virtual void setInfoMessage() = 0;

	/**
	 * Sets the text of infoLabel to 'message' and colorize it to red.
	 */
	virtual void setErrorMessage(const QString & message) = 0;

	virtual void setAdviceMessage(const QString & message){};

	virtual void setProfileName(const QString & profileName){};

	/**
	 * Sets the login label.
	 */
	virtual void setLoginLabel(const QString & message) = 0;

	virtual void setBackPage(QtLoginDialog::LoginDialogPage);

private Q_SLOTS:

	virtual void languageChanged() = 0;

	void goBack();

protected:

	QtLoginDialog * _loginDialog;

	CUserProfileHandler & _cUserProfileHandler;

	QtLoginDialog::LoginDialogPage _backPage;
};

#endif	//ILOGIN_H
