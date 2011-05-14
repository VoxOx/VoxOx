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

#ifndef OWQTLOGACCOUNT_H
#define OWQTLOGACCOUNT_H

#include "ILogin.h"

#include <map>

class CUserProfileHandler;
class QtLoginDialog;
class QWidget;

namespace Ui { class LogAccount; }

/**
 * Qt login window.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 * @author Xavier Desjardins
 */
class QtLogAccount : public ILogin {
	Q_OBJECT
public:

	QtLogAccount(QtLoginDialog * qtLoginDialog, QWidget* parent, CUserProfileHandler & cUserProfileHandler);

	~QtLogAccount();

	/**
	 * Add the given SipAccount to the profile list and select it as default.
	 */
	void setValidAccount(SipAccount sipAccount);

	virtual void load(std::string sipAccountName);

	virtual void initPage();

private Q_SLOTS:

	void createAccountButtonClicked();

	void helpButtonClicked();

	void forgotPasswordButtonClicked();

	void currentIndexChanged(const QString & profileName);

	void loginClicked();
	
	void modifyClicked();

	virtual void languageChanged();

private:

	void setAccountName(const QString & name);

	void setPassword(const QString & password);

	void setRememberPassword(bool remember);

	virtual void setInfoMessage();

	virtual void setErrorMessage(const QString & message);

	virtual void setLoginLabel(const QString & message);

	/**
	 * Initializes the widgets.
	 */
	void init();

	/**
	 * map of profile
	 */
	 std::map<std::string, std::string> profileMap;

	Ui::LogAccount * _ui;
};

#endif	//OWQTLOGACCOUNT_H
