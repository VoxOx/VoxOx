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

#ifndef OWQTCONTACTTREEMOUSEFILTER_H
#define OWQTCONTACTTREEMOUSEFILTER_H

#include <QtCore/QObject>
#include <QtCore/QPoint>

class CContactList;
class ContactProfile;	//VOXOX - JRT - 2009.05.11 
class QTreeWidgetItem;
class QMouseEvent;
class QDragEnterEvent;
class QDropEvent;
class QTreeWidget;
class QDragMoveEvent;
class QHelpEvent;

/**
 * Mouse events manager for QTreeWidget
 *
 * @author Mr K
 */
class QtContactTreeMouseFilter : public QObject {
	Q_OBJECT
public :
	static const QString WENGO_MIME_TYPE;

	QtContactTreeMouseFilter(CContactList & cContactList, QObject * parent, QTreeWidget * treeWidget);

Q_SIGNALS:

	void mouseClicked(Qt::MouseButton button);

	void mergeContacts( QString dstContact, QString srcContact );
	void orderGroups  ( QString dstContact, QString srcContact );

private Q_SLOTS:
//	void handleDrop();			//VOXOX - JRT - 2009.08.09 - Not used

private:
	bool eventFilter(QObject * obj, QEvent * event);

	void mouseToolTipEvent(QHelpEvent * event);	//VOXOX - CJC - 2009.05.09 Add support for tooltip event

	void mousePressEvent  ( QMouseEvent * event );
	void mouseMoveEvent	  ( QMouseEvent * event );
	void mouseReleaseEvent( QMouseEvent * event );

	void dragEnterEvent( QDragEnterEvent * event );
	void dragMoveEvent ( QDragMoveEvent *  event );
	void dropEvent	   ( QDropEvent *      event );
	

//	void moveContact( const ContactProfile& sourceProfile, const QString& destinationContactId );		//VOXOX - JRT - 2009.08.09 - Not used

	QTreeWidget*	 _tree;
	QPoint			 _dstart;
	QString			 _selectedContactId;
	bool			 _selectedIsGroup;		//VOXOX - JRT - 2009.05.11 
	QString			 _dropSourceContactId;
	QTreeWidgetItem* _dragItem;
	QTreeWidgetItem* _dropItem;

	CContactList&	 _cContactList;
};

#endif	//OWQTCONTACTTREEMOUSEFILTER_H
