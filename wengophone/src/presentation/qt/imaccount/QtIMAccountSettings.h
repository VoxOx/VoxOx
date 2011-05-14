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

#ifndef QTIMACCOUNTSETTINGS_H
#define QTIMACCOUNTSETTINGS_H

//#include <presentation/qt/QtEnumIMProtocol.h>
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.05.29 

#include <imwrapper/IMAccount.h>

#include <QtCore/QObject>

class QtIMAccountPlugin;
class UserProfile;

class QWidget;
namespace Ui { class IMAccountTemplate; }

/**
 * Handles QtIMAccountPlugin: IM accounts (MSN, Yahoo, Jabber, AIM...).
 *
 * @author Tanguy Krotoff
 */
class QtIMAccountSettings : public QObject {
	Q_OBJECT
public:

	QtIMAccountSettings(UserProfile & userProfile,
		IMAccount imAccount, QWidget * parent);

	QtIMAccountSettings(UserProfile & userProfile,
		QtEnumIMProtocol::IMProtocol imProtocol, QWidget * parent);

private:

	void createIMProtocolWidget(QWidget * parent, QtEnumIMProtocol::IMProtocol imProtocol);

	Ui::IMAccountTemplate * _ui;

	IMAccount _imAccount;

	UserProfile & _userProfile;

	QtIMAccountPlugin * _imAccountPlugin;
};

#endif	//QTIMACCOUNTSETTINGS_H
