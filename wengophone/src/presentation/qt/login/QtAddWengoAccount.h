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

#ifndef OWQTADDWENGOACCOUNT_H
#define OWQTADDWENGOACCOUNT_H

#include "ILogin.h"

class CUserProfileHandler;
class QtLoginDialog;
class QWidget;

namespace Ui { class AddWengoAccount; }

/**
 * Qt login window.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 * @author Xavier Desjardins
 */
class QtAddWengoAccount : public ILogin {
	Q_OBJECT
public:

	QtAddWengoAccount(QtLoginDialog * qtLoginDialog, QWidget* parent, CUserProfileHandler & cUserProfileHandler);

	~QtAddWengoAccount();

	virtual void load(std::string sipAccountName);
	
	virtual void load(const SipAccount & sipAccount);

	virtual void initPage();
	
private Q_SLOTS:

	void createAccountButtonClicked();

	void helpButtonClicked();

	void forgotPasswordButtonClicked();

	void loginClicked();
	
	virtual void languageChanged();

private:

	virtual void setInfoMessage();

	virtual void setErrorMessage(const QString & message);

	virtual void setLoginLabel(const QString & message);

	void keyPressEvent(QKeyEvent * event);

	Ui::AddWengoAccount * _ui;
};

#endif	//OWQTADDWENGOACCOUNT_H
