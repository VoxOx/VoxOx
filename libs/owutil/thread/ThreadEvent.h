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

#ifndef OWTHREADEVENT_H
#define OWTHREADEVENT_H

#include <thread/owthreaddll.h>

#include <util/Interface.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

/**
 * Event for Thread.
 *
 * Used when sending an event from a thread to another.
 *
 * @interface
 * @see Thread
 * @author Tanguy Krotoff
 */
class IThreadEvent : Interface {
public:

	virtual ~IThreadEvent() {
	}

	/** Calls the callback. */
	virtual void callback() = 0;
};

/**
 * ThreadEvent with 0 argument.
 *
 * The callback inside ThreadEvent has 0 argument.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature>
class ThreadEvent0 : public IThreadEvent {
public:

	template<typename Callback>
	ThreadEvent0(const Callback & callback)
		: IThreadEvent(),
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
 * ThreadEvent with 1 argument.
 *
 * The callback inside ThreadEvent has 1 argument.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature, typename Arg1>
class ThreadEvent1 : public IThreadEvent {
public:

	template<typename Callback>
	ThreadEvent1(const Callback & callback, Arg1 arg1)
		: IThreadEvent(),
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
 * ThreadEvent with 2 arguments.
 *
 * The callback inside ThreadEvent has 2 arguments.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature, typename Arg1, typename Arg2>
class ThreadEvent2 : public IThreadEvent {
public:

	template<typename Callback>
	ThreadEvent2(const Callback & callback, Arg1 arg1, Arg2 arg2)
		: IThreadEvent(),
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
 * ThreadEvent with 3 arguments.
 *
 * The callback inside ThreadEvent has 3 arguments.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature, typename Arg1, typename Arg2, typename Arg3>
class ThreadEvent3 : public IThreadEvent {
public:

	template<typename Callback>
	ThreadEvent3(const Callback & callback, Arg1 arg1, Arg2 arg2, Arg3 arg3)
		: IThreadEvent(),
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
 * ThreadEvent with 4 arguments.
 *
 * The callback inside ThreadEvent has 4 arguments.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
class ThreadEvent4 : public IThreadEvent {
public:

	template<typename Callback>
	ThreadEvent4(const Callback & callback, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
		: IThreadEvent(),
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

/**
 * ThreadEvent with 5 arguments.
 *
 * The callback inside ThreadEvent has 5 arguments.
 *
 * @author Tanguy Krotoff
 */
template<typename Signature, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
class ThreadEvent5 : public IThreadEvent {
public:

	template<typename Callback>
	ThreadEvent5(const Callback & callback, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
		: IThreadEvent(),
		_callback(callback),
		_arg1(arg1),
		_arg2(arg2),
		_arg3(arg3),
		_arg4(arg4),
		_arg5(arg5) {
	}

	void callback() {
		_callback(_arg1, _arg2, _arg3, _arg4, _arg5);
	}

private:

	/** Callback function. */
	boost::function<Signature> _callback;

	Arg1 _arg1;

	Arg2 _arg2;

	Arg3 _arg3;

	Arg4 _arg4;

	Arg5 _arg5;
};

#endif	//OWTHREADEVENT_H
