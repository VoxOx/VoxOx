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

#ifndef OWQTLOGINDIALOG_H
#define OWQTLOGINDIALOG_H

#include <model/account/SipAccount.h>

#include <vector>
#include <string>

#include <QtGui/QDialog>

class CUserProfileHandler;
class QStackedWidget;
class QtWengoPhone;
class ILogin;

/**
 * Qt login window.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 * @author Xavier Desjardins
 */
class QtLoginDialog : public QDialog {
	Q_OBJECT
public:

	/*enum LoginDialogPage {//VOXOX CHANGE by Rolando 02-25-09
		AddAccount,
		AddWengoAccount,
		AddVoxOxAccount,
		AddSIPAccount,
		LogAccount
	};*/

	enum LoginDialogPage {//VOXOX CHANGE by Rolando 02-25-09		
		AddVoxOxAccount,
		LoginMessage
	};

	QtLoginDialog(QtWengoPhone * qtWengoPhone, CUserProfileHandler & cUserProfileHandler);

	~QtLoginDialog();

	void changePage(int newPage, std::string sipAccountName = "");

	void setInfoMessage();

	void setErrorMessage(const QString & message);

	void setAdviceMessage(const QString & message);

	void setProfileName(const QString & profileName);
	
	QtWengoPhone * getQtWengoPhone() {
		return _qtWengoPhone;
	}

	ILogin* currentPage() const;

public Q_SLOTS:
	/**
	 * login cancelled -- enable toolbar menu.
	 */
        void loginRejected();

	/**
	 * Init login entry and display a message on the login to say that the
	 * login/password is invalid.
	 */
	void setInvalidAccount(const SipAccount & sipAccount);

	/**
	 * Add the given SipAccount to the profile list and select it as default.
	 */
	void setValidAccount(SipAccount sipAccount);

private:

	QtWengoPhone * _qtWengoPhone;

	QStackedWidget * _centralWidget;

	std::vector<ILogin *> _loginPage;
};

#endif	//OWQTLOGINDIALOG_H
