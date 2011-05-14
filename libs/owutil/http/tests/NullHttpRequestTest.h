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

#ifndef NULLHTTPREQUESTTEST_H
#define NULLHTTPREQUESTTEST_H

#include "HttpRequestTest.h"

#include <http/NullHttpRequestFactory.h>

class NullHttpRequestTest : public HttpRequestTest {
public:

	NullHttpRequestTest() : HttpRequestTest(new NullHttpRequestFactory()) {
	}

	virtual void testSendRequest() {
		HttpRequestTest::testSendRequest();
	}

	virtual void testSendRequestFailed() {
		HttpRequestTest::testSendRequestFailed();
	}
};

class NullHttpRequestTestSuite : public test_suite {
public:

	NullHttpRequestTestSuite() : test_suite("NullHttpRequestTestSuite") {
		boost::shared_ptr<NullHttpRequestTest> instance(new NullHttpRequestTest());

		test_case * testSendRequest = BOOST_CLASS_TEST_CASE(&NullHttpRequestTest::testSendRequest, instance);
		test_case * testSendRequestFailed = BOOST_CLASS_TEST_CASE(&NullHttpRequestTest::testSendRequestFailed, instance);

		add(testSendRequest);
		add(testSendRequestFailed);
	}
};

#endif	//NULLHTTPREQUESTTEST_H
