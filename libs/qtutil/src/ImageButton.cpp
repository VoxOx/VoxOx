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
* @author Chris Jimenez C 
* @date 2009.07.03
*/

#include <qtutil/ImageButton.h>

#include <QtGui/QApplication>
#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>


ImageButton::ImageButton(QWidget* parent)
	: QPushButton(parent)
{
	setCursor(QCursor(Qt::PointingHandCursor));
	this->installEventFilter( this );	//Enable our own eventFilter!

	_holdTimer = new QTimer(this);
     connect(_holdTimer, SIGNAL(timeout()), this, SLOT(checkHold()));
	
}


ImageButton::~ImageButton() {

	delete _holdTimer;
}



const QString& ImageButton::getNormalPixmap() const
{
	return _normalPixmap;
}

const QString& ImageButton::getPressedPixmap() const
{
	return _pressedPixmap;
}

bool ImageButton::eventFilter(QObject * obj, QEvent * event)
{

     if (event->type() == QEvent::ToolTip) {
         hoverButtonEvent();
     }
     return QWidget::event(event);

}


void ImageButton::mousePressEvent(QMouseEvent * event)
{
	if(event->button() == Qt::LeftButton){//VOXOX - CJC - 2009.07.03 
		_holdTimer->start(650);
		setDown(true);
		leftMouseButtonPress();
	}else if(event->button() == Qt::RightButton){//VOXOX - CJC - 2009.07.03 
		rightClick();
	}

}

void ImageButton::mouseReleaseEvent(QMouseEvent * event)
{
	setDown(false); 
	if(_holdTimer->isActive()){
		_holdTimer->stop();
		showMenu(); 
		toggled(isChecked());
		clicked();
		
	}	
}


void ImageButton::checkHold()
{
	_holdTimer->stop();
	if(isDown()){
		clickAndHold();
	}
	
}



void ImageButton::setNormalPixmap(const QString& pixmap)
{
	_normalPixmap = pixmap;
	updateStyle();
}

void ImageButton::setPressedPixmap(const QString& pixmap)
{
	_pressedPixmap = pixmap;
	updateStyle();
}


void ImageButton::setImages(const QString& normalPixmap,const QString& pressedPixmap, const QString & hoverPixmap, const QString & disablePixmap,const QString & checkedPixmap)
{
	_normalPixmap = normalPixmap;
	_pressedPixmap = pressedPixmap;
	_hoverPixmap = hoverPixmap;
	_disablePixmap = disablePixmap;
	if(this->isCheckable()){
		_checkedPixmap = checkedPixmap;
	}
	updateStyle();
}

void ImageButton::updateStyle()
{
	QString css;
	if(_normalPixmap!=""){
	
		css = "QPushButton{border: 0; image: url("+_normalPixmap+"); }";
	}
	if(_pressedPixmap!=""){
			css +=" QPushButton:pressed{image: url("+_pressedPixmap+"); }";
	}
	if(_hoverPixmap!=""){
			css +=" QPushButton:hover{image: url("+_hoverPixmap+"); }";
	}
	if(_disablePixmap!=""){
			css +=" QPushButton:disabled{image: url("+_disablePixmap+"); }";
	}
	if(_checkedPixmap!=""){
			css +=" QPushButton:checked{image: url("+_checkedPixmap+"); }";
			css +=" QPushButton:unchecked{image: url("+_normalPixmap+"); }";
	}

	setStyleSheet(css);
}
