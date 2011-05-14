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

#include "SkinWidget.h"

#include "SkinInfoReader.h"

#include <qtutil/PaintEventFilter.h>
#include <qtutil/MouseEventFilter.h>
#include <qtutil/EnabledChangeEventFilter.h>

#include <qtutil/Object.h>

#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtGui/QBitmap>
#include <QtGui/QPainter>

#include <iostream>
using namespace std;

SkinWidget::SkinWidget(QWidget * widget) : QObject(widget) {
	_widget = widget;

	_regularPix = NULL;
	_activePix = NULL;
	_currentPix = NULL;
	_hoverPix = NULL;
	_disablePix = NULL;

	_xPos = SkinInfoReader::VALUE_ERROR;
	_yPos = SkinInfoReader::VALUE_ERROR;
}

void SkinWidget::setPosition(int x, int y) {
	if (x == SkinInfoReader::VALUE_ERROR || y == SkinInfoReader::VALUE_ERROR) {
		return;
	}

	_xPos = x;
	_yPos = y;
	_widget->move(_xPos, _yPos);
}

void SkinWidget::setSize(int width, int height) {
	if (width == SkinInfoReader::VALUE_ERROR && height == SkinInfoReader::VALUE_ERROR) {
		return;
	}

	if (width == SkinInfoReader::VALUE_ERROR) {
		width = _widget->width();
	}

	if (height == SkinInfoReader::VALUE_ERROR) {
		height = _widget->height();
	}

	_widget->resize(width, height);
}

void SkinWidget::setPixmaps(QPixmap * regular, QPixmap * active, QPixmap * hover, QPixmap * disable) {
	_regularPix = regular;
	_activePix = active;
	_hoverPix = hover;
	_disablePix = disable;

	//Sets _currentPix depending on if the widget is enabled or not
	enabledChangeEvent();

	//Mouse event filters
	_widget->installEventFilter(new MouseReleaseEventFilter(this, SLOT(mouseReleaseEvent())));
	_widget->installEventFilter(new MousePressEventFilter(this, SLOT(mousePressEvent())));

	//Enable mouse hover event
	_widget->setAttribute(Qt::WA_Hover, true);

	//Mouse hover filters
	_widget->installEventFilter(new MouseHoverLeaveEventFilter(this, SLOT(mouseHoverLeaveEvent())));
	if (!_hoverPix->isNull()) {
		_widget->installEventFilter(new MouseHoverEnterEventFilter(this, SLOT(mouseHoverEnterEvent())));
	}

	if (!_disablePix->isNull()) {
		//Enabled event filter
		_widget->installEventFilter(new EnabledChangeEventFilter(this, SLOT(enabledChangeEvent())));
	}

	if (!_regularPix->isNull()) {
		//Paint event filter
		_widget->installEventFilter(new PaintEventFilter(this, SLOT(paintEvent())));
	}

	if (_xPos != SkinInfoReader::VALUE_ERROR && _yPos != SkinInfoReader::VALUE_ERROR &&
		!_currentPix->isNull()) {
		//_widget->setGeometry(_xPos, _yPos, _currentPix->size().width(), _currentPix->size().height());
		_widget->move(_xPos, _yPos);
		_widget->resize(_currentPix->size().width(), _currentPix->size().height());
	}

	_widget->update();
}

void SkinWidget::hide() {
	_widget->hide();
}

void SkinWidget::setMask(QBitmap * pixmap) {
	if (!pixmap->isNull()) {
		//_widget->setMask(*pixmap);
	}
}

void SkinWidget::setBackgroundPixmap(QPixmap * pixmap) {
	if (pixmap->isNull()) {
		return;
	}

	QPalette palette;
	palette.setBrush(_widget->backgroundRole(), QBrush(*pixmap));
	_widget->setPalette(palette);
}

void SkinWidget::mousePressEvent() {
	cout << "mouse press event" << endl;
	if (_widget->isEnabled()) {
		_currentPix = _activePix;
	}
}

void SkinWidget::mouseReleaseEvent() {
	cout << "mouse release event" << endl;
	if (_widget->isEnabled()) {
		_currentPix = _regularPix;
	}
}

void SkinWidget::mouseHoverEnterEvent() {
	cout << "hover enter event" << endl;
	if (_widget->isEnabled()) {
		_currentPix = _hoverPix;
	}
}

void SkinWidget::mouseHoverLeaveEvent() {
	cout << "hover leave event" << endl;
	if (_widget->isEnabled()) {
		_currentPix = _regularPix;
	}
}

void SkinWidget::enabledChangeEvent() {
	cout << "enabled event" << endl;
	if (_widget->isEnabled()) {
		_currentPix = _regularPix;
	} else {
		_currentPix = _disablePix;
	}
}

void SkinWidget::paintEvent() {
	//cout << "paint event" << endl;
	if (!_currentPix->isNull()) {
		QPainter painter(_widget);
		QPixmap scaledPix = _currentPix->scaled(_widget->width(), _widget->height());
		painter.drawPixmap(0, 0, scaledPix);
	}
}
