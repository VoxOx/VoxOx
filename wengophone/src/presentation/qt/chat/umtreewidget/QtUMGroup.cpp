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



#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtUMGroup.h"
#include <presentation/qt/chat/umtreewidget/QtUMItemListStyle.h>


#include <util/Logger.h>

#include <QtGui/QtGui>

static int GROUP_WIDGET_FRAME_HEIGHT = 21;//VOXOX CHANGE by Rolando - 2009.09.25 

QtUMGroup::QtUMGroup(const QString & groupId, const QString & groupName, QObject * parent)
	:  QObject(parent)
{
	initVars();

	_id = groupId;	
	_groupName = groupName;//VOXOX CHANGE by Rolando - 2009.08.21 	
		
}

void QtUMGroup::initVars()
{
	_item		 = NULL;
	_mouseOn	 = false;
	_openStatus  = false;
	_mouseButton = Qt::NoButton;

	_posXCloseButton = 0;//VOXOX CHANGE by Rolando - 2009.08.27 
	_posYCloseButton = 0;//VOXOX CHANGE by Rolando - 2009.08.27
	_widthCloseButton = 0;//VOXOX CHANGE by Rolando - 2009.08.27
	_heightCloseButton = 0;//VOXOX CHANGE by Rolando - 2009.08.27
	
	//VOXOX CHANGE by Rolando - 2009.08.28 
	_isBlinking = false;
	_blinkNow = false;

}

void QtUMGroup::paint(QPainter * painter, const QStyleOptionViewItem & option) {

	if(_isBlinking){
		_blinkNow = !_blinkNow;
	}

	QtUMItemListStyle * itemListStyle = QtUMItemListStyle::getInstance();

	if (_isSelected) {	

		_backgroundColor = itemListStyle->getItemSelectedBackgroundColorTop();//VOXOX CHANGE by Rolando - 2009.08.28 
		_fontColor = itemListStyle->getItemUserNameSelectedFontColor();
		
		painter->fillRect(option.rect, _backgroundColor);//VOXOX CHANGE by Rolando - 2009.08.28 
	}else{
		if(!_isBlinking){
			_backgroundColor = itemListStyle->getItemNonSelectedBackgroundColor();//VOXOX CHANGE by Rolando - 2009.08.28 
			_fontColor = itemListStyle->getItemUserNameNonSelectedFontColor();//VOXOX CHANGE by Rolando - 2009.09.07 
		}
		else{
			if(_blinkNow){
				_backgroundColor = itemListStyle->getItemBlinkingBackgroundColor();//VOXOX CHANGE by Rolando - 2009.09.07 
				_fontColor = itemListStyle->getItemBlinkingFontColor();//VOXOX CHANGE by Rolando - 2009.09.07 
			}
			else{
				_backgroundColor = itemListStyle->getItemNonSelectedBackgroundColor();//VOXOX CHANGE by Rolando - 2009.08.28 
				_fontColor = itemListStyle->getItemUserNameNonSelectedFontColor();//VOXOX CHANGE by Rolando - 2009.09.07 
			}
		}

		painter->fillRect(option.rect, _backgroundColor);//VOXOX CHANGE by Rolando - 2009.08.28 
	}
	paintForeground(painter, option);

}
//VOXOX CHANGE - Add support for ItemList style and change paint structure.
void QtUMGroup::paintForeground(QPainter * painter, const QStyleOptionViewItem & option) {
	QMutexLocker locker(&_mutex);	

	bool isGroupSelected = false;//VOXOX CHANGE by Rolando - 2009.09.03
	bool isGroupBlinking = false;//VOXOX CHANGE by Rolando - 2009.09.08 
	QtUMItemListStyle * itemListStyle = QtUMItemListStyle::getInstance();//VOXOX CHANGE by Rolando - 2009.09.07 
	
	isGroupBlinking = isBlinking();//VOXOX CHANGE by Rolando - 2009.09.03 
	isGroupSelected = _isSelected;//VOXOX CHANGE by Rolando - 2009.10.01 
	
	if(isGroupSelected)//VOXOX CHANGE by Rolando - 2009.09.04 
	{
		painter->setPen(itemListStyle->getSelectedGroupFontColor());//VOXOX CHANGE by Rolando - 2009.09.04 
	}
	else
	{
		if(isGroupBlinking){

			if(_blinkNow){//VOXOX CHANGE by Rolando - 2009.10.01 
				painter->setPen(itemListStyle->getGroupBlinkingFontColor());//VOXOX CHANGE by Rolando - 2009.10.01 
			}
			else{
				painter->setPen(itemListStyle->getNonSelectedGroupFontColor());//VOXOX CHANGE by Rolando - 2009.10.01 
			}
			
		}
		else{
			painter->setPen(itemListStyle->getNonSelectedGroupFontColor());//VOXOX CHANGE by Rolando - 2009.09.04 
		}
	}
	
	painter->drawPixmap(option.rect.left(),option.rect.top(),getGroupBackGround(option.rect,option, isGroupSelected, isGroupBlinking));

	QFont font = option.font;
	painter->setFont(font);

	QPixmap px;
	if (option.state & QStyle::State_Open) {
		px = itemListStyle->getGroupPixmapOpen();
	} else {
		px = itemListStyle->getGroupPixmapClose();
	}

	int x = option.rect.left();
	QRect r = option.rect;
	painter->drawPixmap(x, r.top() + 3, px);

	_posXArrowButton = x;//VOXOX CHANGE by Rolando - 2009.09.29 
	_posYArrowButton = r.top() + 3;//VOXOX CHANGE by Rolando - 2009.09.29 
	_widthArrowButton = px.width();//VOXOX CHANGE by Rolando - 2009.09.29 
	_heightArrowButton = px.height();//VOXOX CHANGE by Rolando - 2009.09.29 

	x += px.width() + 3;
	r.setLeft(x);
	int y = ((r.bottom()-r.top())-QFontMetrics(font).height())/2;
	r.setTop(y + r.top());

	painter->drawText(r, Qt::AlignLeft, getGroupName(), 0);


	//VOXOX CHANGE by Rolando - 2009.08.25 - getting corresponding close button
	QPixmap closeButtonPixmap;

	if ((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
		closeButtonPixmap = QPixmap(":/pics/chat/umtreewidget/btn_close_selected.png");//VOXOX CHANGE by Rolando - 2009.08.25 

		painter->setPen(_fontColor);//VOXOX CHANGE by Rolando - 2009.09.07 
	} else {
		closeButtonPixmap = QPixmap(":/pics/chat/umtreewidget/btn_close_nonselected.png");//VOXOX CHANGE by Rolando - 2009.08.25 
		painter->setPen(_fontColor);//VOXOX CHANGE by Rolando - 2009.09.07 
	}

	//VOXOX CHANGE by Rolando - 2009.08.25 - painting the close button	
	_posXCloseButton = r.right() - closeButtonPixmap.width() - 5;//VOXOX CHANGE by Rolando - 2009.09.16 
	int centeredCloseButton_y = ((r.bottom() - r.top()) - closeButtonPixmap.size().height()) / 2;	
	painter->drawPixmap(_posXCloseButton, r.top() + centeredCloseButton_y, closeButtonPixmap);

	_posYCloseButton = r.top() + centeredCloseButton_y;
	_widthCloseButton =  closeButtonPixmap.width();//VOXOX CHANGE by Rolando - 2009.08.27
	_heightCloseButton = closeButtonPixmap.height();//VOXOX CHANGE by Rolando - 2009.08.27 	
	
}

QString QtUMGroup::verifyText(QRect & painterRect, QFont font, QString text){

	if(text != ""){//VOXOX CHANGE by Rolando - 2009.08.25 - bug fixed when text is empty
		int xText = painterRect.left();
		int textWidth = QFontMetrics(font).width(text);//gets new "x" position according width's text	
		
		bool fixedText = false;

		while(xText + textWidth > _posXCloseButton && text != ""){//we need to check that we can paint on the QtUMGroup but not on the vertical line
			text = text.remove(text.length()- 1,text.length());
			textWidth = QFontMetrics(font).width(text);//gets new "x" position according width's text
			fixedText = true;
		}
		
		if(fixedText){
			if(text.length() >= 3){			
				text = text.remove(text.length()- 3,text.length() );
				text += "...";
			}		
		}
	}
	return text;
	


	
}

QPixmap QtUMGroup::getGroupBackGround(const QRect & rect, const QStyleOptionViewItem & option, bool isGroupSelected, bool isGroupBlinking) const {

	QPen pen;
	QtUMItemListStyle * itemListStyle = QtUMItemListStyle::getInstance();//VOXOX CHANGE by Rolando - 2009.09.07 
	QColor backgroundColor;//VOXOX CHANGE by Rolando - 2009.09.03
	QColor borderColor;//VOXOX CHANGE by Rolando - 2009.09.07 

	int groupBackground = _groupBackground.width();
	int rectWidth = rect.width();
	bool isNull = _groupBackground.isNull();
			
	_groupBackground = QPixmap(rect.width(),GROUP_WIDGET_FRAME_HEIGHT);
	QPainter painter(&_groupBackground);
	//TODO CHANGE CODE TO GRADIENT IF NEEDED	

	if(!isGroupSelected){//VOXOX CHANGE by Rolando - 2009.09.03
		if(isGroupBlinking){

			if(_blinkNow){//VOXOX CHANGE by Rolando - 2009.10.01 
				backgroundColor = itemListStyle->getGroupBlinkingBackgroundColor();//VOXOX CHANGE by Rolando - 2009.09.03
			}
			else{
				backgroundColor = itemListStyle->getNonSelectedGroupBackgroundColorTop();//VOXOX CHANGE by Rolando - 2009.09.03 
			}
			
		}
		else{
			backgroundColor = itemListStyle->getNonSelectedGroupBackgroundColorTop();//VOXOX CHANGE by Rolando - 2009.09.03
		}
		borderColor = itemListStyle->getNonSelectedGroupBorderColor();//VOXOX CHANGE by Rolando - 2009.09.03
	}
	else{
		backgroundColor = itemListStyle->getSelectedGroupBackgroundColorTop();//VOXOX CHANGE by Rolando - 2009.09.03
		borderColor = itemListStyle->getSelectedGroupBorderColor();//VOXOX CHANGE by Rolando - 2009.09.03
	}

	painter.fillRect(_groupBackground.rect(),QBrush(backgroundColor));//VOXOX CHANGE by Rolando - 2009.09.03 
	
	pen = painter.pen();	

	painter.setPen (borderColor);//VOXOX CHANGE by Rolando - 2009.09.03
	painter.drawLine(QPoint(0,0),QPoint(rect.width(),0));

	if(isOpen() && hasChildIds()){//VOXOX CHANGE by Rolando - 2009.09.29 
		painter.setPen (backgroundColor);//VOXOX CHANGE by Rolando - 2009.09.29 
	}
	else{
		painter.setPen (borderColor);//VOXOX CHANGE by Rolando - 2009.09.03
	}
	
	painter.drawLine(QPoint(0,GROUP_WIDGET_FRAME_HEIGHT-1),QPoint(rect.width(),GROUP_WIDGET_FRAME_HEIGHT-1));

	painter.setPen (pen);	
	painter.end();

	return _groupBackground;	
}

QString QtUMGroup::getId() const 
{
	return _id;
}


int QtUMGroup::getHeight() const 
{	
	return GroupSize;
	
}



//-----------------------------------------------------------------------------

//VOXOX CHANGE by Rolando - 2009.08.27 
bool QtUMGroup::mouseHoveringCloseButton(QPoint mousePosition){
	QPoint rectPoint1 = QPoint(_posXCloseButton, _posYCloseButton);
	QPoint rectPoint2 = QPoint(_posXCloseButton + _widthCloseButton, _posYCloseButton + _heightCloseButton);
	QRect r = QRect(rectPoint1,rectPoint2);
	return r.contains(mousePosition);	
}

//VOXOX CHANGE by Rolando - 2009.09.29 
bool QtUMGroup::mouseHoveringArrowButton(QPoint mousePosition){
	QPoint rectPoint1 = QPoint(_posXArrowButton, _posYArrowButton);
	QPoint rectPoint2 = QPoint(_posXArrowButton + _widthArrowButton, _posYArrowButton + _heightArrowButton);
	QRect r = QRect(rectPoint1,rectPoint2);
	return r.contains(mousePosition);	
}

//VOXOX CHANGE by Rolando - 2009.09.29 
void QtUMGroup::addChildId(QString id)
{
	if(!existsChildId(id)){
		_childListIds << id;//VOXOX CHANGE by Rolando - 2009.09.29
	}
}

//VOXOX CHANGE by Rolando - 2009.09.29 
void QtUMGroup::removeChildId(QString id)
{
	_childListIds.removeAll(id);//VOXOX CHANGE by Rolando - 2009.09.29 
}

//VOXOX CHANGE by Rolando - 2009.09.29 
bool QtUMGroup::existsChildId(QString id)
{
	return _childListIds.contains(id);
}

//VOXOX CHANGE by Rolando - 2009.09.29 
bool QtUMGroup::hasChildIds() const
{
	return !_childListIds.empty();
}