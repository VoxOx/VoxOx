/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* CLASS QtUMItemList 
* @author Rolando 
* @date 2009.09.01
*/

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtUMItemList.h"
#include "ui_UMItemList.h"

#include "QtUMItem.h"//VOXOX CHANGE by Rolando - 2009.08.31 
#include "QtUMItemManager.h"
#include "QtUMTreeViewDelegate.h"
#include <presentation/qt/chat/umtreewidget/QtUMItemListStyle.h>//VOXOX CHANGE by Rolando - 2009.08.30
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>

static inline QPixmap scalePixmap(const QString name) {
	return QPixmap(name).scaled(QSize(20, 20), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QtUMItemList::QtUMItemList(CWengoPhone & cWengoPhone)//VOXOX CHANGE by Rolando - 2009.10.23 
	: QWidget(NULL),
	_cWengoPhone(cWengoPhone)//VOXOX CHANGE by Rolando - 2009.10.23 
{
	_ui = new Ui::UMItemList();
	_ui->setupUi(this);

	_itemManager = new QtUMItemManager(_cWengoPhone, *this, _ui->treeWidget, _ui->treeWidget);

	SAFE_CONNECT(_itemManager, SIGNAL(closeCurrentItem(QString)), SLOT(closeCurrentItemSlot(QString)));//VOXOX CHANGE by Rolando - 2009.08.27 
	SAFE_CONNECT(_itemManager, SIGNAL(closeCurrentGroup(QString)), SLOT(closeCurrentGroupSlot(QString)));
	SAFE_CONNECT(_itemManager, SIGNAL(itemClicked(QString)), SLOT(itemClickedSlot(QString)));//VOXOX CHANGE by Rolando - 2009.08.27
	SAFE_CONNECT(_itemManager, SIGNAL(groupClicked(QString)), SLOT(groupClickedSlot(QString)));//VOXOX CHANGE by Rolando - 2009.08.27
	
	QtUMTreeViewDelegate * delegate = new QtUMTreeViewDelegate(_ui->treeWidget);
	delegate->setParent(_ui->treeWidget->viewport());
	_ui->treeWidget->setItemDelegate(delegate);		

	_ui->treeWidget->viewport()->setFocus();

	updatePresentation();

	_initialLoad = false;		//VOXOX - JRT - 2009.04.12 

	startTimer(500);//VOXOX CHANGE by Rolando - 2009.08.27

}

QtUMItemList::~QtUMItemList() {
	OWSAFE_DELETE(_ui);
	OWSAFE_DELETE(_itemManager);//VOXOX CHANGE by Rolando - 2009.08.30 	
}

void QtUMItemList::setInitialLoad( bool bSet )
{
	LOG_INFO( bSet ? "on" : "off"  );
	_initialLoad = bSet;
	_itemManager->setInitialLoad( bSet );
}
//End Voxox

void QtUMItemList::clear() {
	_itemManager->clear();
}

void QtUMItemList::updatePresentation() {
	QtUMItemListStyle * itemListStyle = QtUMItemListStyle::getInstance();
	_ui->treeWidget->setStyleSheet(QString("QTreeWidget{background-color: %1;}").arg(itemListStyle->getItemNonSelectedBackgroundColor().name()));
	_itemManager->userStateChanged();
	_ui->treeWidget->viewport()->update();
}

//VOXOX CHANGE by Rolando - 2009.08.28 
void QtUMItemList::startBlinkingItem(const QString& id){
	_itemManager->startBlinkingItem(id);	
}

//VOXOX CHANGE by Rolando - 2009.08.28 
void QtUMItemList::stopBlinkingItem(const QString& id){
	_itemManager->stopBlinkingItem(id);	
}

//VOXOX CHANGE by Rolando - 2009.10.02 
void QtUMItemList::startBlinkingGroup(const QString& id){
	_itemManager->startBlinkingGroup(id);	
}

//VOXOX CHANGE by Rolando - 2009.10.02 
void QtUMItemList::stopBlinkingGroup(const QString& id){
	_itemManager->stopBlinkingGroup(id);	
}

//VOXOX CHANGE by Rolando - 2009.08.29 
void QtUMItemList::setCurrentItem(const QString & id){
	_itemManager->setCurrentItem(id);
}

//VOXOX CHANGE by Rolando - 2009.09.03 
QString QtUMItemList::getCurrentItemId()//VOXOX CHANGE by Rolando - 2009.09.03
{
	return _itemManager->getCurrentItemId();//VOXOX CHANGE by Rolando - 2009.09.03 
}


//VOXOX CHANGE by Rolando - 2009.08.28 
void QtUMItemList::itemClickedSlot(QString id){
	itemClicked(id);
}

void QtUMItemList::groupClickedSlot(QString id){
	groupClicked(id);
}

void QtUMItemList::removeGroup(const QString & groupId) {
	_itemManager->removeGroup(groupId);
	updatePresentation();	
}	

//VOXOX CHANGE by Rolando - 2009.08.28 
void QtUMItemList::timerEvent(QTimerEvent * event) {
	_ui->treeWidget->viewport()->update();	
}

//VOXOX CHANGE by Rolando - 2009.09.08 
QStringList QtUMItemList::getItemListKeys(){
	return _itemManager->getItemListKeys();//VOXOX - CJC - 2009.09.02 	
}

//VOXOX CHANGE by Rolando - 2009.09.08 
QStringList QtUMItemList::getItemListKeysOfGroup(const QString & groupId){
	return _itemManager->getItemListKeysOfGroup(groupId);//VOXOX CHANGE by Rolando - 2009.09.08 	
}



//VOXOX CHANGE by Rolando - 2009.09.08 - 
/* Returns empty string if was not found id, otherwise can return:
 * its own id if item not belongs to a group or groupId if this one was found
 */ 
QString QtUMItemList::getGroupIdOfItem(const QString & id){
	return _itemManager->getGroupIdOfItem(id);//VOXOX CHANGE by Rolando - 2009.09.08 
}

//VOXOX CHANGE by Rolando - 2009.10.23 - added parameter contactId
//VOXOX CHANGE by Rolando - 2009.08.31  
void QtUMItemList::addItem(const QString& contactId, const QString& id, const QString & groupId, const QString & displayName, const QString & avatarData, QPixmap normalStatus, QPixmap hoverStatus, bool shouldSelectItem){
	if(existsGroup(groupId) || groupId == ""){
		if(!_itemManager->existsItem(id)){
			_itemManager->handleItemChanged(contactId,id, groupId, displayName, avatarData, normalStatus, hoverStatus, shouldSelectItem);//VOXOX CHANGE by Rolando - 2009.10.26 
		}
		else{
			LOG_DEBUG("Item cannot be added because id: %1 already was added", groupId.toStdString());
		}
	}
	else{
		LOG_DEBUG("Item cannot be added because groupId: %1 not exists", groupId.toStdString());
	}
}

//VOXOX CHANGE by Rolando - 2009.10.26 - added parametr hoverStatus
//VOXOX CHANGE by Rolando - 2009.10.23 - added parameter contactId
//VOXOX CHANGE by Rolando - 2009.09.08 
void QtUMItemList::addItemToGroup(const QString& contactId, const QString& id, const QString & groupId, const QString & displayName, const QString & avatarData, QPixmap normalStatus, QPixmap hoverStatus, bool shouldSelectItem){
	if(existsGroup(groupId)){
		addItem(contactId, id, groupId, displayName, avatarData, normalStatus, hoverStatus,shouldSelectItem);//VOXOX CHANGE by Rolando - 2009.10.26 
	}
	else{
		LOG_DEBUG("Item cannot be added to group: %1, because group not exists", groupId.toStdString());
	}
}

void QtUMItemList::removeItem(const QString & id) {
	_itemManager->removeItem(id);
		
}

//VOXOX CHANGE by Rolando - 2009.10.26 - added parameter normalStatus, hoverStatus
//VOXOX CHANGE by Rolando - 2009.10.23 - added parameter contactId
//VOXOX CHANGE by Rolando - 2009.09.08 
void QtUMItemList::updateItem(const QString& contactId, const QString& id, const QString & groupId, const QString & displayName, const QString & avatarData, QPixmap normalStatus, QPixmap hoverStatus, bool shouldSelectItem) {
	_itemManager->handleItemChanged(contactId, id, groupId, displayName, avatarData, normalStatus, hoverStatus, shouldSelectItem);//VOXOX CHANGE by Rolando - 2009.10.26 
}

//VOXOX CHANGE by Rolando - 2009.10.26 
void QtUMItemList::updateItemStatusPixmap(const QString & id, QPixmap normalStatus, QPixmap hoverStatus){
	_itemManager->updateItemStatusPixmap(id, normalStatus, hoverStatus);//VOXOX CHANGE by Rolando - 2009.10.26 
}

//VOXOX CHANGE by Rolando - 2009.09.24 
void QtUMItemList::addGroup(QString itemGroupId, QString newGroupName, bool shouldSelectGroup) {
	if(!existsGroup(itemGroupId)){
		_itemManager->addGroup(itemGroupId, newGroupName, shouldSelectGroup);//VOXOX CHANGE by Rolando - 2009.09.24 
	}
	else{
		LOG_DEBUG("groupId: %1 already exists, group not added", itemGroupId.toStdString());
	}
}

void QtUMItemList::renameGroup(QString itemGroupId, QString newGroupName) {
	if(existsGroup(itemGroupId)){
		_itemManager->renameGroup(itemGroupId, newGroupName);//VOXOX CHANGE by Rolando - 2009.08.26 
	}
	else{
		LOG_DEBUG("groupId: %1 not exists, group name not modified", itemGroupId.toStdString());
	}
	
}

bool QtUMItemList::existsGroup(QString groupId){
	return _itemManager->existsGroup(groupId);//VOXOX CHANGE by Rolando - 2009.09.03 
}

void QtUMItemList::closeCurrentItemSlot(QString id){
	itemClosed(id);//VOXOX CHANGE by Rolando - 2009.08.27 
}

//VOXOX CHANGE by Rolando - 2009.09.28 
void QtUMItemList::closeCurrentGroupSlot(QString id){
	groupClosed(id);
}
//-----------------------------------------------------------------------------
