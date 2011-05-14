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

#include <qtutil/WindowsDesktopService.h>

#include <QtCore/QDir.h>

#include <QtGui/QPainter.h>
#include <QtGui/QPixmap.h>

#include <shlobj.h>

// Maximum size of an icon in Windows file manager
static const int MAX_ICON_SIZE = 48;

static void appendSpecialFolder(QStringList& list, int csidl) {
	TCHAR buffer[MAX_PATH];
	HRESULT result = SHGetFolderPath(0 /* hwnd */, csidl, 0 /* hToken */, SHGFP_TYPE_CURRENT, buffer);
	if (!SUCCEEDED(result)) {
		return;
	}
	
	QString path;
	#ifdef UNICODE
	path = QString::fromUtf16((ushort*)buffer);
	#else
	path = QString::fromLocal8Bit(buffer);
	#endif
	
	list << path;
}

QStringList WindowsDesktopService::startFolderList() {
	QStringList list;

	// Add special folders
	QList<int> folderIDs;
	folderIDs
		<< CSIDL_DESKTOPDIRECTORY
		<< CSIDL_PERSONAL
		<< CSIDL_MYPICTURES
		<< CSIDL_NETWORK
	;

	Q_FOREACH(int id, folderIDs) {
		appendSpecialFolder(list, id);
	}

	// Add drives
	Q_FOREACH(QFileInfo fileInfo, QDir::drives()) {
		list << fileInfo.absoluteFilePath();
	}

	return list;
}

QString WindowsDesktopService::userFriendlyNameForPath(const QString & path) {
	QFileInfo info(path);
	QString name = info.fileName();
	if (name.isEmpty()) {
		name = info.absoluteFilePath();
	}
	return QDir::convertSeparators(name);
}

QPixmap WindowsDesktopService::desktopIconPixmap(DesktopIconType iconType, int size) {
	int iconSize = size;

	// Windows does not return icons bigger than MAX_ICON_SIZE
	if (iconSize > MAX_ICON_SIZE) {
		iconSize = MAX_ICON_SIZE;
	}
	QPixmap pix = DesktopService::desktopIconPixmap(iconType, iconSize);

	if (iconSize == size) {
		return pix;
	}

	QPixmap bigPix(size, size);
	bigPix.fill(QColor(0,0,0,0));
	QPainter painter(&bigPix);
	painter.drawPixmap((size - pix.width()) / 2, (size - pix.height()) / 2, pix);
	return bigPix;
}
