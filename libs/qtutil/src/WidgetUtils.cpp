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
#include <qtutil/WidgetUtils.h>

#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>

#include <cutil/global.h>

#if defined(OS_WINDOWS)
	#include <windows.h>
#elif defined (OS_LINUX)
	#include <X11/Xutil.h>
	#include <QtGui/QX11Info>
#elif defined (OS_MACOSX)
	#include <Notification.h>
#endif

namespace WidgetUtils {

int computeListViewMinimumWidth(QAbstractItemView* view) {
	int minWidth = 0;
	QAbstractItemModel* model = view->model();

	// Too bad view->viewOptions() is protected
	// FIXME: Handle the case where text is below icon
	QStyleOptionViewItem option;
	option.decorationSize = view->iconSize();

	int rowCount = model->rowCount();
	for (int row = 0; row<rowCount; ++row) {
		QModelIndex index = model->index(row, 0);
		QSize size = view->itemDelegate()->sizeHint(option, index);
		minWidth = qMax(size.width(), minWidth);
	}
	minWidth += 2 * view->frameWidth();
	return minWidth;
}

void flashWindow(QWidget* widget) {
#if defined(OS_WINDOWS)
	bool oneTopLevelVisible = false;
	Q_FOREACH(QWidget *widget, QApplication::topLevelWidgets()) {
		if (widget->isVisible()) {
			oneTopLevelVisible = true;
			break;
		}
	}
	FLASHWINFO flashInfo;
	flashInfo.cbSize = sizeof(FLASHWINFO);
	flashInfo.hwnd = widget->winId();
	flashInfo.uCount = 5;
	flashInfo.dwTimeout = 500;
	if (oneTopLevelVisible) {
		flashInfo.dwFlags = FLASHW_TRAY;
	} else {
		flashInfo.dwFlags = FLASHW_TRAY|FLASHW_TIMERNOFG;
	}
	FlashWindowEx(&flashInfo);
#elif defined(OS_LINUX)
	// This code comes from http://www.qtforum.org/article/12334/Taskbar-flashing.html
	Display *xdisplay = QX11Info::display();
	Window rootwin = QX11Info::appRootWindow();
	Window winId = widget->winId();

	static Atom demandsAttention = XInternAtom(xdisplay, "_NET_WM_STATE_DEMANDS_ATTENTION", true);
	static Atom wmState = XInternAtom(xdisplay, "_NET_WM_STATE", true);

	XEvent event;
	event.xclient.type = ClientMessage;
	event.xclient.message_type = wmState;
	event.xclient.display = xdisplay;
	event.xclient.window = winId;
	event.xclient.format = 32;
	event.xclient.data.l[0] = 1; // 1==on, 0==off
	event.xclient.data.l[1] = demandsAttention;
	event.xclient.data.l[2] = 0l;
	event.xclient.data.l[3] = 0l;
	event.xclient.data.l[4] = 0l;
	XSendEvent(xdisplay, rootwin, False, (SubstructureRedirectMask | SubstructureNotifyMask), &event);
#elif defined(OS_MACOSX)
	// This code comes from Qt 4.3,
	// file: src/gui/kernel/qapplication_mac.cpp
	// function: qt_mac_send_notification

	if (!QApplication::activeWindow()) { //VOXOX CHANGE by ASV 07-31-2009: fix icon bounce when any other window is active on Mac
		static NMRec mac_notification = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		mac_notification.nmMark = 1; //non-zero magic number
		mac_notification.qType = nmType;
		NMInstall(&mac_notification);
	}
#else
	// Unsupported platform
#endif
}

} // namespace
