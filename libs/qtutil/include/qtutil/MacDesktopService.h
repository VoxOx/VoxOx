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
#ifndef MACDESKTOPSERVICE_H
#define MACDESKTOPSERVICE_H

#include <qtutil/DesktopService.h>

/**
 * Implementation of @ref DesktopService using Mac native API
 */
class MacDesktopService : public DesktopService {
	virtual QPixmap pixmapForPath(const QString & file, int size);
	virtual QPixmap desktopIconPixmap(DesktopIconType iconType, int size);
	virtual QStringList startFolderList();
	virtual QString MacDesktopService::userFriendlyNameForPath(const QString & path);
};

#endif /* MACDESKTOPSERVICE_H */
