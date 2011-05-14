/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#include "qtutil/DropDownWindow.h"

#include <QtGui/QAbstractButton>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include <qtutil/SafeConnect.h>

#include "cutil/global.h"

DropDownWindow::DropDownWindow(QAbstractButton* button)
: QFrame(button, Qt::Popup | Qt::FramelessWindowHint)
, _button(button) {
	SAFE_CONNECT(_button, SIGNAL(toggled(bool)),
		SLOT(slotButtonToggled(bool)) );
}
	
void DropDownWindow::showEvent(QShowEvent*) {
	QRect rect = QApplication::desktop()->availableGeometry(_button);
	int windowWidth = sizeHint().width();
	int windowHeight = sizeHint().height();

	QPoint point = _button->mapToGlobal(QPoint(0, 0));

	// Make sure the dialog fit in the screen horizontally
	if (point.x() + windowWidth > rect.right()) {
		point.setX(point.x() + _button->width() - windowWidth);
	}

	// Make sure the dialog fit in the screen vertically
	if (point.y() + _button->height() + windowHeight > rect.bottom()) {
		point.setY(point.y() - windowHeight);
	} else {
		point.setY(point.y() + _button->height());
	}
	move(point);
}

void DropDownWindow::hideEvent(QHideEvent* event) {
	QFrame::hideEvent(event);
	// This is a bit hackish: the window hide itself when one click outside it
	// (because it has the Qt::Popup flag). In this case we want to uncheck its
	// associated button.

	// On MacOS X we must always uncheck the button ourself.
	// On Linux and Windows, we must not uncheck it ourself if the cursor is
	// over the button, because the button receives the click and will reopen
	// itself.

	if (!_button->isChecked()) {
		return;
	}

#ifndef OS_MACOSX
	// Note: Do not use _button->underMouse(), because it returns false if the
	// user has clicked on a widget inside the DropDownWindow instance.
	QPoint pos = _button->mapFromGlobal(QCursor::pos());
	bool underMouse = _button->rect().contains(pos);
	if (underMouse) {
		return;
	}
#endif

	_button->blockSignals(true);
	_button->setChecked(false);
	_button->blockSignals(false);
}

void DropDownWindow::slotButtonToggled(bool on) {
	if (on) {
		show();
	} else {
		hide();
	}
}

QAbstractButton* DropDownWindow::button() const {
	return _button;
}
