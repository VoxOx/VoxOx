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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtToaster.h"

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/WidgetBackgroundImage.h>

#include <QtGui/QtGui>

#ifdef OS_MACOSX
#include <MacWindows.h>
#endif

static const unsigned TIME_TO_SHOW = 20;

QtToaster::QtToaster(QWidget * toaster, QFrame * toasterWindowFrame)
	: QObject(toaster) {

	_timer = NULL;
	_show = true;

	_toaster = toaster;
	
	QString backgroundImagePath = QString(":pics/toaster/toaster-background.png");
#ifdef OS_MACOSX
	_toaster->setParent(_toaster->parentWidget(), Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	// Make sure the other windows of the application are not raised when the
	// toaster appears
	WId wid = _toaster->winId();
	SetWindowGroup((WindowRef)wid, NULL);
#else
	_toaster->setParent(_toaster->parentWidget(), Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
#endif

	
	QPixmap p = QPixmap(backgroundImagePath);
	_toaster->setMask( p.mask() );
	toasterWindowFrame->setStyleSheet("QFrame{background-color:#0c0b0b;}");

}

void QtToaster::setTimeOnTop(unsigned time) {
	_timeOnTop = time;
}

void QtToaster::close() {
	if (_timer) {
		_timer->stop();
	}
	_toaster->close();
}

void QtToaster::show() {
	//10 pixels of margin
#if !defined(OS_WINDOWS)
	static const int MARGIN_X = 30;
#else
	static const int MARGIN_X = 10;
#endif

	QDesktopWidget * desktop = QApplication::desktop();
	QRect screenGeometry = desktop->screenGeometry(desktop->primaryScreen());

	_toaster->move(screenGeometry.right() - _toaster->size().width() - MARGIN_X, screenGeometry.bottom());

	_toaster->show();

	_timer = new QTimer(this);
	SAFE_CONNECT(_timer, SIGNAL(timeout()), SLOT(changeToasterPosition()));
	_timer->start(TIME_TO_SHOW);
}

void QtToaster::changeToasterPosition() {
	QDesktopWidget * desktop = QApplication::desktop();
	QPoint p = _toaster->pos();

	//Toaster is showing slowly
	if (_show) {
		_toaster->move(p.x(), p.y() - 3);

		QRect desktopGeometry = desktop->availableGeometry(desktop->primaryScreen());

		if (p.y() < (desktopGeometry.bottom() - _toaster->size().height() - 5)) {
			//Toaster should be hidden now
			_show = false;
			_timer->stop();
			//Waits 5 seconds with the toaster on top
			_timer->start(_timeOnTop);
		}
	}

	//Toaster is hiding slowly
	else {
		_toaster->move(p.x(), p.y() + 3);
		_toaster->repaint();
		QRect screenGeometry = desktop->screenGeometry(desktop->primaryScreen());

		_timer->stop();
		_timer->start(TIME_TO_SHOW);

		if (p.y() > (screenGeometry.bottom())) {
			//Closes the toaster -> hide it completely
			close();
		}
	}
}
