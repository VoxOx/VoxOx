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

#ifndef OWTHREAD_H
#define OWTHREAD_H

#include <thread/owthreaddll.h>
#include <thread/Condition.h>
#include <thread/Mutex.h>

#include <util/Interface.h>

#include <queue>

class IThreadEvent;
namespace boost { class thread; }

/**
 * Thread helper for Boost.
 *
 * Example:
 * <pre>
 * class MyThread : public Thread {
 * private:
 *     void run() {
 *         std::cout << "Hello World!" << std::endl;
 *     }
 * };
 *
 * MyThread * myThread = new MyThread();
 * //start() calls MyThread::run() in a different thread
 * myThread->start();
 * </pre>
 *
 * @see boost::thread
 * @see ThreadPrivate
 * @see QThread
 * @see java.lang.Thread
 * @see java.lang.Runnable
 * @see java.lang.Object
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class Thread : Interface {
public:

	OWTHREAD_API Thread();

	/**
	 * Events contained in the Event Queue are executed
	 * before deleting this Thread instance.
	 */
	OWTHREAD_API virtual ~Thread();

	/**
	 * Starts/creates the Thread with a given priority.
	 *
	 * Calls the pure virtual method run().
	 */
	OWTHREAD_API void start();

	/**
	 * Blocks a Thread.
	 *
	 * The Thread will block until the thread associated with this
	 * Thread object has finished execution (i.e. when it returns from run()).
	 *
	 * This provides similar functionality to the POSIX pthread_join() function.
	 */
	OWTHREAD_API void join();

	/**
	 * Adds a ThreadEvent to the thread main loop.
	 *
	 * The ThreadEvent will be executed in a threaded way inside the run() method.
	 * Warning: do not delete ThreadEvent, the method runEvents() will do it
	 * once the ThreadEvent has been executed.
	 *
	 * @param event to inject inside the thread main loop
	 */
	OWTHREAD_API void postEvent(IThreadEvent * event);

	/**
	 * Defines whether the thread should delete itself at end.
	 */
	OWTHREAD_API void setAutoDelete(bool);

	/**
	 * Returns true if the thread will autodelete itself.
	 */
	OWTHREAD_API bool getAutoDelete() const;

	/**
	 * Causes the current thread to sleep.
	 *
	 * System independent sleep.
	 *
	 * @param seconds number of seconds the current thread should sleep
	 */
	OWTHREAD_API static void sleep(unsigned long seconds);

	/**
	 * Causes the current thread to sleep.
	 *
	 * System independent sleep.
	 *
	 * @see sleep()
	 * @param milliseconds number of milliseconds the current thread should sleep
	 */
	OWTHREAD_API static void msleep(unsigned long milliseconds);

	//Get current threadId
	OWTHREAD_API static unsigned long getCurrentThreadId();		//VOXOX - JRT - 2009.09.24 

protected:

	/**
	 * Thread main loop.
	 *
	 * Starting the Thread using the method start() causes this method
	 * to be called in a separately executing Thread.
	 * Returning from this method will end the execution of the thread.
	 *
	 * Example of implementation:
	 * <pre>
	 * mutable Mutex _mutex;
	 * bool _terminate = false;
	 *
	 * void init() {
	 *     ...
	 * }
	 *
	 * void terminate() {
	 *     _terminate = true;
	 * }
	 *
	 * void run() {
	 *     init();
	 *
	 *     //Keeps the thread running until terminate() is called
	 *     while (!_terminate) {
	 *         runEvents();
	 *     }
	 * }
	 * </pre>
	 */
	OWTHREAD_API virtual void run() = 0;

	/**
	 * Runs the events inserted inside the main thread loop via postEvent().
	 */
	OWTHREAD_API void runEvents();

	/**
	 * Terminates the execution of the thread.
	 *
	 * Typical implementation:
	 * <pre>
	 * void terminate() {
	 *     _terminate = true;
	 * }
	 *
	 * void run() {
	 *     //Keeps the thread running until terminate() is called
	 *     while (!_terminate) {
	 *         ...
	 *     }
	 * }
	 * </pre>
	 */
	OWTHREAD_API void terminate();

	/** Defines the vector of ThreadEvent. */
	typedef std::queue < IThreadEvent * > Events;

	/** List of PostEvent. */
	Events _eventQueue;

	/**
	 * Mutex used for postEvent() and runEvents().
	 */
	mutable Mutex _threadMutex;

	/**
	 * Condition used for postEvent().
	 */
	Condition _threadCondition;

	/**
	 * If this thread should be terminate or not.
	 *
	 * By default _terminate == false.
	 * @see terminate()
	 */
	bool _terminate;

	/**
	 * If set to true, the class will automatically delete itself when run()
	 * returns.
	 * default is false.
	 */
	bool _autoDelete;

private:

	/**
	 * Proxy function for runnning thread.
	 */
	void runThread();

	/**
	 * True if the thread is running.
	 */
	bool _threadRunning;

	/** Boost thread. */
	boost::thread * _thread;
};

#endif	//OWTHREAD_H
