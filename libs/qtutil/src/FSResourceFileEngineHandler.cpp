/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#include <qtutil/FSResourceFileEngineHandler.h>

#include <QtCore/QDir>
#include <QtCore/QFSFileEngine>

FSResourceFileEngineHandler::FSResourceFileEngineHandler(const QString& baseDir) {
	_baseDir = QDir::convertSeparators(baseDir);
	if (!_baseDir.endsWith(QDir::separator())) {
		_baseDir.append(QDir::separator());
	}
}

QAbstractFileEngine* FSResourceFileEngineHandler::create(const QString& path_) const {
	QString path = path_;
	if (path.size() > 0 && path.startsWith(QLatin1Char(':'))) {
		path = _baseDir + path.mid(1);
		if (QFile::exists(path)) {
			QFSFileEngine* engine = new QFSFileEngine;
			engine->setFileName(path);
			return engine;
		}
	}
	return 0;
}
