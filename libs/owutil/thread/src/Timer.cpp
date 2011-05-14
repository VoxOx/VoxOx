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

#include <thread/Timer.h>

#include <thread/Thread.h>
#include <util/SafeDelete.h>

class PrivateThread : public Thread {
public:

	Event<void (PrivateThread & thread)> timeoutEvent;

	Event<void (PrivateThread & thread)> lastTimeoutEvent;

	PrivateThread();

	virtual ~PrivateThread();

	void start(unsigned firstTime, unsigned timeout, unsigned nbShots);

	void stop();

	bool isStopped() const {
		return _stop;
	}

private:

	/**
	 * Sleeps and check at constant interval if the Timer
	 * must be stopped. If so, the method returns
	 */
	void msleepAndCheck(unsigned time);

	/** @see Thread::start() */
	virtual void run();

	/** Number of retries currenlty done, _nbRetry is always <= _nbShots. */
	unsigned _nbRetry;

	/** @see start() */
	unsigned _nbShots;

	/** @see start() */
	unsigned _timeout;

	/** @see start() */
	unsigned _firstTime;

	/** Stops or not the timer. */
	bool _stop;
};

PrivateThread::PrivateThread() {
	_firstTime = 0;
	_timeout = 0;
	_nbShots = 0;

	_stop = false;
	_nbRetry = 0;
}

PrivateThread::~PrivateThread() {
	stop();
}

void PrivateThread::start(unsigned firstTime, unsigned timeout, unsigned nbShots) {
	_firstTime = firstTime;
	_timeout = timeout;
	_nbShots = nbShots;

	_stop = false;
	_nbRetry = 0;

	Thread::start();
}

void PrivateThread::stop() {
	_stop = true;
}

void PrivateThread::run() {
	msleepAndCheck(_firstTime);

	while (!_stop && ((_nbShots == 0) || (_nbRetry < _nbShots))) {
		_nbRetry++;

		if (_nbRetry == _nbShots) {
			lastTimeoutEvent(*this);
		} else {
			timeoutEvent(*this);
		}

		msleepAndCheck(_timeout);
	}
}

void PrivateThread::msleepAndCheck(unsigned time) {
	unsigned sleepTime = time;

	if (sleepTime > 500) {
		sleepTime = 500;
	}

	unsigned remainingTime = time;
	unsigned actualSleepTime = 0;
	while (remainingTime > 0) {
		if (_stop) {
			return;
		}

		if (((int) remainingTime - (int) sleepTime) < 0) {
			actualSleepTime = remainingTime;
		} else {
			actualSleepTime = sleepTime;
		}

		msleep(actualSleepTime);
		remainingTime -= actualSleepTime;
	}
}

Timer::Timer() {
	_thread = new PrivateThread();
	_thread->timeoutEvent += boost::bind(&Timer::timeoutEventHandler, this);
	_thread->lastTimeoutEvent += boost::bind(&Timer::lastTimeoutEventHandler, this);
}

Timer::~Timer() {
	stop();
	OWSAFE_DELETE(_thread);
}

void Timer::start(unsigned firstTime, unsigned timeout, unsigned nbShots) {
	_thread->stop();
	_thread->join();
	_thread->start(firstTime, timeout, nbShots);
}

void Timer::stop() {
	_thread->stop();
}

void Timer::timeoutEventHandler() {
	if (!_thread->isStopped()) {
		timeoutEvent(*this);
	}
}

void Timer::lastTimeoutEventHandler() {
	if (!_thread->isStopped()) {
		lastTimeoutEvent(*this);
	}
}
