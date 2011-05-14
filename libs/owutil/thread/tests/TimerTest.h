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
#ifndef TIMERTEST_H
#define TIMERTEST_H

#include <iostream>

#include <util/String.h>

#include <thread/Thread.h>
#include <thread/Timer.h>

#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;


/**
 * This helper class connects to the timeout events of a timer and counts how
 * many times they are emitted.
 */
class TimerEventReceiver : public Trackable {
public:
	TimerEventReceiver(Timer* timer)
	: _timeoutEventCount(0)
	, _lastTimeoutEventCount(0) {
		timer->timeoutEvent += boost::bind(&TimerEventReceiver::timeoutEventHandler, this);
		timer->lastTimeoutEvent += boost::bind(&TimerEventReceiver::lastTimeoutEventHandler, this);
	}

	int timeoutEventCount() const {
		return _timeoutEventCount;
	}

	int lastTimeoutEventCount() const {
		return _lastTimeoutEventCount;
	}

private:
	void timeoutEventHandler() {
		++_timeoutEventCount;
	}

	void lastTimeoutEventHandler() {
		++_lastTimeoutEventCount;
	}

	int _timeoutEventCount;
	int _lastTimeoutEventCount;
};


class TimerTest {
public:
	void testSingleShot() {
		Timer timer;
		TimerEventReceiver receiver(&timer);

		timer.start(10, 0, 1);
		Thread::msleep(200);

		BOOST_CHECK_EQUAL(receiver.timeoutEventCount(), 0);
		BOOST_CHECK_EQUAL(receiver.lastTimeoutEventCount(), 1);
	}

	void testMultiShot() {
		Timer timer;
		TimerEventReceiver receiver(&timer);

		timer.start(0, 10, 4);
		Thread::msleep(200);

		BOOST_CHECK_EQUAL(receiver.timeoutEventCount(), 3);
		BOOST_CHECK_EQUAL(receiver.lastTimeoutEventCount(), 1);
	}

	void testThreadResourceError() {
		Timer timer;
		// On my Linux machine, it fails at 380, so this value should be enough
		const int MAX_LOOP = 10000;
		for(int loop=0; loop < MAX_LOOP; ++loop) {
			if (loop % 100 == 0) {
				std::cout << "Thread #" << loop << std::endl;
			}
			try {
				timer.start(10, 10, 1);
			} catch (boost::thread_resource_error&) {
				std::string msg = "Failed at " + String::fromNumber(loop);
				BOOST_FAIL(msg);
			}
			Thread::msleep(1);
			timer.stop();
		}
	}
};


class TimerTestSuite : public test_suite {
public:

	TimerTestSuite()
	: test_suite("TimerTestSuite") {
		boost::shared_ptr<TimerTest> instance(new TimerTest());

		add(BOOST_CLASS_TEST_CASE(&TimerTest::testSingleShot, instance));
		add(BOOST_CLASS_TEST_CASE(&TimerTest::testMultiShot, instance));
		add(BOOST_CLASS_TEST_CASE(&TimerTest::testThreadResourceError, instance));
	}
};

#endif /* TIMERTEST_H */
