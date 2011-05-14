/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either versio
 n 2 of the License, or
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

#include "stdafx.h"	
#include "QtUMTreeViewDelegate.h"

#include <QtGui/QtGui>	//VOXOX - 2009.09.09
#include <presentation/qt/chat/umtreewidget/QtUMItemListStyle.h>
#include "QtUMItem.h"
#include "QtUMGroup.h"
#include "QtUMItemManager.h"//VOXOX CHANGE by Rolando - 2009.08.24 

QtUMTreeViewDelegate::QtUMTreeViewDelegate(QObject * parent)
	: QItemDelegate(parent){
}

void QtUMTreeViewDelegate::setParent(QWidget * parent) {
	_parent = parent;
}

void QtUMTreeViewDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const 
{

	QtUMItem* umItem = getItemData( index );

	if ( umItem )
	{
		umItem->paint( painter, option );
	}
	else{
		QtUMGroup* umGroup = getGroupData( index );

		if ( umGroup )
		{
			umGroup->paint( painter, option );
		}
		
	}
}

QSize QtUMTreeViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const {
	QSize orig = QItemDelegate::sizeHint(option, index);
	QtUMItem* umItem = getItemData( index );

	if (umItem) 
	{
		return QSize(orig.width(), umItem->getHeight() );
	} 
	else 
	{
		QtUMGroup * umGroup = getGroupData( index );//VOXOX CHANGE by Rolando - 2009.09.25 

		if(umGroup){//VOXOX CHANGE by Rolando - 2009.09.25 
			return (QSize(orig.width(), umGroup->getHeight()));//VOXOX CHANGE by Rolando - 2009.09.25 
		}

	}
	return orig;
}

QtUMItem* QtUMTreeViewDelegate::getItemData( const QModelIndex & index ) const
{
	QtUMItem* result = index.model()->data( index, Qt::UserRole ).value<QtUMItem*>();
	
	return result;
}

QtUMGroup* QtUMTreeViewDelegate::getGroupData( const QModelIndex & index ) const
{
	QtUMGroup* result = index.model()->data( index, Qt::UserRole ).value<QtUMGroup*>();
	
	return result;
}
