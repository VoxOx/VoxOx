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

#ifndef CURLHTTPREQUESTTEST_H
#define CURLHTTPREQUESTTEST_H

#include "HttpRequestTest.h"

#include <http/CurlHttpRequestFactory.h>

class CurlHttpRequestTest : public HttpRequestTest {
public:

	CurlHttpRequestTest() : HttpRequestTest(new CurlHttpRequestFactory()) {
	}

	virtual void testSendRequest() {
		HttpRequestTest::testSendRequest();
	}

	virtual void testSendRequestFailed() {
		HttpRequestTest::testSendRequestFailed();
	}
};

class CurlHttpRequestTestSuite : public test_suite {
public:

	CurlHttpRequestTestSuite() : test_suite("HttpRequestTestSuite") {
		boost::shared_ptr<CurlHttpRequestTest> instance(new CurlHttpRequestTest());

		test_case * testSendRequest = BOOST_CLASS_TEST_CASE(&CurlHttpRequestTest::testSendRequest, instance);
		test_case * testSendRequestFailed = BOOST_CLASS_TEST_CASE(&CurlHttpRequestTest::testSendRequestFailed, instance);

		add(testSendRequest);
		add(testSendRequestFailed);
	}
};

#endif	//CURLHTTPREQUESTTEST_H
