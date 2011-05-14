/*
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


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtUMItem.h"
#include <presentation/qt/chat/umtreewidget/QtUMItemListStyle.h>
#include <presentation/qt/contactlist/QtContactPixmap.h>
#include <presentation/qt/contactlist/QtContactNetworkMenu.h>//VOXOX CHANGE by Rolando - 2009.10.23 
#include <util/Logger.h>

#include <QtGui/QtGui>

//VOXOX - JRT - 2009.05.04 - TODO: these Ht/Wd defines are same as enum in QtUMItem.h.  We should use the enum.
const int AVATAR_HEIGHT		 =		28;
const int AVATAR_WIDTH		 =		28;
const int CONTACT_MARGIN	 =		 5;//VOXOX CHANGE by Rolando - 2009.08.31 
const int CONTACT_MARGIN_IN_GROUP = 16;//VOXOX CHANGE by Rolando - 2009.08.31 
const int AVATAR_TEXT_MARGIN =		 4;
const int AVATAR_MARGIN_TOP	 =		 4;

QtUMItem::QtUMItem(CWengoPhone & cWengoPhone, const QString & contactId, const QString & id, const QString & groupId, const QString & displayName, const QString & groupName, QString avatarData, QPixmap normalStatusPixmap, QPixmap hoverStatusPixmap, QObject * parent, bool bGroup, bool isSelected )
	:  QObject(parent),
	_cWengoPhone(cWengoPhone)
{
	initVars();

	_isAGroupItem  = bGroup;
	_id = id;
	_contactId = contactId;//VOXOX CHANGE by Rolando - 2009.10.23 
	_groupKey  = groupId;					//VOXOX - JRT - 2009.08.09 - Multiple groups.
	_key	   = id + groupId;		//VOXOX - JRT - 2009.08.10 - Multiple groups.
	_groupName = groupName;//VOXOX CHANGE by Rolando - 2009.08.21 
	_displayName = displayName;//VOXOX CHANGE by Rolando - 2009.08.31
	_avatarData = avatarData;//VOXOX CHANGE by Rolando - 2009.08.31 
	_normalStatusPixmap = normalStatusPixmap;//VOXOX CHANGE by Rolando - 2009.10.26
	_hoverStatusPixmap = hoverStatusPixmap;//VOXOX CHANGE by Rolando - 2009.10.26

	_isSelected = isSelected;//VOXOX CHANGE by Rolando - 2009.09.29

	_isNetworkIconUnderMouse = false;//VOXOX CHANGE by Rolando - 2009.10.26 

	if(_groupKey != ""){//VOXOX CHANGE by Rolando - 2009.09.25 
		_isAGroupItem = true;//VOXOX CHANGE by Rolando - 2009.09.25 
	}
	else{
		_isAGroupItem = false;	//VOXOX CHANGE by Rolando - 2009.09.25 
	}

	_contactNetworkMenu = new QtContactNetworkMenu(_contactId.toStdString(),_cWengoPhone , NULL);//VOXOX CHANGE by Rolando - 2009.10.23 
	

	
}

void QtUMItem::initVars()
{
	_item		 = NULL;
	_mouseOn	 = false;
	_openStatus  = false;
	_mouseButton = Qt::NoButton;

	_posXNetworkStatus = 0;
	_posXCloseButton = 0;//VOXOX CHANGE by Rolando - 2009.08.27 
	_posYCloseButton = 0;//VOXOX CHANGE by Rolando - 2009.08.27
	_widthCloseButton = 0;//VOXOX CHANGE by Rolando - 2009.08.27
	_heightCloseButton = 0;//VOXOX CHANGE by Rolando - 2009.08.27

	_posXNetworkButton = 0;//VOXOX CHANGE by Rolando - 2009.10.21 
	_posYNetworkButton = 0;//VOXOX CHANGE by Rolando - 2009.10.21 
	_widthNetworkButton = 0;//VOXOX CHANGE by Rolando - 2009.10.21 
	_heightNetworkButton = 0;//VOXOX CHANGE by Rolando - 2009.10.21 
	
	//VOXOX CHANGE by Rolando - 2009.08.28 
	_isBlinking = false;
	_blinkNow = false;

	
}

//VOXOX CHANGE by Rolando - 2009.08.28 
void QtUMItem::setBlinking(bool isBlinking){
	_isBlinking = isBlinking;
}

void QtUMItem::paint(QPainter * painter, const QStyleOptionViewItem & option) {

	if(_isBlinking){
		_blinkNow = !_blinkNow;
	}

	QtUMItemListStyle * itemListStyle = QtUMItemListStyle::getInstance();

	if (!_isSelected) {//VOXOX CHANGE by Rolando - 2009.09.30 
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

		
	}
	else{
		_backgroundColor = itemListStyle->getItemSelectedBackgroundColorTop();//VOXOX CHANGE by Rolando - 2009.10.01 
		_fontColor = itemListStyle->getItemUserNameSelectedFontColor();//VOXOX CHANGE by Rolando - 2009.10.01 
	}

	painter->fillRect(option.rect, _backgroundColor);//VOXOX CHANGE by Rolando - 2009.08.28
	
	paintForeground(painter, option);
}
//VOXOX CHANGE - Add support for ItemList style and change paint structure.
void QtUMItem::paintForeground(QPainter * painter, const QStyleOptionViewItem & option) {
	QMutexLocker locker(&_mutex);
	
	QtContactPixmap * spx = QtContactPixmap::getInstance();
	QtUMItemListStyle * itemListStyle = QtUMItemListStyle::getInstance();

	//VOXOX CHANGE by Rolando - 2009.08.25 - getting corresponding close button
	QPixmap closeButtonPixmap;

	if ((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
		closeButtonPixmap = QPixmap(":/pics/chat/umtreewidget/btn_close_selected.png");//VOXOX CHANGE by Rolando - 2009.08.25 
		painter->setPen(_fontColor);//VOXOX CHANGE by Rolando - 2009.09.07 
	} else {
		closeButtonPixmap = QPixmap(":/pics/chat/umtreewidget/btn_close_nonselected.png");//VOXOX CHANGE by Rolando - 2009.08.25 
		painter->setPen(_fontColor);//VOXOX CHANGE by Rolando - 2009.09.07 
	}

	QRect painterRect = option.rect;

	if(!isAGroupItem()){
		//VOXOX CHANGE by Rolando - 2009.08.25 - painting the close button	
		_posXCloseButton = painterRect.right() - closeButtonPixmap.width() - 5;//VOXOX CHANGE by Rolando - 2009.09.16 
		int centeredCloseButton_y = ((painterRect.bottom() - painterRect.top()) - closeButtonPixmap.size().height()) / 2;	
		painter->drawPixmap(_posXCloseButton, painterRect.top() + centeredCloseButton_y, closeButtonPixmap);

		_posYCloseButton = painterRect.top() + centeredCloseButton_y;
		_widthCloseButton =  closeButtonPixmap.width();//VOXOX CHANGE by Rolando - 2009.08.27
		_heightCloseButton = closeButtonPixmap.height();//VOXOX CHANGE by Rolando - 2009.08.27 
	}
	else{
		_posXCloseButton = painterRect.right();//VOXOX CHANGE by Rolando - 2009.09.25 
	}

	//painting the status icon
	QPixmap normalPx = getCurrentStatusPixmap();//VOXOX CHANGE by Rolando - 2009.10.26 
	_posXNetworkStatus = _posXCloseButton - normalPx.width() - 4;//VOXOX CHANGE by Rolando - 2009.10.26 
	_posXNetworkButton = _posXNetworkStatus;
	int centeredPx_y = ((painterRect.bottom() - painterRect.top()) - normalPx.size().height()) / 2;//VOXOX CHANGE by Rolando - 2009.10.26 
	_posYNetworkButton = painterRect.top() + centeredPx_y;//VOXOX CHANGE by Rolando - 2009.10.26 
	painter->drawPixmap(_posXNetworkStatus, painterRect.top() + centeredPx_y, normalPx);//VOXOX CHANGE by Rolando - 2009.10.26 

	_widthNetworkButton = normalPx.size().width();//VOXOX CHANGE by Rolando - 2009.10.21 
	_heightNetworkButton = normalPx.size().height();//VOXOX CHANGE by Rolando - 2009.10.21 

	std::string foregroundPixmapData = getAvatarData().toStdString();

	QPixmap avatar;
	avatar.loadFromData ((uchar*) foregroundPixmapData.c_str(), foregroundPixmapData.size());
	avatar = avatar.scaled ( AVATAR_HEIGHT, AVATAR_WIDTH, Qt::KeepAspectRatio, Qt::SmoothTransformation) ;
	
	if(getGroupName() == ""){//VOXOX CHANGE by Rolando - 2009.08.31 
		painter->drawPixmap(CONTACT_MARGIN, painterRect.top() + (painterRect.height()  - AVATAR_HEIGHT) /2, avatar);//VOXOX CHANGE by Rolando - 2009.08.31 
		painterRect.setLeft(CONTACT_MARGIN+AVATAR_WIDTH+AVATAR_TEXT_MARGIN);//VOXOX CHANGE by Rolando - 2009.08.31 
	}
	else{
		painter->drawPixmap(CONTACT_MARGIN_IN_GROUP, painterRect.top() + (painterRect.height()  - AVATAR_HEIGHT) /2, avatar);//VOXOX CHANGE by Rolando - 2009.08.31 
		painterRect.setLeft(CONTACT_MARGIN_IN_GROUP+AVATAR_WIDTH+AVATAR_TEXT_MARGIN);//VOXOX CHANGE by Rolando - 2009.08.31 
	}


	// Draw the text
	painter->setFont(option.font);

	// Center the text vertically
	QRect textRect = painterRect;
	int textY = painterRect.top()+AVATAR_MARGIN_TOP+3;
	textRect.setTop(textY);

	QString text = fixHtmlString(getDisplayName());
		
	text = verifyText(textRect,option.font,text);
	painter->drawText(textRect, Qt::AlignLeft, text, 0);
	
}

QString QtUMItem::verifyText(QRect & painterRect, QFont font, QString text){

	if(text != ""){//VOXOX CHANGE by Rolando - 2009.08.25 - bug fixed when text is empty
		int xText = painterRect.left();
		int textWidth = QFontMetrics(font).width(text);//gets new "x" position according width's text	
		
		bool fixedText = false;

		while(xText + textWidth > _posXNetworkStatus && text != ""){//we need to check that we can paint on the QtUMItem but not on the vertical line
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

QString QtUMItem::getKey() const 
{
	return _key;
}

QString QtUMItem::getId() const 
{
	return _id;
}

QString QtUMItem::getUserName() const 
{
	return getDisplayName();
}

//VOXOX CHANGE by Rolando - 2009.08.21 
QString QtUMItem::getGroupName() const 
{
	return _groupName;//VOXOX CHANGE by Rolando - 2009.08.21 
}


int QtUMItem::getHeight() const 
{
	if (_openStatus) 
	{
		return UserOpenSize;
	}
	else
	{
		return UserSize;
	}

}

//End VoxOx


QString QtUMItem::getDisplayName() const 
{
	if ( isAGroupItem() )
	{
		return getGroupName();
	}
	else
	{
		return _displayName;
	}
}

QString QtUMItem::getEscapedDisplayName()
{
	return fixHtmlString( getDisplayName() );
}

//VOXOX CHANGE by Rolando - 2009.10.26 
void QtUMItem::setNormalStatusPixmap(QtContactPixmap::ContactPixmap status) 
{
	QtContactPixmap * spx = QtContactPixmap::getInstance();	
	QPixmap px = spx->getPixmap(status);
	
	_normalStatusPixmap = px;//VOXOX CHANGE by Rolando - 2009.08.31  
}

//VOXOX CHANGE by Rolando - 2009.10.26 
void QtUMItem::setNormalStatusPixmap(QPixmap statusPixmap) 
{	
	_normalStatusPixmap = statusPixmap;//VOXOX CHANGE by Rolando - 2009.08.31 
}

//VOXOX CHANGE by Rolando - 2009.10.26 
void QtUMItem::setHoverStatusPixmap(QtContactPixmap::ContactPixmap status) 
{
	QtContactPixmap * spx = QtContactPixmap::getInstance();	
	QPixmap px = spx->getPixmap(status);
	
	_hoverStatusPixmap = px;//VOXOX CHANGE by Rolando - 2009.08.31  
}

//VOXOX CHANGE by Rolando - 2009.10.26 
void QtUMItem::setHoverStatusPixmap(QPixmap statusPixmap) 
{	
	_hoverStatusPixmap = statusPixmap;//VOXOX CHANGE by Rolando - 2009.08.31 
}

//VOXOX - CJC - 2009.05.09 Get avatar
QPixmap QtUMItem::getAvatarPixmap() 
{
	std::string foregroundPixmapData = getAvatarData().toStdString();	
	QPixmap avatar;
	avatar.loadFromData ((uchar*) foregroundPixmapData.c_str(), foregroundPixmapData.size());
	return avatar;
}

//VOXOX CHANGE CJC, remove unescaped chars from sting
//TODO: QT does NOT provide an unescape() but they do provide an escape().  Move this to QtUtil project.
//TODO: I think String class does this.
QString QtUMItem::fixHtmlString(QString text)
{
    QString result = text;

	result = result.replace("&quot;", "\"");
	result = result.replace("&amp;",  "&" );
	result = result.replace("&gt;",   ">" );
	result = result.replace("&lt;",   "<" );
	result = result.replace("&apos;", "\'");
        
    return result;
}

//-----------------------------------------------------------------------------

//VOXOX CHANGE by Rolando - 2009.08.27 
bool QtUMItem::mouseHoveringCloseButton(QPoint mousePosition){
	QPoint rectPoint1 = QPoint(_posXCloseButton, _posYCloseButton);
	QPoint rectPoint2 = QPoint(_posXCloseButton + _widthCloseButton, _posYCloseButton + _heightCloseButton);
	QRect r = QRect(rectPoint1,rectPoint2);
	return r.contains(mousePosition);	
}

//VOXOX CHANGE by Rolando - 2009.10.21 
bool QtUMItem::mouseHoveringNetworkButton(QPoint mousePosition){
	QPoint rectPoint1 = QPoint(_posXNetworkButton, _posYNetworkButton);
	QPoint rectPoint2 = QPoint(_posXNetworkButton + _widthNetworkButton, _posYNetworkButton + _heightNetworkButton);
	QRect r = QRect(rectPoint1,rectPoint2);
	return r.contains(mousePosition);	
}

//VOXOX CHANGE by Rolando - 2009.10.26 
void QtUMItem::networkIconClicked(QPoint mousePosition){
	if(_contactNetworkMenu){
		QPoint p = QPoint(mousePosition.x(), _heightNetworkButton/2 + mousePosition.y());		
		_contactNetworkMenu->move(p);//VOXOX CHANGE by Rolando - 2009.10.21 
		_contactNetworkMenu->show();
	}

}

//VOXOX CHANGE by Rolando - 2009.10.26 
QPixmap	QtUMItem::getCurrentStatusPixmap()
{
	if(_isNetworkIconUnderMouse){
		if(!_hoverStatusPixmap.isNull()){
			return _hoverStatusPixmap;
		}
		else{
			return _normalStatusPixmap;//VOXOX CHANGE by Rolando - 2009.10.26 - TODO: resolve if we should return an empty pixmap or normal pixmap in this case
		}
	}
	else{
		return _normalStatusPixmap;
	}
}

//VOXOX CHANGE by Rolando - 2009.10.26 
void QtUMItem::setNetworkIconIsUnderMouse(bool isUnderMouse)
{
	_isNetworkIconUnderMouse = isUnderMouse;
}
