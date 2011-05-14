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

#ifndef OWCONTACTLISTMANAGER_H
#define OWCONTACTLISTMANAGER_H

#include "QtContact.h"
#include "QtContactPixmap.h"

#include <QtCore/QString>

class CContact;
class QMutex;
class QTreeWidget;

/**
 * QtContact storage class for fast lookup
 *
 * @author Mr K.
 */
class QtContactListManager 
{
public:
	static QtContactListManager * getInstance();

	virtual ~QtContactListManager();

	bool addContact   (QtContact * qtContact);
	void removeContact(QtContact * qtContact);

	bool		isBlocked (const QString & contactId) const;	//VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
	QtContact * getContact(const QString & contactId) const;

//	void paintContact(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index);	//VOXOX - JRT - 2009.08.10 

	QTreeWidget* getTreeWidget() const						{ return _tree; }
	void		 setTreeWidget(QTreeWidget * tree)			{ _tree = tree; }

	void openProfile(const QString & contactId);
	void clear();

private:
	QtContactListManager();

	QtContactListManager(const QtContactListManager & other)
		: _tree(other._tree),
		_contactList(other._contactList) { }

	QtContactListManager & operator=(const QtContactListManager & other) 
	{
		_tree = other._tree;
		_contactList = other._contactList;
		return *this;
	}

	bool contains(const QString & contactId) const;


	QTreeWidget* _tree;

	QHash <QString, QtContact *> _contactList;

	QString _lastMouseOn;

	mutable QMutex* _mutex;

	static QtContactListManager* _instance;
};

#endif	//OWCONTACTLISTMANAGER_H
