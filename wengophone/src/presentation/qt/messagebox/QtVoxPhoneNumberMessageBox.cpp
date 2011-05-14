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
#include "QtVoxPhoneNumberMessageBox.h"

#include "ui_VoxPhoneNumberMessageBox.h"

#include <qtutil/SafeConnect.h>

#include <util/SafeDelete.h>


#include <qtutil/Widget.h>
#include <qtutil/VoxOxToolTipLineEdit.h>
#include <QtGui/QtGui>

QtVoxPhoneNumberMessageBox::QtVoxPhoneNumberMessageBox(QWidget * parent)
	: QDialog(parent){
	
	_ui = new Ui::VoxPhoneNumberMessageBox();
	_ui->setupUi(this);

	_qtVoxOxCallBarFrame = new QtVoxOxCallBarFrame(this);

	//VOXOX CHANGE by Rolando - 2009.06.16 
	_qtVoxOxCallBarFrame->updateStyleSheet("808080","666666","#000000","#ffffff","none","transparent","white", "black", "gray");//VOXOX CHANGE by Rolando - 2009.06.15 

	_qtVoxOxCallBarFrame->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->frmText)->addWidget(_qtVoxOxCallBarFrame);

#ifdef OS_WINDOWS//VOXOX CHANGE by Rolando - 2009.07.13 
	_qtVoxOxCallBarFrame->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13	
#endif
		
	SAFE_CONNECT(_ui->buttonBox,		SIGNAL(accepted()), SLOT(accepted()));
	SAFE_CONNECT(_ui->buttonBox,		SIGNAL(rejected()), SLOT(rejected()));

}

QtVoxPhoneNumberMessageBox::~QtVoxPhoneNumberMessageBox() {

	OWSAFE_DELETE(_ui);
}


void QtVoxPhoneNumberMessageBox::setDialogText(QString newtext) {
	_ui->lblText->setText(newtext);
}

void QtVoxPhoneNumberMessageBox::setTitle(QString newtitle) {
	setWindowTitle(newtitle);
}

void QtVoxPhoneNumberMessageBox::setNumberTypeVisible(bool visible) {
	_ui->cmbPhoneType->setVisible(visible);
}



QString QtVoxPhoneNumberMessageBox::getNumber() {
	return _qtVoxOxCallBarFrame->getFullPhoneNumberText();
}

QtEnumPhoneType::Type QtVoxPhoneNumberMessageBox::getPhoneType(){

	QString cmbText = _ui->cmbPhoneType->currentText();
	QtEnumPhoneType::Type type = QtEnumPhoneType::Unknown;

	if(cmbText == "Mobile"){
		type =  QtEnumPhoneType::Mobile;
	}else if(cmbText == "Home"){
		type =  QtEnumPhoneType::Home;
	}else if(cmbText == "Work"){
		type =  QtEnumPhoneType::Work;
	}else if(cmbText == "Fax"){
		type =  QtEnumPhoneType::Fax;
	}else if(cmbText == "Other"){
		type =  QtEnumPhoneType::Other;
	}
	
	return type;

}

void QtVoxPhoneNumberMessageBox::accepted() {
	this->accept();
}

void QtVoxPhoneNumberMessageBox::rejected() {
	this->reject();
}
