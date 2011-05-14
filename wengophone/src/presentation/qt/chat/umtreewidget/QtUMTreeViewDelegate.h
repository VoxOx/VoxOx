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

#ifndef OWQTUMTREEVIEWDELEGATE_H
#define OWQTUMTREEVIEWDELEGATE_H

#include <QtGui/QPixmap>
#include <QtGui/QItemDelegate>	//VOXOX - JRT - 2009.09.09

class QtUMItemManager;
class QtUMItem;
class QtUMGroup;//VOXOX CHANGE by Rolando - 2009.09.25 


class QtUMTreeViewDelegate : public QItemDelegate {
	Q_OBJECT
public:

	QtUMTreeViewDelegate(QObject * parent = 0);

	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

	void setParent(QWidget * parent);

	QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

private:

	QtUMItem* getItemData( const QModelIndex & index ) const;

	QtUMGroup* getGroupData( const QModelIndex & index ) const;//VOXOX CHANGE by Rolando - 2009.09.25 

	const QWidget * _parent;

	mutable QPixmap _groupBackground;

	QtUMItemManager * _qtItemManager;
};

#endif	//OWQTUMTREEVIEWDELEGATE_H
