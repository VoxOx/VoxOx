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
#include "QtContactTreeMouseFilter.h"
#include "QtContactListManager.h"
#include "QtContactTooltipManager.h"
#include <control/contactlist/CContactList.h>

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <util/Logger.h>

#include <QtCore/QEvent>
#include <QtCore/QMimeData>
#include <QtCore/QTimer>

#include <QtGui/QApplication>
#include <QtGui/QDrag>
#include <QtGui/QMouseEvent>
#include <QtGui/QTreeWidget>
#include <QtGui/QLabel>

//VOXOX - JRT - 2009.08.09 - TODO: this is no longer used.  Remove it from the project along with .h file.

const QString QtContactTreeMouseFilter::WENGO_MIME_TYPE = "application/x-wengo-user-data";
Q_DECLARE_METATYPE(QtContact*)	//VOXOX - JRT - 2009.04.15 

QtContactTreeMouseFilter::QtContactTreeMouseFilter(CContactList & cContactList, QObject * parent, QTreeWidget * treeWidget)
	: QObject(parent),
	_cContactList(cContactList) {
	_tree = treeWidget;

	//The event filter must be installed in the viewport of the QTreeWidget
	_tree->viewport()->installEventFilter(this);

	_dropItem		 = NULL;
	_dragItem		 = NULL;
	_selectedIsGroup = false;
}

bool QtContactTreeMouseFilter::eventFilter(QObject * obj, QEvent * event) 
{
	switch (event->type()) 
	{
	//VOXOX - CJC - 2009.05.09 Add support for tooltip event
	case QEvent::ToolTip:
		mouseToolTipEvent((QHelpEvent *) event);
		return false;

	case QEvent::MouseButtonPress:
		mousePressEvent((QMouseEvent *) event);
		return false;

	case QEvent::MouseButtonRelease:
		mouseReleaseEvent((QMouseEvent *) event);
		return false;

	case QEvent::MouseMove:
		mouseMoveEvent((QMouseEvent *) event);
		event->accept();
		return true;

	case QEvent::DragEnter:
		dragEnterEvent((QDragEnterEvent *) event);
		event->accept();
		return true;

	case QEvent::Drop:
		dropEvent((QDropEvent *) event);
		event->accept();
		return true;

	case QEvent::DragMove:
		dragMoveEvent((QDragMoveEvent *) event);
		event->accept();
		return true;

	default:
		return QObject::eventFilter(obj, event);
	}
}

void QtContactTreeMouseFilter::mousePressEvent(QMouseEvent * event) 
{
	mouseClicked(event->button());
	QtContactTooltipManager * manager = QtContactTooltipManager::getInstance(_cContactList.getCWengoPhone());
	manager->closeCurrentTooltip();
	QTreeWidgetItem * item = _tree->itemAt(event->pos());
//	QtContactListManager * ul = QtContactListManager::getInstance();
	//VOXOX - JRT - 2009.04.16 
	if (item) 
	{
//		if (item->parent())			//VOXOX - JRT - 2009.05.11 - This prevents dragging groups.
		{
			QtContact* qtContact = item->data( 0, Qt::UserRole ).value<QtContact*>();

			if ( qtContact )
			{
				_selectedContactId = item->text(0);
				_selectedIsGroup   = qtContact->isGroup();	//VOXOX - JRT - 2009.05.11 
				_dragItem		   = item;					//VOXOX - JRT - 2009.05.11 

				qtContact->resetMouseStatus();
//				qtContact->setButton(_selectedContactId, event->button());
				qtContact->setButton( event->button() );

				if (event->button() == Qt::LeftButton) 
				{
					_dstart = event->pos();
				}
			}
		}
	}
}
//VOXOX - CJC - 2009.05.09 Mouse Tooltip Event
void QtContactTreeMouseFilter::mouseToolTipEvent(QHelpEvent * event) {

	//mouseClicked(event->button());

	QTreeWidgetItem * item = _tree->itemAt(event->pos());
//	QtContactListManager * ul = QtContactListManager::getInstance();
	//VOXOX - JRT - 2009.04.16 
	QtContactTooltipManager * manager = QtContactTooltipManager::getInstance(_cContactList.getCWengoPhone());
	if (item) 
	{
		if (item->parent()) 
		{
			QtContact* qtContact = item->data( 0, Qt::UserRole ).value<QtContact*>();
			
			manager->openTooltip(qtContact,0);
		}
	}else{
		manager->closeCurrentTooltip();
	}
}



void QtContactTreeMouseFilter::mouseReleaseEvent(QMouseEvent * event) {
	if (event->button() == Qt::RightButton) {
		mouseClicked(Qt::RightButton);
	} else {
		mouseClicked(Qt::NoButton);
	}
}

void QtContactTreeMouseFilter::mouseMoveEvent(QMouseEvent * event) {
	if (_selectedContactId.isNull()) {
		return;
	}

	if (!(event->buttons() & Qt::LeftButton)) {
		return;
	}

	if ((event->pos() - _dstart).manhattanLength() < QApplication::startDragDistance()) {
		return;
	}

	//Define a new empty custom data
	QByteArray custom;
	QDrag * drag = new QDrag(_tree);
	QMimeData * mimeData = new QMimeData;
//	QtContactListManager * ul = QtContactListManager::getInstance();

	//VOXOX - JRT - 2009.04.15 
	QString itemId;
//	QtContact * contact  = ul->getContact(_selectedContactId);
	QtContact* qtContact = _dragItem->data( 0, Qt::UserRole ).value<QtContact*>();

	if ( qtContact )
	{
		itemId = qtContact->getDisplayName();
	}

	if ( !itemId.isEmpty() )
	{
		custom.append( itemId );		//VOXOX - JRT - 2009.05.11 
		mimeData->setText( itemId );
		mimeData->setData(WENGO_MIME_TYPE, custom);

		QLabel label( itemId );
		QPixmap pixmap( 100,16 );	//VOXOX - JRT - 2009.05.11 JRT - get text metrics.
		label.render( &pixmap );

		drag->setMimeData(mimeData);
		drag->setPixmap(pixmap);	//VOXOX - JRT - 2009.05.11 
		drag->start(Qt::MoveAction);
	}
}

void QtContactTreeMouseFilter::dragEnterEvent(QDragEnterEvent * event) {
	if (event->mimeData()->hasFormat(WENGO_MIME_TYPE)) {
		event->acceptProposedAction();
	}
}

void QtContactTreeMouseFilter::dropEvent(QDropEvent * event) 
{
	QTreeWidgetItem * item = _tree->itemAt(event->pos());

	if (!event->mimeData()->hasFormat(WENGO_MIME_TYPE)) {
		return;
	}
	event->acceptProposedAction();

	if (_selectedContactId.isNull()) {
		// No contact selected. Should not happen.
		LOG_WARN("No contact selected. This should not happen.");
		return;
	}

	if (item) 
	{
		if (_selectedContactId == item->text(0)) 
		{
			//Dropped on same item. Nothing to do.
			_selectedContactId = QString::null;
			return;
		}

		_dropSourceContactId = _selectedContactId;
		_dropItem			 = item;

		// Call handleDrop() through a QTimer::singleShot, so that we are out
		// of the event loop. This is necessary because code in handleDrop()
		// can show dialog messages, causing crashes under X11.
		QTimer::singleShot(0, this, SLOT(handleDrop()));
	}

	_selectedContactId = QString::null;
}

//void QtContactTreeMouseFilter::handleDrop()	//VOXOX - JRT - 2009.08.09 - Not used
//{
//	Q_ASSERT(_dropItem);
//	QString destinationContactId = _dropItem->text(0);
//
//	QtContact* dragQtContact = _dragItem->data( 0, Qt::UserRole ).value<QtContact*>();
//	QtContact* dropQtContact = _dropItem->data( 0, Qt::UserRole ).value<QtContact*>();
//
//	if ( _selectedIsGroup )			//Dragging a group
//	{
//		if ( dropQtContact->isGroup() )	//Group reorder
//		{
//			orderGroups( dragQtContact->getId(), dropQtContact->getId() );
//		}
//		else
//		{
//			//Illegal. Do nothing.
//		}
//	}
//	else							//Dragging a contact
//	{
//		//VOXOX - JRT - 2009.07.27 - TODO: don't use getContactProfile().
//		ContactProfile sourceProfile      = _cContactList.getContactProfile(_dropSourceContactId.toStdString());
//		ContactProfile destinationProfile = _cContactList.getContactProfile(destinationContactId.toStdString());
//
//		if ( dropQtContact->isGroup() )	//This is a contact move from group to group.
//		{
//			moveContact( sourceProfile, destinationContactId );
//		}
//		else						//This is a contact merge.
//		{
//			//VOXOX - JRT - 2009.05.11 TODO: we currently restrict merging to the same user group.  This needs to change.
//			bool bMerge = sourceProfile.getFirstUserGroupId() == destinationProfile.getFirstUserGroupId();	//VOXOX - JRT - 2009.05.06 JRT-GRPS
//
//			if ( bMerge )
//			{
//				mergeContacts(_dropSourceContactId, destinationContactId);
//			}
//		}
//	}
//
//	_dropItem = NULL;
//}

void QtContactTreeMouseFilter::dragMoveEvent(QDragMoveEvent * event) {
	QTreeWidgetItem * item = _tree->itemAt(event->pos());
	event->setDropAction(Qt::IgnoreAction);

	if (item) 
	{
		event->setDropAction(Qt::MoveAction);
		if (event->mimeData()->hasFormat(WENGO_MIME_TYPE)) 
		{
//			event->acceptProposedAction();	//VOXOX - JRT - 2009.05.12 - This defaults/resets dropAction to CopyAction
			event->accept();
		}
	}
}

//VOXOX - JRT - 2009.08.09 - Not used
//void QtContactTreeMouseFilter::moveContact( const ContactProfile& sourceProfile, const QString& destinationContactId )
//{
//	QString groupId;
//	if (_dropItem->parent()) 
//	{
//		//The destination is a contact, move the contact to the destination group
//		groupId = _dropItem->parent()->text(0);
//	} 
//	else 
//	{
//		//The destination is a group
//		groupId = destinationContactId;
//	}
//
//	//VOXOX - JRT - 2009.05.06  - JRT-GRPS - let controller do the work.
////		sourceProfile.setGroupId(groupId.toStdString());
////		sourceProfile.changeUserGroup( ContactGroup( groupId.toStdString(), EnumGroupType::GroupType_User) );
////		_cContactList.updateContact(sourceProfile);
//	_cContactList.moveContact( sourceProfile, const_cast<ContactProfile&>(sourceProfile).getFirstUserGroupId(), groupId.toStdString() );
//}
