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

#ifndef QTPRESENCESIPMENUMANAGER_H
#define QTPRESENCESIPMENUMANAGER_H

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <model/presence/Presence.h>
#include "QtPresenceMenuManager.h"

//class CUserProfileHandler;
class CUserProfile;
class QAction;
class QActionGroup;
class QMenu;
class QPixmap;
class QString;
class QtWengoPhone;


/**
 * This class manages the menu actions for the global presence
 *
 * @author Aurelien Gateau
 */
class QtPresenceSIPMenuManager : public QtPresenceMenuManager {
	Q_OBJECT
public:
	/*QtPresenceSIPMenuManager(QObject * parent, CUserProfileHandler * cUserProfileHandler, 
		QtWengoPhone * qtWengoPhone);*/

	QtPresenceSIPMenuManager(QObject * parent, CUserProfile * cUserProfile, 
		QtWengoPhone * qtWengoPhone);

	~QtPresenceSIPMenuManager();


	/**
	 * Fills menu with actions to modify the global presence
	 */
	virtual void addPresenceActions(QMenu* menu, EnumPresenceState::PresenceState currentPresenceState = EnumPresenceState::PresenceStateUnavailable);

public Q_SLOTS:
	void slotGlobalPresenceActionTriggered();
	void logOff();
};

#endif	// QTPRESENCESIPMENUMANAGER_H
