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
* CLASS DESCRIPTION 
* @author Jeff
* @date 2009.06.13
*/


#include "stdafx.h"	
#include "VoxContactTreeWidget.h"

#include <QtCore/QEvent>
#include <QtCore/QMimeData>
#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include <QtGui/QDrag>
#include <QtGui/QLabel>
#include <QtGui/QApplication>

#include "QtContactListManager.h"
#include "QtContactTooltipManager.h"
#include <control/contactlist/CContactList.h>
#include <qtutil/SafeConnect.h>

//For painting
#include "QtContactListStyle.h"
#include "QtContactList.h"			//Only provide DEFAULT_GROUP_NAME.

#include <util/Logger.h>

//=============================================================================
//VOXOX - JRT - 2009.10.15 - Moved handling to QtContact.
//QSize QtContactTreeViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const 
//{
//	QSize orig = option.rect.size();
//	QtContact* qtContact = getItemData( index );		//VOXOX - JRT - 2009.08.10 
//
//	if (qtContact) 
//	{
//		return QSize(orig.width(), qtContact->getHeight() );
//	} 
//
//	return orig;
//}
//
////-----------------------------------------------------------------------------
//
//void QtContactTreeViewDelegate::paint(QPainter *,const QStyleOptionViewItem &,const QModelIndex &) const
//{
//}
//
////-----------------------------------------------------------------------------
//
//QtContact* QtContactTreeViewDelegate::getItemData( const QModelIndex & index ) const
//{
//	QtContact* result = index.model()->data( index, Qt::UserRole ).value<QtContact*>();
//	
//	return result;
//}

//=============================================================================


//=============================================================================

//const int VoxContactListTreeWidget::_GroupWidgetFrameHeight	= 21;		//VOXOX - JRT - 2009.05.13 TODO get from common location
//const int VoxContactListTreeWidget::_AvatarHeight			= 28;
//const int VoxContactListTreeWidget::_AvatarWidth			= 28;
//const int VoxContactListTreeWidget::_ContactMargin			= 16;
//const int VoxContactListTreeWidget::_AvatarTextMargin		=  4;
//const int VoxContactListTreeWidget::_AvatarMarginTop		=  4;

const QString VoxContactListTreeWidget::WENGO_MIME_TYPE = "application/x-wengo-user-data";
Q_DECLARE_METATYPE(QtContact*)

//-----------------------------------------------------------------------------

VoxContactListTreeWidget::VoxContactListTreeWidget( QWidget* parent ) : QTreeWidget(parent)
{
//	_dragItem			= NULL;
//	_dropItem			= NULL;
	_dragItemId			= "";		//VOXOX - JRT - 2009.09.21 
	_dropItemId		    = "";		//VOXOX - JRT - 2009.09.21 
	_selectedIsGroup	= false;

	_cContactList		= NULL;
	_button				= Qt::NoButton;		//Clear
	_timerInterval		= qApp->doubleClickInterval();
	_waitForDoubleClick = false;
	_lastColumnClicked  = 0;

	resetOpenKeys();

	_mutex = new QMutex(QMutex::Recursive);


	//VOXOX - JRT - 2009.10.15 
//	QtContactTreeViewDelegate* pDelegate = new QtContactTreeViewDelegate();
//	pDelegate->setParent(this);
//	delegate->setParent(_ui->treeWidget->viewport());
//	setItemDelegate(pDelegate);
	
	this->installEventFilter( this );	//Enable our own eventFilter!
	
	//Interpret left/right/double clicks.
	connect( this, SIGNAL(itemClicked(QTreeWidgetItem*, int )), SLOT( myClickHandler(QTreeWidgetItem*, int )));
	connect( this, SIGNAL(itemSelectionChanged()),				SLOT(treeViewSelectionChanged()		     ));
}

//-----------------------------------------------------------------------------

VoxContactListTreeWidget::~VoxContactListTreeWidget()
{
	//delete _toolTipManager;
	delete _mutex;
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::resetOpenKeys()
{	
	_openContactKey		= "";
	_openContactId		= "";
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::setCContactList( CContactList* cContactList )
{
	_cContactList = cContactList;
}

//-----------------------------------------------------------------------------

bool VoxContactListTreeWidget::eventFilter(QObject * obj, QEvent * event) 
{
	bool bRet = false;

	switch (event->type()) 
	{
	case QEvent::ToolTip:
		mouseToolTipEvent((QHelpEvent *) event);
		bRet = false;
		break;

	default:
		bRet = false;
	}

	return bRet;
}


//-----------------------------------------------------------------------------
//Mouse-related methods
//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::mousePressEvent(QMouseEvent * event) 
{

	QTreeWidgetItem * item = this->itemAt(event->pos());

	if (item) 
	{
		QtContact* qtContact = getItemData( item );

		if ( qtContact && qtContact->canDrag() )		//VOXOX - JRT - 2009.08.13 
		{
			//VOXOX - JRT - 2009.09.21 - We save the dragged items ID instead of the TreeWidgetItem ptr itself.
			//							 We do so because the tree can be redrawn during the DND, thereby
			//						      making the widget ptr invalid and resulting in a crash.
			//							 When we do the drop, we will access ContactListManager to get
			//							  the needed data to complete the drop.
			_selectedContactId = item->text(0);
			_selectedIsGroup   = qtContact->isGroup();
//			_dragItem		   = item;
			_dragItemId		   = qtContact->getKey();	//VOXOX - JRT - 2009.09.21 

			if (event->button() == Qt::LeftButton) 
			{
				_dstart = event->pos();
			}
		}
	}

	QTreeWidget::mousePressEvent(event);

	handleToolTipEvent( event->pos() );//VOXOX - CJC - 2009.10.20 Task UPDATE: UCL when clicking on contact
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::mouseReleaseEvent(QMouseEvent * event) 
{
	QTreeWidgetItem * item = itemAt(event->pos());

	if ( item )
	{
		_button = event->button();		//Save so our itemClicked handler can use.
	}
	else
	{
		_button = Qt::NoButton;		//Clear
	}

	QTreeWidget::mouseReleaseEvent( event );
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::mouseMoveEvent(QMouseEvent * event) 
{
	//Drag and drop checks
	bool bDnd = false;

//	if ( !_selectedContactId.isNull() && _dragItem )				//Must have selected item (you clicked on it) //VOXOX - JRT - 2009.07.11 
	if ( !_selectedContactId.isNull() && !_dragItemId.isNull() )	//Must have selected item (you clicked on it) //VOXOX - JRT - 2009.09.21 
	{
		if ((event->buttons() & Qt::LeftButton))	//Doing a left-mouse click and drag
		{
			if ((event->pos() - _dstart).manhattanLength() >= QApplication::startDragDistance()) 
			{
				bDnd = true;
			}
		}
	}

	if ( bDnd )
	{
		//Define a new empty custom data
		QByteArray custom;
		QDrag * drag = new QDrag(this);
		QMimeData * mimeData = new QMimeData;

		QString itemId;
//		QtContact* qtContact = getItemData( _dragItem );
		QtContact* qtContact = getQtContact( _dragItemId );	//VOXOX - JRT - 2009.09.21 

		if ( qtContact )
		{
			itemId = qtContact->getDisplayName();
		}

		if ( !itemId.isEmpty() )
		{
			custom.append( itemId );
			mimeData->setText( itemId );
			mimeData->setData(WENGO_MIME_TYPE, custom);

			QLabel label( itemId );
			QPixmap pixmap( 100,16 );		//VOXOX - JRT - 2009.05.11 JRT - get text metrics.
			label.render( &pixmap );

			drag->setMimeData(mimeData);
			drag->setPixmap(pixmap);		//VOXOX - JRT - 2009.05.11 
			drag->start(Qt::MoveAction);	//This captures remaining mouse moves until release.
		}
	}
	else
	{
		if ( getTooltipMgr()->isTooltipActive() )
		{
			handleToolTipEvent( event->pos() );
		}
	}
}

//-----------------------------------------------------------------------------
//End Mouse-related methods
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//Mouse tooltip methods	- VOXOX - CJC - 2009.05.09 Mouse Tooltip Event
//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::mouseToolTipEvent(QHelpEvent * event) 
{
	handleToolTipEvent( event->pos() );
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::handleToolTipEvent( QPoint pos ) 
{
	QTreeWidgetItem * item = itemAt( pos );
	
	if (item) 
	{
		if (item->parent())			//This is top level
		{
			QtContact* qtContact = getItemData( item );
			getTooltipMgr()->openTooltip(qtContact,0);
		}
		else		//No tool tip on groups.
		{
			getTooltipMgr()->closeCurrentTooltip();
		}
	}
	else
	{
		getTooltipMgr()->closeCurrentTooltip();
	}
}

//-----------------------------------------------------------------------------

QtContactTooltipManager* VoxContactListTreeWidget::getTooltipMgr()
{
	return QtContactTooltipManager::getInstance(_cContactList->getCWengoPhone());
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//Drag and drop
//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::dragEnterEvent(QDragEnterEvent * event) 
{
	if (event->mimeData()->hasFormat(WENGO_MIME_TYPE)) 
	{
		event->acceptProposedAction();
	}
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::dropEvent(QDropEvent * event) 
{
	QTreeWidgetItem * item = this->itemAt(event->pos());

	if (!event->mimeData()->hasFormat(WENGO_MIME_TYPE)) {
		return;
	}
	event->acceptProposedAction();

	if (_selectedContactId.isNull()) 
	{
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
//		_dropItem			 = item;
		_dropItemId			 = item->text(0);	//VOXOX - JRT - 2009.09.21 
		

		// Call handleDrop() through a QTimer::singleShot, so that we are out
		// of the event loop. This is necessary because code in handleDrop()
		// can show dialog messages, causing crashes under X11.
		QTimer::singleShot(0, this, SLOT(handleDrop()));
	}

	_selectedContactId = QString::null;
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::handleDrop() 
{
//	Q_ASSERT(_dropItem);
	Q_ASSERT( !_dropItemId.isNull() );	//VOXOX - JRT - 2009.09.21 

//	QString destinationId = _dropItem->text(0);		//Could be ContactId or GroupId
	QString destinationId = _dropItemId;			//Could be ContactId or GroupId

//	QtContact* dragQtContact = getItemData( _dragItem );
//	QtContact* dropQtContact = getItemData( _dropItem );
	QtContact* dragQtContact = getQtContact( _dragItemId );	//VOXOX - JRT - 2009.09.21 
	QtContact* dropQtContact = getQtContact( _dropItemId );	//VOXOX - JRT - 2009.09.21 

	if ( dragQtContact != NULL && dropQtContact != NULL )
	{
	if ( _selectedIsGroup )			//Dragging a group
	{
		if ( dropQtContact->isGroup() )	//Group reorder
		{
			orderGroups( dragQtContact->getKey(), dropQtContact->getKey() );	//VOXOX - JRT - 2009.08.10 - Multiple groups.
		}
		else
		{
			//Illegal. Do nothing.
		}
	}
	else							//Dragging a contact
	{
		if ( dropQtContact->isGroup() )	//This is a contact move from group to group.
		{
			moveContactToGroup( _dropSourceContactId.toStdString(), dragQtContact->getGroupKey(), destinationId.toStdString() );	//VOXOX - JRT - 2009.08.08 
		}
		else						//This is a contact merge.
		{
			//VOXOX - JRT - 2009.05.11 TODO: we currently restrict merging to the same user group.  This needs to change.
			bool bMerge = dragQtContact->getGroupKey() == dropQtContact->getGroupKey();	//VOXOX - JRT - 2009.08.08 

			if ( bMerge )
			{
				mergeContacts(_dropSourceContactId, destinationId);
			}
		}
	}
	}

//	_dropItem = NULL;
//	_dragItem = NULL;
	_dragItemId = "";	//VOXOX - JRT - 2009.09.21 
	_dropItemId = "";	//VOXOX - JRT - 2009.09.21 
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::dragMoveEvent(QDragMoveEvent * event) 
{
	QTreeWidgetItem * item = this->itemAt(event->pos());
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

	this->doAutoScroll();
}

//-----------------------------------------------------------------------------
//End Drag and drop
//-----------------------------------------------------------------------------


//------------------------------------------------------------------------------
//Work methods
//------------------------------------------------------------------------------

void VoxContactListTreeWidget::moveContactToGroup( const std::string& sourceContactId, const std::string& sourceGroupId, const std::string& destGroupId )
{
	//VOXOX - JRT - 2009.05.06  - JRT-GRPS - let controller do the work.
	//VOXOX - JRT - 2009.07.27 - TODO: don't use getContactProfile().
	ContactProfile sourceProfile = _cContactList->getContactProfile( sourceContactId );
	_cContactList->moveContact( sourceProfile, sourceGroupId, destGroupId );	//VOXOX - JRT - 2009.08.08 
}

//-----------------------------------------------------------------------------
//End work methods
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//Helper methods
//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::clear()
{
	QTreeWidget::clear();

//	_openContactKey = "";	//VOXOX - JRT - 2009.10.14 - We need this to be able to reopen this widget.
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::removeItem( QTreeWidgetItem* item )
{
	QMutexLocker locker(_mutex);	//VOXOX - JRT - 2009.10.13 

	if ( getOpenItem() == item) 
	{
		resetOpenKeys();
	}
}

//-----------------------------------------------------------------------------
	
QTreeWidgetItem* VoxContactListTreeWidget::getOpenItem()
{ 
	//VOXOX - JRT - 2009.07.20 - TODO: just maintain a separate list of open widgets.
	//							 Then add/remove as opened or closed.
	QMutexLocker locker(_mutex);	

	QTreeWidgetItem* result		= NULL;
	QTreeWidgetItem* item		= NULL;
	QtContact*		 qtContact  = NULL;

	if ( !_openContactKey.empty() )
	{
		int topCount = topLevelItemCount();

		for (int topIndex = 0; topIndex < topCount; ++topIndex) 
		{
			QTreeWidgetItem* groupItem = topLevelItem(topIndex);

			int count = groupItem->childCount();

			for ( int index = 0; index < count; ++index ) 
			{
				item	  = groupItem->child(index);
				qtContact = getItemData( item );

				if ( qtContact )
				{
					if ( qtContact->getKey().toStdString() == _openContactKey )	//VOXOX - JRT - 2009.08.10 - Multiple groups.
					{
						if ( qtContact->isOpen() )
						{
							result = item;
							break;
						}
					}
				}
			}

			if ( result )
			{
				break;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------

QTreeWidgetItem* VoxContactListTreeWidget::getItemByContactKey( const std::string contactKey, bool isKey )
{
	QMutexLocker locker(_mutex);	

	QTreeWidgetItem* result		= NULL;
	QTreeWidgetItem* item		= NULL;
	QtContact*		 qtContact  = NULL;

	if ( !contactKey.empty() )
	{
		int topCount = topLevelItemCount();

		for (int topIndex = 0; topIndex < topCount; ++topIndex) 
		{
			QTreeWidgetItem* groupItem = topLevelItem(topIndex);

			int count = groupItem->childCount();

			for ( int index = 0; index < count; ++index ) 
			{
				item	  = groupItem->child(index);
				qtContact = getItemData( item );

				if ( qtContact )
				{
					if ( isKey )
					{
					if ( qtContact->getKey().toStdString() == contactKey )	//VOXOX - JRT - 2009.08.10 - Multiple groups
					{
						result = item;
						break;
					}
				}
					else
					{
						if ( qtContact->getId().toStdString() == contactKey )
						{
							result = item;
							break;
						}
					}
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------

QtContact* VoxContactListTreeWidget::getQtContact( const QString& itemKey ) const
{
	QtContact* result = NULL;

	QtContactListManager*	ul = QtContactListManager::getInstance();

	if ( ul )
	{
		result = ul->getContact( itemKey );
	}

	return result;
}

//-----------------------------------------------------------------------------

QtContact* VoxContactListTreeWidget::getItemData( QTreeWidgetItem* item ) const
{
	QtContact* pRet = NULL;

	if ( item )
	{
		pRet = item->data( 0, Qt::UserRole ).value<QtContact*>();
	}

	return pRet;
}

//-----------------------------------------------------------------------------

QtContact* VoxContactListTreeWidget::getItemDataFromIndex( const QModelIndex& index ) const
{
	return getItemData( itemFromIndex( index ) );
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::toggleExpandedContact( QTreeWidgetItem* item )
{
	QMutexLocker locker(_mutex);	//VOXOX - JRT - 2009.10.13 

	if (item->parent())			//This is contact
	{
		if ( getOpenItem() == item)		//VOXOX - JRT - 2009.07.12 
		{
			closeContactInfo();
		} 
		else 
		{
			openContactInfo(item);
		}
	}
	else
	{
		toggleExpanded( item );
	}

	viewport()->update();
}

//-----------------------------------------------------------------------------

bool VoxContactListTreeWidget::toggleExpanded( QTreeWidgetItem* item )
{
	bool bOpen = false;

	if ( item )
	{
		if ( isItemExpanded(item) ) 
		{
			collapseItem(item);
			bOpen = false;
		} 
		else 
		{
			expandItem(item);
			bOpen = true;
		}
	}

	return bOpen;
}

//-----------------------------------------------------------------------------
//End helper methods
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//Begin click handling/filtering methods
//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::treeViewSelectionChanged() 
{
	if (_button == Qt::NoButton) 
	{
		closeContactInfo();
	}
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::myClickHandler( QTreeWidgetItem* item, int column )
{
	//We have to handle fact that on a double-click, QT also generates TWO clicks.
	//	To do this, we capture the click, start a timer and make a decision after the timer expires.
	_currentItemClicked = item;
	waitForClickTimer( item, column );
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::emitClickedSignal( bool bIsDoubleClick )
{
	if ( _lastItemClicked )
	{
		if ( bIsDoubleClick )
		{
	//		itemDoubleClicked( _lastItemClicked, _lastColumnClicked );	//No need to do this.  QT already does it.
		}
		else
		{
			switch ( _button )
			{
			case Qt::LeftButton:
				toggleExpandedContact( _currentItemClicked );
				itemLeftClicked( _currentItemClicked, _lastColumnClicked );
				break;

			case Qt::RightButton:
				itemRightClicked( _currentItemClicked, _lastColumnClicked );
				break;

			case Qt::NoButton:
				Q_ASSERT(false);		//Should not happen;
			}
		}

		_lastItemClicked    = NULL;
		_currentItemClicked = NULL;
		_lastColumnClicked  = 0;
	}
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::waitForClickTimer( QTreeWidgetItem* item, int column ) 
{
	if ((_lastItemClicked == item) && (_waitForDoubleClick)) 
	{
		emitClickedSignal( true );
	}
	else
	{
		_lastItemClicked   = item;
		_lastColumnClicked = column;

		emitClickedSignal( false );
	}
}

//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::handleDoubleClickTimer() 
{
	emitClickedSignal( false );
}

//-----------------------------------------------------------------------------
//End click handling/filtering methods
//-----------------------------------------------------------------------------

void VoxContactListTreeWidget::closeContactInfo() 
{
	QMutexLocker locker(_mutex);

	bool bDone = false;

	//VOXOX - JRT - 2009.07.20 - Due to 'search' group, the same contact could be in more than one group, so we need to find and close all.
	while ( ! bDone )
	{
		QTreeWidgetItem* openItem = getOpenItem();

		if ( openItem )
		{
			if (!openItem->parent())
			{
				return;	//It's a group
			}

			closePersistentEditor( openItem );
			QtContact* qtContact = getItemData( openItem );

			if ( qtContact )
			{
				qtContact->setIsOpen( false );
//				openItem->setSizeHint(0, QSize(-1, qtContact->getHeight()) );	//VOXOX - JRT - 2009.10.15 - TODO: Is this needed, since we moved logic to QtContact?
			}
		}
		else
		{
			bDone = true;
		}
	}

	resetOpenKeys();

	viewport()->update();
	doItemsLayout();
}

void VoxContactListTreeWidget::openContactInfo( QTreeWidgetItem * item ) 
{
	QMutexLocker locker(_mutex);

	closeContactInfo();	//Does check for _previous.

	if (item)
	{
		if (item->parent()) 
		{
			QtContact* qtContact = getItemData( item );

			if ( qtContact )
			{
				if ( qtContact->shouldExpand() )
				{
					_openContactKey = qtContact->getKey().toStdString();	//VOXOX - JRT - 2009.08.10 
					_openContactId  = qtContact->getId().toStdString();		//VOXOX - JRT - 2009.10.15 

					qtContact->setIsOpen( true );

					//VOXOX - JRT - 2009.10.15 - Moved the view delegate logic to QtContact.
//					item->setSizeHint(0, QSize(-1, qtContact->getHeight() ));
					
					// VOXOX -- 2009.10.18: attempt to fix selected contact background paint issue on Mac
					openPersistentEditor(item);
					viewport()->update();
					doItemsLayout();
					scrollToItem (item);
				}
			}
			else
			{
				LOG_INFO( "VoxContactListTreeWidget::openContactInfo() - qtContact == NULL" );
			}
		}
	}
}

void VoxContactListTreeWidget::reExpandItems() 
{
	if ( _openContactKey != "" )
	{
		bool		 found = false;
		QMutexLocker locker(_mutex);

		QTreeWidgetItem* item = getItemByContactKey( _openContactKey, true );

		//We most likely toggled Group By, so find by contactId
		if ( item == NULL )
		{
			item = getItemByContactKey( _openContactId, false );
		}

		if (item)
		{
			QtContact* qtContact = getItemData( item );

			if ( qtContact )
			{
				qtContact->setIsOpen( true );
				found = true;
			}
		}
		
		//If not found, then clear openKey, since it no longer exists in this list.
		if ( !found )
		{
			resetOpenKeys();
		}
	}
}

//-----------------------------------------------------------------------------
	
void VoxContactListTreeWidget::positionItemAtTop( QtContact* topQtContact )
{
	if ( topQtContact )
	{
		bool		 found = false;
		QMutexLocker locker(_mutex);

		QTreeWidgetItem* item = getItemByContactKey( topQtContact->getKey().toStdString(), true );

		//Group option may have changed, so try contactId.
		if ( item == NULL )
		{
			item = getItemByContactKey( topQtContact->getId().toStdString(), false );
		}

		if (item)
		{
			QModelIndex index = indexFromItem( item  );
			scrollTo( index, QAbstractItemView::PositionAtTop );
		}
	}
}

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//Paint/Draw methods
////VOXOX - JRT - 2009.05.14 TODO: Ideally, all drawing methods would be done here.
//  These methods work, but we still need to 'connect' with the QtContactWidget (Expanded contact view)
//-----------------------------------------------------------------------------

//void VoxContactListTreeWidget::drawRow( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
//{
//	QtContact* qtContact = getItemDataFromIndex( index );
//	
//	if (!index.parent().isValid()) 
//	{
//		drawGroup(painter, option, index);
//	} 
//	else 
//	{
//		if ( qtContact )
//		{
//			if (qtContact->isOpen() )
//			{
//				drawExpandedContact( painter, option, qtContact );
//			}
//			else
//			{
//				drawContact(painter, option, qtContact );
//			}
//		}
//	}
//}

//-----------------------------------------------------------------------------

//void VoxContactListTreeWidget::drawGroup( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const 
//{
//	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();
//	painter->setPen(contactListStyle->getGroupFontColor());
//	painter->drawPixmap(option.rect.left(),option.rect.top(),getGroupBackGround(option.rect,option));
//
//	QFont font = option.font;
//	painter->setFont(font);
//
//	//VOXOX - JRT - 2009.05.02 - This works but is not needed right now.
////	QtContact* pItemData = index.model()->data( index, Qt::UserRole ).value<QtContact*>();
//	//End VoxOx
//
//	//QtContactPixmap * spx = QtContactPixmap::getInstance();
//	QPixmap px;
//	if (option.state & QStyle::State_Open) 
//	{
//		px = contactListStyle->getGroupPixmapOpen();
//	} 
//	else 
//	{
//		px = contactListStyle->getGroupPixmapClose();
//	}
//
//	int x = option.rect.left();
//	QRect r = option.rect;
//	painter->drawPixmap(x, r.top() + 3, px);
//	x += px.width() + 3;
//	r.setLeft(x);
//	int y = ((r.bottom()-r.top())-QFontMetrics(font).height())/2;
//	r.setTop(y + r.top());
//	r.setLeft(r.left() + 10);
//
//	//VOXOX - JRT - 2009.05.04 -TODO: this is all business logic that should not be here.
//	//	We should have a simple getGroupText() method that provides this.
//	QString groupName;
//	std::string groupId;
//	QString groupNameTmp;
//	QString groupContactsOnline;
//
//	if (index.data().toString() == QtContactList::DEFAULT_GROUP_NAME) 
//	{
//		//VOXOX CHANGE CJC< WE DONT WANT TO SHOW CONTACT LIST WHEN GROUPS ARE HIDDEN. WE CAN'T REMOVE GROUP, SO JUST REMOVE NAME
//		groupName	 = tr("Contacts");
//		groupNameTmp = groupName;
//	} 
//	else 
//	{
//		const QtContact* qtContact = getItemDataFromIndex( index );
//
//		groupName			= index.data().toString();
//		groupId				= groupName.toStdString();
//		groupNameTmp		= qtContact->getGroupName();
//		groupContactsOnline = qtContact->getContactGroupContactsOnline();
//	}
//
//	groupName = groupNameTmp + groupContactsOnline;
//	//End business logic.
//
//	QString str = groupName;
//	painter->drawText(r, Qt::AlignLeft, str, 0);
//}
//
////-----------------------------------------------------------------------------
//
//QPixmap VoxContactListTreeWidget::getGroupBackGround(const QRect & rect, const QStyleOptionViewItem & option) const 
//{
//	if (_groupBackground.isNull() || _groupBackground.width() != rect.width()) 
//	{
//		//This colors cant be hardcoded.
//	/*	QLinearGradient lg(QPointF(1.0, 0.0), QPointF(1.0, GROUP_WIDGET_FRAME_HEIGHT));
//		lg.setColorAt(0.8, QColor(212, 208, 200));
//		lg.setColorAt(0.0, QColor(255, 255, 255));*/
//
//		QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();
//
//		_groupBackground = QPixmap(rect.width(), _GroupWidgetFrameHeight );
//		QPainter painter(&_groupBackground);
//		//TODO CHANGE CODE TO GRADIENT IF NEEDED
//		painter.fillRect(_groupBackground.rect(),QBrush(contactListStyle->getGroupBackgroundColorTop()));
//
//		//Borders
//		QPen pen = painter.pen();
//		painter.setPen (contactListStyle->getGroupBorderColor());	
//		painter.drawLine(QPoint(0,0),QPoint(rect.width(),0));
//		painter.drawLine(QPoint(0,_GroupWidgetFrameHeight-1),QPoint(rect.width(),_GroupWidgetFrameHeight-1));
//		painter.setPen (pen);	
//		painter.end();
//	}
//	return _groupBackground;
//}
//
////-----------------------------------------------------------------------------
//
//void VoxContactListTreeWidget::drawContact(QPainter * painter, const QStyleOptionViewItem & option, QtContact* qtContact) const
//{
//	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();
//
//	if ((option.state & QStyle::State_Selected) == QStyle::State_Selected) 
//	{
//		painter->fillRect(option.rect, contactListStyle->getContactSelectedBackgroundColorTop());
//	}
//	else
//	{
//		painter->fillRect(option.rect, contactListStyle->getContactNonSelectedBackgroundColor());
//	}
//
//	paintContactForeground(painter, option, qtContact );
//}
//
////-----------------------------------------------------------------------------
//
////VOXOX CHANGE - Add support for ContactList style and change paint structure.
//void VoxContactListTreeWidget::paintContactForeground(QPainter * painter, const QStyleOptionViewItem & option, QtContact* qtContact ) const
//{
//	QtContactPixmap * spx = QtContactPixmap::getInstance();
//	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();
//
//	if ((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
//		painter->setPen(contactListStyle->getContactUserNameSelectedFontColor());
//	} else {
//		painter->setPen(contactListStyle->getContactUserNameNonSelectedFontColor());
//	}
//
//	// Draw the status pixmap
//	QtContactPixmap::ContactPixmap status = qtContact->getStatus();
//
//	QPixmap px = spx->getPixmap(status);
//
//	QRect painterRect = option.rect;
//
//	//painting the status icon
//	int x = painterRect.right() - px.width() - 2;
//	_posXNetworkStatus = x;
//
//	int centeredPx_y = ((painterRect.bottom() - painterRect.top()) - px.size().height()) / 2;
//	
//	painter->drawPixmap(x, painterRect.top() + centeredPx_y, px);	
//
//	
//	std::string foregroundPixmapData = qtContact->getContact()->getIcon().getData();	//VOXOX - JRT - 2009.05.13
//
//	QPixmap avatar;
//	avatar.loadFromData ((uchar*) foregroundPixmapData.c_str(), foregroundPixmapData.size());
////	avatar = avatar.scaled ( AVATAR_HEIGHT, AVATAR_WIDTH, Qt::KeepAspectRatio, Qt::SmoothTransformation) ;
//	avatar = avatar.scaled ( _AvatarHeight, _AvatarWidth, Qt::KeepAspectRatio, Qt::SmoothTransformation) ;
//	
//	painter->drawPixmap( _ContactMargin, painterRect.top() + _AvatarMarginTop, avatar);		
//
//	painterRect.setLeft( _ContactMargin + _AvatarWidth + _AvatarTextMargin );
//
//	// Draw the text
//	painter->setFont(option.font);
//
//	// Center the text vertically
//	QRect textRect = painterRect;
//	int textY = painterRect.top() + _AvatarMarginTop + 3;
//	textRect.setTop(textY);
//
//	QString text = qtContact->getEscapedDisplayName();										//VOXOX - JRT - 2009.05.13
//		
//	text = verifyText(textRect,option.font,text);
//	painter->drawText(textRect, Qt::AlignLeft, text, 0);
//
//	if ((option.state & QStyle::State_Selected) == QStyle::State_Selected) 
//	{
//		painter->setPen(contactListStyle->getContactSocialStatusSelectedFontColor());
//	} 
//	else 
//	{
//		painter->setPen(contactListStyle->getContactSocialStatusNonSelectedFontColor());
//	}
//
//	int fontHeight = QFontMetrics(option.font).height()-3;
//	textY +=fontHeight;
//	textRect.setTop(textY);
//
//	text = qtContact->getEscapedStatusMessage();										//VOXOX - JRT - 2009.05.13
//	text = verifyText(textRect, option.font, text);
//	painter->drawText(textRect, Qt::AlignLeft, text, 0);
//}
//
////-----------------------------------------------------------------------------
//
//QString VoxContactListTreeWidget::verifyText(QRect & painterRect, QFont font, QString text) const
//{
//	int xText	  = painterRect.left();
//	int textWidth = QFontMetrics(font).width(text);	//gets new "x" position according width's text	
//	
//	bool fixedText = false;
//
//	while(xText + textWidth > _posXNetworkStatus)	//we need to check that we can paint on the QtContactWidget but not on the vertical line
//	{
//		text	  = text.remove(text.length()- 1,text.length());
//		textWidth = QFontMetrics(font).width(text);//gets new "x" position according width's text
//		fixedText = true;
//	}
//	
//	if(fixedText)
//	{
//		if(text.length() >= 3)
//		{			
//			text = text.remove(text.length()- 3,text.length() );
//			text += "...";
//		}		
//	}
//
//	return text;
//}
//
////-----------------------------------------------------------------------------
//
//void VoxContactListTreeWidget::drawExpandedContact(QPainter * painter, const QStyleOptionViewItem & option, QtContact* qtContact) const
//{
//}

//-----------------------------------------------------------------------------