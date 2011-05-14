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

#ifndef OWQTLOGINMESSAGE_H
#define OWQTLOGINMESSAGE_H

#include "ILogin.h"

class CUserProfileHandler;
class QtLoginDialog;
class QWidget;

namespace Ui { class LoginMessage; }

/**
 * Qt login window.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 * @author Xavier Desjardins
 */
class QtLoginMessage : public ILogin {
	Q_OBJECT
public:

	QtLoginMessage(QtLoginDialog * qtLoginDialog, QWidget* parent, CUserProfileHandler & cUserProfileHandler);

	~QtLoginMessage();

	virtual void setProfileName(const QString & profileName);

	virtual void load(std::string sipAccountName);
	
	virtual void load(const SipAccount & sipAccount);

	virtual void initPage();
	
private Q_SLOTS:

	void helpButtonClicked();

	void tryAgainButtonClicked();
	
	virtual void languageChanged();

private:

	virtual void setInfoMessage();

	virtual void setErrorMessage(const QString & message);

	virtual void setAdviceMessage(const QString & message);

	virtual void setLoginLabel(const QString & message);

	//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo
	//void updateAvatarLabel();

	void keyPressEvent(QKeyEvent * event);

	Ui::LoginMessage * _ui;

	std::string _profileName;
};

#endif	//OWQTLOGINMESSAGE_H
