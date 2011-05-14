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
#ifndef QTTHUMBNAILDIRMODEL_H
#define QTTHUMBNAILDIRMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QDir>
#include <QtCore/QStringList>


class QtThumbnailManager;


/**
 * A Qt model, similar to QDirModel but which does not store the whole file
 * tree and does not require to go through a QSortFilterProxyModel to be
 * filtered.
 * Using QDirModel and QSortFilterProxyModel causes troubles because
 * QSortFilterProxyModel will filter all parents of the view root index on
 * first call. This operation can be slow if the filter is CPU expensive.
 */
class QtThumbnailDirModel : public QAbstractListModel {
	Q_OBJECT
public:	
	QtThumbnailDirModel(QObject* parent=0);

	/**
	 * Sets the size of the item pixmaps
	 */
	void setPixmapSize(int size);

	/**
	 * Sets the dir to list
	 */
	void setDir(const QString& dir);

	/**
	 * Returns the listed dir
	 */
	QString dir() const;

	QFileInfo fileInfo(const QModelIndex&) const;

	int rowCount(const QModelIndex& parent=QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

public Q_SLOTS:
	void refresh();

private Q_SLOTS:
	void slotPixmapUpdated(const QFileInfo&);

private:
	QtThumbnailManager* _thumbnailManager;
	QDir _dir;
	QFileInfoList _infoList;
	QStringList _imageSuffixList;
};


#endif /* QTTHUMBNAILDIRMODEL_H */
