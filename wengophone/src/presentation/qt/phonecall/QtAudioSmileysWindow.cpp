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
#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtAudioSmileysWindow.h"

#include <QtGui/QAbstractButton>
#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtGui/QDesktopWidget>
#include <QtGui/QVBoxLayout>

#include "../dialpad/QtDialpad.h"//VOXOX CHANGE by Rolando 04-29-09, now QtDialpad is in dialpad folder

QtAudioSmileysWindow::QtAudioSmileysWindow(CDtmfThemeManager& cDtmfThemeManager, QtWengoPhone* qtWengoPhone)
: QFrame(0, Qt::Popup)
, _button(0) {
	setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

	_dialpad = new QtDialpad(cDtmfThemeManager, qtWengoPhone);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(2);
	_dialpad->layout()->setMargin(0);
	layout->addWidget(_dialpad);
}


void QtAudioSmileysWindow::setButton(QAbstractButton* button) {
	_button = button;
}

void QtAudioSmileysWindow::showEvent(QShowEvent*) {
	if (!_button) {
		return;
	}

	QRect rect = QApplication::desktop()->screenGeometry(_button);
	int windowWidth = sizeHint().width();
	int windowHeight = sizeHint().height();

	QPoint point = _button->parentWidget()->mapToGlobal(_button->pos());

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

void QtAudioSmileysWindow::hideEvent(QHideEvent*) {
	// This is a bit hackish: the window hide itself when one click outside it
	// (because it has the Qt::Popup flag). In this case we want to uncheck its
	// associated button.  But we don't want to do so if the user clicked on
	// the button, otherwise events will get mixed and the window will
	// reappear.
	QPoint cursorPos = QCursor::pos();
	cursorPos = _button->mapFromGlobal(cursorPos);
	if (_button->isChecked() && !_button->rect().contains(cursorPos)) {
		_button->blockSignals(true);
		_button->setChecked(false);
		_button->blockSignals(false);
	}
}

QtDialpad* QtAudioSmileysWindow::dialpad() const {
	return _dialpad;
}
