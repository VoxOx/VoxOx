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
#include <qtutil/DesktopService.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QtGlobal>

#include <QtGui/QPixmap>

#include <cutil/global.h>

#if defined OS_MACOSX
#include <qtutil/MacDesktopService.h>
#elif defined OS_WINDOWS
#include <qtutil/WindowsDesktopService.h>
#endif


DesktopService::DesktopService() {
}


DesktopService::~DesktopService() {
}


DesktopService* DesktopService::getInstance() {
	static DesktopService* service = 0;
	if (!service) {
		if (qgetenv("OW_DESKTOP_USE_GENERIC").isNull()) {
#if defined OS_MACOSX
			service = new MacDesktopService();
#elif defined OS_WINDOWS
			service = new WindowsDesktopService();
#else
			service = new DesktopService();
#endif
		} else {
			service = new DesktopService();
		}
	}

	return service;
}


QPixmap DesktopService::pixmapForPath(const QString& path, int size) {
	QFileInfo fileInfo(path);
	if (fileInfo.isDir()) {
		return desktopIconPixmap(FolderIcon, size);
	} else {
		return desktopIconPixmap(FileIcon, size);
	}
}


QPixmap DesktopService::desktopIconPixmap(DesktopIconType icon, int size) {
	QString name;
	switch (icon) {
	case FolderIcon:
		name = "folder.png";
		break;
	case FileIcon:
		name = "file.png";
		break;
	}

	QString sizeDir;
	if (size <= 16) {
		sizeDir = "16x16";
	} else if (size <= 32) {
		sizeDir = "32x32";
	} else if (size <= 64) {
		sizeDir = "64x64";
	} else {
		sizeDir = "128x128";
	}

	QPixmap pix(":/pics/desktopservice/" + sizeDir + "/" + name);
	Q_ASSERT(!pix.isNull());
	return pix.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}


QStringList DesktopService::startFolderList() {
	QDir home = QDir::home();

	QStringList list;
	list << home.absolutePath();

	QStringList candidates;
	candidates << "Desktop" << "Pictures" << "Documents";

	Q_FOREACH(QString folder, candidates) {
		if (home.exists(folder)) {
			list << home.absoluteFilePath(folder);
		}
	}

	QFileInfoList driveList = QDir::drives();
	Q_FOREACH(QFileInfo drive, driveList) {
		list << drive.absoluteFilePath();
	}

	return list;
}


QString DesktopService::userFriendlyNameForPath(const QString & path) {
	if (path == "/") {
		// FIXME: This should move to UnixDesktopService when this function is
		// implemented in WindowsDesktopService and MacDesktopService
		return QCoreApplication::instance()->translate("DesktopService", "Root Folder");
	}
	QFileInfo info(path);
	return info.fileName();
}
