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

/****************************************************************************
** Filename: LINKLabel.h
** Last updated [dd/mm/yyyy]: 14/02/2005
**
** QLabel subclass with LINK handling and more.
**
** Copyright(C) 2005 Angius Fabrizio. All rights reserved.
**
** Based on the LGPL v.2 licensed KLINKLabel from the KDE libraries by
** Kurt Granroth <granroth@kde.org> and Peter Putzer <putzer@kde.org>
**
** Changes made to the KLINKLabel code:
**  - link color is no longer taken from KGlobalSettings but from qApp->palette().active().link()
**  - removed virtual_hook() member function
**  - replaced KCursor::handCursor() with QCursor(Qt::PointingHandCursor)
**  - added context menu (see mouseReleaseEvent method)
**
** This file is part of the OSDaB project(http://osdab.sourceforge.net/).
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See the file LICENSE.GPL that came with this software distribution or
** visit http://www.gnu.org/copyleft/gpl.html for GPL licensing information.
**
**********************************************************************/

#ifndef VOXOXCLICKABLELABEL_H
#define VOXOXCLICKABLELABEL_H

#include <QtGui/QLabel>

/**
 * This class provides an hyperlink-like QLabel. It should be removed when we
 * make the move to Qt 4.2, since QLabel 4.2 provides all the
 * necessary functionality (and more).
 *
 * This code is based on URLLabel from the OSDaB project
 * (http://osdab.sourceforge.net)
 * Copyright(C) 2005 Angius Fabrizio. All rights reserved.
 *
 * URLLabel is based on the KURLLabel class from the KDE libraries.
 * by Kurt Granroth <granroth@kde.org> and Peter Putzer <putzer@kde.org>
 *
 * URLLabel has been adapted to provide a similar interface to QLabel 4.2 (at
 * least as far as signals are concerned), to reduce the amount of work
 * necessary when VoxOxClickableLabel get replaced with QLabel 4.2
 *
 * @author Aurelien Gateau
 */
class VoxOxClickableLabel : public QLabel
{
	Q_OBJECT
public:
	VoxOxClickableLabel(QWidget* parent = 0);


Q_SIGNALS:

	/**
	 * Emitted whenever the user clicks on the label
	 */
	void clicked();	

protected:
	virtual void mouseReleaseEvent(QMouseEvent*);
};

#endif // VOXOXCLICKABLELABEL_H
