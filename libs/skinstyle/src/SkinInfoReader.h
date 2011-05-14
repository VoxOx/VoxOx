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

#ifndef SKININFOREADER_H
#define SKININFOREADER_H

#include <QtCore/QString>

class QSettings;

class SkinInfoReader {
public:

	static const int VALUE_ERROR = -1;

	SkinInfoReader();

	~SkinInfoReader();

	QString getSkinPath() const {
		return _skinPath;
	}

	bool contains(const QString & objectName) const;

	int getXPosition(const QString & objectName) const;

	int getYPosition(const QString & objectName) const;

	int getWidth(const QString & objectName) const;

	int getHeight(const QString & objectName) const;

	QString getRegularPixmap(const QString & objectName) const;

	QString getActivePixmap(const QString & objectName) const;

	QString getHoverPixmap(const QString & objectName) const;

	QString getDisablePixmap(const QString & objectName) const;

	QString getMask(const QString & objectName) const;

	QString getBackgroundPixmap(const QString & objectName) const;

	bool isHidden(const QString & objectName) const;

private:

	QSettings * _settings;

	QString _skinPath;

	static const QString PIXMAP_FILE_EXTENSION;
};

#endif	//SKININFOREADER_H
