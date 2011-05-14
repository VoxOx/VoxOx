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

#ifndef QTIMACCOUNTMANAGER_H
#define QTIMACCOUNTMANAGER_H

#include <memory>

#include <util/NonCopyable.h>

#include <QtCore/QObject>

class CWengoPhone;
class IMAccount;
class UserProfile;

class QWidget;
class QAction;
class QTreeWidget;
class QTreeWidgetItem;

namespace Ui { class IMAccountManager; }

/**
 * Manages IM accounts: add/delete/modify IM accounts.
 *
 * Can be an embedded widget or a separated window.
 *
 * Very similar to QtIMContactManager but for IM accounts.
 *
 * @see QtIMContactManager
 * @see UserProfile
 * @see IMAccount
 * @author Tanguy Krotoff
 */
class QtIMAccountManager : public QObject, NonCopyable {
	Q_OBJECT
public:

	QtIMAccountManager(UserProfile & userProfile, bool showAsDialog, QWidget * parent);

	~QtIMAccountManager();

	QWidget * getWidget() const {
		return _imAccountManagerWidget;
	}

private Q_SLOTS:

	void addIMAccount(QAction * action);

	void deleteIMAccount();

	void modifyIMAccount();

	void itemDoubleClicked(QTreeWidgetItem * item, int column);

	void updateButtons();

private:

	void loadIMAccounts();

	UserProfile & _userProfile;

	Ui::IMAccountManager * _ui;

	QWidget * _imAccountManagerWidget;

	std::auto_ptr<IMAccount> getIMAccountFromItem(QTreeWidgetItem*);
};

#endif	//QTIMACCOUNTMANAGER_H
