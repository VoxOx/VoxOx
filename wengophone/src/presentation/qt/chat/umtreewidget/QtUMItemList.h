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


#ifndef OWQTUMITEMLIST_H
#define OWQTUMITEMLIST_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QWidget>	//VOXOX - JRT - 2009.09.09
#include <presentation/qt/contactlist/QtContactPixmap.h>//VOXOX CHANGE by Rolando - 2009.08.31 
#include <control/CWengoPhone.h>//VOXOX CHANGE by Rolando - 2009.10.23 

class QtUMItemManager;
class QtUMItem;

namespace Ui { class UMItemList; }


class QtUMItemList : public QWidget{
	Q_OBJECT
public:	

	QtUMItemList(CWengoPhone & cWengoPhone);

	virtual ~QtUMItemList();//VOXOX CHANGE by Rolando - 2009.10.23 

	QtUMItemManager * getItemManager() const {
		return _itemManager;

	}


	void updatePresentation();

	void setInitialLoad( bool bSet ); 
	bool getInitialLoad()		const			{ return _initialLoad;	}


	QStringList getItemListKeysOfGroup(const QString & groupId);//VOXOX CHANGE by Rolando - 2009.09.08 

	QStringList getItemListKeys();//VOXOX CHANGE by Rolando - 2009.09.08 
	
	//VOXOX CHANGE by Rolando - 2009.10.26 - added parameter hoverStatus
	//VOXOX CHANGE by Rolando - 2009.10.23 - added parameter contactId
	void addItem(const QString& contactId,const QString& id, const QString & groupId, const QString & displayName, const QString & avatarData, QPixmap normalStatus, QPixmap hoverStatus, bool shouldSelectItem);//VOXOX CHANGE by Rolando - 2009.10.26 

	void removeItem(const QString & id);//VOXOX CHANGE by Rolando - 2009.08.19 
	
	//VOXOX CHANGE by Rolando - 2009.10.26 - added parameter QPixmap normalStatus, QPixmap hoverStatus
	//VOXOX CHANGE by Rolando - 2009.10.23 - added parameter contactId
	//VOXOX CHANGE by Rolando - 2009.09.08 
	void updateItem(const QString& contactId, const QString& id,  const QString & groupId, const QString & displayName, const QString & avatarData, QPixmap normalStatusPixmap, QPixmap hoverStatusPixmap, bool shouldSelectItem);

	void addGroup(QString itemGroupId, QString newGroupName, bool shouldSelectGroup);//VOXOX CHANGE by Rolando - 2009.09.24 
	//VOXOX CHANGE by Rolando - 2009.10.23 
	void addItemToGroup(const QString& contactId,const QString& id, const QString & groupId, const QString & displayName, const QString & avatarData, QPixmap normalStatus, QPixmap hoverStatus, bool shouldSelectItem);//VOXOX CHANGE by Rolando - 2009.09.08 
	void removeGroup(const QString & groupId);//VOXOX CHANGE by Rolando - 2009.08.18 
	bool existsGroup(QString groupId);//VOXOX CHANGE by Rolando - 2009.09.03 
	void renameGroup(QString itemGroupId, QString newGroupName);//VOXOX CHANGE by Rolando - 2009.08.26

	//VOXOX CHANGE by Rolando - 2009.09.08 - 
	/* Returns empty string if was not found id, otherwise can return:
	 *its own id if item not belongs to a group or groupId if this one was found
	 */
	QString getGroupIdOfItem(const QString & id);//VOXOX CHANGE by Rolando - 2009.09.08 

	void startBlinkingItem(const QString& id);//VOXOX CHANGE by Rolando - 2009.08.28

	void stopBlinkingItem(const QString& id);//VOXOX CHANGE by Rolando - 2009.08.28

	void startBlinkingGroup(const QString& id);//VOXOX CHANGE by Rolando - 2009.10.02 

	void stopBlinkingGroup(const QString& id);//VOXOX CHANGE by Rolando - 2009.10.02 

	void setCurrentItem(const QString & id);//VOXOX CHANGE by Rolando - 2009.08.29

	QString getCurrentItemId();//VOXOX CHANGE by Rolando - 2009.09.03 

	void clear();//VOXOX CHANGE by Rolando - 2009.08.30

	void updateItemStatusPixmap(const QString & id, QPixmap normalStatus, QPixmap hoverStatus);//VOXOX CHANGE by Rolando - 2009.10.26  

	//VOXOX CHANGE by Rolando - 2009.09.03 
	QSize sizeHint() const {
		return QSize(150, 500 );		
	}

	
public Q_SLOTS:

	void closeCurrentGroupSlot(QString id);//VOXOX CHANGE by Rolando - 2009.09.28 
	void closeCurrentItemSlot(QString id);//VOXOX CHANGE by Rolando - 2009.08.27
	void itemClickedSlot(QString id);//VOXOX CHANGE by Rolando - 2009.08.28

	void groupClickedSlot(QString id);//VOXOX CHANGE by Rolando - 2009.09.08 

Q_SIGNALS:
	void itemClosed(QString id);//VOXOX CHANGE by Rolando - 2009.08.27
	void groupClosed(QString id);//VOXOX CHANGE by Rolando - 2009.09.28 
	void itemClicked(QString id);//VOXOX CHANGE by Rolando - 2009.08.28
	void groupClicked(QString id);//VOXOX CHANGE by Rolando - 2009.09.08 

private:

	void timerEvent(QTimerEvent * event);	

	QtUMItemManager * _itemManager;

	Ui::UMItemList * _ui;
	
	bool			_initialLoad;	//VOXOX - JRT - 2009.04.12

	CWengoPhone & _cWengoPhone;//VOXOX CHANGE by Rolando - 2009.10.23 
	
};

#endif	//OWQTUMITEMLIST_H

