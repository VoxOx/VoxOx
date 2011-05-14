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
#ifndef DESKTOPSERVICE_H
#define DESKTOPSERVICE_H

class QPixmap;
class QString;
class QStringList;


/**
 * This class provides facilities to integrate with the user desktop.
 * It provides methods to get desktop icons and should soon provide other
 * features.
 *
 * By default it will try to use the native implementation if available (@ref
 * MacDesktopService on MacOSX for example). If you want to test the generic
 * version, define the OW_DESKTOP_USE_GENERIC environment variable.
 * 
 * This class implements the Singleton pattern.
 */
class DesktopService {
public:
	virtual ~DesktopService();

	enum DesktopIconType {
		FolderIcon,
		FileIcon
	};

	/**
	 * Returns a pixmap for the given path
	 * @p path the path we want a pixmap for
	 * @p size the size of the pixmap in pixels
	 */
	virtual QPixmap pixmapForPath(const QString & path, int size);

	/**
	 * Returns a pixmap for a desktop icon
	 * @p iconType the wanted icon
	 * @p size the size of the pixmap in pixels
	 */
	virtual QPixmap desktopIconPixmap(DesktopIconType iconType, int size);

	/**
	 * Returns a list of start folders.
	 * This is usefull to provide starting points in file dialogs.
	 */
	virtual QStringList startFolderList();

	/**
	 * Returns a user friendly name for a path
	 */
	virtual QString userFriendlyNameForPath(const QString & path);

	/**
	 * Returns an instance of the DesktopService.
	 */
	static DesktopService* getInstance();

protected:
	DesktopService();
};

#endif /* DESKTOPSERVICE_H */
