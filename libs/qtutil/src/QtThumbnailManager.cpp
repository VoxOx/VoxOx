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
#include "QtThumbnailManager.h"

#include <qtutil/DesktopService.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QFileInfo>

#include <util/Logger.h>

// Uncomment to debug
//#define DEBUG_THUMBNAIL_MANAGER
#ifdef DEBUG_THUMBNAIL_MANAGER
#define LOCAL_LOG_DEBUG(x) LOG_DEBUG(QString(x).toStdString())
#else
#define LOCAL_LOG_DEBUG(x)
#endif


struct ThumbnailEvent : public QEvent {
	ThumbnailEvent(const QString& path, const QImage& image)
	: QEvent(User)
	, _path(path)
	, _image(image) {}

	QString _path;
	QImage _image;
};


static QImage generateThumbnailForPath(const QString & path, int size) {
	QImage image;
	if (!image.load(path)) return QImage();

	if (image.width() > size || image.height() > size) {
		image = image.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	return image;
}


QtThumbnailManager::QtThumbnailManager(QObject* parent)
	: QThread(parent)
	, _size(48)
	, _clearing(false)
{
}


QtThumbnailManager::~QtThumbnailManager() {
	clear();
	wait();
}


void QtThumbnailManager::setSize(int size) {
	QMutexLocker lock(&_mutex);
	_size = size;
	_filePixmap = DesktopService::getInstance()->desktopIconPixmap(DesktopService::FileIcon, size);
}


QPixmap QtThumbnailManager::getThumbnail(const QFileInfo & fileInfo) {
	QString path = fileInfo.absoluteFilePath();
	ThumbnailMap::const_iterator it = _map.find(path);
	if (it != _map.end()) {
		return it.value();
	}

	if (fileInfo.isDir()) {
		QPixmap pixmap = DesktopService::getInstance()->pixmapForPath(path, _size);
		_map[path] = pixmap;
		return pixmap;
	}

	QMutexLocker lock(&_mutex);
	_clearing = false;
	if (!_pendingQueue.contains(path)) {
		_pendingQueue.enqueue(path);
		LOCAL_LOG_DEBUG(QString("Queuing %1").arg(path));
		if (!isRunning()) {
			start();
		}
	} else {
		LOCAL_LOG_DEBUG(QString("%1 is already in queue").arg(path));
	}
	return _filePixmap;
}


void QtThumbnailManager::run() {
	while (true) {
		QString path;
		{
			QMutexLocker lock(&_mutex);
			if (_pendingQueue.size() == 0) {
				LOCAL_LOG_DEBUG("Stopped by end of queue");
				return;
			}
			path = _pendingQueue.dequeue();
		}

		QImage image = generateThumbnailForPath(path, _size);
		if (image.isNull()) {
			continue;
		}

		{
			QMutexLocker lock(&_mutex);
			if (_clearing) {
				LOCAL_LOG_DEBUG("Stopped by _clearing");
				return;
			}
		}
		ThumbnailEvent* event = new ThumbnailEvent(path, image);
		QCoreApplication::postEvent(this, event);
	}
}


void QtThumbnailManager::customEvent(QEvent* genericEvent) {
	Q_ASSERT(genericEvent->type() == QEvent::User);

	ThumbnailEvent* event = static_cast<ThumbnailEvent*>(genericEvent);

	QString path = event->_path;
	QFileInfo fileInfo(path);
	_map[path] = QPixmap::fromImage(event->_image);
	thumbnailUpdated(fileInfo);
}


void QtThumbnailManager::clear() {
	_map.clear();
	QMutexLocker lock(&_mutex);
	_pendingQueue.clear();
	_clearing = true;
}
