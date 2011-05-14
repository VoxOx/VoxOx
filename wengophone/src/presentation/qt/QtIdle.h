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

#ifndef OWQTIDLE_H
#define OWQTIDLE_H

#include <qtutil/QObjectThreadSafe.h>

#include <util/Trackable.h>

#include <QtCore/QList>

class Settings;
class UserProfile;
class Idle;

/**
 * Detects idle status for IM presence.
 *
 * @see Idle
 * @author Tanguy Krotoff
 */
class QtIdle : public QObjectThreadSafe, public Trackable {
	Q_OBJECT
public:

	QtIdle(UserProfile & userProfile, QObject * parent);

	virtual ~QtIdle();

private Q_SLOTS:
	void slotSecondsIdle(int);

private:

	void initThreadSafe() { }

	void configChangedEventHandler(const std::string & key);

	void configChangedEventHandlerThreadSafe(std::string key);

	/**
	 * Set presence of all online IMAccounts to away
	 */
	void setOnlineIMAccountsAway();

	/**
	 * Restore IMAccounts which were changed from away to online by
	 * setOnlineIMAccountsAway
	 */
	void restoreOnlineIMAccounts();

	UserProfile & _userProfile;

	QList<std::string> _onlineIMAccountIdList;

	Idle * _idle;

	bool _autoAway;
};

#endif	//OWQTIDLE_H
