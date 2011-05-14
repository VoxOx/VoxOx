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

#ifndef PHAPITEST_H
#define PHAPITEST_H

#include <phapi.h>
#include <phapi-old.h>

#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/**
 * TODO
 * - replace phcfg direct accesses by getters
 * - unify NULL parameter passing handling
 */

class PhApiTest {
public:

	void testOwplConfigSetLocalHttpProxy() {
		char buffer[32];
		int port;

		BOOST_CHECK(owplConfigSetLocalHttpProxy("127.0.0.1", 8080, "username", "password") == OWPL_RESULT_SUCCESS);
		
		BOOST_CHECK(owplConfigLocalHttpProxyGetAddr(buffer, sizeof(buffer)) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(buffer, "127.0.0.1") == 0,
			"Local HTTP proxy address not set properly");

		BOOST_CHECK(owplConfigLocalHttpProxyGetPort(&port) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(port == 8080,
			"Local HTTP proxy port not set properly");

		BOOST_CHECK(owplConfigLocalHttpProxyGetUserName(buffer, sizeof(buffer)) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(buffer, "username") == 0,
			"Local HTTP proxy username not set properly");

		BOOST_CHECK(owplConfigLocalHttpProxyGetPasswd(buffer, sizeof(buffer)) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(buffer, "password") == 0,
			"Local HTTP proxy password not set properly");
	}

	void testOwplConfigSetTunnel() {
		BOOST_CHECK(owplConfigSetTunnel("127.0.0.1", 80, OWPL_TUNNEL_USE | OWPL_TUNEL_AUTOCONF) == OWPL_RESULT_SUCCESS);

		BOOST_CHECK_MESSAGE(strcmp(phcfg.httpt_server, "127.0.0.1") == 0,
			"Tunnel server address not set properly");

		BOOST_CHECK_MESSAGE(phcfg.httpt_server_port == 80,
			"Tunnel server port not set properly");

		BOOST_CHECK_MESSAGE(phcfg.use_tunnel & (OWPL_TUNNEL_USE | OWPL_TUNEL_AUTOCONF),
			"Tunnel mode not set properly");
	}

	void testOwplConfigSetNat() {
		BOOST_CHECK(owplConfigSetNat(OWPL_NAT_TYPE_AUTO, 25) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.nattype, "auto") == 0,
			"Nat type not set properly");
		BOOST_CHECK_MESSAGE(phcfg.nat_refresh_time == 25,
			"Nat refresh time not set properly");

		BOOST_CHECK(owplConfigSetNat(OWPL_NAT_TYPE_FCONE, 24) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.nattype, "fcone") == 0,
			"Nat type not set properly");
		BOOST_CHECK_MESSAGE(phcfg.nat_refresh_time == 24,
			"Nat refresh time not set properly");

		BOOST_CHECK(owplConfigSetNat(OWPL_NAT_TYPE_NONE, 23) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.nattype, "none") == 0,
			"Nat type not set properly");
		BOOST_CHECK_MESSAGE(phcfg.nat_refresh_time == 23,
			"Nat refresh time not set properly");

		BOOST_CHECK(owplConfigSetNat(OWPL_NAT_TYPE_PRCONE, 22) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.nattype, "prcone") == 0,
			"Nat type not set properly");
		BOOST_CHECK_MESSAGE(phcfg.nat_refresh_time == 22,
			"Nat refresh time not set properly");

		BOOST_CHECK(owplConfigSetNat(OWPL_NAT_TYPE_RCONE, 21) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.nattype, "rcone") == 0,
			"Nat type not set properly");
		BOOST_CHECK_MESSAGE(phcfg.nat_refresh_time == 21,
			"Nat refresh time not set properly");

		BOOST_CHECK(owplConfigSetNat(OWPL_NAT_TYPE_SYMETRIC, 20) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.nattype, "sym") == 0,
			"Nat type not set properly");
		BOOST_CHECK_MESSAGE(phcfg.nat_refresh_time == 20,
			"Nat refresh time not set properly");
	}

	void testOwplConfigSetOutboundProxy() {
		BOOST_CHECK(owplConfigSetOutboundProxy("213.9.91.206") == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.proxy, "213.9.91.206") == 0,
			"Outbound proxy not properly set");

		BOOST_CHECK(owplConfigSetOutboundProxy(NULL) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.proxy, "") == 0,
			"NULL value as outbound proxy address should reset to empty string");

		BOOST_CHECK(owplConfigSetOutboundProxy("") == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.proxy, "") == 0,
			"Empty string as outbound proxy address should reset to empty string");
	}

	void testOwplConfigSetAudioCodecs() {
		char tooLongList[130];

		BOOST_CHECK(owplConfigSetAudioCodecs("codec1,codec2,codec3,codec4") == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.audio_codecs, "codec1,codec2,codec3,codec4") == 0,
			"Audio codecs not properly set");

		BOOST_CHECK(owplConfigSetAudioCodecs(NULL) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.audio_codecs, "") == 0,
			"NULL value as codecs should reset to empty string");

		BOOST_CHECK(owplConfigSetAudioCodecs("") == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.audio_codecs, "") == 0,
			"Empty string as codecs should reset to empty string");

		memset(tooLongList, 57, sizeof(tooLongList));
		tooLongList[sizeof(tooLongList)-1] = 0;
		BOOST_CHECK_MESSAGE(owplConfigSetAudioCodecs(tooLongList) == OWPL_RESULT_INVALID_ARGS,
			"Too long (>128) value should return OWPL_RESULT_INVALID_ARGS");
	}

	void testOwplConfigGetAudioCodecs() {
		char buffer1[128];
		char buffer2[1];

		strncpy(phcfg.audio_codecs, "codec1,codec2,codec3,codec4", sizeof(phcfg.audio_codecs));
		BOOST_CHECK(owplConfigGetAudioCodecs(buffer1, sizeof(buffer1)) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(buffer1, "codec1,codec2,codec3,codec4") == 0,
			"Getting audio codecs failed");

		BOOST_CHECK(owplConfigGetAudioCodecs(buffer1, 0) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Buffer size of value 0 should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigGetAudioCodecs(buffer2, sizeof(buffer2)) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Insufficient buffer size should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigGetAudioCodecs(NULL, 0) == OWPL_RESULT_INVALID_ARGS,
			"Null value as buffer should return OWPL_RESULT_INVALID_ARGS");
	}

	void testOwplConfigAddAudioCodecByName() {
		BOOST_CHECK(owplConfigAddAudioCodecByName("PCMU/8000") == OWPL_RESULT_SUCCESS);

		BOOST_CHECK_MESSAGE(owplConfigAddAudioCodecByName(NULL) == OWPL_RESULT_INVALID_ARGS,
			"NULL value as audio codec name should fail");

		BOOST_CHECK_MESSAGE(owplConfigAddAudioCodecByName("") == OWPL_RESULT_INVALID_ARGS,
			"Empty string as audio codec name should fail");
	}

	void testOwplConfigSetVideoCodecs() {
		char tooLongList[130];

		BOOST_CHECK(owplConfigSetVideoCodecs("codec1,codec2,codec3,codec4") == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.video_codecs, "codec1,codec2,codec3,codec4") == 0,
			"Audio codecs not properly set");

		BOOST_CHECK(owplConfigSetVideoCodecs(NULL) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.video_codecs, "") == 0,
			"NULL value as codecs should reset to empty string");

		BOOST_CHECK(owplConfigSetVideoCodecs("") == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.video_codecs, "") == 0,
			"Emty string as codecs should reset to empty string");

		memset(tooLongList, 57, sizeof(tooLongList));
		tooLongList[sizeof(tooLongList)-1] = 0;
		BOOST_CHECK_MESSAGE(owplConfigSetAudioCodecs(tooLongList) == OWPL_RESULT_INVALID_ARGS,
			"Too long (>128) value should return OWPL_RESULT_INVALID_ARGS");
	}

	void testOwplConfigGetVideoCodecs() {
		char buffer1[128];
		char buffer2[1];

		strncpy(phcfg.video_codecs, "codec1,codec2,codec3,codec4", sizeof(phcfg.video_codecs));
		BOOST_CHECK(owplConfigGetVideoCodecs(buffer1, sizeof(buffer1)) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(buffer1, "codec1,codec2,codec3,codec4") == 0,
			"Getting audio codecs failed");

		BOOST_CHECK(owplConfigGetVideoCodecs(buffer1, 0) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Buffer size of value 0 should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigGetVideoCodecs(buffer2, sizeof(buffer2)) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Insufficient buffer size should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigGetVideoCodecs(NULL, 0) == OWPL_RESULT_INVALID_ARGS,
			"Null value as buffer should return OWPL_RESULT_INVALID_ARGS");
	}

	void testOwplConfigAddVideoCodecByName() {
		BOOST_CHECK(owplConfigAddVideoCodecByName("H263") == OWPL_RESULT_SUCCESS);

		BOOST_CHECK_MESSAGE(owplConfigAddVideoCodecByName(NULL) == OWPL_RESULT_INVALID_ARGS,
			"NULL value as video codec name should fail");

		BOOST_CHECK_MESSAGE(owplConfigAddVideoCodecByName("") == OWPL_RESULT_INVALID_ARGS,
			"Empty string as video codec name should fail");
	}

	void testOwplConfigSetAsyncCallbackMode() {
		BOOST_CHECK(owplConfigSetAsyncCallbackMode(0) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK(phcfg.asyncmode == 0);

		BOOST_CHECK(owplConfigSetAsyncCallbackMode(1) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK(phcfg.asyncmode == 1);

		BOOST_CHECK_MESSAGE(owplConfigSetAsyncCallbackMode(10) == OWPL_RESULT_INVALID_ARGS,
			"A value different of 0 or 1 as boolean should fail");
	}

	void testOwplConfigGetBoundLocalAddr() {
		char buffer1[32];
		char buffer2[1];

		BOOST_CHECK(owplConfigGetBoundLocalAddr(buffer1, sizeof(buffer1)) == OWPL_RESULT_SUCCESS);

		BOOST_CHECK(owplConfigGetBoundLocalAddr(buffer1, 0) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Buffer size of value 0 should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigGetBoundLocalAddr(buffer2, sizeof(buffer2)) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Insufficient buffer size should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigGetBoundLocalAddr(NULL, 0) == OWPL_RESULT_INVALID_ARGS,
			"Null value as buffer should return OWPL_RESULT_INVALID_ARGS");
	}

	void testOwplConfigLocalHttpProxyGetAddr() {
		char buffer1[32];
		char buffer2[1];

		BOOST_CHECK(owplConfigSetLocalHttpProxy("127.0.0.1", 8080, "username", "password") == OWPL_RESULT_SUCCESS);

		BOOST_CHECK(owplConfigLocalHttpProxyGetAddr(buffer1, sizeof(buffer1)) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(buffer1, "127.0.0.1") == 0,
			"Local HTTP proxy address not properly retrieved");

		BOOST_CHECK_MESSAGE(owplConfigLocalHttpProxyGetAddr(buffer1, 0) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Buffer size of value 0 should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigLocalHttpProxyGetAddr(buffer2, sizeof(buffer2)) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Insufficient buffer size should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigLocalHttpProxyGetAddr(NULL, 0) == OWPL_RESULT_INVALID_ARGS,
			"Null value as buffer should return OWPL_RESULT_INVALID_ARGS");
	}

	void testOwplConfigLocalHttpProxyGetPasswd() {
		char buffer1[32];
		char buffer2[1];

		BOOST_CHECK(owplConfigSetLocalHttpProxy("127.0.0.1", 8080, "username", "password") == OWPL_RESULT_SUCCESS);

		BOOST_CHECK(owplConfigLocalHttpProxyGetPasswd(buffer1, sizeof(buffer1)) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(buffer1, "password") == 0,
			"Local HTTP proxy password not properly retrieved");

		BOOST_CHECK_MESSAGE(owplConfigLocalHttpProxyGetPasswd(buffer1, 0) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Buffer size of value 0 should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigLocalHttpProxyGetPasswd(buffer2, sizeof(buffer2)) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Insufficient buffer size should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigLocalHttpProxyGetPasswd(NULL, 0) == OWPL_RESULT_INVALID_ARGS,
			"Null value as buffer should return OWPL_RESULT_INVALID_ARGS");
	}

	void testOwplConfigLocalHttpProxyGetPort() {
		int buffer;

		BOOST_CHECK(owplConfigSetLocalHttpProxy("127.0.0.1", 8080, "username", "password") == OWPL_RESULT_SUCCESS);

		BOOST_CHECK(owplConfigLocalHttpProxyGetPort(&buffer) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(buffer == 8080,
			"Local HTTP proxy port not properly retrieved");

		BOOST_CHECK_MESSAGE(owplConfigLocalHttpProxyGetPort(NULL) == OWPL_RESULT_INVALID_ARGS,
			"NULL value as buffer should return OWPL_RESULT_INVALID_ARGS");
	}

	void testOwplConfigLocalHttpProxyGetUserName() {
		char buffer1[32];
		char buffer2[1];

		BOOST_CHECK(owplConfigSetLocalHttpProxy("127.0.0.1", 8080, "username", "password") == OWPL_RESULT_SUCCESS);

		BOOST_CHECK(owplConfigLocalHttpProxyGetUserName(buffer1, sizeof(buffer1)) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(buffer1, "username") == 0,
			"Local HTTP proxy username not properly retrieved");

		BOOST_CHECK_MESSAGE(owplConfigLocalHttpProxyGetUserName(buffer1, 0) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Buffer size of value 0 should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigLocalHttpProxyGetUserName(buffer2, sizeof(buffer2)) == OWPL_RESULT_INSUFFICIENT_BUFFER,
			"Insufficient buffer size should return OWPL_RESULT_INSUFFICIENT_BUFFER");

		BOOST_CHECK_MESSAGE(owplConfigLocalHttpProxyGetUserName(NULL, 0) == OWPL_RESULT_INVALID_ARGS,
			"Null value as buffer should return OWPL_RESULT_INVALID_ARGS");
	}


	void testOwplAudioSetConfigString() {
		char tooLongString[66];

		memset(tooLongString, 57, sizeof(tooLongString));
		tooLongString[sizeof(tooLongString)-1] = 0;

		BOOST_CHECK(owplAudioSetConfigString("Audio config string") == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.audio_dev, "Audio config string") == 0,
			"Audio config string not properly set");

		BOOST_CHECK(owplAudioSetConfigString("") == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.audio_dev, "") == 0,
			"Empty string as audio config string should reset");

		BOOST_CHECK(owplAudioSetConfigString(NULL) == OWPL_RESULT_SUCCESS);
		BOOST_CHECK_MESSAGE(strcmp(phcfg.audio_dev, "") == 0,
			"NULL value as audio config string should reset");

		BOOST_CHECK_MESSAGE(owplAudioSetConfigString(tooLongString) == OWPL_RESULT_INVALID_ARGS,
			"Value of length > 64 should fail");
	}

};

class PhApiTestSuite : public test_suite {
public:

	PhApiTestSuite() : test_suite("PhApiTestSuite") {
		boost::shared_ptr<PhApiTest> instance(new PhApiTest());

		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigSetLocalHttpProxy, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigSetTunnel, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigSetNat, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigSetOutboundProxy, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigSetAudioCodecs, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigGetAudioCodecs, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigAddAudioCodecByName, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigSetVideoCodecs, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigGetVideoCodecs, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigAddVideoCodecByName, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigSetAsyncCallbackMode, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigGetBoundLocalAddr, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigLocalHttpProxyGetAddr, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigLocalHttpProxyGetPasswd, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigLocalHttpProxyGetPort, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplConfigLocalHttpProxyGetUserName, instance));
		add(BOOST_CLASS_TEST_CASE(&PhApiTest::testOwplAudioSetConfigString, instance));
	}
};

#endif /* PHAPITEST_H */