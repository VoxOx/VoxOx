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

#ifndef QOBJECTTHREADSAFE_H
#define QOBJECTTHREADSAFE_H

#include <qtutil/owqtutildll.h>
#include <qtutil/PostEvent.h>

#include <QtCore/QObject>

#include <string>

/**
 * For thread safety between the model thread and the Qt thread.
 *
 * Warning: do not inherit from QObjectThreadSafe and QObject
 * (or other classes like QWidget, QMainWindow...).
 *
 * @see PostEvent
 * @see QCoreApplication::postEvent()
 * @author Tanguy Krotoff
 */
class OWQTUTIL_API QObjectThreadSafe : public QObject {
	Q_OBJECT
public:

	QObjectThreadSafe(QObject * parent);

	virtual ~QObjectThreadSafe();

	/**
	 * Initialization of the QObjectThreadSafe subclass object.
	 *
	 * Inside the QObjectThreadSafe subclass,
	 * this code should be written:
	 * <pre>
	 * typedef PostEvent0<void ()> MyPostEvent;
	 * MyPostEvent * event = new MyPostEvent(boost::bind(&__SUBCLASS__::initThreadSafe, this));
	 * postEvent(event);
	 * </pre>
	 * __SUBCLASS__ should be replaced by the QObjectThreadSafe subclass name.
	 */
	virtual void initThreadSafe() = 0;

	/**
	 * Posts via QCoreApplication::postEvent() the specific QEvent.
	 *
	 * @param event PostEvent to post
	 */
	void postEvent(PostEvent * event);

	/**
	 * Blocks/unblocks the reception of events send via postEvent().
	 *
	 * @param block true then block events; if false unblock events
	 */
	void blockEvents(bool block);

private Q_SLOTS:

	/**
	 * Receives QObject events.
	 *
	 * This method filters the even PostEvent and calls the callback
	 * specified inside PostEvent.
	 *
	 * @param event event to process
	 * @return true if the event was recognized and processed
	 */
	bool event(QEvent * event);

private:

	/** Blocks/unblocks the reception of events send via postEvent(). */
	bool _blockEvents;
};

#endif	//QOBJECTTHREADSAFE_H
