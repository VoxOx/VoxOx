/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#include "SkinInfoReader.h"

#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QStringList>

#include <iostream>
using namespace std;

const QString SkinInfoReader::PIXMAP_FILE_EXTENSION = ".png";

SkinInfoReader::SkinInfoReader() {
	QDir dir(QDir::currentPath() + "/skins/");
	QStringList list = dir.entryList(QDir::Dirs);

	_skinPath = "skins/future/";
	_settings = new QSettings(_skinPath + "skin.ini", QSettings::IniFormat);

	cout << _settings->fileName().toStdString() << endl;
}

SkinInfoReader::~SkinInfoReader() {
	delete _settings;
}

bool SkinInfoReader::contains(const QString & objectName) const {
	QStringList keys = _settings->allKeys();
	QString str = keys.join(QString::null);

	return str.contains(objectName);
}

int SkinInfoReader::getXPosition(const QString & objectName) const {
	return _settings->value(objectName + "/xPosition", VALUE_ERROR).toInt();
}

int SkinInfoReader::getYPosition(const QString & objectName) const {
	return _settings->value(objectName + "/yPosition", VALUE_ERROR).toInt();
}

int SkinInfoReader::getWidth(const QString & objectName) const {
	return _settings->value(objectName + "/width", VALUE_ERROR).toInt();
}

int SkinInfoReader::getHeight(const QString & objectName) const {
	return _settings->value(objectName + "/height", VALUE_ERROR).toInt();
}

QString SkinInfoReader::getRegularPixmap(const QString & objectName) const {
	return _skinPath + objectName + PIXMAP_FILE_EXTENSION;
}

QString SkinInfoReader::getActivePixmap(const QString & objectName) const {
	return _skinPath + objectName + "-active" + PIXMAP_FILE_EXTENSION;
}

QString SkinInfoReader::getHoverPixmap(const QString & objectName) const {
	return _skinPath + objectName + "-hover" + PIXMAP_FILE_EXTENSION;
}

QString SkinInfoReader::getDisablePixmap(const QString & objectName) const {
	return _skinPath + objectName + "-disable" + PIXMAP_FILE_EXTENSION;
}

QString SkinInfoReader::getMask(const QString & objectName) const {
	return _skinPath + objectName + "-mask" + PIXMAP_FILE_EXTENSION;
}

QString SkinInfoReader::getBackgroundPixmap(const QString & objectName) const {
	return _skinPath + objectName + "-background" + PIXMAP_FILE_EXTENSION;
}

bool SkinInfoReader::isHidden(const QString & objectName) const {
	return _settings->value(objectName + "/hide", false).toBool();
}
