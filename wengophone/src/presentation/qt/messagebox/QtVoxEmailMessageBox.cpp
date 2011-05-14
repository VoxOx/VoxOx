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
* @date 2009.06.10
*/

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtVoxEmailMessageBox.h"

#include "ui_VoxEmailMessageBox.h"

#include <qtutil/SafeConnect.h>

#include <util/SafeDelete.h>


#include <qtutil/Widget.h>
#include <QtGui/QtGui>

QtVoxEmailMessageBox::QtVoxEmailMessageBox(QWidget * parent)
	: QDialog(parent){
	
	_ui = new Ui::VoxEmailMessageBox();
	_ui->setupUi(this);

	SAFE_CONNECT(_ui->buttonBox,		SIGNAL(accepted()), SLOT(accepted()));
	SAFE_CONNECT(_ui->buttonBox,		SIGNAL(rejected()), SLOT(rejected()));

}

QtVoxEmailMessageBox::~QtVoxEmailMessageBox() {

	OWSAFE_DELETE(_ui);
}


void QtVoxEmailMessageBox::setDialogText(QString newtext) {
	_ui->lblText->setText(newtext);
}

void QtVoxEmailMessageBox::setTitle(QString newtitle) {
	setWindowTitle(newtitle);
}

void QtVoxEmailMessageBox::setEmailTypeVisible(bool visible) {
	_ui->cmbEmailType->setVisible(visible);
}



QString QtVoxEmailMessageBox::getEmail() {
	return _ui->lineEdit->text();
}

QtEnumEmailType::Type QtVoxEmailMessageBox::getEmailType(){

	QString cmbText = _ui->cmbEmailType->currentText();
	QtEnumEmailType::Type type = QtEnumEmailType::Unknown;

	if(cmbText == "Personal"){
		type =  QtEnumEmailType::Personal;
	}else if(cmbText == "Work"){
		type =  QtEnumEmailType::Work;
	}else if(cmbText == "Other"){
		type =  QtEnumEmailType::Other;
	}
	
	return type;

}

void QtVoxEmailMessageBox::accepted() {
	this->accept();
}

void QtVoxEmailMessageBox::rejected() {
	this->reject();
}
