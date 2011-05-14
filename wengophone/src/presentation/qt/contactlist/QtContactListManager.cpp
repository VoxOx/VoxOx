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
#include "QtContactListManager.h"

#include <util/Logger.h>

#include <QtGui/QtGui>

QtContactListManager * QtContactListManager::_instance = NULL;

QtContactListManager * QtContactListManager::getInstance() {
	if (!_instance) {
		_instance = new QtContactListManager;
	}
	return _instance;
}

QtContactListManager::QtContactListManager() {
	_mutex = new QMutex(QMutex::Recursive);
}

QtContactListManager::~QtContactListManager() {
	delete _mutex;
}

//Called only from QtTreeViewDelegate
//VOXOX - JRT - 2009.08.10 
//void QtContactListManager::paintContact(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) 
//{
//	QMutexLocker locker(_mutex);
//	//VOXOX - JRT - 2009.04.11 - This code requires TWO searches of the list.
//	//		First to determine if contact exists, then to actually retrieve the ptr.
//	//		Let's just retreive the ptr and then evaluate that.
//	//QtContact * qtContact;
//	//if (contains(index.data().toString())) {
//	//	qtContact = _contactList[index.data().toString()];
//	//	qtContact->paint(painter, option);
//	//}
//	QtContact * qtContact = _contactList[index.data().toString()];
//	if ( qtContact )
//	{
//		qtContact->paint(painter, option);
//	}
//}

//VOXOX - JRT - 2009.04.11 - NOTE: _contactList actually stores the ptrs!  So don't delete them. ODD.
//JRT - TODO: So, what happens on an update?  Does QHash automatically delete the old ptr?  I doubt it.
bool QtContactListManager::addContact(QtContact * qtContact) {
	QMutexLocker locker(_mutex);
	bool bAdded = false;

	if (qtContact) 
	{
		int nOrigSize = _contactList.size();
		int nNewSize  = 0;

		QString key = qtContact->getKey();		//VOXOX - JRT - 2009.08.10 

		QtContact* pOld = _contactList[key];

		//Remove old entry, but preserve some important values.
		if ( pOld )
		{
			qtContact->setIsOpen( pOld->isOpen() );	

			qtContact->setTreeWidgetItem( pOld->getTreeWidgetItem() );
			delete pOld;
		}

		_contactList[key] = qtContact;

		nNewSize = _contactList.size();
		bAdded   = ( nOrigSize != nNewSize);
	}

	return bAdded;
}
//End VoxOx

void QtContactListManager::clear() {
	QMutexLocker locker(_mutex);
	QHash <QString, QtContact *>::iterator it;
	for (it = _contactList.begin(); it != _contactList.end(); it++) {
		delete((*it));
	}
	_contactList.clear();
}

void QtContactListManager::removeContact(QtContact  * qtContact) {
	QMutexLocker locker(_mutex);
	if (qtContact) 
	{
		_contactList.remove(qtContact->getKey());	//VOXOX - JRT - 2009.08.10 
		delete qtContact;
	}
}

bool QtContactListManager::contains(const QString & contactId) const {
	QMutexLocker locker(_mutex);
	return _contactList.contains(contactId);
}

QtContact * QtContactListManager::getContact(const QString & contactId) const {
	QMutexLocker locker(_mutex);

	QtContact* qtContact = NULL;

	if (contains(contactId)) 
	{
		qtContact = _contactList[contactId];
	}

	return qtContact;
}

bool QtContactListManager::isBlocked(const QString & contactId) const{
	QMutexLocker locker(_mutex);
	if (contains(contactId)) {
		return _contactList[contactId]->isBlocked();
	}
	return false;
}
