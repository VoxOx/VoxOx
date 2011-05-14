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
* @Message box
* @author Chris Jimenez C 
* @date 2009.06.10
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtVoxMessageBox.h"

#include <QtGui/QtGui>

QtVoxMessageBox::QtVoxMessageBox(QWidget * parent)
	: QMessageBox(parent){

		setIconPixmap (QPixmap(":pics/messagebox/voxox_icon.png"));
	
}


QtVoxMessageBox::~QtVoxMessageBox() {

}
