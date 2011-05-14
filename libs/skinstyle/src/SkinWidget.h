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

#ifndef SKINWIDGET_H
#define SKINWIDGET_H

#include <QtCore/QObject>

class QWidget;
class QPixmap;
class QBitmap;

/**
 * A skinned widget.
 *
 * Two pixmaps are associated with the widget:
 * - one for its regular normal state (mouse is released)
 * - one for its activated state (mouse is pressed)
 *
 * This skinned widget is generic and can be used for buttons and other widgets
 * that have two states (regular and activated).
 *
 * @see http://www.codeskipper.com/article/1/
 * @author Tanguy Krotoff
 */
class SkinWidget : public QObject {
	Q_OBJECT
public:

	SkinWidget(QWidget * widget);

	QWidget * getWidget() const {
		return _widget;
	}

	/**
	 * Sets the position of the widget within its parent widget.
	 *
	 * @param x X position
	 * @param y Y position
	 */
	virtual void setPosition(int x, int y);

	virtual void setSize(int width, int height);

	/**
	 * Sets the pixmaps associated with the widget.
	 *
	 * @param regular pixmap showed when the widget is in its regular (button released) state
	 * @param active pixmap showed when the widget is in its actived (button pressed) state
	 */
	virtual void setPixmaps(QPixmap * regular, QPixmap * active, QPixmap * hover, QPixmap * disable);

	virtual void setMask(QBitmap * pixmap);

	virtual void setBackgroundPixmap(QPixmap * pixmap);

	virtual void hide();

protected Q_SLOTS:

	void mousePressEvent();

	void mouseReleaseEvent();

	void mouseHoverEnterEvent();

	void mouseHoverLeaveEvent();

	void enabledChangeEvent();

	virtual void paintEvent();

protected:

	QWidget * _widget;

	QPixmap * _regularPix;

	QPixmap * _activePix;

	QPixmap * _currentPix;

	QPixmap * _hoverPix;

	QPixmap * _disablePix;

	/** X position of the widget within its parent widget. */
	int _xPos;

	/** Y position of the widget within its parent widget. */
	int _yPos;
};

#endif	//SKINWIDGET_H
