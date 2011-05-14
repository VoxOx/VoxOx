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

#include <qtutil/MouseEventFilter.h>

#include <util/Logger.h>

#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>

#include <exception>

MouseMoveEventFilter::MouseMoveEventFilter(QObject * receiver, const char * member)
	: EventFilter(receiver, member) {
}

bool MouseMoveEventFilter::eventFilter(QObject * watched, QEvent * event) {
	if (event->type() == QEvent::MouseMove) {
		filter(event);
		return false;
	}
	return EventFilter::eventFilter(watched, event);
}

MousePressEventFilter::MousePressEventFilter(QObject * receiver, const char * member, Qt::MouseButton button)
	: EventFilter(receiver, member),
	_button(button) {
}

bool MousePressEventFilter::eventFilter(QObject * watched, QEvent * event) {
	if (event->type() == QEvent::MouseButtonPress) {
		try {
			QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);

			if ((_button == Qt::NoButton) || (mouseEvent->button() == _button)) {
				filter(event);
				return false;
			}
		} catch (std::bad_cast) {
			LOG_FATAL("exception when casting a QEvent to a QMouseEvent");
		}
	}
	return EventFilter::eventFilter(watched, event);
}

MouseReleaseEventFilter::MouseReleaseEventFilter(QObject * receiver, const char * member, Qt::MouseButton button)
	: EventFilter(receiver, member),
	_button(button) {
}

bool MouseReleaseEventFilter::eventFilter(QObject * watched, QEvent * event) {
	if (event->type() == QEvent::MouseButtonRelease) {
		try {
			QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);

			if ((_button == Qt::NoButton) || (mouseEvent->button() == _button)) {
				filter(event);
				return false;
			}
		} catch (std::bad_cast) {
			LOG_FATAL("exception when casting a QEvent to a QMouseEvent");
		}
	}
	return EventFilter::eventFilter(watched, event);
}

MouseHoverEnterEventFilter::MouseHoverEnterEventFilter(QObject * receiver, const char * member)
	: EventFilter(receiver, member) {
}

bool MouseHoverEnterEventFilter::eventFilter(QObject * watched, QEvent * event) {
	if (event->type() == QEvent::HoverEnter) {
		filter(event);
		return false;
	}
	return EventFilter::eventFilter(watched, event);
}

MouseHoverLeaveEventFilter::MouseHoverLeaveEventFilter(QObject * receiver, const char * member)
	: EventFilter(receiver, member) {
}

bool MouseHoverLeaveEventFilter::eventFilter(QObject * watched, QEvent * event) {
	if (event->type() == QEvent::HoverLeave) {
		filter(event);
		return false;
	}
	return EventFilter::eventFilter(watched, event);
}
