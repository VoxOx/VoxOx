
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
* CLASS QtUMGroup 
* @author Rolando 
* @date 2009.09.25
*/


#ifndef OWQTUMGROUP_H
#define OWQTUMGROUP_H

#include <QtCore/QMutex>				//VOXOX - JRT - 2009.09.09
#include <QtCore/QStringList>				//VOXOX CHANGE by Rolando - 2009.09.29 
#include <QtGui/QStyleOptionViewItem>	//VOXOX - JRT - 2009.09.09
class QTreeWidgetItem;					//VOXOX - JRT - 2009.09.09


class QtUMGroup : QObject {
	Q_OBJECT
public:

	enum SizeHint { GroupSize = 21 };

	//VOXOX CHANGE by Rolando - 2009.08.31 
	QtUMGroup(const QString & groupId, const QString & groupName,QObject * parent);

	virtual void	paint(QPainter * painter, const QStyleOptionViewItem & option);

	QString			getId() const;

	
	int				getHeight() const;
	Qt::MouseButton getButton() const;
	
	void			toggleOpenStatus()								{ _openStatus = !_openStatus;	}
	void			setOpenStatus(bool value)						{ _openStatus = value;			}
	bool			isOpen() const									{ return _openStatus; }

	bool			isSelected()									{ return _isSelected; } //VOXOX CHANGE by Rolando - 2009.09.25  
	void			setIsSelected( bool value)						{ _isSelected = value; }//VOXOX CHANGE by Rolando - 2009.09.25 

	void			setIsBlinking( bool value)						{ _isBlinking = value; }//VOXOX CHANGE by Rolando - 2009.09.25 
	bool			isBlinking()									{ return _isBlinking; } //VOXOX CHANGE by Rolando - 2009.09.25  

	void			setTreeWidgetItem( QTreeWidgetItem* item )		{ _item		  = item;			}
	QTreeWidgetItem* getTreeWidgetItem()							{ return _item;	}

	bool mouseHoveringCloseButton(QPoint mousePosition);

	bool mouseHoveringArrowButton(QPoint mousePosition);//VOXOX CHANGE by Rolando - 2009.09.29 

	QString getGroupName(){ return _groupName;}
	void setGroupName(QString groupName){ _groupName = groupName;}

	void mousePressEvent(QMouseEvent * event);//VOXOX CHANGE by Rolando - 2009.09.25

	void addChildId(QString id);//VOXOX CHANGE by Rolando - 2009.09.29

	void removeChildId(QString id);//VOXOX CHANGE by Rolando - 2009.09.29 

	bool existsChildId(QString id);//VOXOX CHANGE by Rolando - 2009.09.29

	bool hasChildIds() const;//VOXOX CHANGE by Rolando - 2009.09.29 

private:
	void initVars();
	
	void paintForeground(QPainter * painter, const QStyleOptionViewItem & option);
	QString verifyText(QRect & painterRect, QFont font, QString text);

public Q_SLOTS:

Q_SIGNALS:

	void clicked(QtUMGroup * umItem, int prt);

private:

	
	QPixmap getGroupBackGround(const QRect & rect, const QStyleOptionViewItem & option, bool isGroupSelected, bool isGroupBlinking) const;//VOXOX CHANGE by Rolando - 2009.09.25 
	bool _mouseOn;
	bool _openStatus;

	Qt::MouseButton _mouseButton;

	QString		_id;

	QMutex			_mutex;

	int				_posXCloseButton;//VOXOX CHANGE by Rolando - 2009.08.25
	int				_posYCloseButton;//VOXOX CHANGE by Rolando - 2009.08.27
	int				_widthCloseButton;//VOXOX CHANGE by Rolando - 2009.08.27
	int				_heightCloseButton;//VOXOX CHANGE by Rolando - 2009.08.27 

	int				_posXArrowButton;//VOXOX CHANGE by Rolando - 2009.09.29 
	int				_posYArrowButton;//VOXOX CHANGE by Rolando - 2009.09.29 
	int				_widthArrowButton;//VOXOX CHANGE by Rolando - 2009.09.29 
	int				_heightArrowButton;//VOXOX CHANGE by Rolando - 2009.09.29 
	
	QTreeWidgetItem* _item;	

	bool _isBlinking;//VOXOX CHANGE by Rolando - 2009.08.28 
	bool _blinkNow;//VOXOX CHANGE by Rolando - 2009.08.28 
	QColor _backgroundColor;//VOXOX CHANGE by Rolando - 2009.08.28
	QColor _fontColor;//VOXOX CHANGE by Rolando - 2009.09.07 	

	QString _groupName;//VOXOX CHANGE by Rolando - 2009.09.25 

	bool _isOpen;
	bool _isSelected;

	mutable QPixmap _groupBackground;//VOXOX CHANGE by Rolando - 2009.09.25\


	QStringList _childListIds;//VOXOX CHANGE by Rolando - 2009.09.29 
	
};

#endif	//OWQTUMGROUP_H
