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

#ifndef POSTEVENTFILTER_H
#define POSTEVENTFILTER_H

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <QtCore/QEvent>

/**
 * PostEvent for Qt.
 *
 * Used when sending a Qt event from another thread than the Qt thread.
 * This event permits thread safety. This has to be used with QObjectThreadSafe.
 *
 * @interface
 * @see QCoreApplication::postEvent()
 * @see QObjectThreadSafe
 * @author Tanguy Krotoff
 */
class PostEvent : public QEvent {
public:

	/** PostEvent is a QEvent with a specifi value. */
	static const int PostEventValue = QEvent::User + 1;

	PostEvent() : QEvent((QEvent::Type) PostEventValue) {
	}

	/** Calls the callback. */
	virtual void callback() = 0;
};

/**
 * PostEvent with 0 argument.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature>
class PostEvent0 : public PostEvent {
public:

	template<typename Callback>
	PostEvent0(const Callback & callback)
		: PostEvent(),
		_callback(callback) {
	}

	void callback() {
		_callback();
	}

private:

	/** Callback function. */
	boost::function<Signature> _callback;
};

/**
 * PostEvent with 1 argument.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature, typename Arg1>
class PostEvent1 : public PostEvent {
public:

	template<typename Callback>
	PostEvent1(const Callback & callback, Arg1 arg1)
		: PostEvent(),
		_callback(callback),
		_arg1(arg1) {
	}

	void callback() {
		_callback(_arg1);
	}

private:

	/** Callback function. */
	boost::function<Signature> _callback;

	Arg1 _arg1;
};

/**
 * PostEvent with 2 arguments.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature, typename Arg1, typename Arg2>
class PostEvent2 : public PostEvent {
public:

	template<typename Callback>
	PostEvent2(const Callback & callback, Arg1 arg1, Arg2 arg2)
		: PostEvent(),
		_callback(callback),
		_arg1(arg1),
		_arg2(arg2) {
	}

	void callback() {
		_callback(_arg1, _arg2);
	}

private:

	/** Callback function. */
	boost::function<Signature> _callback;

	Arg1 _arg1;

	Arg2 _arg2;
};

/**
 * PostEvent with 3 arguments.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature, typename Arg1, typename Arg2, typename Arg3>
class PostEvent3 : public PostEvent {
public:

	template<typename Callback>
	PostEvent3(const Callback & callback, Arg1 arg1, Arg2 arg2, Arg3 arg3)
		: PostEvent(),
		_callback(callback),
		_arg1(arg1),
		_arg2(arg2),
		_arg3(arg3) {
	}

	void callback() {
		_callback(_arg1, _arg2, _arg3);
	}

private:

	/** Callback function. */
	boost::function<Signature> _callback;

	Arg1 _arg1;

	Arg2 _arg2;

	Arg3 _arg3;
};

/**
 * PostEvent with 4 arguments.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
class PostEvent4 : public PostEvent {
public:

	template<typename Callback>
	PostEvent4(const Callback & callback, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
		: PostEvent(),
		_callback(callback),
		_arg1(arg1),
		_arg2(arg2),
		_arg3(arg3),
		_arg4(arg4) {
	}

	void callback() {
		_callback(_arg1, _arg2, _arg3, _arg4);
	}

private:

	/** Callback function. */
	boost::function<Signature> _callback;

	Arg1 _arg1;

	Arg2 _arg2;

	Arg3 _arg3;

	Arg4 _arg4;
};

#endif	//POSTEVENTFILTER_H
