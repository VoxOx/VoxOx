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
#include "QtVoxTextMessageBox.h"

#include "ui_VoxTextMessageBox.h"

#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

QtVoxTextMessageBox::QtVoxTextMessageBox(QWidget * parent)
	: QDialog(parent){
	
	_ui = new Ui::VoxTextMessageBox();
	_ui->setupUi(this);

	SAFE_CONNECT(_ui->buttonBox,		SIGNAL(accepted()), SLOT(accepted()));
	SAFE_CONNECT(_ui->buttonBox,		SIGNAL(rejected()), SLOT(rejected()));

}

QtVoxTextMessageBox::~QtVoxTextMessageBox() {
	OWSAFE_DELETE(_ui);
}


void QtVoxTextMessageBox::setDialogText(QString newtext) {
	_ui->lblText->setText(newtext);
}

void QtVoxTextMessageBox::setTitle(QString newtitle) {
	setWindowTitle(newtitle);
}

QString QtVoxTextMessageBox::exec() {
	QDialog::exec();
	return _ui->lineEdit->text();

}


void QtVoxTextMessageBox::accepted() {
	QDialog::accept();
}

void QtVoxTextMessageBox::rejected() {
	QDialog::reject();
}
