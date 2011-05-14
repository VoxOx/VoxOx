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

#ifndef OWQTTHREADEVENT_H
#define OWQTTHREADEVENT_H

#include <qtutil/owqtutildll.h>

#include <thread/ThreadEvent.h>

#include <QtCore/QEvent>

/**
 * ThreadEvent for Qt.
 *
 * Used when sending a Qt event from another thread than the Qt thread.
 * This event permits thread safety.
 *
 * @see QCoreApplication::postEvent()
 * @author Tanguy Krotoff
 */
class OWQTUTIL_API QtThreadEvent : public QEvent, public IThreadEvent {
public:

	/** QtThreadEvent is a QEvent with a specific value. */
	static const int EventValue = QEvent::User + 4;

	QtThreadEvent(IThreadEvent * event)
		: QEvent((QEvent::Type) EventValue),
		IThreadEvent() {

		_threadEventPrivate = event;
	}

	~QtThreadEvent() {
		delete _threadEventPrivate;
		_threadEventPrivate = NULL;
	}

	void callback() {
		_threadEventPrivate->callback();
	}

private:

	IThreadEvent * _threadEventPrivate;
};

#endif	//OWQTTHREADEVENT_H
