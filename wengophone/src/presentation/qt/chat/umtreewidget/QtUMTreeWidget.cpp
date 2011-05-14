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

#include "stdafx.h"	
#include "QtUMTreeWidget.h"
#include <qtutil/SafeConnect.h>
#include "QtUMItem.h"
#include "QtUMGroup.h"
#include <util/Logger.h>

#include <QtGui/QHeaderView>	//VOXOX - JRT - 2009.09.09
#include <QtGui/QMouseEvent>	//VOXOX - JRT - 2009.09.09
#include <QtGui/QToolTip>		//VOXOX - JRT - 2009.09.11

//=============================================================================

QSize QtUMItemTreeViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const 
{
	QSize orig = option.rect.size();
	QtUMItem* umItem = getItemData( index );

	if (umItem) 
	{
		return QSize(orig.width(), umItem->getHeight() );
	} 

	return orig;
}

//-----------------------------------------------------------------------------

QtUMItem* QtUMItemTreeViewDelegate::getItemData( const QModelIndex & index ) const
{
	QtUMItem* result = index.model()->data( index, Qt::UserRole ).value<QtUMItem*>();
	
	return result;
}

QtUMGroup* QtUMItemTreeViewDelegate::getGroupData( const QModelIndex & index ) const
{
	QtUMGroup* result = index.model()->data( index, Qt::UserRole ).value<QtUMGroup*>();
	
	return result;
}

//=============================================================================


Q_DECLARE_METATYPE(QtUMItem*)
Q_DECLARE_METATYPE(QtUMGroup*)//VOXOX CHANGE by Rolando - 2009.09.25 


//-----------------------------------------------------------------------------

QtUMTreeWidget::QtUMTreeWidget( QWidget* parent ) : QTreeWidget(parent)
{

	_mutex = new QMutex(QMutex::Recursive);
	setMouseTracking(true);//VOXOX CHANGE by Rolando - 2009.09.08 
	
	setEditTriggers(QAbstractItemView::NoEditTriggers);//VOXOX CHANGE by Rolando - 2009.09.08 
	setProperty("showDropIndicator", QVariant(true));//VOXOX CHANGE by Rolando - 2009.09.08 
	setAlternatingRowColors(false);//VOXOX CHANGE by Rolando - 2009.09.08 
	setSelectionMode(QAbstractItemView::MultiSelection);//VOXOX CHANGE by Rolando - 2009.09.08 
	setSelectionBehavior(QAbstractItemView::SelectRows);//VOXOX CHANGE by Rolando - 2009.09.08  
	
	setIndentation(0);//VOXOX CHANGE by Rolando - 2009.09.08 
	setRootIsDecorated(false);//VOXOX CHANGE by Rolando - 2009.09.08 

	setUniformRowHeights(false);//VOXOX CHANGE by Rolando - 2009.09.08 
	header()->hide();//VOXOX CHANGE by Rolando - 2009.09.08 
}

//-----------------------------------------------------------------------------

QtUMTreeWidget::~QtUMTreeWidget()
{
	delete _mutex;
}

//VOXOX CHANGE by Rolando - 2009.08.19 
void QtUMTreeWidget::mousePressEvent(QMouseEvent * event) 
{
	QTreeWidgetItem * item = itemAt(event->pos());
	
	QTreeWidgetItem * parent = NULL;

	if ( item )
	{
		switch ( event->button() )
		{
		case Qt::LeftButton:{
					
			QtUMItem* umItem = getItemData( item );
			QPoint mousePosition = event->pos();//VOXOX CHANGE by Rolando - 2009.08.27 
			if (umItem) {

				umItem->setNetworkIconIsUnderMouse(false);//VOXOX CHANGE by Rolando - 2009.10.26
				
				if(!umItem->isAGroupItem()){//VOXOX CHANGE by Rolando - 2009.09.25 
					if(umItem->mouseHoveringCloseButton(mousePosition)){//VOXOX CHANGE by Rolando - 2009.08.27 - if mouse position is hover close button and left button was pressed
						closeCurrentItem(umItem);//VOXOX CHANGE by Rolando - 2009.08.27
						setCursor(QCursor(Qt::ArrowCursor));//VOXOX CHANGE by Rolando - 2009.08.27 
						setToolTip(QString(""));//VOXOX CHANGE by Rolando - 2009.08.27 
					}
					else{
						if(umItem->mouseHoveringNetworkButton(mousePosition)){//VOXOX CHANGE by Rolando - 2009.10.21  - if mouse position is hover network icon and left button was pressed
							umItem->setNetworkIconIsUnderMouse(true);//VOXOX CHANGE by Rolando - 2009.10.26
							umItem->networkIconClicked(mapToGlobal(event->pos()));//VOXOX CHANGE by Rolando - 2009.10.26  

							setCursor(QCursor(Qt::ArrowCursor));//VOXOX CHANGE by Rolando - 2009.10.21  
							setToolTip(QString(""));//VOXOX CHANGE by Rolando - 2009.10.21

							viewport()->update();//VOXOX CHANGE by Rolando - 2009.10.26 
						}
						else{
							itemLeftClicked( item, 0 );//VOXOX CHANGE by Rolando - 2009.09.08 
						}
					}
				}
				else{
					parent = item->parent();//VOXOX CHANGE by Rolando - 2009.09.28 
					groupLeftClicked( parent, 0 );//VOXOX CHANGE by Rolando - 2009.09.28 
				}
			}
			else{
				QtUMGroup * umGroup = getGroupData( item );
				if(umGroup){					
					if(umGroup->mouseHoveringCloseButton(event->pos())){//VOXOX CHANGE by Rolando - 2009.08.27 - if mouse position is hover close button and left button was pressed
						closeCurrentGroup(umGroup);//VOXOX CHANGE by Rolando - 2009.08.27
						setCursor(QCursor(Qt::ArrowCursor));//VOXOX CHANGE by Rolando - 2009.08.27 
						setToolTip(QString(""));//VOXOX CHANGE by Rolando - 2009.08.27 
					}
					else{		
						if(umGroup->mouseHoveringArrowButton(event->pos())){//VOXOX CHANGE by Rolando - 2009.09.29  - if mouse position is hover arrow button and left button was pressed
							toggleExpandedItem( item );	//VOXOX CHANGE by Rolando - 2009.09.28
							setCursor(QCursor(Qt::ArrowCursor));//VOXOX CHANGE by Rolando - 2009.09.29  
							setToolTip(QString(""));//VOXOX CHANGE by Rolando - 2009.09.29 
						}	

						groupLeftClicked( item, 0 );//VOXOX CHANGE by Rolando - 2009.09.08 
						
					}
				}
				
			}
							}
			break;

		case Qt::RightButton:
			itemRightClicked( item, 0 );
			break;

		case Qt::NoButton:
			Q_ASSERT(false);		//Should not happen;
		}		
		
	}	

}

//VOXOX CHANGE by Rolando - 2009.08.26 
void QtUMTreeWidget::mouseMoveEvent(QMouseEvent * event) {
	if (!(event->buttons() & Qt::LeftButton)) {//checks if none of buttons was pressed while the mouse moves
		QTreeWidgetItem * item = itemAt(event->pos());
		QtUMItem* umItem = getItemData( item );
		QPoint mousePosition = event->pos();
		if (umItem) {

			umItem->setNetworkIconIsUnderMouse(false);//VOXOX CHANGE by Rolando - 2009.10.26

			if(!umItem->isAGroupItem()){//VOXOX CHANGE by Rolando - 2009.09.25			

				if(umItem->mouseHoveringCloseButton(mousePosition)){//Fix to able the hovering close button in UMItem
					setCursor(QCursor(Qt::PointingHandCursor));	
					setToolTip(QString("Close Chat"));
				}
				else{
					if(umItem->mouseHoveringNetworkButton(mousePosition)){//VOXOX CHANGE by Rolando - 2009.10.21 - Fix to able the hovering network icon in UMItem
						umItem->setNetworkIconIsUnderMouse(true);//VOXOX CHANGE by Rolando - 2009.10.26
						setCursor(QCursor(Qt::PointingHandCursor));	//VOXOX CHANGE by Rolando - 2009.10.21 
						setToolTip(QString("Network Status"));//VOXOX CHANGE by Rolando - 2009.10.21

						viewport()->update();//VOXOX CHANGE by Rolando - 2009.10.26 
					}
					else{
						setCursor(QCursor(Qt::ArrowCursor));//VOXOX CHANGE by Rolando - 2009.10.21 
						setToolTip("");//VOXOX CHANGE by Rolando - 2009.10.21 
						QToolTip::hideText();//VOXOX CHANGE by Rolando - 2009.10.21 
					} 
				}
			}
		}
		else{
			QtUMGroup* umGroup = getGroupData( item );
			if (umGroup) {
				if(umGroup->mouseHoveringCloseButton(mousePosition)){//Fix to able the hovering close button in QtUMGroup
					setCursor(QCursor(Qt::PointingHandCursor));	
					setToolTip(QString("Close Group Chat"));//VOXOX CHANGE by Rolando - 2009.09.28 
				}
				else{
					if(umGroup->mouseHoveringArrowButton(mousePosition)){//Fix to able the hovering close button in QtUMGroup
						setCursor(QCursor(Qt::PointingHandCursor));	
						setToolTip(QString("Expand/Collapse Group Chat"));//VOXOX CHANGE by Rolando - 2009.09.28 
					}
					else{
						setCursor(QCursor(Qt::ArrowCursor));
						setToolTip("");//VOXOX CHANGE by Rolando - 2009.09.28  
						QToolTip::hideText();//VOXOX CHANGE by Rolando - 2009.09.28 
					}
				}

			}
			else{//VOXOX CHANGE by Rolando - 2009.10.27 
				setCursor(QCursor(Qt::ArrowCursor));//VOXOX CHANGE by Rolando - 2009.10.27 
				setToolTip("");//VOXOX CHANGE by Rolando - 2009.10.27  
				QToolTip::hideText();//VOXOX CHANGE by Rolando - 2009.10.27

				QTreeWidgetItemIterator it(this);//VOXOX CHANGE by Rolando - 2009.10.27 
				while (*it) {//VOXOX CHANGE by Rolando - 2009.10.27 
					QtUMItem* umItem = getItemData( *it );//VOXOX CHANGE by Rolando - 2009.10.27 
					if(umItem){//VOXOX CHANGE by Rolando - 2009.10.27 
						umItem->setNetworkIconIsUnderMouse(false);//VOXOX CHANGE by Rolando - 2009.10.27 
					}
					 ++it;//VOXOX CHANGE by Rolando - 2009.10.27 
				}
			}
		}
	}
}

void QtUMTreeWidget::keyPressEvent(QKeyEvent * event) {
	QTreeWidgetItem * item = currentItem();
	switch (event->key()) {
	case Qt::Key_Left:
		closeItemInfo(item);
		break;
	case Qt::Key_Right:
		openItemInfo(item);
		break;
	default:
		QTreeWidget::keyPressEvent(event);//VOXOX CHANGE by Rolando - 2009.09.07 
	}

}

void QtUMTreeWidget::clear()
{
	QTreeWidget::clear();
}

QtUMItem* QtUMTreeWidget::getItemData( QTreeWidgetItem* item ) const
{
	QtUMItem* pRet = NULL;

	if ( item )
	{
		pRet = item->data( 0, Qt::UserRole ).value<QtUMItem*>();
	}

	return pRet;
}

QtUMGroup* QtUMTreeWidget::getGroupData( QTreeWidgetItem* item ) const
{
	QtUMGroup* pRet = NULL;

	if ( item )
	{
		pRet = item->data( 0, Qt::UserRole ).value<QtUMGroup*>();
	}

	return pRet;
}


QtUMItem* QtUMTreeWidget::getItemDataFromIndex( const QModelIndex& index ) const
{
	return getItemData( itemFromIndex( index ) );
}

QtUMGroup* QtUMTreeWidget::getGroupDataFromIndex( const QModelIndex& index ) const
{
	return getGroupData( itemFromIndex( index ) );
}


//-----------------------------------------------------------------------------
//VOXOX CHANGE by Rolando - 2009.08.19 
void QtUMTreeWidget::toggleExpandedItem( QTreeWidgetItem* item )
{
	if (getGroupData(item))			//This is a group //VOXOX CHANGE by Rolando - 2009.09.25 
	{
		toggleExpanded( item );
		viewport()->update();
	}

	
}

//-----------------------------------------------------------------------------

bool QtUMTreeWidget::toggleExpanded( QTreeWidgetItem* item )
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


//VOXOX CHANGE by Rolando - 2009.09.08 
void QtUMTreeWidget::closeItemInfo(QTreeWidgetItem * item) 
{	

	if ( item ) 
	{
		if (getGroupData(item))			//This is a group
		{
			//it is a group
			toggleExpanded( item );
		}
			
	}

}

//VOXOX CHANGE by Rolando - 2009.08.20 
void QtUMTreeWidget::openItemInfo( QTreeWidgetItem * item ) 
{
	if ( item )
	{
		QtUMItem* umItem = getItemData( item );//VOXOX CHANGE by Rolando - 2009.08.25 

		if (!umItem) 		
		{
			if ( !isItemExpanded(item) ) 
			{
				expandItem(item);					
			}
	
		}
			
	}
}