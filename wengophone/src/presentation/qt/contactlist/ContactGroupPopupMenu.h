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

#ifndef OWCONTACTGROUPPOPUPMENU_H
#define OWCONTACTGROUPPOPUPMENU_H

#include <util/NonCopyable.h>

#include <QtGui/QMenu>

class CContactList;
class QTreeWidgetItem;
class QModelIndex;

/**
 * Menu of a contact group, popped each time a user clicks on a contact group using
 * the right button of the mouse.
 *
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class ContactGroupPopupMenu : public QMenu, NonCopyable {
	Q_OBJECT
public:

	/**
	 * Constructs a ContactGroupPopupMenu.
	 *
	 * @param parent parent widget
	 */
	ContactGroupPopupMenu(CContactList & cContactList, QWidget * parent);

public Q_SLOTS:

	/**
	 * The popup menu is being showed.
	 *
	 * @param groupId the UUID of the group to diplay the menu
	 */
	void showMenu(const QString & groupId);

private Q_SLOTS:

	/**
	 * Removes a contact group from ContactList.
	 *
	 * The user clicks on "remove contact group" in the menu.
	 */
	void removeContactGroup() const;

	/**
	 * Rename a contact group.
	 *
	 * The user clicks on "rename contact group" in the menu.
	 */
	void renameContactGroup() const;

	/**
	 * Sends a SMS to a contact group.
	 *
	 * The user clicks on "Send SMS" in the menu.
	 */
//	void sendSms();	//VOXOX - JRT - 2009.09.01 - Empty method

private:

	QString _groupId;

	CContactList & _cContactList;
};

#endif //OWCONTACTGROUPPOPUPMENU_H
