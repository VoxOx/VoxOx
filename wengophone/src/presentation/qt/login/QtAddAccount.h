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

#ifndef OWQTADDACCOUNT_H
#define OWQTADDACCOUNT_H

#include "ILogin.h"

class CUserProfileHandler;
class QtLoginDialog;
class QWidget;

namespace Ui { class AddAccount; }

/**
 * Qt login window.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 * @author Xavier Desjardins
 */
class QtAddAccount : public ILogin {
	Q_OBJECT
public:

	QtAddAccount(QtLoginDialog * qtLoginDialog, QWidget* parent, CUserProfileHandler & cUserProfileHandler);

	~QtAddAccount();

	virtual void load(std::string sipAccountName);

private Q_SLOTS:

	void createAccountClicked(const QString & url);

	void helpButtonClicked();

	void nextClicked();
	
	virtual void languageChanged();

private:

	virtual void setInfoMessage();

	virtual void setErrorMessage(const QString & message);

	virtual void setLoginLabel(const QString & message);

	void keyPressEvent(QKeyEvent * event);

	void initPage();

	Ui::AddAccount * _ui;
};

#endif	//OWQTADDACCOUNT_H
