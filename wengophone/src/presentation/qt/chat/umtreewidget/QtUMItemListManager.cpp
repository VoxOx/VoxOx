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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtUMItemListManager.h"

#include <util/Logger.h>

#include <QtGui/QtGui>
#include <QtCore/QTimerEvent>	//VOXOX - 2009.09.09

QtUMItemListManager * QtUMItemListManager::_instance = NULL;

QtUMItemListManager * QtUMItemListManager::getInstance() {
	if (!_instance) {
		_instance = new QtUMItemListManager;
	}
	return _instance;
}

QtUMItemListManager::QtUMItemListManager() {
	_mutex = new QMutex(QMutex::Recursive);
}

QtUMItemListManager::~QtUMItemListManager() {
	delete _mutex;
}

bool QtUMItemListManager::addItem(QtUMItem * umItem) {


	QMutexLocker locker(_mutex);
	bool bAdded = false;

	if (umItem) 
	{
		QString id = umItem->getId();//VOXOX CHANGE by Rolando - 2009.09.01

		if(_itemListToDraw.contains(id)){			
			bAdded = false;
		}
		else{
			bAdded = true;
		}

		_itemListToDraw[id] = umItem;//VOXOX CHANGE by Rolando - 2009.08.24	
		
	}	

	return bAdded;	
}
//End VoxOx

void QtUMItemListManager::clear() {
	QMutexLocker locker(_mutex);
	QMap <QString, QtUMItem *>::iterator it;
	for (it = _itemList.begin(); it != _itemList.end(); it++) {
		delete((*it));
	}
	_itemList.clear();
}

//VOXOX CHANGE by Rolando - 2009.09.08 
bool QtUMItemListManager::existsItem(const QString & id) {
	QMap <QString, QtUMItem *>::iterator it = _itemListToDraw.find(id);
	return it != _itemListToDraw.end();//VOXOX CHANGE by Rolando - 2009.09.08 - if already exists item
} 

void QtUMItemListManager::removeItem(QtUMItem  * umItem) {
	QMutexLocker locker(_mutex);
	if (umItem) 
	{
		_itemList.remove(umItem->getId());
		_itemListToDraw.remove(umItem->getId()); //VOXOX CHANGE by Rolando - 2009.08.25 
		delete umItem;
	}
}

bool QtUMItemListManager::contains(const QString & id) const {
	QMutexLocker locker(_mutex);
	return _itemList.contains(id);
}

QtUMItem * QtUMItemListManager::getItem(const QString & id) const {
	QMutexLocker locker(_mutex);

	QtUMItem* umItem = NULL;

	if (contains(id)) 
	{
		umItem = _itemList[id];
	}

	return umItem;
}

