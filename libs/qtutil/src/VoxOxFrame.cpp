/****************************************************************************
** Filename: URLLabel.cpp
** Last updated [dd/mm/yyyy]: 14/02/2005
**
** QLabel subclass with URL handling and more.
**
** Copyright(C) 2005 Angius Fabrizio. All rights reserved.
**
** Based on the LGPL v.2 licensed KURLLabel from the KDE libraries by
** Kurt Granroth <granroth@kde.org> and Peter Putzer <putzer@kde.org>
**
** Changes made to the KURLLabel code:
**  - link color is no longer taken from KGlobalSettings but from qApp->palette().active().link()
**  - removed virtual_hook() member function
**  - replaced KCursor::handCursor() with QCursor(Qt::PointingHandCursor)
**  - added context menu (see mouseReleaseEvent method)
**
** This file is part of the OSDaB project(http://osdab.sourceforge.net/).
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See the file LICENSE.GPL that came with this software distribution or
** visit http://www.gnu.org/copyleft/gpl.html for GPL licensing information.
**
**********************************************************************/

#include <qtutil/VoxOxFrame.h>

VoxOxFrame::VoxOxFrame(QWidget* parent)
	: QFrame(parent)
{
	_currentStyleSheet = styleSheet();//VOXOX CHANGE by Rolando - 2009.07.07 
}


VoxOxFrame::~VoxOxFrame(){
}

//VOXOX CHANGE by Rolando - 2009.07.07 - sends a signal to know that mouse had left the frame
void VoxOxFrame::leaveEvent ( QEvent * event ){
	setCursor(QCursor(Qt::ArrowCursor));//VOXOX CHANGE by Rolando - 2009.07.07 
	mouseLeave();//VOXOX CHANGE by Rolando - 2009.07.07 
	QFrame::leaveEvent(event);	
}

//VOXOX CHANGE by Rolando - 2009.07.07 - sends a signal to know that mouse had entered into the frame 
void VoxOxFrame::enterEvent ( QEvent * event ){
	setCursor(QCursor(Qt::PointingHandCursor));//VOXOX CHANGE by Rolando - 2009.07.07 
	mouseEntered();//VOXOX CHANGE by Rolando - 2009.07.07 
	QFrame::enterEvent(event);
}

void VoxOxFrame::hideBorder(){
	setStyleSheet(QString("VoxOxFrame{ border: 1px solid transparent; }"));//VOXOX CHANGE by Rolando - 2009.07.07 	 
	update();//VOXOX CHANGE by Rolando - 2009.07.07 
}

void VoxOxFrame::showBorder(){
	setStyleSheet(_currentStyleSheet);//VOXOX CHANGE by Rolando - 2009.07.07 
	update();//VOXOX CHANGE by Rolando - 2009.07.07 
}
