/* VOXOX CHANGE for VoxOx by Rolando 01-09-09
 * Included the library QtGui to be able send by parameter QPixmap in the method named mergeFromPixmap
 * OWQTUTIL_API static QPixmap mergeFromPixmap(const std::string & foregroundPixmapFilename, const std::string & backgroundPixmapFilename);
 * OWQTUTIL_API static QPixmap mergeFromPixmap(QPixmap foregroundPixmap,QPixmap backgroundPixmap, QPoint offset);
 * OWQTUTIL_API static QPixmap mergeFromPixmap(QPixmap foregroundPixmap,QPixmap backgroundPixmap);
 */


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

#ifndef OWPIXMAPMERGING_H
#define OWPIXMAPMERGING_H

#include <qtutil/owqtutildll.h>
//VOXOX CHANGE for VoxOx by Rolando 01-09-09 added library QtGui
#include <QtGui/QtGui>
#include <util/NonCopyable.h>

#include <string>

class QPixmap;

/**
 * Merges 2 pixmaps together.
 *
 * @author Tanguy Krotoff
 */
class PixmapMerging : NonCopyable {
public:

	/**
	 * Merges 2 pixmaps together.
	 *
	 * Used to have a background picture (a cadre) and a picture inside it.
	 * Currently used to a the avatar of a contact inside a nice background.
	 *
	 * @param foregroundPixmapData an 'array' with the foreground picture data
	 * @param backgroundPixmapFilename background pixmap filename e.g ":/pics/avatar_background.png"
	 */
	OWQTUTIL_API static QPixmap merge(const std::string & foregroundPixmapData, const std::string & backgroundPixmapFilename, Qt::AspectRatioMode foregroundAspectRadio = Qt::KeepAspectRatioByExpanding, Qt::Alignment horizontalAlignment = Qt::AlignHCenter, Qt::Alignment verticalAlignment = Qt::AlignVCenter);//VOXOX CHANGE by Rolando - 2009.06.27 
//VOXOX CHANGE for VoxOx by Rolando 01-09-09 added mergeFromPixmap
	OWQTUTIL_API static QPixmap mergeFromPixmap(const std::string & foregroundPixmapFilename, const std::string & backgroundPixmapFilename, Qt::AspectRatioMode foregroundAspectRadio = Qt::KeepAspectRatioByExpanding, Qt::Alignment horizontalAlignment = Qt::AlignHCenter, Qt::Alignment verticalAlignment = Qt::AlignVCenter);//VOXOX CHANGE by Rolando - 2009.06.27 
//VOXOX CHANGE for VoxOx by Rolando 01-09-09 added mergeFromPixmap
	OWQTUTIL_API static QPixmap mergeFromPixmap(QPixmap foregroundPixmap,QPixmap backgroundPixmap, QPoint offset);
//VOXOX CHANGE Rolando 03-25-09
	OWQTUTIL_API static QPixmap concatenatePixmaps(QPixmap leftPixmap,QPixmap rightPixmap, QSize finalPixmapSize, QPoint offsetRightPixmap, QPoint offsetLeftPixmap = QPoint(0,0));
//VOXOX CHANGE Rolando 03-25-09
	OWQTUTIL_API static QPixmap concatenatePixmaps(QPixmap leftPixmap,QPixmap rightPixmap, QSize finalPixmapSize);

//VOXOX CHANGE for VoxOx by Rolando 01-09-09 added mergeFromPixmap
	OWQTUTIL_API static QPixmap mergeFromPixmap(QPixmap foregroundPixmap,QPixmap backgroundPixmap, Qt::AspectRatioMode foregroundAspectRadio = Qt::KeepAspectRatioByExpanding, Qt::Alignment horizontalAlignment = Qt::AlignHCenter, Qt::Alignment verticalAlignment = Qt::AlignVCenter);//VOXOX CHANGE by Rolando - 2009.06.27 
};

#endif	//OWPIXMAPMERGING_H
