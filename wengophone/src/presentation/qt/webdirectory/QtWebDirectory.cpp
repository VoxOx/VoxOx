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
#include "QtWebDirectory.h"

#include <model/webservices/url/WsUrl.h>

#include <qtutil/LanguageChangeEventFilter.h>
#include <qtutil/CloseEventFilter.h>

#include <QtGui/QtGui>

QtWebDirectory::QtWebDirectory(QWidget * parent)
	: QtBrowser(parent) {

	QWidget * widget = (QWidget *) getWidget();

	CloseEventFilter * closeEventFilter = new CloseEventFilter(this, SLOT(CloseEventFilterSlot()));
	widget->installEventFilter(closeEventFilter);

	widget->resize(715, 569);
	LANGUAGE_CHANGE(widget);
	init();
}

QtWebDirectory::~QtWebDirectory() {
}

void QtWebDirectory::raise() {
	QWidget * widget = (QWidget *) getWidget();
	widget->show();
	widget->raise();
}

void QtWebDirectory::init() {
	setUrl(WsUrl::getWengoDirectoryUrl());
	QWidget * widget = (QWidget *) getWidget();
	widget->setWindowTitle(tr("@product@ - Directory"));
}

void QtWebDirectory::languageChanged() {
	init();
}

void QtWebDirectory::CloseEventFilterSlot() {
	setUrl(WsUrl::getWengoDirectoryUrl());
}
