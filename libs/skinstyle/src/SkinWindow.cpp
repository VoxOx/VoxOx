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

#include "SkinWindow.h"

#include <qtutil/MouseEventFilter.h>

#include <QtGui/QWidget>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtGui/QBrush>
#include <QtGui/QMouseEvent>

#include <iostream>
using namespace std;

SkinWindow::SkinWindow(QWidget * window) : SkinWidget(window) {
	_window = window;

	//Transparency
	//_window->setWindowOpacity(0.8);

	//Window without border
	//_window->setWindowFlags(Qt::FramelessWindowHint);

	//Mouse event filters
	_window->installEventFilter(new MouseMoveEventFilter(this, SLOT(mouseMoveEvent(QEvent *))));
	_window->installEventFilter(new MousePressEventFilter(this, SLOT(mousePressEvent(QEvent *))));
}

void SkinWindow::setBackgroundPixmap(QPixmap * pixmap) {
	if (pixmap->isNull()) {
		return;
	}

	SkinWidget::setBackgroundPixmap(pixmap);

	//Resize the widget to the size of the background pixmap
	_window->setMinimumWidth(pixmap->size().width());
	_window->setMaximumWidth(pixmap->size().width());
	_window->setMinimumHeight(pixmap->size().height());
	_window->setMaximumHeight(pixmap->size().height());
	_window->resize(pixmap->size().width(), pixmap->size().height());
}

void SkinWindow::mouseMoveEvent(QEvent * event) {
	QMouseEvent * e = (QMouseEvent *) event;

	//New absolute mouse cursor position
	QPoint newMousePos = e->globalPos();

	//Calculates the window top left corner position as the
	//difference between the new and initial mouse cursor positions
	QPoint upLeft = _initialWindowPos + newMousePos - _initialMousePos;

	_window->move(upLeft);
}

void SkinWindow::mousePressEvent(QEvent * event) {
	QMouseEvent * e = (QMouseEvent *) event;

	//Initial mouse cursor absolute position
	_initialMousePos = e->globalPos();

	//Initial position of the window left corner
	_initialWindowPos = _window->pos();
}
