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

#ifndef OWEVENTFILTER_H
#define OWEVENTFILTER_H

#include <qtutil/owqtutildll.h>

#include <util/NonCopyable.h>

#include <QtCore/QObject>

class QEvent;

/**
 * EventFilter for QObject.
 *
 * Permits to make some special actions on Qt events.
 * Example:
 * <code>
 * QMainWindow * widget = new QMainWindow();
 * CloseEventFilter * closeFilter = new CloseEventFilter(this, SLOT(printHelloWorld()));
 * ResizeEventFilter * resizeFilter = new ResizeEventFilter(this, SLOT(printHelloWorld()));
 * widget->installEventFilter(closeFilter);
 * widget->installEventFilter(resizeFilter);
 * </code>
 *
 * @author Tanguy Krotoff
 */
class OWQTUTIL_API EventFilter : public QObject, NonCopyable {
	Q_OBJECT
public:

	/**
	 * Filters an event.
	 *
	 * @param receiver object receiver of the filter signal
	 * @param member member of the object called by the filter signal
	 * @param watched watched object the filter is going to be applied on
	 */
	EventFilter(QObject * receiver, const char * member);

protected:

	/**
	 * Emits the filter signal.
	 *
	 * @param event event filtered
	 */
	void filter(QEvent * event);

	/**
	 * Filters the event.
	 *
	 * @param watched watched object
	 * @param event event filtered of the watched object
	 * @return true then stops the event being handled further
	 */
	virtual bool eventFilter(QObject * watched, QEvent * event);

Q_SIGNALS:

	void activate(QEvent * event);
};

#endif	//OWEVENTFILTER_H
