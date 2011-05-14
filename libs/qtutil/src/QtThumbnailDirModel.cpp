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
#include "QtThumbnailDirModel.h"

#include <QtGui/QImageReader>
#include <QtGui/QPixmap>
#include <QtCore/QTimer>


#include <qtutil/DesktopService.h>
#include <qtutil/SafeConnect.h>

#include "QtThumbnailManager.h"

QtThumbnailDirModel::QtThumbnailDirModel(QObject* parent)
	: QAbstractListModel(parent)
{
	_dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	_dir.setSorting(QDir::Name | QDir::DirsFirst | QDir::LocaleAware | QDir::IgnoreCase);

	_thumbnailManager = new QtThumbnailManager(this);
	
	Q_FOREACH(QByteArray format, QImageReader::supportedImageFormats()) {
		QString suffix(format);
		suffix = suffix.toLower();
		_imageSuffixList.append(suffix);
	}
	
	SAFE_CONNECT(_thumbnailManager, SIGNAL(thumbnailUpdated(const QFileInfo&)),
		SLOT(slotPixmapUpdated(const QFileInfo&)) );
}


void QtThumbnailDirModel::setPixmapSize(int size) {
	_thumbnailManager->setSize(size);
}


void QtThumbnailDirModel::setDir(const QString& dir) {
	_thumbnailManager->clear();
	_dir.setPath(dir);
	refresh();
}


QString QtThumbnailDirModel::dir() const {
	return _dir.absolutePath();
}


QFileInfo QtThumbnailDirModel::fileInfo(const QModelIndex& index) const {
	int row = index.row();
	int column = index.column();
	if (column > 0) {
		return QFileInfo();
	}

	if (row >= _infoList.size()) {
		return QFileInfo();
	}
	return _infoList[row];
}


int QtThumbnailDirModel::rowCount(const QModelIndex& /*parent*/) const {
	return _infoList.size();
}


QVariant QtThumbnailDirModel::data(const QModelIndex& index, int role) const {
	int row = index.row();
	int column = index.column();
	if (column > 0) {
		return QVariant();
	}

	if (row >= _infoList.size()) {
		return QVariant();
	}

	if (role == Qt::DisplayRole) {
		QString path = _infoList[row].absoluteFilePath();
		return DesktopService::getInstance()->userFriendlyNameForPath(path);
		
	} else if (role == Qt::DecorationRole) {
		return _thumbnailManager->getThumbnail(_infoList[row]);
	}
	
	return QVariant();
}


void QtThumbnailDirModel::refresh() {
	QFileInfoList infoList = _dir.entryInfoList();
	_infoList.clear();
	Q_FOREACH(QFileInfo fileInfo, infoList) {
		// Only add dirs and images
		QString suffix = fileInfo.suffix().toLower();
		if (fileInfo.isDir() || _imageSuffixList.contains(suffix)) {
			if (fileInfo.isSymLink()) {
				// Store the real file in fileInfo
				QString path = fileInfo.readLink();
				fileInfo = QFileInfo(path);
			}
			_infoList << fileInfo;
		}
	}
	reset();
}


void QtThumbnailDirModel::slotPixmapUpdated(const QFileInfo& updatedFileInfo) {
	int row = 0;
	Q_FOREACH(QFileInfo fileInfo, _infoList) {
		if (fileInfo == updatedFileInfo) {
			QModelIndex itemIndex = index(row);
			dataChanged(itemIndex, itemIndex);	
			break;
		}
		++row;
	}
}
