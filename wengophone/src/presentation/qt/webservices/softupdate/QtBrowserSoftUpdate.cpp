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
#include "QtBrowserSoftUpdate.h"

#include "ui_BrowserSoftUpdateWindow.h"

#include <model/webservices/url/WsUrl.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/MouseEventFilter.h>
#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

QtBrowserSoftUpdate::QtBrowserSoftUpdate(const std::string & downloadUrl,
				unsigned long long buildId,
				const std::string & version,
				unsigned fileSize, QWidget * parent)
	: QObject(parent) {

	_softUpdateWindow = new QDialog(parent);

	_ui = new Ui::BrowserSoftUpdateWindow();
	_ui->setupUi(_softUpdateWindow);

	QString originalLabelText = _ui->updateTextLabel->text()
				.arg(QString::fromStdString(version))
				.arg(fileSize);
	_ui->updateTextLabel->setText(originalLabelText);

	MousePressEventFilter * mouseFilter = new MousePressEventFilter(this, SLOT(showWengoDownloadWebPage()));
	_ui->updateTextLabel->installEventFilter(mouseFilter);

	_softUpdateWindow->exec();
}

QtBrowserSoftUpdate::~QtBrowserSoftUpdate() {
	OWSAFE_DELETE(_ui);
}

void QtBrowserSoftUpdate::showWengoDownloadWebPage() {
	WsUrl::showWengoDownload();
	_softUpdateWindow->accept();
}
