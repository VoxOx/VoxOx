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
#ifndef QTPRESENCEPIXMAPHELPER_H
#define QTPRESENCEPIXMAPHELPER_H

#include <list>

#include <model/presence/PresenceHandler.h>

class IMAccountList;
class QPixmap;
class QString;

/**
 * This is a small helper class to generate status pixmaps for QtProfileBar and
 * QtSystray
 *
 * @author Aurelien Gateau
 */
class QtPresencePixmapHelper {
public:
	QtPresencePixmapHelper(const IMAccountList& accountList);

	/**
	 * Create a status pixmap according to the presence of the various accounts
	 * If the accounts are not all using the same presence, it will return a
	 * pixmap containing pies of the corresponding presence images.
	 *
	 * @param name: template for presence image names. %1 will be replaced
	 *   by the string representation of the presence.
	 * @param centerX: X coordinate of the center
	 * @param centerY: Y coordinate of the center
	 * @param radius: radius of the circle
	 */
	QPixmap createPixmap(const QString& name, int centerX, int centerY, int radius);

private:
	typedef std::list<EnumPresenceState::PresenceState> PresenceSummary;
	PresenceSummary _summary;

	int _centerX;
	int _centerY;
	int _radius;

	QPixmap compose(QPixmap pix1, QPixmap pix2, int startAngle, int stopAngle);
};


#endif /* QTPRESENCEPIXMAPHELPER_H */
