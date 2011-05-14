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
#include "QtFileTransferNotifyDialog.h"
#include "ui_NotifyDialog.h"

#include <qtutil/WidgetBackgroundImage.h>
#include <util/SafeDelete.h>

QtFileTransferNotifyDialog::QtFileTransferNotifyDialog(QWidget * parent)
	: QDialog(parent) {

	_ui = new Ui::NotifyDialog();
	_ui->setupUi(this);

	//install header image
	WidgetBackgroundImage::setBackgroundImage(_ui->headerLabel, ":pics/headers/file-transfer.png", WidgetBackgroundImage::AdjustSize);
	////
}

QtFileTransferNotifyDialog::~QtFileTransferNotifyDialog() {
	OWSAFE_DELETE(_ui);
}

void QtFileTransferNotifyDialog::setTitle(const QString & title) {

	QString temp = QString("<html><head><meta name=\"qrichtext\" content=\"1\" />"
		"</head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-size:8.25pt;"
		"font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px;"
		"margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
		"<span style=\" font-size:18pt; font-weight:600; color:#ffffff;\">%1"
		"</span></p></body></html>").arg(title);
	_ui->headerLabel->setText(temp);
}

void QtFileTransferNotifyDialog::setMessage(const QString & message) {

	QString temp = QString("<html><head><meta name=\"qrichtext\" content=\"1\" />"
		"</head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg;"
		"font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px;"
		"margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;>"
		"<span style=\" font-weight:600;\">%1</span></p></body></html>").arg(message);
	_ui->statusLabel->setText(temp);
}
