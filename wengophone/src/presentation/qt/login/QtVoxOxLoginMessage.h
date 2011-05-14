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

#ifndef OWQTVOXOXLOGINMESSAGE_H
#define OWQTVOXOXLOGINMESSAGE_H

#include <model/account/SipAccount.h>
#include <QtGui/QtGui>
class CUserProfileHandler;
class QWidget;
class QtWengoPhone;

namespace Ui { class LoginMessage; }

/**
 * Qt login window.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 * @author Xavier Desjardins
 */
class QtVoxOxLoginMessage : public QWidget {
	Q_OBJECT
public:

	QtVoxOxLoginMessage(QWidget* parent, QtWengoPhone * qtWengoPhone, CUserProfileHandler & cUserProfileHandler);

	~QtVoxOxLoginMessage();

	enum MessageType {		
		ErrorMessage,
		LoadingMessage
	};

	void setProfileName(const QString & profileName);

	void setErrorMessage(const QString & message);

	void setAdviceMessage(const QString & message);

Q_SIGNALS:

	void tryAgainButton(std::string profileName);
	
private Q_SLOTS:

	void tryAgainButtonClicked();
	
	void languageChanged();

private:

	//void updateAvatarLabel();//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo

	void keyPressEvent(QKeyEvent * event);

	Ui::LoginMessage * _ui;

	std::string _profileName;

	QtWengoPhone * _qtWengoPhone;

	CUserProfileHandler & _cUserProfileHandler;
};

#endif	//OWQTVOXOXLOGINMESSAGE_H
