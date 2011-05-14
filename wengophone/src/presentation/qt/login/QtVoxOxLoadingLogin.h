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

#ifndef OWQTVOXOXLOADINGLOGIN_H
#define OWQTVOXOXLOADINGLOGIN_H
#include <QtGui/QtGui>
class CUserProfileHandler;
class QWidget;
class QtWengoPhone;

namespace Ui { class VoxOxLoadingLogin; }

/**
 * 
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 * @author Xavier Desjardins
 */
class QtVoxOxLoadingLogin : public QWidget {
	Q_OBJECT
public:

	QtVoxOxLoadingLogin(QWidget* parent, QtWengoPhone * qtWengoPhone, CUserProfileHandler & cUserProfileHandler);

	~QtVoxOxLoadingLogin();

	void setProfileName(const QString & profileName);

	//void setLoadingMessage(const QString & message);


private Q_SLOTS:
	
	void languageChanged();

	//void updateLoadingMessage();

private:

	//void updateAvatarLabel();//VOXOX CHANGE by Rolando - 2009.05.21 -commented line, we are now using a logo

	Ui::VoxOxLoadingLogin * _ui;

	std::string _profileName;

	QtWengoPhone * _qtWengoPhone;

	CUserProfileHandler & _cUserProfileHandler;

	//QTimer * _timer;

	//int _seconds;

	//QString _loadingMessage;
};

#endif	//OWQTVOXOXLOADINGLOGIN_H
