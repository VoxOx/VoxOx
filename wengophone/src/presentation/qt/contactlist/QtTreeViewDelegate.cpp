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
#include "QtTreeViewDelegate.h"

#include "QtContactWidget.h"
#include "QtContact.h"
#include "QtContactPixmap.h"
#include "QtContactList.h"
#include "QtContactListStyle.h"

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <util/Logger.h>
#include <util/StringList.h>

#include <QtGui/QtGui>
#include <QtCore/QMetaType>

Q_DECLARE_METATYPE(QtContact*)

static int GROUP_WIDGET_FRAME_HEIGHT = 21;

QtTreeViewDelegate::QtTreeViewDelegate(CWengoPhone & cWengoPhone, QtContactManager * qtContactManager, QObject * parent)
	: QItemDelegate(parent),
	_qtContactManager(qtContactManager),
	_cWengoPhone(cWengoPhone) 
{
}

void QtTreeViewDelegate::setParent(QWidget * parent) 
{
	_parent = parent;
}


QWidget * QtTreeViewDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
	QtContact* qtContact = getItemData( index );		//VOXOX - JRT - 2009.08.10 
//	QtContactWidget * widget = new QtContactWidget( qtContact->getId().toStdString(), qtContact->getKey().toStdString(), _cWengoPhone, _qtContactManager, parent);	//VOXOX - JRT - 2009.08.10 - multiple groups.
	QtContactWidget * widget = new QtContactWidget( qtContact->getId().toStdString(), qtContact->getKey().toStdString(), _cWengoPhone, parent);		//VOXOX - JRT - 2009.10.15 
	return widget;
}

void QtTreeViewDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const 
{
	QtContactWidget * widget = qobject_cast<QtContactWidget *>(editor);

	if (widget) 
	{
		widget->setText(index.model()->data(index).toString());
	}
}

void QtTreeViewDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const 
{
	QtContactWidget * widget = qobject_cast<QtContactWidget *>(editor);

	if (widget) 
	{
		model->setData(index, widget->text());
	}
}

void QtTreeViewDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const 
{
	//VOXOX - JRT - 2009.08.10 - TODO: move this and drawGroup to QtContact class
	if (!index.parent().isValid()) 
	{
		drawGroup(painter, option, index);
	} 
	else 
	{
		//VOXOX - JRT - 2009.08.10 
		QtContact* qtContact = getItemData( index );

		if ( qtContact )
		{
//			if ( qtContact->isOpen() )
//			{
//				QWidget * parent = const_cast<QWidget*>(_parent);
////				QtContactWidget * widget = new QtContactWidget( qtContact->getId().toStdString(), qtContact->getKey().toStdString(), _cWengoPhone, _qtContactManager, parent);	//VOXOX - JRT - 2009.08.10 - multiple groups.
//				QtContactWidget * widget = new QtContactWidget( qtContact->getId().toStdString(), qtContact->getKey().toStdString(), _cWengoPhone, parent);	//VOXOX - JRT - 2009.10.15 
////				setSizeHint( 0, QSize(-1, getHeight() ) );
//			}
//			else
			{
			qtContact->paint( painter, option );
		}
	}
		else
		{
			int xxx = 1;
		}
	}
}

QSize QtTreeViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const 
{
	QSize orig = QItemDelegate::sizeHint(option, index);
	QtContact* qtContact = getItemData( index );			//VOXOX - JRT - 2009.08.10 - Multiple groups

	if (qtContact) 
	{
		return QSize(orig.width(), qtContact->getHeight() );
	} 
	else 
	{
		if (!index.parent().isValid()) 
		{
			return (QSize(orig.width(), GROUP_WIDGET_FRAME_HEIGHT));
		}
	}

	return orig;
}

void QtTreeViewDelegate::drawGroup(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const 
{
	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();
	painter->setPen(contactListStyle->getGroupFontColor());
	painter->drawPixmap(option.rect.left(),option.rect.top(),getGroupBackGround(option.rect,option));

	QFont font = option.font;
	painter->setFont(font);

	QPixmap px;
	if (option.state & QStyle::State_Open) 
	{
		px = contactListStyle->getGroupPixmapOpen();
	} 
	else 
	{
		px = contactListStyle->getGroupPixmapClose();
	}

	int x = option.rect.left();
	QRect r = option.rect;
	painter->drawPixmap(x, r.top() + 3, px);
	x += px.width() + 3;
	r.setLeft(x);
	int y = ((r.bottom()-r.top())-QFontMetrics(font).height())/2;
	r.setTop(y + r.top());
	r.setLeft(r.left() + 10);

	//VOXOX - JRT - 2009.05.04 -TODO: this is all business logic that should not be here.
	//	We should have a simple getGroupText() method that provides this.
	QString groupName;
	std::string groupId;
	std::string groupNameTmp;
	std::string groupContactsOnline;

	if (index.data().toString() == QtContactList::DEFAULT_GROUP_NAME) 
	{
		//VOXOX - JRT - 2009.09.03 - Add 0/0 for this special group.
		int total  = 0;
		int online = 0;

		_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().getContactList().getCounts( total, online );
		groupContactsOnline = " (" + String::fromNumber( online ) + "/" + String::fromNumber( total ) + ")";

		//VOXOX CHANGE CJC - We don't want to show ContactList when groups are hidden. We can't remove group, so just remove name.
		groupName    = tr("All");
		groupNameTmp = std::string(groupName.toUtf8().data());
	} 
	else 
	{
		groupName			= index.data().toString();
		groupId				= groupName.toStdString();
		groupNameTmp		= _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().getContactGroupName(groupId);
		groupContactsOnline = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().getContactGroupContactsOnline(groupId);
	}

	groupName = QString::fromUtf8(groupNameTmp.c_str(), groupNameTmp.size()) + QString::fromUtf8(groupContactsOnline.c_str(), groupContactsOnline.size());
	//End business logic.

	QString str = groupName;
	painter->drawText(r, Qt::AlignLeft, str, 0);
}

QPixmap QtTreeViewDelegate::getGroupBackGround(const QRect & rect, const QStyleOptionViewItem & option) const 
{
	if (_groupBackground.isNull() || _groupBackground.width() != rect.width()) 
	{
		//This colors cant be hardcoded.
	/*	QLinearGradient lg(QPointF(1.0, 0.0), QPointF(1.0, GROUP_WIDGET_FRAME_HEIGHT));
		lg.setColorAt(0.8, QColor(212, 208, 200));
		lg.setColorAt(0.0, QColor(255, 255, 255));*/

		QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();

		_groupBackground = QPixmap(rect.width(),GROUP_WIDGET_FRAME_HEIGHT);
		QPainter painter(&_groupBackground);
		//TODO CHANGE CODE TO GRADIENT IF NEEDED
		painter.fillRect(_groupBackground.rect(),QBrush(contactListStyle->getGroupBackgroundColorTop()));
		//Borders
		QPen pen = painter.pen();
		painter.setPen (contactListStyle->getGroupBorderColor());	

		painter.drawLine(QPoint(0,0),QPoint(rect.width(),0));

		painter.drawLine(QPoint(0,GROUP_WIDGET_FRAME_HEIGHT-1),QPoint(rect.width(),GROUP_WIDGET_FRAME_HEIGHT-1));

		painter.setPen (pen);	

		painter.end();
	}
	return _groupBackground;
}

QtContact* QtTreeViewDelegate::getItemData( const QModelIndex & index ) const
{
	QtContact* result = index.model()->data( index, Qt::UserRole ).value<QtContact*>();
	
	return result;
}
