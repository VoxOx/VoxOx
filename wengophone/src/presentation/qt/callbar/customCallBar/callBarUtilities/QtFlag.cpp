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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.05
#include "QtFlag.h"

#include <util/Logger.h>

#include <QtGui/QtGui>


QtFlag::QtFlag(int width, int height) {
	_pixmap = QPixmap(width,height);
	_pixmap.fill(Qt::transparent);
	_path = QString::null;
	_flagName = QString::null;
	_countryName = QStringList();
	_countryCode = QString::null;
	_regExp = QString::null;
	_buttonPixmap = QPixmap(width,height);
}

QtFlag::QtFlag(const QtFlag & source) {
	_path = source._path;
	_flagName = source._flagName;
	_countryName = QStringList(source._countryName);
	_countryCode = source._countryCode;
	_pixmap = source._pixmap;
	_regExp = source._regExp;
	_buttonPixmap = source._buttonPixmap;
}

QtFlag::~QtFlag() {
}

QString QtFlag::getHtml() const {
	return QString("<img src=\"%1\" />").arg(_path);
}

QString QtFlag::getHtmlRegExp() const {
	return QString("<img src=\"%1\" />").arg(_path);
}

bool QtFlag::isNullCountryCode() const {
	return _countryCode.isEmpty();
}

bool QtFlag::isNullCountryName() const {
	return _countryName.isEmpty();
}

void QtFlag::setRegExp(const QString & regExp) {
	_regExp = regExp;
}

QtFlag & QtFlag::operator=(const QtFlag & source) {
	_path = source._path;
	_flagName = source._flagName;
	_countryName = source._countryName;
	_countryCode = source._countryCode;
	_flagName = source._flagName;
	_regExp = source._regExp;
	_pixmap = source._pixmap;
	_buttonPixmap = source._buttonPixmap;
	return *this;
}
