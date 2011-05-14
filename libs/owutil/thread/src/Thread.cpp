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

#include <thread/Thread.h>

#include <thread/ThreadEvent.h>

#include <cutil/global.h>
#include <util/SafeDelete.h>

#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

#ifdef OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

Thread::Thread() {
	_terminate = false;
	_threadRunning = false;
	_thread = NULL;
	_autoDelete = false;
}

Thread::~Thread() {
	Mutex::ScopedLock scopedLock(_threadMutex);

	if (_thread && _threadRunning) {
		scopedLock.unlock();
		terminate();
		join();
	}

	OWSAFE_DELETE(_thread);
}

void Thread::start() {
	// If _autoDelete is set to true, it is possible that the thread complete
	// *before* returning from new (!). If we don't lock _threadMutex, 'this'
	// will be deleted before we write in '_thread'.
	Mutex::ScopedLock scopedLock(_threadMutex);
	_terminate = false;
	OWSAFE_DELETE(_thread);
	_thread = new boost::thread(boost::bind(&Thread::runThread, this));
}

void Thread::runThread() {
	{
		Mutex::ScopedLock scopedLock(_threadMutex);
		_threadRunning = true;
	}

	run();

	bool autoDelete;
	{
		Mutex::ScopedLock scopedLock(_threadMutex);
		_threadRunning = false;
		autoDelete = _autoDelete;
	}

	if (autoDelete) {
		delete this;
	}
}

void Thread::join() {
	Mutex::ScopedLock scopedLock(_threadMutex);

	if (_threadRunning) {
		scopedLock.unlock();
		_thread->join();
	}
}

void Thread::postEvent(IThreadEvent * event) {
	Mutex::ScopedLock scopedLock(_threadMutex);
	_eventQueue.push(event);
	scopedLock.unlock();

	_threadCondition.notify_all();
}

void Thread::sleep(unsigned long seconds) {
	if (seconds == 0) {
		boost::thread::yield();
		return;
	}

	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC);
	xt.sec += seconds;
	boost::thread::sleep(xt);
}

void Thread::msleep(unsigned long milliseconds) {
#ifdef OS_POSIX
	usleep(milliseconds * 1000);
#elif defined(OS_WINDOWS)
	Sleep(milliseconds);
#else
	#error Platform not supported
#endif
}

void Thread::runEvents() {
	Mutex::ScopedLock scopedLock(_threadMutex);

	while (true) {
		while (!_eventQueue.empty()) {
			IThreadEvent *event = _eventQueue.front();
			_eventQueue.pop();

			scopedLock.unlock();
			event->callback();
			OWSAFE_DELETE(event);
			scopedLock.lock();
		}

		if (_terminate) {
			return;
		}

		_threadCondition.wait(scopedLock);
	}
}

void Thread::terminate() {
	Mutex::ScopedLock scopedLock(_threadMutex);
	_terminate = true;
	scopedLock.unlock();

	_threadCondition.notify_all();
}

void Thread::setAutoDelete(bool value) {
	Mutex::ScopedLock scopedLock(_threadMutex);
	_autoDelete = value;
}

bool Thread::getAutoDelete() const {
	Mutex::ScopedLock scopedLock(_threadMutex);
	return _autoDelete;
}

OWTHREAD_API static unsigned long getCurrentThreadId()		//VOXOX - JRT - 2009.09.24 
{
	unsigned long threadId = 0;

#ifdef OS_POSIX
	threadId  = (unsigned long)pthread_self();
#elif defined(OS_WINDOWS)
	threadId = ::GetCurrentThreadId();
#else
	#error Platform not supported
#endif

	return threadId;
}

