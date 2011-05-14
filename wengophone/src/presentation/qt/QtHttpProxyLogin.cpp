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
#include "QtHttpProxyLogin.h"

#include "ui_HttpProxyLoginWindow.h"

#include <qtutil/WidgetBackgroundImage.h>

#include <QtGui/QtGui>

QtHttpProxyLogin::QtHttpProxyLogin(QWidget * parent, const std::string & proxyAddress, unsigned proxyPort)
	: QObject(parent) {

	_httpProxyLoginWindow = new QDialog(parent);

	_ui = new Ui::HttpProxyLoginWindow();
	_ui->setupUi(_httpProxyLoginWindow);

	WidgetBackgroundImage::setBackgroundImage(_ui->httpProxyLabel, ":pics/headers/login.png", WidgetBackgroundImage::AdjustSize);

	//addressLineEdit
	_ui->addressLineEdit->setText(QString::fromStdString(proxyAddress));

	//portLineEdit
	_ui->portLineEdit->setText(QString::number(proxyPort));
}

std::string QtHttpProxyLogin::getLogin() const {
	return _ui->loginLineEdit->text().toStdString();
}

std::string QtHttpProxyLogin::getPassword() const {
	return _ui->passwordLineEdit->text().toStdString();
}

std::string QtHttpProxyLogin::getProxyAddress() const {
	return _ui->addressLineEdit->text().toStdString();
}

unsigned QtHttpProxyLogin::getProxyPort() const {
	return _ui->portLineEdit->text().toUInt();
}

int QtHttpProxyLogin::show() {
	return _httpProxyLoginWindow->exec();
}
