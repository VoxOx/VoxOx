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
#include "QtUMItemManager.h"
#include "QtUMItemList.h"
#include "QtUMItemInfo.h"
#include "QtUMItemListManager.h"

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/LanguageChangeEventFilter.h>

#include <QtCore/QTimerEvent>	//VOXOX - 2009.09.09

#ifdef _WINDOWS		//VOXOX - JRT - 2009.04.08 
#include "windows.h"
#endif

//VOXOX CHANGE by Rolando - 2009.10.23 
QtUMItemManager::QtUMItemManager(CWengoPhone & cWengoPhone, QtUMItemList & qtUMItemList, QObject * parent, QTreeWidget * target)
: QObject(parent),
	_qtUMItemList(qtUMItemList),
	_cWengoPhone(cWengoPhone)
{

	LANGUAGE_CHANGE(this);
	languageChanged();

	_tree		= dynamic_cast<QtUMTreeWidget*>(target);
	_sortItems = true;

	_sortTimerId = -1;
	_showTimerId = -1;
	_canSort	= true;
	_wantSort	= false;
	_canShow	= true;
	_wantShow	= false;
	_initialLoad = false;	//VOXOX - JRT - 2009.04.07 

	SAFE_CONNECT(_tree, SIGNAL(closeCurrentItem(QtUMItem *)), SLOT(closeCurrentItemSlot(QtUMItem *)));//VOXOX CHANGE by Rolando - 2009.08.28 
	SAFE_CONNECT(_tree, SIGNAL(closeCurrentGroup(QtUMGroup *)), SLOT(closeCurrentGroupSlot(QtUMGroup *)));
	SAFE_CONNECT(_tree, SIGNAL(itemLeftClicked( QTreeWidgetItem*, int)), SLOT(itemClickedSlot( QTreeWidgetItem*, int)));//VOXOX CHANGE by Rolando - 2009.08.28 
	SAFE_CONNECT(_tree, SIGNAL(groupLeftClicked( QTreeWidgetItem*, int)), SLOT(groupClickedSlot( QTreeWidgetItem*, int)));//VOXOX CHANGE by Rolando - 2009.08.28 

}


void QtUMItemManager::closeCurrentItemSlot(QtUMItem * umItem){
	closeCurrentItem(umItem->getId());//VOXOX CHANGE by Rolando - 2009.09.28 
}

//VOXOX CHANGE by Rolando - 2009.09.28 
void QtUMItemManager::closeCurrentGroupSlot(QtUMGroup * umGroup){
	closeCurrentGroup(umGroup->getId());//VOXOX CHANGE by Rolando - 2009.09.28 
}

//VOXOX CHANGE by Rolando - 2009.09.08 
void QtUMItemManager::itemClickedSlot(QTreeWidgetItem* item, int column){

	QtUMItem * umItem = getQtUMItemFromItem(item);//VOXOX CHANGE by Rolando - 2009.09.08 
	if(umItem){
		itemClicked(umItem->getId());//VOXOX CHANGE by Rolando - 2009.09.08 		
		setCurrentItem(item);//VOXOX CHANGE by Rolando - 2009.09.04
		
		_tree->viewport()->update();//VOXOX CHANGE by Rolando - 2009.10.01 		
	}	

	
}

void QtUMItemManager::groupClickedSlot(QTreeWidgetItem* item, int column){

	QtUMGroup * umGroup = getQtUMGroupFromItem(item);//VOXOX CHANGE by Rolando - 2009.09.08 
	if(umGroup){//VOXOX CHANGE by Rolando - 2009.09.28 
		setGroupClicked( item );//VOXOX CHANGE by Rolando - 2009.09.04 
		_tree->viewport()->update();//VOXOX CHANGE by Rolando - 2009.09.04 
	}

	
}

void QtUMItemManager::setGroupClicked(QTreeWidgetItem * item) 
{
	groupClicked(item->text(0));//VOXOX CHANGE by Rolando - 2009.09.08 
	setGroupOpen( item->text(0), item->isExpanded() );	
	clearSelection();
	setGroupSelected(item->text(0), true);//VOXOX CHANGE by Rolando - 2009.09.04
	stopBlinkingGroup(item->text(0));//VOXOX CHANGE by Rolando - 2009.09.08 
	_tree->setCurrentItem(item);//VOXOX CHANGE by Rolando - 2009.10.02 
}

//VOXOX CHANGE by Rolando - 2009.09.04 
void QtUMItemManager::clearSelection() {	 
	QTreeWidgetItem* item = NULL;//VOXOX CHANGE by Rolando - 2009.09.04
	QtUMItem * umItem = NULL;//VOXOX CHANGE by Rolando - 2009.09.29 
	QtUMGroup * umGroup = NULL;//VOXOX CHANGE by Rolando - 2009.09.29
	int topCount = _tree->topLevelItemCount();

	for (int topIndex = 0; topIndex < topCount; ++topIndex) 
	{
		QTreeWidgetItem* item = _tree->topLevelItem(topIndex);//VOXOX CHANGE by Rolando - 2009.09.04 
		item->setSelected(false);
		umItem = getQtUMItemFromItem(item);//VOXOX CHANGE by Rolando - 2009.10.01 

		if(umItem){
			umItem->setIsSelected(false);
		}
		else{
			umGroup =getQtUMGroupFromItem(item);//VOXOX CHANGE by Rolando - 2009.10.01
			if(umGroup){
				umGroup->setIsSelected(false);//VOXOX CHANGE by Rolando - 2009.10.01 
				setGroupSelected(item->text(0), false);	
			}
		}			
	}
}


void QtUMItemManager::clearTreeSelection() {
	QList <QTreeWidgetItem * > selectedList = _tree->selectedItems();
	QList <QTreeWidgetItem * >::iterator it;
	for (it = selectedList.begin(); it != selectedList.end(); it ++) {
		_tree->setItemSelected((*it), false);
	}
}


void QtUMItemManager::safeUserStateChanged() {

	redrawItems();
}

void QtUMItemManager::userStateChanged() {
	safeUserStateChanged();
}

//-----------------------------------------------------------------------------
//Begin Sort related methods
//-----------------------------------------------------------------------------

void QtUMItemManager::sortItems(bool bypassTimer) {
	safeSortItems(bypassTimer);
}

bool QtUMItemManager::shouldSortItems(bool bypassTimer) 
{
	bool bSort = true;

	if (_sortItems) 
	{
		if (!bypassTimer) 
		{
			if (_canSort) 
			{
				_canSort = false;

				if (_sortTimerId != -1) 
				{
					killTimer(_sortTimerId);
				}

				_sortTimerId = startTimer(1000);
				_wantSort    = false;
			} 
			else 
			{
				_wantSort = true;
				bSort     = false;
			}
		}
	}
	else
	{
		bSort = false;
	}

	return bSort;
}

	
void QtUMItemManager::safeSortItems(bool bypassTimer)
{
	if ( shouldSortItems( bypassTimer ) )
	{
		RecursiveMutex::ScopedLock lock(_mutex);
		_tree->lock(); 

		setInitialLoad( true );

		int topCount = _tree->topLevelItemCount();

		if ( topCount > 1 ) 
		{
			QTreeWidgetItem* root = _tree->invisibleRootItem();

			//first sort groups title and items
			sortChildren( root );
			
		}

		for (int topIndex = 0; topIndex < topCount; ++topIndex) 
		{
			QTreeWidgetItem* groupItem = _tree->topLevelItem(topIndex);

			if(getQtUMGroupFromItem(groupItem)){//VOXOX CHANGE by Rolando - 2009.08.31 - if it is a group item

				sortChildren( groupItem);//VOXOX CHANGE by Rolando - 2009.08.31
				restoreGroupExpandedStatus();//VOXOX CHANGE by Rolando - 2009.08.31 
			}

		}

		_tree->unlock();

		setInitialLoad( false );
	}
}

void QtUMItemManager::sortChildren( QTreeWidgetItem* parentItem)
{
	int count = parentItem->childCount();

	QtUMItemInfoList itemInfoList;
		
	int				 sortOrder = 0;
	bool			 isGroup   = 0;
	QString			 userName  = "";
	QTreeWidgetItem* item	   = NULL;
	QtUMItem*		 pItem  = NULL;
	QtUMGroup*		 pGroup  = NULL;
	
	EnumPresenceState::PresenceState presence = EnumPresenceState::PresenceStateUnknown;

	for (int index = 0; index < count; ++index) 
	{
		item	 = parentItem->child(index);
		pItem = getQtUMItemFromItem( item );
		pGroup = getQtUMGroupFromItem( item );

		//TODO: VOXOX CHANGE by Rolando - 2009.08.31 - to check it because here there is an error
		if ( !pItem )//VOXOX CHANGE by Rolando - 2009.08.31 - if item is a group
		{
			if(pGroup){
				QString groupId = QString( item->text(0) );
				userName = getGroupName(groupId);//VOXOX CHANGE by Rolando - 2009.09.03 
				presence = EnumPresenceState::PresenceStateUnknown;
				isGroup   = true;
			}
		}
		else
		{			
			userName  = (pItem ? QString(pItem->getDisplayName() ) : "" );
			presence  = (pItem ? pItem->getPresenceState()         : EnumPresenceState::PresenceStateUnknown);
			isGroup   = false;			
		}


		QtUMItemInfo info = QtUMItemInfo(item, parentItem, item->text(0), userName, index, isGroup);
		itemInfoList.append(info);
		//End VoxOx
	}

	qSort(itemInfoList);

	Q_FOREACH(QtUMItemInfo info, itemInfoList) 
	{
		QTreeWidgetItem* item = info.getItem();
		parentItem->takeChild  ( parentItem->indexOfChild(item));
		parentItem->insertChild( parentItem->childCount(), item);
	}
}

void QtUMItemManager::restoreGroupExpandedStatus()
{
	int topCount = _tree->topLevelItemCount();
	QtUMItem * pItem = NULL;//VOXOX CHANGE by Rolando - 2009.10.01 
	QtUMGroup * pGroup = NULL;//VOXOX CHANGE by Rolando - 2009.10.01 
	QTreeWidgetItem* item = NULL;//VOXOX CHANGE by Rolando - 2009.10.01 

	for ( int x = 0; x < topCount; x++ )
	{
		item = _tree->topLevelItem(x);
		pItem = getQtUMItemFromItem( item );
		pGroup = getQtUMGroupFromItem( item );//VOXOX CHANGE by Rolando - 2009.10.01 

		if(!pItem){//VOXOX CHANGE by Rolando - 2009.08.31 - if item is a group
			if(pGroup){//VOXOX CHANGE by Rolando - 2009.10.01 
				QString itemGroupId = QString( item->text(0) );
				if(_groupList.contains(itemGroupId)){
					_tree->setItemExpanded( item, _groupList[itemGroupId]->isOpen() );
				}
			}
		}
		
	}
}

//-----------------------------------------------------------------------------
//End sort related methods
//-----------------------------------------------------------------------------

//VOXOX CHANGE by Rolando - 2009.08.28 
void QtUMItemManager::startBlinkingItem(const QString& id){
	QTreeWidgetItem * item	   = NULL;//VOXOX CHANGE by Rolando - 2009.09.08 
	QTreeWidgetItem * parentItem	   = NULL;	
	QtUMGroup * umGroup = NULL;
	QtUMItem * umItem = NULL;//VOXOX CHANGE by Rolando - 2009.10.01 
	item = findItem(id);

	if(item){
		parentItem = item->parent();//VOXOX CHANGE by Rolando - 2009.09.08 

		if(parentItem){
			umGroup = getQtUMGroupFromItem(parentItem);//VOXOX CHANGE by Rolando - 2009.10.01
			if(!umGroup->isSelected()){//VOXOX CHANGE by Rolando - 2009.10.01 
				startBlinkingGroup(parentItem->text(0));
			}
		}
		else{
			QtUMItem * umItem = getItemById(id);
			if(umItem){
				if(!umItem->isSelected()){//VOXOX CHANGE by Rolando - 2009.10.01 
					umItem->setBlinking(true);
				}
			}
			else{
				umGroup = getQtUMGroupFromItem(item);//VOXOX CHANGE by Rolando - 2009.10.02 
				if(umGroup){
					if(!umGroup->isSelected()){//VOXOX CHANGE by Rolando - 2009.10.01 
						startBlinkingGroup(item->text(0));
					}
				}
			}
		}
	}
}

//VOXOX CHANGE by Rolando - 2009.10.02 
void QtUMItemManager::stopBlinkingItem(const QString & id){
	QTreeWidgetItem * item	   = NULL;//VOXOX CHANGE by Rolando - 2009.09.08 
	QTreeWidgetItem * parentItem	   = NULL;	
	QtUMGroup * umGroup = NULL;
	QtUMItem * umItem = NULL;//VOXOX CHANGE by Rolando - 2009.10.01 
	item = findItem(id);

	if(item){
		parentItem = item->parent();//VOXOX CHANGE by Rolando - 2009.09.08 

		if(parentItem){
			umGroup = getQtUMGroupFromItem(parentItem);//VOXOX CHANGE by Rolando - 2009.10.01
			if(umGroup){//VOXOX CHANGE by Rolando - 2009.10.01 
				stopBlinkingGroup(parentItem->text(0));
			}
		}
		else{
			QtUMItem * umItem = getItemById(id);
			if(umItem){
				umItem->setBlinking(false);

			}
			else{
				umGroup = getQtUMGroupFromItem(item);//VOXOX CHANGE by Rolando - 2009.10.02 
				if(umGroup){
					stopBlinkingGroup(item->text(0));
				}
			}
		}
	}
}

//VOXOX CHANGE by Rolando - 2009.09.08 
void QtUMItemManager::startBlinkingGroup(const QString & groupId){
	int count = 0;
	QTreeWidgetItem * item	   = NULL;
	QtUMItem	    * pItem  = NULL;
	QTreeWidgetItem * groupItem = findGroupItem(groupId);

	if(groupItem){
		
		count = groupItem->childCount();		

		for (int index = 0; index < count; ++index) 
		{
			item = groupItem->child(index);
			if(item){
				pItem = getQtUMItemFromItem( item );
				if(pItem){
					if(!pItem->isSelected()){
						setGroupBlinking(groupId,true);//VOXOX CHANGE by Rolando - 2009.10.01 - needed here to synchronize blinking color in all group
						pItem->setBlinking(true);
					}
				}
			}
		}	
	}
}

//VOXOX CHANGE by Rolando - 2009.09.08 
void QtUMItemManager::stopBlinkingGroup(const QString & groupId){
	int count = 0;
	QTreeWidgetItem * item	   = NULL;
	QtUMItem	    * pItem  = NULL;
	QTreeWidgetItem * groupItem = findGroupItem(groupId);

	if(groupItem){
		setGroupBlinking(groupId,false);
		count = groupItem->childCount();		

		for (int index = 0; index < count; ++index) 
		{
			item = groupItem->child(index);
			if(item){
				pItem = getQtUMItemFromItem( item );
				if(pItem){
					pItem->setBlinking(false);
				}
			}
		}	
	}
}

//VOXOX CHANGE by Rolando - 2009.08.29 
void QtUMItemManager::setCurrentItem(const QString & id){

	QTreeWidgetItem * item = findItem(id);
	if(item){
		setCurrentItem(item);
	}	
}

//VOXOX CHANGE by Rolando - 2009.08.29 
void QtUMItemManager::setCurrentItem(QTreeWidgetItem * item){
	clearSelection();//VOXOX CHANGE by Rolando - 2009.09.29 

	QtUMItem * umItem = NULL;//VOXOX CHANGE by Rolando - 2009.09.29 
	QTreeWidgetItem * parent = item->parent();
	_tree->setCurrentItem(item);

	if(parent){//VOXOX CHANGE by Rolando - 2009.09.04 - if item belongs to a group
		setGroupSelected(parent->text(0), true);//VOXOX CHANGE by Rolando - 2009.09.04 
	}
	else{
		 
		item->setSelected(true);//VOXOX CHANGE by Rolando - 2009.09.08
		umItem = getQtUMItemFromItem(item);//VOXOX CHANGE by Rolando - 2009.09.29 
		if(umItem){//VOXOX CHANGE by Rolando - 2009.09.29 
			umItem->setIsSelected(true);//VOXOX CHANGE by Rolando - 2009.09.29
		}
	}

}

//VOXOX CHANGE by Rolando - 2009.09.03 
QString QtUMItemManager::getCurrentItemId()
{
	QTreeWidgetItem * item = _tree->currentItem();
	if(item){
		return item->text(0);//VOXOX CHANGE by Rolando - 2009.09.03 - check it when be implemented the group chat
	}
	else{
		return "";
	}
}

void QtUMItemManager::redrawItems() 
{
	RecursiveMutex::ScopedLock lock(_mutex);
	_tree->lock();

	if (_canShow) 
	{
		_canShow = false;
		if (_showTimerId != -1) 
		{
			killTimer(_showTimerId);
		}
		_showTimerId = startTimer(500);
		_wantShow = false;
	} 
	else 
	{
		_wantShow = true;
		return;
	}

	setInitialLoad( true );	

	//First, remove all item from the treeview and all users from userlist
	clear();

	//Second, add needed items
	QtUMItemListManager *	ul				= QtUMItemListManager::getInstance();
	QMap <QString, QtUMItem *> itemList = ul->getItemListToDraw();	
	QtUMItem * item = NULL;//VOXOX CHANGE by Rolando - 2009.09.03 

	redrawGroups();//VOXOX CHANGE by Rolando - 2009.09.02 

	for ( QMap <QString, QtUMItem *>::iterator itemsIter = itemList.begin(); itemsIter != itemList.end(); itemsIter++) 
	{
		item = itemsIter.value();//VOXOX CHANGE by Rolando - 2009.09.03 
		if(item){
			QString id = item->getId();//VOXOX CHANGE by Rolando - 2009.09.01 
			QString groupId   = item->getGroupKey();//VOXOX CHANGE by Rolando - 2009.08.18 
			QString contactId = item->getContactId();//VOXOX CHANGE by Rolando - 2009.10.23 
			QString displayName = item->getDisplayName();//VOXOX CHANGE by Rolando - 2009.08.31 
			
			if(id != ""){//VOXOX CHANGE by Rolando - 2009.08.21 
				//VOXOX CHANGE by Rolando - 2009.10.23 
				handleItemChanged(contactId, id, groupId, displayName, item->getAvatarData(), item->getNormalStatusPixmap(), item->getHoverStatusPixmap(), item->isSelected() );//VOXOX CHANGE by Rolando - 2009.10.26 
			}
		}
	}

	//Third, sort
	safeSortItems(true);
	setInitialLoad( false ); 

	_tree->unlock();
}

//VOXOX CHANGE by Rolando - 2009.08.30 
void QtUMItemManager::redrawGroups(){

	QTreeWidgetItem * item = NULL;
	
	for ( QMap <QString, QtUMGroup * >::iterator groupListIterator = _groupList.begin(); groupListIterator != _groupList.end(); groupListIterator++) 
	{
		item = new QTreeWidgetItem(_tree);	

		item->setText(0, groupListIterator.key());
		item->setData( 0, Qt::UserRole, qVariantFromValue(groupListIterator.value()));//VOXOX CHANGE by Rolando - 2009.08.26 

		_tree->setItemExpanded( item, groupListIterator.value()->isOpen() );
	}	

}

void QtUMItemManager::clear()
{
	QtUMItemListManager * ul = QtUMItemListManager::getInstance();
	_tree->clear();
	ul->clear();
}

//VOXOX CHANGE by Rolando - 2009.08.18 
QtUMItem* QtUMItemManager::getItemById( const QString & id )
{
	QtUMItemListManager *	ul				= QtUMItemListManager::getInstance();
	QMap <QString, QtUMItem *> itemList = ul->getItemListToDraw();
	QMap <QString, QtUMItem *>::iterator iter= itemList.find(id);//VOXOX CHANGE by Rolando - 2009.08.18 

	if(iter ==  itemList.end()){
		return NULL;
	}
	else{
		return iter.value();
	}
}

QtUMItem* QtUMItemManager::getCurrentItem()
{
	RecursiveMutex::ScopedLock lock(_mutex);

	QtUMItem* pItem = NULL;//VOXOX CHANGE by Rolando - 2009.08.18 

	QTreeWidgetItem * item = _tree->currentItem();
	if ( item ) 
	{
		QString id = item->text(0);
		pItem = getItemById( id );
	}

	return pItem;
}

QString QtUMItemManager::getCurrentItemItemKey()
{
	RecursiveMutex::ScopedLock lock(_mutex);

	QString result = "";
	QtUMItem* pItem = getCurrentItem();//VOXOX CHANGE by Rolando - 2009.08.18 

	if ( pItem )
	{
		result = pItem->getKey();
	}

	return result;
}

QString QtUMItemManager::getCurrentItemItemId()
{
	RecursiveMutex::ScopedLock lock(_mutex);

	QString result = "";
	QtUMItem* pItem = getCurrentItem();//VOXOX CHANGE by Rolando - 2009.08.18

	if ( pItem )
	{
		result = pItem->getId();//VOXOX CHANGE by Rolando - 2009.08.18 
	}

	return result;
}

//VOXOX CHANGE by Rolando - 2009.08.26 
void QtUMItemManager::renameGroup(const QString & itemGroupId, const QString & newGroupName){
	RecursiveMutex::ScopedLock lock(_mutex);

	if(_groupList.contains(itemGroupId)){
		if(newGroupName != ""){
			_groupList[itemGroupId]->setGroupName(newGroupName);//VOXOX CHANGE by Rolando - 2009.09.25 
			redrawItems();//VOXOX CHANGE by Rolando - 2009.08.30 
		}
	}
}

//VOXOX CHANGE by Rolando - 2009.08.31 
void QtUMItemManager::removeGroup(const QString & groupId){
	RecursiveMutex::ScopedLock lock(_mutex);//VOXOX CHANGE by Rolando - 2009.08.31
	QTreeWidgetItem* groupItem = NULL;//VOXOX CHANGE by Rolando - 2009.09.24
	QTreeWidgetItem * childItem = NULL;//VOXOX CHANGE by Rolando - 2009.09.24
	QTreeWidgetItem * newCurrentItem = NULL;//VOXOX CHANGE by Rolando - 2009.09.28
	QTreeWidgetItem * itemAbove = NULL;//VOXOX CHANGE by Rolando - 2009.09.28
	QTreeWidgetItem * itemBelow = NULL;//VOXOX CHANGE by Rolando - 2009.09.28
	QtUMGroup * umGroup = NULL;//VOXOX CHANGE by Rolando - 2009.09.28
	QtUMItem * umItem = NULL;//VOXOX CHANGE by Rolando - 2009.09.28 
	int itemCount = 0;//VOXOX CHANGE by Rolando - 2009.09.24
	int itemIndex = 0;//VOXOX CHANGE by Rolando - 2009.09.24 

	if(_groupList.contains(groupId)){
		groupItem = findGroupItem(groupId);//VOXOX CHANGE by Rolando - 2009.09.04		
		itemAbove = _tree->itemAbove(groupItem);//VOXOX CHANGE by Rolando - 2009.09.28
		if(itemAbove){//VOXOX CHANGE by Rolando - 2009.09.28 
			if(itemAbove->parent()){//VOXOX CHANGE by Rolando - 2009.09.28 
				itemAbove = itemAbove->parent();//VOXOX CHANGE by Rolando - 2009.09.28 
				if(itemAbove == groupItem){
					itemAbove = _tree->topLevelItem(_tree->indexOfTopLevelItem(itemAbove) - 1);//VOXOX CHANGE by Rolando - 2009.09.28 
				}
			}
		}
		itemBelow = _tree->itemBelow(groupItem);//VOXOX CHANGE by Rolando - 2009.09.28
		if(itemBelow){//VOXOX CHANGE by Rolando - 2009.09.28 
			if(itemBelow->parent()){//VOXOX CHANGE by Rolando - 2009.09.28 
				itemBelow = itemBelow->parent();//VOXOX CHANGE by Rolando - 2009.09.28
				if(itemBelow == groupItem){
					itemBelow = _tree->topLevelItem(_tree->indexOfTopLevelItem(itemBelow) + 1);//VOXOX CHANGE by Rolando - 2009.09.28 
				}
			}
		}

		itemCount = groupItem->childCount();
		
		for (itemIndex = 0; itemIndex < itemCount; ++itemIndex) 
		{
			childItem      = groupItem->child(itemIndex);
			if(childItem){
				umItem = getQtUMItemFromItem(childItem);
				if(umItem){
					removeItem(umItem->getId(), false);
				}
			}
		}

		newCurrentItem = (itemBelow)? itemBelow: itemAbove;//VOXOX CHANGE by Rolando - 2009.09.28

		umGroup = getQtUMGroupFromItem(groupItem);//VOXOX CHANGE by Rolando - 2009.10.01 - get QtUMGroup of group to remove

		bool isGroupSelected = umGroup->isSelected();//VOXOX CHANGE by Rolando - 2009.09.28 - if group to remove is selected

		_tree->removeItemWidget(groupItem,0);//VOXOX CHANGE by Rolando - 2009.09.28
		delete groupItem;//VOXOX CHANGE by Rolando - 2009.09.28 

		 

		if(isGroupSelected){//VOXOX CHANGE by Rolando - 2009.09.28 			

			if(newCurrentItem){//VOXOX CHANGE by Rolando - 2009.09.28 
				_tree->setCurrentItem(newCurrentItem);//VOXOX CHANGE by Rolando - 2009.10.02 
				umGroup = getQtUMGroupFromItem(newCurrentItem);//VOXOX CHANGE by Rolando - 2009.09.28 
				if(umGroup){
					groupClickedSlot(newCurrentItem,0);//VOXOX CHANGE by Rolando - 2009.09.28 
				}
				else{
					itemClickedSlot(newCurrentItem,0);//VOXOX CHANGE by Rolando - 2009.09.28  
				}

			}
		}
		else{			

			if(_tree->topLevelItemCount() == 1){//VOXOX CHANGE by Rolando - 2009.09.28 - before to delete the item we need to check if will be more than 1 item
				if(newCurrentItem){
					umGroup = getQtUMGroupFromItem(newCurrentItem);//VOXOX CHANGE by Rolando - 2009.09.28 
					if(umGroup){
						groupClickedSlot(newCurrentItem,0);//VOXOX CHANGE by Rolando - 2009.09.28 
					}
					else{
						itemClickedSlot(newCurrentItem,0);//VOXOX CHANGE by Rolando - 2009.09.28  
					}
				}
			}


		}


		_groupList.remove(groupId);//VOXOX CHANGE by Rolando - 2009.08.31
		
		redrawItems();//VOXOX CHANGE by Rolando - 2009.08.30 		
	}
}

//VOXOX CHANGE by Rolando - 2009.09.24 
void QtUMItemManager::addGroup(const QString & itemGroupId, const QString & newGroupName, bool shouldSelectGroup) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	if(!_groupList.contains(itemGroupId)){//VOXOX CHANGE by Rolando - 2009.08.30 
		if(newGroupName != ""){
			QtUMGroup* umGroup = new QtUMGroup(itemGroupId, newGroupName, this);//VOXOX CHANGE by Rolando - 2009.09.25 
			umGroup->setGroupName(newGroupName);//VOXOX CHANGE by Rolando - 2009.09.25 
			umGroup->setOpenStatus(false);//VOXOX CHANGE by Rolando - 2009.09.25 
			umGroup->setIsSelected(shouldSelectGroup);//VOXOX CHANGE by Rolando - 2009.09.25 
			umGroup->setIsBlinking(false);//VOXOX CHANGE by Rolando - 2009.09.25 
			_groupList[itemGroupId] = umGroup;//VOXOX CHANGE by Rolando - 2009.09.25 
			QTreeWidgetItem * item = new QTreeWidgetItem(_tree);
			item->setText(0, itemGroupId);
			item->setFlags(item->flags() | Qt::ItemIsEditable);

			setGroupData(item, umGroup);//VOXOX CHANGE by Rolando - 2009.09.28 
			
			_tree->insertTopLevelItem(_tree->topLevelItemCount(),item);//VOXOX CHANGE by Rolando - 2009.08.24 
			
			if(shouldSelectGroup){//VOXOX CHANGE by Rolando - 2009.09.29 
				clearSelection();//VOXOX CHANGE by Rolando - 2009.09.29 
			}

			setGroupSelected(itemGroupId, shouldSelectGroup);//VOXOX CHANGE by Rolando - 2009.09.24			
		}
	}
}

//VOXOX CHANGE by Rolando - 2009.09.03 
bool QtUMItemManager::existsGroup(QString groupId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	return _groupList.contains(groupId);//VOXOX CHANGE by Rolando - 2009.09.03 
}


//VOXOX - JRT - 2009.04.07 - To improve initial data load.
void QtUMItemManager::setInitialLoad( bool bSet )	
{ 
	if ( _initialLoad != bSet )
	{
		_initialLoad = bSet;

		if ( _tree )
		{
			_tree->setUpdatesEnabled(!bSet);

			if ( !bSet )
			{
				redrawItems();
			}
		}
	}
}

QString QtUMItemManager::getGroupName( const QString& itemGroupId )
{
	if(_groupList.contains(itemGroupId)){
		return _groupList[itemGroupId]->getGroupName();//VOXOX CHANGE by Rolando - 2009.08.20
	}
	else
	{
		return "";//VOXOX CHANGE by Rolando - 2009.09.03 
	}
}

//VOXOX CHANGE by Rolando - 2009.09.03
bool QtUMItemManager::isGroupSelected(const QString & groupId) 
{
	if(_groupList.contains(groupId)){
		return _groupList[groupId]->isSelected();//VOXOX CHANGE by Rolando - 2009.09.04 
	}
	else
	{
		return false;//VOXOX CHANGE by Rolando - 2009.09.03 
	} 
}

//VOXOX CHANGE by Rolando - 2009.09.04 
void QtUMItemManager::setGroupSelected( const QString& groupId, bool bSelected )
{
	if(_groupList.contains(groupId)){

		QTreeWidgetItem* groupItem  = findGroupItem(groupId);//VOXOX CHANGE by Rolando - 2009.09.04 
		QTreeWidgetItem * childItem = NULL;
		QtUMItem * umItem = NULL;
		int itemCount = groupItem->childCount();
		int itemIndex = 0;

		_groupList[groupId]->setIsSelected(bSelected);//VOXOX CHANGE by Rolando - 2009.09.25 
		
		groupItem->setSelected(bSelected);//VOXOX CHANGE by Rolando - 2009.09.28 
		 
		for (itemIndex = 0; itemIndex < itemCount; ++itemIndex) 
		{
			childItem      = groupItem->child(itemIndex);
			childItem->setSelected(bSelected);
			umItem = getQtUMItemFromItem(childItem);
			if(umItem){
				umItem->setIsSelected(bSelected);
				updateItem(umItem);
			}
		}

		
	}

}

//VOXOX CHANGE by Rolando - 2009.09.08 
bool QtUMItemManager::isGroupBlinking(const QString & itemGroupId)
{
	if(_groupList.contains(itemGroupId)){
		return _groupList[itemGroupId]->isBlinking();//VOXOX CHANGE by Rolando - 2009.09.25 
	}
	else
	{
		return false;//VOXOX CHANGE by Rolando - 2009.09.08  
	}
}

//VOXOX CHANGE by Rolando - 2009.09.08 
void QtUMItemManager::setGroupBlinking(const QString & groupId, bool bIsBlinking)
{
	
	if(_groupList.contains(groupId)){
		_groupList[groupId]->setIsBlinking(bIsBlinking);//VOXOX CHANGE by Rolando - 2009.09.25 
	}
}

void QtUMItemManager::setGroupOpen( const QString& groupId, bool bOpen )
{
	if(_groupList.contains(groupId)){
		_groupList[groupId]->setOpenStatus(bOpen);//VOXOX CHANGE by Rolando - 2009.09.25 
	}
}

//VOXOX CHANGE by Rolando - 2009.09.08 
bool QtUMItemManager::existsItem(const QString & id){
	QtUMItemListManager *	ul				= QtUMItemListManager::getInstance();
	return ul->existsItem(id);
}

//VOXOX CHANGE by Rolando - 2009.09.08 - 
/* Returns empty string if was not found id, otherwise can return:
 *its own id if item not belongs to a group or groupId if this one was found
 */
QString QtUMItemManager::getGroupIdOfItem(const QString & id){
	QString groupId = "";

	QTreeWidgetItem * item = findItem(id);
	if(item){
		QtUMItem* pItem = getItemById( id );
		if(pItem){
			return pItem->getGroupKey();
		}
		else{
			return id;
		}
	}

	return groupId;
}

void QtUMItemManager::addItem( QtUMItem* umItem, bool shouldSelectItem ) //VOXOX CHANGE by Rolando - 2009.09.08 
{
	QString groupId = umItem->getGroupKey();
	addItem(umItem, groupId, umItem->getGroupName(),shouldSelectItem);//VOXOX CHANGE by Rolando - 2009.08.26 
}

void QtUMItemManager::addItem( QtUMItem* umItem, const QString & groupId,const QString & newGroupName, bool shouldSelectItem) 
{
	
	RecursiveMutex::ScopedLock lock(_mutex);
	int insertIndex = -1;//VOXOX CHANGE by Rolando - 2009.08.26 
	QTreeWidgetItem * item = NULL;//VOXOX CHANGE by Rolando - 2009.09.03 
	QTreeWidgetItem* groupItem  = findGroupItem(groupId);//VOXOX CHANGE by Rolando - 2009.08.26 

	if(!groupItem && newGroupName == "" && groupId != ""){
		LOG_FATAL("Item not added because no exists group and groupName is empty");//VOXOX CHANGE by Rolando - 2009.09.02 
	}
	else{

		item = new QTreeWidgetItem();
		item->setText(0, umItem->getId()  );
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setSelected(shouldSelectItem);//VOXOX CHANGE by Rolando - 2009.10.01
		umItem->setIsSelected(shouldSelectItem);//VOXOX CHANGE by Rolando - 2009.10.01

		setItemData( item, umItem );

		if(groupItem){		
			bool isSelected = isGroupSelected(groupId);
			groupItem->setSelected(isSelected);//VOXOX CHANGE by Rolando - 2009.09.04 
			item->setSelected(isSelected);//VOXOX CHANGE by Rolando - 2009.10.01 
			umItem->setIsSelected(isSelected);//VOXOX CHANGE by Rolando - 2009.10.01 

			if ( _qtUMItemList.getInitialLoad() )
			{
				groupItem->addChild( item );
			}
			else
			{
				insertIndex = findInsertPoint( groupItem, umItem );
				groupItem->insertChild(insertIndex, item);
			}

			_groupList[groupId]->addChildId(umItem->getId());//VOXOX CHANGE by Rolando - 2009.09.29
			_groupList[groupId]->setOpenStatus(true);//VOXOX CHANGE by Rolando - 2009.09.29 
		}
		else{
			if(groupId == ""){//VOXOX CHANGE by Rolando - 2009.08.24 
				_tree->insertTopLevelItem(_tree->topLevelItemCount(),item);//VOXOX CHANGE by Rolando - 2009.08.24 
			}
			else{
				if(!findGroupItem(groupId)){
					if(newGroupName != ""){//VOXOX CHANGE by Rolando - 2009.09.02 
						addGroup(groupId, newGroupName,shouldSelectItem);
						groupItem  = findGroupItem(groupId);
						insertIndex = findInsertPoint( groupItem, umItem );					
						groupItem->insertChild(insertIndex, item);
						_groupList[groupId]->addChildId(umItem->getId());//VOXOX CHANGE by Rolando - 2009.09.29
						_groupList[groupId]->setOpenStatus(true);//VOXOX CHANGE by Rolando - 2009.09.29 
					}
					else{
						LOG_FATAL("Name of group cannot be empty");//VOXOX CHANGE by Rolando - 2009.09.02 
					}
				}
			}
			
			LOG_DEBUG("groupItem = NULL");//VOXOX CHANGE by Rolando - 2009.08.26 
		}
		if(shouldSelectItem){
			
			setCurrentItem(umItem->getId());//VOXOX CHANGE by Rolando - 2009.09.02 
		}
	}
	
}

int QtUMItemManager::findInsertPoint( QTreeWidgetItem* groupItem, QtUMItem* umItem )
{
	int insertIndex  = 0;

	RecursiveMutex::ScopedLock lock(_mutex);
	_tree->lock();

	if ( groupItem && umItem )
	{
		int itemCount = groupItem->childCount();
		int itemIndex = 0;
		
		insertIndex  = itemCount;

		QString					id		= umItem->getId().toUpper();//VOXOX CHANGE by Rolando - 2009.09.01 
		QtUMItemListManager *	ul				= QtUMItemListManager::getInstance();
		QTreeWidgetItem *		childItem		= NULL;
		QtUMItem*				childQtUMItem	= NULL;

		 for (itemIndex = 0; itemIndex < itemCount; ++itemIndex) 
		 {
			childItem      = groupItem->child(itemIndex);
			childQtUMItem = getQtUMItemFromItem( childItem );

			if ( childQtUMItem )
			{
				if (childQtUMItem->getId() > id )
				{
					insertIndex = itemCount;
					break;
				}
			}
		}
	}

	 _tree->unlock();

	 return insertIndex;
}

//VOXOX CHANGE by Rolando - 2009.09.28 
void QtUMItemManager::removeItem(const QString & id, bool sendSignal) 
{
	RecursiveMutex::ScopedLock lock(_mutex); 
	_tree->lock();

	QTreeWidgetItem* umItem = findItem(id);

	if (umItem) 
	{
		removeItem( umItem,sendSignal );//VOXOX CHANGE by Rolando - 2009.09.28 
	}

	_tree->unlock();
}


void QtUMItemManager::removeItem( QTreeWidgetItem* item, bool sendSignal )
{

	RecursiveMutex::ScopedLock lock(_mutex);
	_tree->lock();

	QTreeWidgetItem*		groupItem = item->parent();
	QTreeWidgetItem*		newCurrentItem = NULL;//VOXOX CHANGE by Rolando - 2009.09.08
	QTreeWidgetItem*		itemAbove = NULL;
	QTreeWidgetItem*		itemBelow = NULL;
	QtUMItemListManager*	ul		  = QtUMItemListManager::getInstance();
	QtUMItem*				umItem	  = getQtUMItemFromItem( item );
	QtUMGroup*				umGroup   = NULL;//VOXOX CHANGE by Rolando - 2009.09.29 
	bool					itemIsSelected = item->isSelected();//VOXOX CHANGE by Rolando - 2009.09.28 

	if(umItem){
		itemIsSelected = umItem->isSelected();
		if(itemIsSelected){//VOXOX CHANGE by Rolando - 2009.09.10 - fix to complete task "Vertical tabs - mod for closing tabs"
			itemAbove = _tree->itemAbove(item);//VOXOX CHANGE by Rolando - 2009.09.28
			if(itemAbove){//VOXOX CHANGE by Rolando - 2009.09.28 
				if(itemAbove->parent()){//VOXOX CHANGE by Rolando - 2009.09.28 
					itemAbove = itemAbove->parent();//VOXOX CHANGE by Rolando - 2009.09.28 
					if(itemAbove == groupItem){
						itemAbove = _tree->topLevelItem(_tree->indexOfTopLevelItem(itemAbove) - 1);//VOXOX CHANGE by Rolando - 2009.09.28 
					}
				}
			}
			itemBelow = _tree->itemBelow(item);//VOXOX CHANGE by Rolando - 2009.09.28
			if(itemBelow){//VOXOX CHANGE by Rolando - 2009.09.28 
				if(itemBelow->parent()){//VOXOX CHANGE by Rolando - 2009.09.28 
					itemBelow = itemBelow->parent();//VOXOX CHANGE by Rolando - 2009.09.28 
					if(itemBelow == groupItem){
						itemBelow = _tree->topLevelItem(_tree->indexOfTopLevelItem(itemBelow) - 1);//VOXOX CHANGE by Rolando - 2009.09.28 
					}
				}
			}

			newCurrentItem = (itemBelow)? itemBelow: itemAbove;//VOXOX CHANGE by Rolando - 2009.09.28

		}

		if(groupItem){//VOXOX CHANGE by Rolando - 2009.09.29 
			umGroup = getQtUMGroupFromItem(groupItem);//VOXOX CHANGE by Rolando - 2009.09.29 
			if(umGroup){//VOXOX CHANGE by Rolando - 2009.09.29 
				umGroup->removeChildId(umItem->getId());//VOXOX CHANGE by Rolando - 2009.09.29 
			}
		}

		_tree->removeItemWidget(item,0);//VOXOX CHANGE by Rolando - 2009.10.01 
		delete item;//VOXOX CHANGE by Rolando - 2009.10.01 

		ul->removeItem(umItem);


		if(!groupItem){//VOXOX CHANGE by Rolando - 2009.09.29 - we don't have to change current item selected if item to remove is inside a group chat
			if(newCurrentItem && sendSignal){//VOXOX CHANGE by Rolando - 2009.09.08
				

				umItem = getQtUMItemFromItem(newCurrentItem);//VOXOX CHANGE by Rolando - 2009.09.08 
				if(umItem){					
					itemClickedSlot(newCurrentItem,0);//VOXOX CHANGE by Rolando - 2009.09.08 
				}
				else{
					groupClickedSlot(newCurrentItem,0);//VOXOX CHANGE by Rolando - 2009.09.08 
				}
				 

			}
		}

		if(newCurrentItem){//VOXOX CHANGE by Rolando - 2009.10.05 - updates current item after removing the last current item.
			_tree->setCurrentItem(newCurrentItem);//VOXOX CHANGE by Rolando - 2009.10.05 
		}
	}
	else 
	{
		LOG_WARN("item to remove is not a QtUMItem variable");
	}

#if QT_VERSION < 0x40300
// This is needed on Qt 4.2 and lower to workaround this bug:
// Assuming the list looks like this:
// ( ) A
// ( ) B
//     +---+ ...
//     |   | ...
//     +---+
//
// if 'A' gets hidden, the 'B' line moves up, but its QtUMItemWidget stays
// were it was.
_tree->doItemsLayout();
#endif
	
_tree->unlock();

}

QTreeWidgetItem * QtUMItemManager::findGroupItem(const QString & groupId) const 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	if(groupId != "")//VOXOX CHANGE by Rolando - 2009.08.30 
	{

		QList < QTreeWidgetItem * > list = _tree->findItems(groupId, Qt::MatchExactly);
		if (list.isEmpty()) 
		{
			return NULL;
		}

		return list[0];
	}
	else{
		return NULL;
	}

}

QTreeWidgetItem * QtUMItemManager::findItem(const QString & id) const 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	QList < QTreeWidgetItem * > list = _tree->findItems(id, Qt::MatchExactly | Qt::MatchRecursive);
	if (list.isEmpty()) 
	{
		return 0;
	}

	return list[0];
}

void QtUMItemManager::timerEvent(QTimerEvent * event) 
{
	if (event->timerId() == _sortTimerId) 
	{
		killTimer(_sortTimerId);
		_sortTimerId = -1;
		_canSort     = true;

		if (_wantSort) 
		{
			safeSortItems(true);
			_wantSort = false;
		}

		return;
	}

	if (event->timerId() == _showTimerId) 
	{
		killTimer(_showTimerId);
		_showTimerId = -1;
		_canShow     = true;

		if (_wantShow) 
		{
			redrawItems();
			_wantShow = false;
		}
		return;
	}

	QObject::timerEvent(event);
}

void QtUMItemManager::languageChanged() 
{
	
}

//VOXOX CHANGE by Rolando - 2009.09.28 - Returns a list with every groupId and itemId if this one not belongs to a group chat
QStringList QtUMItemManager::getItemListKeys(){
	QStringList list;
	QtUMItemListManager *	ul				= QtUMItemListManager::getInstance();

	QMap <QString, QtUMItem *> itemList = ul->getItemListToDraw();
	for ( QMap <QString, QtUMItem *>::iterator itemsIter = itemList.begin(); itemsIter != itemList.end(); itemsIter++) 
	{
		if( !itemsIter.value()->isAGroupItem() ){
			list << itemsIter.value()->getId();//VOXOX - CJC - 2009.09.02
		}
	}

	QMap <QString, QtUMGroup*>::iterator groupIterator = _groupList.begin();
	for ( ; groupIterator != _groupList.end(); groupIterator++) 
	{
		if( groupIterator.value() ){
			list << (*groupIterator)->getId();//VOXOX CHANGE by Rolando - 2009.09.28 
		}
	}

	return list;
}

//VOXOX CHANGE by Rolando - 2009.09.08 
QStringList QtUMItemManager::getItemListKeysOfGroup(const QString & groupId)
{
	QStringList list;
	QtUMItemListManager *	ul = QtUMItemListManager::getInstance();

	QMap <QString, QtUMItem *> itemList = ul->getItemListToDraw();
	for ( QMap <QString, QtUMItem *>::iterator itemsIter = itemList.begin(); itemsIter != itemList.end(); itemsIter++) 
	{
		if(itemsIter.value()->getGroupKey() == groupId){
			list << itemsIter.value()->getId();//VOXOX CHANGE by Rolando - 2009.09.08 
		}
	}

	return list;
}

//VOXOX CHANGE by Rolando - 2009.10.23 
void QtUMItemManager::handleItemChanged(const QString & contactId, const QString & id, const QString & groupId, const QString & displayName,QString avatarData, QPixmap normalStatusPixmap, QPixmap hoverStatusPixmap, bool shouldSelectItem ) 
{

	QtUMItemListManager* lm = QtUMItemListManager::getInstance();

   //VOXOX CHANGE by Rolando - 2009.09.03 - bug here when groupId is not empty but its groupName it is.
	//	This will determine if we have an 'add' or an 'update'.  We only have 'remove' in case of canShowUser() == false.
	if(getGroupName(groupId) != "" || groupId == ""){ 
		//VOXOX CHANGE by Rolando - 2009.10.26 
		QtUMItem*			  umItem = new QtUMItem(_cWengoPhone, contactId, id, groupId, displayName,getGroupName(groupId), avatarData, normalStatusPixmap, hoverStatusPixmap, _tree, false, shouldSelectItem);	//Do NOT delete this ptr.  It is owned by lm.
		
		bool				  bAdded	= lm->addItem(umItem);
		
		//Setting initialLoad before addItem speeds up the Tree inserts immensely.
		//  It also allows us to skip updates which aren't being redrawn any.
		if ( bAdded )
		{
			addItem( umItem, shouldSelectItem );		

		}
		else
		{

			//This needs to be in lm->addItem(umItem).
			setItemData( umItem->getTreeWidgetItem(), umItem );
			
			//The data has been updated already.  This would just redraw the Tree item.
			updateItem( umItem );				
				
			
			
		}
	}
	else{
		LOG_FATAL("Item not added because no exists group and groupName is empty");//VOXOX CHANGE by Rolando - 2009.09.02 
	}
	
}

//VOXOX CHANGE by Rolando - 2009.10.26  
void QtUMItemManager::updateItemStatusPixmap(const QString & id, QPixmap normalStatus, QPixmap hoverStatus){
	 QTreeWidgetItemIterator it(_tree);
     while (*it) {
		 if ((*it)->text(0) == id){
             QtUMItem * umItem = getQtUMItemFromItem(*it);
			 if(umItem){
				umItem->setNormalStatusPixmap(normalStatus);//VOXOX CHANGE by Rolando - 2009.10.26 
				umItem->setHoverStatusPixmap(hoverStatus);//VOXOX CHANGE by Rolando - 2009.10.26 
			 }
		 }
		 
         ++it;
     }

	 _tree->viewport()->update();//VOXOX CHANGE by Rolando - 2009.10.26 
}

void QtUMItemManager::updateItem( QtUMItem* umItem ) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	//QtUMItem should contain the TreeWidget item.
	QTreeWidgetItem* item = umItem->getTreeWidgetItem();
	QTreeWidgetItem* groupItem  = findGroupItem(umItem->getGroupKey());//VOXOX CHANGE by Rolando - 2009.09.30
	int insertIndex = 0;//VOXOX CHANGE by Rolando - 2009.09.30
	QString newGroupName = "";//VOXOX CHANGE by Rolando - 2009.09.30 
	
	if ( item )
	{
		assert( item->text(0) == umItem->getId() );	//VOXOX - JRT - 2009.08.10 - OK use of getId().

		if(umItem){//VOXOX CHANGE by Rolando - 2009.09.29 
			item->setSelected(umItem->isSelected());//VOXOX CHANGE by Rolando - 2009.09.29 
		}

		// Repaint only the item. We adjust the rect to avoid repainting neighbour items. 
		// It does not matter because QtUMItem::paint repaint the whole item, regardless of the area to update.
		QRect rect = _tree->visualItemRect(item);
		rect.adjust(0, 1, 0, -1);
		_tree->viewport()->update(rect);				
	}
	else{
		item = new QTreeWidgetItem();
		item->setText(0, umItem->getId()  );
		item->setSelected(umItem->isSelected());//VOXOX CHANGE by Rolando - 2009.09.30 
		item->setFlags(item->flags() | Qt::ItemIsEditable);

		setItemData( item, umItem );

		if(groupItem){	
			QString groupId = groupItem->text(0);//VOXOX CHANGE by Rolando - 2009.09.30 
			bool isSelected = isGroupSelected(groupId);			 
			
			if ( _qtUMItemList.getInitialLoad() )
			{
				groupItem->addChild( item );
			}
			else
			{
				insertIndex = findInsertPoint( groupItem, umItem );
				groupItem->insertChild(insertIndex, item);
			}

			this->setGroupSelected(groupId, isSelected);//VOXOX CHANGE by Rolando - 2009.09.30

			_groupList[groupId]->addChildId(umItem->getId());//VOXOX CHANGE by Rolando - 2009.09.29
			_groupList[groupId]->setOpenStatus(true);//VOXOX CHANGE by Rolando - 2009.09.29 
		}
		else{
			if(umItem->getGroupKey() == ""){//VOXOX CHANGE by Rolando - 2009.08.24 
				_tree->insertTopLevelItem(_tree->topLevelItemCount(),item);//VOXOX CHANGE by Rolando - 2009.08.24 
			}			
			 
		}	
	}
}

void  QtUMItemManager::setItemData( QTreeWidgetItem* item, QtUMItem* umItem )
{
	RecursiveMutex::ScopedLock lock(_mutex);

	if ( item )
	{
		item->setData( 0, Qt::UserRole, qVariantFromValue(umItem) );
		umItem->setTreeWidgetItem( item );
	}
}

void  QtUMItemManager::setGroupData( QTreeWidgetItem* item, QtUMGroup* umGroup )
{
	RecursiveMutex::ScopedLock lock(_mutex);

	if ( item )
	{
		item->setData( 0, Qt::UserRole, qVariantFromValue(umGroup) );
		umGroup->setTreeWidgetItem( item );
	}
}


QtUMItem* QtUMItemManager::getQtUMItemFromItem( QTreeWidgetItem* item ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	QtUMItem* umItem = NULL;

	if ( item )
	{
		umItem = item->data( 0, Qt::UserRole ).value<QtUMItem*>();
	}

	return umItem;
}

//VOXOX CHANGE by Rolando - 2009.09.28 
QtUMGroup* QtUMItemManager::getQtUMGroupFromItem( QTreeWidgetItem* item ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	QtUMGroup* umGroup = NULL;

	if ( item )
	{
		umGroup = item->data( 0, Qt::UserRole ).value<QtUMGroup*>();
	}

	return umGroup;
}
//End VoxOx
