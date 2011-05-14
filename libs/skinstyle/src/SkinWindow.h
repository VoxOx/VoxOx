/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef SKINWINDOW_H
#define SKINWINDOW_H

#include "SkinWidget.h"

#include <QtCore/QObject>
#include <QtCore/QPoint>

class QWidget;
class QEvent;
class QPixmap;

/**
 * A skinned independant window.
 *
 * @author Tanguy Krotoff
 */
class SkinWindow : public SkinWidget {
	Q_OBJECT
public:

	SkinWindow(QWidget * window);

	virtual void setBackgroundPixmap(QPixmap * pixmap);

private Q_SLOTS:

	void mouseMoveEvent(QEvent * event);

	void mousePressEvent(QEvent * event);

private:

	QWidget * _window;

	/** Initial mouse cursor absolute position. */
	QPoint _initialMousePos;

	/** Initial position of the window left corner. */
	QPoint _initialWindowPos;
};

#endif	//SKINWINDOW_H
