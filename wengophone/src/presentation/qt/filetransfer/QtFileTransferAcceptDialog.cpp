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
#include "QtFileTransferAcceptDialog.h"
#include "ui_AcceptDialog.h"

#include <qtutil/WidgetBackgroundImage.h>
#include <util/SafeDelete.h>

QtFileTransferAcceptDialog::QtFileTransferAcceptDialog(QWidget * parent)
	: QDialog(parent) {

	_ui = new Ui::AcceptDialog();
	_ui->setupUi(this);

	//install header image
	WidgetBackgroundImage::setBackgroundImage(_ui->headerLabel, ":pics/headers/file-transfer.png", WidgetBackgroundImage::AdjustSize);
	////
}

QtFileTransferAcceptDialog::~QtFileTransferAcceptDialog() {
	OWSAFE_DELETE(_ui);
}

void QtFileTransferAcceptDialog::setFileName(const QString & fileName) {
	_ui->filenameLabel->setText(fileName);
}

void QtFileTransferAcceptDialog::setContactName(const QString & contactName) {
	_ui->contactLabel->setText(contactName);
}
