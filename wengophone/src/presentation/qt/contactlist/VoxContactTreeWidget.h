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

#ifndef VOX_CONTACTLIST_TREE_H
#define VOX_CONTACTLIST_TREE_H

#include <QtGui/QTreeWidget>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QMutex>

class CContactList;
class QtContact;
class ContactProfile;
class QtContactTooltipManager;
class QEvent;

namespace Ui { class ContactList; }

//====================================================================
//VOXOX - JRT - 2009.10.15 - Moved logic to QtContact
//class QtContactTreeViewDelegate : public QAbstractItemDelegate 
//{
//	Q_OBJECT
//public:
//
//	//QtTreeViewDelegate(CWengoPhone & cWengoPhone,
//	//	QtContactManager * qtContactManager, QObject * parent = 0);
//
//	//QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
//
//	//void setEditorData(QWidget * editor, const QModelIndex & index) const;
//
//	//void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;
//
//	//void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
//
//	//void setParent(QWidget * parent);
//
//	QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const ;
//	void  paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
//
//private:
//	QtContact* getItemData( const QModelIndex & index ) const;	//VOXOX - JRT - 2009.08.10 
//
//	//void drawGroup(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
//
//	//QPixmap getGroupBackGround(const QRect & rect, const QStyleOptionViewItem & option) const;
//
//	//const QWidget * _parent;
//
//	//QPixmap _menuIcon;
//
//	//mutable QPixmap _groupBackground;
//
//	//QtContactManager * _qtContactManager;
//
//	//CWengoPhone & _cWengoPhone;
//};

//=============================================================================

//=============================================================================

class VoxContactListTreeWidget : public QTreeWidget
{
	Q_OBJECT
public:
	static const int _GroupWidgetFrameHeight;
	static const int _AvatarHeight;
	static const int _AvatarWidth;
	static const int _ContactMargin;
	static const int _AvatarTextMargin;
	static const int _AvatarMarginTop;

	static const QString WENGO_MIME_TYPE;

	explicit VoxContactListTreeWidget(QWidget *parent = 0);
	virtual ~VoxContactListTreeWidget();

	void setCContactList( CContactList* cContactList );	//VOXOX - JRT - 2009.05.13 - Cannot see how to do this in default ctor.

	QtContact* getItemData		   (       QTreeWidgetItem* item  ) const;
	QtContact* getItemDataFromIndex( const QModelIndex&     index ) const;

	bool toggleExpanded		  ( QTreeWidgetItem* item );
	void toggleExpandedContact( QTreeWidgetItem* item );
	void openContactInfo	  ( QTreeWidgetItem* item );
	void closeContactInfo();
	void reExpandItems();			//VOXOX - JRT - 2009.10.15 
	void positionItemAtTop( QtContact* topQtContact  );	//VOXOX - JRT - 2009.10.15 

	void clear();
	void removeItem( QTreeWidgetItem* item );

	void lock()										{ _mutex->lock();	}
	void unlock()									{ _mutex->unlock();	}

	QTreeWidgetItem*  getItemByContactKey( const std::string contactKey, bool isKey );

Q_SIGNALS:
	void mergeContacts( QString dstContact, QString srcContact );
	void orderGroups  ( QString dstContact, QString srcContact );
    
	void itemLeftClicked ( QTreeWidgetItem* item, int column );
	void itemRightClicked( QTreeWidgetItem* item, int column );

private Q_SLOTS:
	void handleDrop();
	void myClickHandler( QTreeWidgetItem* item, int column );

	void handleDoubleClickTimer() ;
	void treeViewSelectionChanged();

private:
	QtContact* getQtContact( const QString& itemKey ) const;

	bool eventFilter(QObject * obj, QEvent * event);

	void mouseToolTipEvent ( QHelpEvent * event);
	void handleToolTipEvent( QPoint pos ) ;

	//Overrides
	void mousePressEvent  ( QMouseEvent * event );
	void mouseMoveEvent	  ( QMouseEvent * event );
	void mouseReleaseEvent( QMouseEvent * event );

	void dragEnterEvent( QDragEnterEvent * event );
	void dragMoveEvent ( QDragMoveEvent *  event );
	void dropEvent	   ( QDropEvent *      event );

	//Click interpretation methods.
	void waitForClickTimer( QTreeWidgetItem* item, int column );
	void emitClickedSignal( bool bIsDoubleClick );
	
	void closeCurrentToolTip();
	
	QtContactTooltipManager* getTooltipMgr();

	void moveContactToGroup( const std::string& sourceContactId, const std::string& sourceGroupId, const std::string& destGroupId );		//VOXOX - JRT - 2009.08.08 

	//Paint methods	//VOXOX - JRT - 2009.05.14 TODO complete this.
//	void	drawRow( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;

//	void	drawContact			  (QPainter * painter, const QStyleOptionViewItem & option, QtContact* qtContact) const;
//	void	paintContactForeground(QPainter * painter, const QStyleOptionViewItem & option, QtContact* qtContact) const;
//
//	void	drawExpandedContact	  (QPainter * painter, const QStyleOptionViewItem & option, QtContact* qtContact) const;
//
//	void	drawGroup			  (QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
//	QPixmap getGroupBackGround    (const QRect & rect, const QStyleOptionViewItem & option) const;
//
//	QString verifyText			  (QRect & painterRect, QFont font, QString text) const;
	//End paint methods

	//We need accessors because Tree and its widgets can be redrawn at any time.
	QTreeWidgetItem*  getOpenItem();	//VOXOX - JRT - 2009.07.20 
	void			  resetOpenKeys();	//VOXOX - JRT - 2009.10.15 

	//Click handling
	Qt::MouseButton	 _button;
	int				 _timerInterval;
	bool			 _waitForDoubleClick;
	int				 _lastColumnClicked;
	QTreeWidgetItem* _lastItemClicked;
	QTreeWidgetItem* _currentItemClicked;
	std::string		 _openContactKey;
	std::string		 _openContactId;

	//Drag and Drop
	QPoint			 _dstart;
	QString			 _selectedContactId;
	bool			 _selectedIsGroup;
	QString			 _dropSourceContactId;
	QString			 _dragItemId;
	QString			_dropItemId;

	//Painting
//	mutable QPixmap _groupBackground;
//	mutable int		_posXNetworkStatus;

	//Business objects
	CContactList*	 _cContactList;
		
	mutable QMutex* _mutex;
};

#endif	//VOX_CONTACTLIST_TREE_H

