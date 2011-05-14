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

#ifndef QTIMACCOUNTPLUGIN_H
#define QTIMACCOUNTPLUGIN_H

#include <QtCore/QObject>

#include <util/Interface.h>
#include <imwrapper/IMAccount.h>	//VOXOX - JRT - 2009.07.14 

class IMAccount;
class UserProfile;
class QDialog;

/**
 * Interface for IM accounts (MSN, AIM, Yahoo, Jabber...).
 *
 * @author Tanguy Krotoff
 */
class QtIMAccountPlugin : public QObject, Interface {
	Q_OBJECT
public:

	QtIMAccountPlugin(UserProfile & userProfile, IMAccount & imAccount, QDialog * parent);

	virtual QWidget * getWidget() const = 0;

	/**
	 * @return true if entered value are valid.
	 */
	virtual bool isValid() const = 0;

	/**
	 * Sets entered data on IMAccount.
	 */
	virtual void setIMAccount() = 0;

public Q_SLOTS:

	void checkAndSave();

protected:

	virtual void init() = 0;

	/**
	 * Saves the entered data.
	 */
	void save();

	IMAccount&	_imAccount;
	IMAccount	_origIMAccount;	//VOXOX - JRT - 2009.07.14 

	UserProfile & _userProfile;

	QDialog * _parentDialog;
};

#endif	//QTIMACCOUNTPLUGIN_H
