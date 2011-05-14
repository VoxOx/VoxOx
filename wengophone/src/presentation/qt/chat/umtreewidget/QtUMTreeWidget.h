/*

 VOXOX !!!
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

#ifndef OWQTUMTREEWIDGET_H
#define OWQTUMTREEWIDGET_H

#include <QtGui/QAbstractItemDelegate>	//VOXOX - JRT - 2009.09.09
#include <QtGui/QStyleOptionViewItem>	//VOXOX - JRT - 2009.09.09
#include <QtGui/QTreeWidget>			//VOXOX - JRT - 2009.09.09
#include <QtCore/QMutex>				//VOXOX - JRT - 2009.09.09

class QtUMItem;
class QtUMGroup;//VOXOX CHANGE by Rolando - 2009.09.25 

namespace Ui { class UMItemList; }

//====================================================================

class QtUMItemTreeViewDelegate : public QAbstractItemDelegate 
{
	Q_OBJECT
public:

	QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const ;

private:
	QtUMItem* getItemData( const QModelIndex & index ) const;
	QtUMGroup* getGroupData( const QModelIndex & index ) const;
	
};

//=============================================================================

//=============================================================================

class QtUMTreeWidget : public QTreeWidget
{
	Q_OBJECT
public:

	explicit QtUMTreeWidget(QWidget *parent = 0);
	virtual ~QtUMTreeWidget();
	
	QtUMItem* getItemData		   (       QTreeWidgetItem* item  ) const;
	QtUMItem* getItemDataFromIndex( const QModelIndex&     index ) const;

	QtUMGroup* getGroupData		   (       QTreeWidgetItem* item  ) const;//VOXOX CHANGE by Rolando - 2009.09.25 
	QtUMGroup* getGroupDataFromIndex( const QModelIndex&     index ) const;//VOXOX CHANGE by Rolando - 2009.09.25 

	bool toggleExpanded			( QTreeWidgetItem* item );
	void toggleExpandedItem		( QTreeWidgetItem* item ); 
	void openItemInfo			( QTreeWidgetItem* item );
	void closeItemInfo			( QTreeWidgetItem * item );//VOXOX CHANGE by Rolando - 2009.09.08 

	void clear();
	
	void lock()										{ _mutex->lock();	}
	void unlock()									{ _mutex->unlock();	}

	void setBlinkingItem( QTreeWidgetItem * item );//VOXOX CHANGE by Rolando - 2009.08.27 

	void stopBlinkingItem( QTreeWidgetItem * item );//VOXOX CHANGE by Rolando - 2009.08.27

Q_SIGNALS:
    
	void itemLeftClicked ( QTreeWidgetItem* item, int column );
	void groupLeftClicked ( QTreeWidgetItem* item, int column );//VOXOX CHANGE by Rolando - 2009.09.08 
	void itemRightClicked( QTreeWidgetItem* item, int column );
	void closeCurrentItem( QtUMItem * umItem );//VOXOX CHANGE by Rolando - 2009.08.27
	void closeCurrentGroup( QtUMGroup * umGroup );//VOXOX CHANGE by Rolando - 2009.09.28 

	
private:
	
	void mouseMoveEvent	  ( QMouseEvent * event );//VOXOX CHANGE by Rolando - 2009.09.08 
	void mousePressEvent( QMouseEvent * event );//VOXOX CHANGE by Rolando - 2009.09.08 
	void keyPressEvent(QKeyEvent * event);//VOXOX CHANGE by Rolando - 2009.09.07 
	mutable QMutex* _mutex;
	
};

#endif	//OWQTUMTREEWIDGET_H

