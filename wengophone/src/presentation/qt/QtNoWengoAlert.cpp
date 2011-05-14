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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtNoWengoAlert.h"

#include "ui_NoWengoAlert.h"

#include <model/webservices/url/WsUrl.h>

//#include <control/CWengoPhone.h>
//#include <control/profile/CUserProfileHandler.h>

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/QtToolBar.h>

#include <qtutil/SafeConnect.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

QtNoWengoAlert::QtNoWengoAlert(QWidget * parent, QtWengoPhone & qtWengoPhone)
	: _qtWengoPhone(qtWengoPhone){
	
	_NoWengoAlert = new QDialog(parent);

	_ui = new Ui::NoWengoAlert();
	_ui->setupUi(_NoWengoAlert);

	SAFE_CONNECT(_ui->haveBtn, SIGNAL(clicked()), SLOT(haveButtonClicked()));
	SAFE_CONNECT(_ui->createBtn, SIGNAL(clicked()), SLOT(createButtonClicked()));
}

QtNoWengoAlert::QtNoWengoAlert(QWidget * parent, QtWengoPhone & qtWengoPhone, QString title, QString maintext)
	: _qtWengoPhone(qtWengoPhone){
	
	_NoWengoAlert = new QDialog(parent);

	_ui = new Ui::NoWengoAlert();
	_ui->setupUi(_NoWengoAlert);
	
	_NoWengoAlert->setWindowTitle(title);
	_ui->mainText->setText(maintext);

	SAFE_CONNECT(_ui->haveBtn, SIGNAL(clicked()), SLOT(haveButtonClicked()));
	SAFE_CONNECT(_ui->createBtn, SIGNAL(clicked()), SLOT(createButtonClicked()));
}

QtNoWengoAlert::~QtNoWengoAlert() {
	OWSAFE_DELETE(_ui);
}

void QtNoWengoAlert::haveButtonClicked() {
	//dear user enjoy a disconnection....
//	_qtWengoPhone.getQtToolBar()->logOff();
	_qtWengoPhone.logOff();	//VOXOX - JRT - 2009.07.12 
}

void QtNoWengoAlert::createButtonClicked() {
	WsUrl::showWengoAccountCreation();
}

void QtNoWengoAlert::setDialogText(QString newtext) {
	_ui->mainText->setText(newtext);
}

void QtNoWengoAlert::setTitle(QString newtitle) {
	_NoWengoAlert->setWindowTitle(newtitle);
}
