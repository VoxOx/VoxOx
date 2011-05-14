/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can
 redistribute it and/or modify
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

#ifndef OWQTUMITEMMANAGER_H
#define OWQTUMITEMMANAGER_H

#include <thread/RecursiveMutex.h>
#include <presentation/qt/contactlist/QtContactPixmap.h>//VOXOX CHANGE by Rolando - 2009.08.31 
#include "QtUMTreeWidget.h"
#include "QtUMGroup.h"//VOXOX CHANGE by Rolando - 2009.09.25
#include <control/CWengoPhone.h>//VOXOX CHANGE by Rolando - 2009.10.23 

class QtUMItemList;
class QtUMItemInfo;
class QtUMTreeWidget;
class CWengoPhone;//VOXOX CHANGE by Rolando - 2009.10.26 

Q_DECLARE_METATYPE(QtUMItem*)
Q_DECLARE_METATYPE(QtUMGroup*)//VOXOX CHANGE by Rolando - 2009.09.25  

class QtUMItemManager : public QObject 
{
Q_OBJECT
public:
	QtUMItemManager(CWengoPhone & cWengoPhone, QtUMItemList & qtUMItemList, QObject * parent, QTreeWidget * target);
	~QtUMItemManager(){};
	/**
	 * Removes a Item from the QtUMItemManager.
	 *
	 * @param id the Id of the Item to remove
	 */
	void removeItem(const QString & id, bool sendSignal = true);//VOXOX CHANGE by Rolando - 2009.09.28 
	bool existsItem(const QString & id);//VOXOX CHANGE by Rolando - 2009.09.08 

	QStringList getItemListKeys();//VOXOX - CJC - 2009.09.02

	QStringList getItemListKeysOfGroup(const QString & groupId);//VOXOX CHANGE by Rolando - 2009.09.08 
	
	//VOXOX CHANGE by Rolando - 2009.10.26 - added parameter hoverStatusPixmap
	void handleItemChanged(const QString & contactId, const QString & id, const QString & groupId, const QString & displayName, QString avatarData, QPixmap normalStatusPixmap, QPixmap hoverStatusPixmap, bool shouldSelectItem );//VOXOX CHANGE by Rolando - 2009.08.31 
	
	void setInitialLoad( bool bSet );	//To improve initial data load.

	QString getGroupName( const QString & itemGroupId );//VOXOX CHANGE by Rolando - 2009.08.24
	bool isGroupSelected(const QString & itemGroupId);//VOXOX CHANGE by Rolando - 2009.09.03 
	void setGroupSelected( const QString & groupId, bool bSelected );//VOXOX CHANGE by Rolando - 2009.09.04 
	bool isGroupBlinking(const QString & itemGroupId);//VOXOX CHANGE by Rolando - 2009.09.08 
	void setGroupBlinking(const QString & groupId, bool bSelected);//VOXOX CHANGE by Rolando - 2009.09.08 

	void addGroup(const QString & itemGroupId, const QString & newGroupName, bool shouldSelectGroup);//VOXOX CHANGE by Rolando - 2009.09.24 

	//VOXOX CHANGE by Rolando - 2009.09.08 - 
	/* Returns empty string if was not found id, otherwise can return:
	 *its own id if item not belongs to a group or groupId if this one was found
	 */
	QString getGroupIdOfItem(const QString & id);

	void removeGroup(const QString & groupId);//VOXOX CHANGE by Rolando - 2009.08.31 
	bool existsGroup(QString groupId);//VOXOX CHANGE by Rolando - 2009.09.03 
	void renameGroup(const QString & itemGroupId, const QString & newGroupName);//VOXOX CHANGE by Rolando - 2009.08.28 

	void startBlinkingItem(const QString & id);//VOXOX CHANGE by Rolando - 2009.08.28

	void stopBlinkingItem(const QString & id);//VOXOX CHANGE by Rolando - 2009.08.28

	//VOXOX CHANGE by Rolando - 2009.09.08 
	void startBlinkingGroup(const QString & groupId);

	//VOXOX CHANGE by Rolando - 2009.09.08 
	void stopBlinkingGroup(const QString & groupId);

	void setCurrentItem(const QString & id);//VOXOX CHANGE by Rolando - 2009.08.29

	QString getCurrentItemId();//VOXOX CHANGE by Rolando - 2009.09.03 

	void setCurrentItem(QTreeWidgetItem* item);//VOXOX CHANGE by Rolando - 2009.09.02 

	void clear();//VOXOX CHANGE by Rolando - 2009.08.30

	void updateItemStatusPixmap(const QString & id, QPixmap normalStatus, QPixmap hoverStatus);//VOXOX CHANGE by Rolando - 2009.10.26  

Q_SIGNALS:
	void closeCurrentItem(const QString & id);//VOXOX CHANGE by Rolando - 2009.08.27
	void closeCurrentGroup(const QString & id);//VOXOX CHANGE by Rolando - 2009.09.28 
	void itemClicked(const QString & id);//VOXOX CHANGE by Rolando - 2009.08.28
	void groupClicked(const QString & id);//VOXOX CHANGE by Rolando - 2009.09.08 

public Q_SLOTS:
	void userStateChanged();
	void closeCurrentGroupSlot(QtUMGroup *);//VOXOX CHANGE by Rolando - 2009.09.28 
	void closeCurrentItemSlot(QtUMItem *);//VOXOX CHANGE by Rolando - 2009.08.27
	void itemClickedSlot(QTreeWidgetItem* item, int column);
	void groupClickedSlot(QTreeWidgetItem* item, int column);//VOXOX CHANGE by Rolando - 2009.09.08 
	
private Q_SLOTS:	

	void redrawItems();	 
	void sortItems(bool bypassTimer = false);
	void languageChanged();

private:
	
	void setGroupOpen( const QString& groupId, bool bOpen );

	typedef QList <QtUMItemInfo> QtUMItemInfoList;

	void safeSortItems(bool bypassTimer);

	void safeUserStateChanged();

	void clearTreeSelection();

	void clearSelection();//VOXOX CHANGE by Rolando - 2009.09.04 

	void setGroupClicked(QTreeWidgetItem * item);

	QTreeWidgetItem * findGroupItem(const QString & groupId) const;

	QTreeWidgetItem * findItem(const QString & id) const;//VOXOX CHANGE by Rolando - 2009.09.01 
	
	void addItem   ( QtUMItem* umItem, bool shouldSelectItem );	//VOXOX CHANGE by Rolando - 2009.09.08 

	//VOXOX CHANGE by Rolando - 2009.08.20 
	void addItem   ( QtUMItem* umItem, const QString & groupId,const QString & newGroupName, bool shouldSelectItem);
	void updateItem( QtUMItem* umItem ) ;
	void removeItem( QTreeWidgetItem* item, bool sendSignal = true);//VOXOX CHANGE by Rolando - 2009.09.28 

	void timerEvent(QTimerEvent * event);

	QtUMItem* getCurrentItem();//VOXOX CHANGE by Rolando - 2009.08.18 
	QtUMItem* getItemById( const QString & id);//VOXOX CHANGE by Rolando - 2009.08.18 
	
	QString getCurrentItemItemKey();
	QString getCurrentItemItemId();

	int			findInsertPoint( QTreeWidgetItem* groupItem, QtUMItem* umItem );
	QtUMItem*	getQtUMItemFromItem( QTreeWidgetItem* item ) const;
	QtUMGroup*	getQtUMGroupFromItem( QTreeWidgetItem* item ) const;//VOXOX CHANGE by Rolando - 2009.09.28 
	void		setItemData ( QTreeWidgetItem* item, QtUMItem* umItem );
	void		setGroupData( QTreeWidgetItem* item, QtUMGroup* umGroup );//VOXOX CHANGE by Rolando - 2009.09.28 

	void redrawGroups();//VOXOX CHANGE by Rolando - 2009.08.30
	

	bool		shouldSortItems(bool bypassTimer) ;
	void		sortChildren( QTreeWidgetItem* parentItem);
	void		restoreGroupExpandedStatus();

	QtUMTreeWidget* _tree;
	bool				_sortItems;
	int					_sortTimerId;
	int					_showTimerId;
	bool				_canSort;
	bool				_wantSort;
	bool				_canShow;
	bool				_wantShow;
	bool				_initialLoad;

	QtUMItemList &			_qtUMItemList;//VOXOX CHANGE by Rolando - 2009.08.31 

	mutable RecursiveMutex _mutex;

	QMap <QString, QtUMGroup*> _groupList;//VOXOX CHANGE by Rolando - 2009.08.20

	CWengoPhone & _cWengoPhone;//VOXOX CHANGE by Rolando - 2009.10.23 


};

#endif	//OWQTUMITEMMANAGER_H
