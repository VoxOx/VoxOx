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
#ifndef QTTHUMBNAILMANAGER_H
#define QTTHUMBNAILMANAGER_H

#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QThread>
#include <QtGui/QPixmap>


class QEvent;
class QFileInfo;


/**
 * This class generates thumbnails for images. Thumbnails are generated
 * in a separated thread and cached.
 *
 * @author Aurélien Gâteau
 */
class QtThumbnailManager : public QThread {
	Q_OBJECT
public:
	QtThumbnailManager(QObject* parent = 0);
	~QtThumbnailManager();

	/**
	 * Generates a thumbnail for @param fileInfo.
	 * @return the thumbnail if already available, a null pixmap otherwise.
	 */
	QPixmap getThumbnail(const QFileInfo& fileInfo);

	/**
	 * Clear the thumbnail cache
	 */
	void clear();

	/**
	 * Sets the thumbnail size in pixel
	 */
	void setSize(int size);

Q_SIGNALS:
	/**
	 * Emitted whenever a thumbnail requested with @ref getThumbnail has been
	 * generated.
	 */
	void thumbnailUpdated(const QFileInfo& fileInfo);

protected:
	virtual void run();
	virtual void customEvent(QEvent*);

private:
	typedef QMap<QString, QPixmap> ThumbnailMap;
	typedef QQueue<QString> PendingThumbnailQueue;

	QPixmap _filePixmap;
	ThumbnailMap _map;
	int _size;

	QMutex _mutex; /** This mutex protects _pendingQueue and _clearing */
	PendingThumbnailQueue _pendingQueue;
	bool _clearing; /** Set to true if clear() has been called */
};


#endif /* QTTHUMBNAILMANAGER_H */
