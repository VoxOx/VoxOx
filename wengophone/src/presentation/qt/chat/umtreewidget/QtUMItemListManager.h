/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as publis
 hed by
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

#ifndef OWUMITEMLISTMANAGER_H
#define OWUMITEMLISTMANAGER_H

#include "QtUMItem.h"
#include <QtCore/QString>

class QMutex;
class QTreeWidget;

/**
 * QtUMItem storage class for fast lookup
 *
 * @author Mr K.
 */
class QtUMItemListManager 
{
public:
	static QtUMItemListManager * getInstance();

	virtual ~QtUMItemListManager();

	bool addItem   (QtUMItem * umItem);
	void removeItem(QtUMItem * umItem);

	QtUMItem * getItem(const QString & id) const;
	QTreeWidget* getTreeWidget() const						{ return _tree; }
	void		 setTreeWidget(QTreeWidget * tree)			{ _tree = tree; }

	void openProfile(const QString & id);
	void clear();

	bool existsItem(const QString & id);//VOXOX CHANGE by Rolando - 2009.09.08 

	QMap <QString, QtUMItem *> getItemListToDraw(){	return _itemListToDraw;}

private:
	QtUMItemListManager();

	QtUMItemListManager(const QtUMItemListManager & other)
		: _tree(other._tree),
		_itemList(other._itemList) { }

	QtUMItemListManager & operator=(const QtUMItemListManager & other) 
	{
		_tree = other._tree;
		_itemList = other._itemList;
		return *this;
	}

	bool contains(const QString & id) const;


	QTreeWidget* _tree;

	QMap <QString, QtUMItem *> _itemList;//VOXOX CHANGE by Rolando - 2009.08.19 changed to map instead of hash
	QMap <QString, QtUMItem *> _itemListToDraw;//VOXOX CHANGE by Rolando - 2009.08.19

	
	QString _lastMouseOn;

	mutable QMutex* _mutex;

	static QtUMItemListManager* _instance;
};

#endif	//OWUMITEMLISTMANAGER_H
