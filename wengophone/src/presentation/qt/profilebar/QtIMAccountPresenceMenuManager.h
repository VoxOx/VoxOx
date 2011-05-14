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

#ifndef QTIMACCOUNTPRESENCEMENUMANAGER_H
#define QTIMACCOUNTPRESENCEMENUMANAGER_H

#include <QtCore/QMap>
#include <QtCore/QObject>

#include <model/presence/Presence.h>
#include <imwrapper/IMAccount.h>

class CUserProfile;
class QAction;
class QIcon;
class QMenu;
class QPixmap;
class QString;
class QActionGroup;


/**
 * This class manages the menu actions for an IM account
 *
 * @author Aurelien Gateau
 */
class QtIMAccountPresenceMenuManager : public QObject 
{
	Q_OBJECT
public:
	QtIMAccountPresenceMenuManager(QObject* parent, CUserProfile&, std::string imAccountId);
	~QtIMAccountPresenceMenuManager();
	/**
	 * Fills menu with actions to modify the IM account presence
	 */
	void addPresenceActions(QMenu* menu,EnumPresenceState::PresenceState presenceState, const QString& errMsg );

private Q_SLOTS:
	void slotPresenceActionTriggered();
//	void disconnect();	//VOXOX - JRT - 2009.09.14 
	void deleteAccount();
	void viewLoginError();
	void updateActions();
	void showUpdate();
	void addFriendsFromMySpace();

private:
	void addPresenceAction(QMenu* menu, EnumPresenceState::PresenceState state, EnumPresenceState::PresenceState currentPresenceState, const QPixmap& pix, const QString& text);
	void addSpecificNetworkPresenceActions(QMenu* menu, std::string imAccountId, EnumPresenceState::PresenceState currentPresenceState);
	void setupDisconnectAction();	//VOXOX - JRT - 2009.09.14 


	QMap<QObject*, EnumPresenceState::PresenceState> _actionToState;
	CUserProfile& _cUserProfile;
	std::string _imAccountId;
	QAction* _disconnectAction;//VOXOX CHANGE Rolando 03-20-09
	QAction* _modifyAccountAction;//VOXOX CHANGE Rolando 04-22-09
	QActionGroup * _actionsGroup;//VOXOX CHANGE Rolando 03-20-09
	QMap<QObject*,QAction*> _actionToObject;//VOXOX CHANGE Rolando 03-20-09

	QString _errMsg;
};

#endif	// QTIMACCOUNTPRESENCEMENUMANAGER_H
