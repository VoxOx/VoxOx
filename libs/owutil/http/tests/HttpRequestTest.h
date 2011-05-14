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

#ifndef HTTPREQUESTTEST_H
#define HTTPREQUESTTEST_H

#include <http/HttpRequest.h>
#include <http/HttpRequestFactory.h>
#include <http/DefaultHttpRequestFactory.h>
#include <StringList.h>

#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include <iostream>

class HttpRequestTest {
public:

	HttpRequestTest(HttpRequestFactory * factory = new DefaultHttpRequestFactory()) {
		//Creates a new HttpRequestTest
		//so that the callback has time to be called
		//otherwise the callback is never called

		HttpRequest::setFactory(factory);
		//HttpRequest::setProxy("proxy.wengo.fr", 8080, "myProxyUsername", "myProxyPassword");
		testSendRequest();
		testSendRequestFailed();
	}

	virtual ~HttpRequestTest() {
	}

	virtual void testSendRequest() {
		MyHttpRequest * http = new MyHttpRequest();

		//Login tests with SSL
		http->sendRequest(true, "ws.wengo.fr", 443, "/softphone-sso/sso.php", "login=lolokiri@orange.fr&password=lopopo", true);
		http->sendRequest(true, "ws.wengo.fr", 443, "/softphone-sso/sso.php", "login=lolokiri@orange.fr&password=tata", true);
		http->sendRequest(true, "ws.wengo.fr", 443, "/softphone-sso/sso.php", "login=lolokiri@orange.fr&password=lopopo", false);
		http->sendRequest(true, "ws.wengo.fr", 443, "/softphone-sso/sso.php", "login=lolokiri@orange.fr&password=tata", false);
		http->sendRequest(true, "ws.wengo.fr", 443, "/softphone-sso/sso.php", "login=lolokiri@orange.fr&password=tata", false);

		//Login test without SSL
		http->sendRequest(false, "ws.wengo.fr", 80, "/softphone-sso/sso.php", "login=lolokiri@orange.fr&password=lopopo", true);
		http->sendRequest(false, "ws.wengo.fr", 80, "/softphone-sso/sso.php", "login=lolokiri@orange.fr&password=tata", true);
		http->sendRequest(false, "ws.wengo.fr", 80, "/softphone-sso/sso.php", "login=lolokiri@orange.fr&password=lopopo", false);
		http->sendRequest(false, "ws.wengo.fr", 80, "/softphone-sso/sso.php", "login=lolokiri@orange.fr&password=tata", false);
		/*
		//SMS tests without SSL
		http->sendRequest(false, "ws.wengo.fr",80 ,"/sms/sendsms.php", "login=lolokiri@orange.fr&password=lopopo&message=megatest&target=0663736683", true);
		http->sendRequest(false, "ws.wengo.fr",80 ,"/sms/sendsms.php", "login=lolokiri@orange.fr&password=lopopo&message=megatest&target=0663736683", false);
		//SMS tests with SSL
		http->sendRequest(true, "ws.wengo.fr",443 ,"/sms/sendsms.php", "login=lolokiri@orange.fr&password=lopopo&message=megatest&target=0663736683", true);
		http->sendRequest(true, "ws.wengo.fr",443 ,"/sms/sendsms.php", "login=lolokiri@orange.fr&password=lopopo&message=megatest&target=0663736683", false);
		*/
		//Check update tests
		http->sendRequest(false, "ws.wengo.fr",80 ,"/softphone-version/version.php", "", true);
		http->sendRequest(false, "ws.wengo.fr",80 ,"/softphone-version/version.php", "", false);
		http->sendRequest(true, "ws.wengo.fr",80 ,"/softphone-version/version.php", "", true);
		http->sendRequest(true, "ws.wengo.fr",80 ,"/softphone-version/version.php", "", false);

		//Other tests
		http->sendRequest("http://www.google.fr", "");
		http->sendRequest("http://curl.haxx.se", "");
	}

	virtual void testSendRequestFailed() {
		MyHttpRequest * http = new MyHttpRequest();
		http->sendRequest("http://toto123.fr", String::null);
		http->sendRequest("http://5454874767.fr", String::null);
		http->sendRequest(false, "www.wenkdfhdfgo.fr", 80, "", "", false);
		http->sendRequest(true, "www.wenkdfhdfgo.fr", 80, "", "", false);
		http->sendRequest(false, "www.wenkdfhdfgo.fr", 80, "", "", true);
		http->sendRequest(true, "www.wenkdfhdfgo.fr", 80, "", "", true);
		http->sendRequest(true, "www.google.fr", 80, "", "", false);
	}

private:

	class MyHttpRequest : public HttpRequest {
	public:

		MyHttpRequest() {
		}

		virtual void sendRequest(const std::string & url, const std::string & data, bool postMethod = false) {
			_url = url;
			HttpRequest::sendRequest(url, data, postMethod);
		}

		virtual void sendRequest(bool sslProtocol, const std::string & hostname, unsigned int hostPort,
			const std::string & path, const std::string & data, bool postMethod) {
			HttpRequest::sendRequest(sslProtocol, hostname, hostPort, path, data, postMethod);
		}

		//Callback called when the server answer to the HTTP request.
		virtual void answerReceived(const std::string & answer, Error error) {
			if (error != NoError) {
				//BOOST_CHECK(answer.empty());
				//not a good test in the case of a partial anwser
			} else {
				BOOST_CHECK(!answer.empty());
			}
			std::cout << "HTTP server answer: " << answer << std::endl;
		}

	private:

		std::string _url;
	};
};

class HttpRequestTestSuite : public test_suite {
public:

	HttpRequestTestSuite() : test_suite("HttpRequestTestSuite") {
		boost::shared_ptr<HttpRequestTest> instance(new HttpRequestTest());

		test_case * testSendRequest = BOOST_CLASS_TEST_CASE(&HttpRequestTest::testSendRequest, instance);
		test_case * testSendRequestFailed = BOOST_CLASS_TEST_CASE(&HttpRequestTest::testSendRequestFailed, instance);

		add(testSendRequest);
		add(testSendRequestFailed);
	}
};

#endif	//HTTPREQUESTTEST_H
