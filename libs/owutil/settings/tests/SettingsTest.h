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

#ifndef SETTINGSTEST_H
#define SETTINGSTEST_H

#include <settings/Settings.h>
#include <settings/SettingsXMLSerializer.h>
#include <settings/CascadingSettings.h>
#include <settings/CascadingSettingsXMLSerializer.h>

#include <string>
#include <util/File.h>
#include <util/String.h>

#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

const char* TEST_SYSTEM_XML=
"<settings>"
"<key1><int>12</int></key1>"
"<key2><string>blah</string></key2>"
"<resource_key><string>a/system/resource</string></resource_key>"
"</settings>";

const char* TEST_USER_XML=
"<settings>"
"<key1><int>24</int></key1>"
"</settings>";

const char* KEY1 = "key1";
const int VALUE1 = 12;
const int USER_VALUE1 = 24;
const char* KEY2 = "key2";
const char* VALUE2 = "blah";
const char* RESOURCE_KEY = "resource_key";
const char* RESOURCE_VALUE = "a/system/resource";

template <class T>
class ValueChangedEventCounter {
public:
	ValueChangedEventCounter(T& settings) {
		settings.valueChangedEvent += boost::bind(&ValueChangedEventCounter::valueChangedEventHandler, this, _1);
	}

	void valueChangedEventHandler(const std::string& key) {
		if (_keyCount.find(key) == _keyCount.end()) {
			_keyCount[key] = 1;
		} else {
			++_keyCount[key];
		}
	}

	std::map<std::string, int> _keyCount;
};

class SettingsTest {
public:

	SettingsTest() {
	}

	void testSettings() {
		Settings settings;
		SettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML);
		ValueChangedEventCounter<Settings> counter(settings);
		
		int ret1 = settings.getIntegerKeyValue(KEY1);
		BOOST_CHECK_EQUAL(ret1, VALUE1);
		
		std::string ret2 = settings.getStringKeyValue(KEY2);
		BOOST_CHECK_EQUAL(ret2, VALUE2);

		settings.set(KEY2, std::string("boom"));
		BOOST_CHECK_EQUAL(counter._keyCount[KEY2], 1);
		
		ret2 = settings.getStringKeyValue(KEY2);
		BOOST_CHECK_EQUAL(ret2, "boom");
	}
	
	void testAllKeys() {
		Settings settings;
		SettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML);

		StringList keyList, keysFromSettings;

		keyList += KEY1;
		keyList += KEY2;
		keyList += RESOURCE_KEY;
		keyList.sort();

		//All key must be present and sorted
		keysFromSettings = settings.getAllKeys();

		BOOST_CHECK_EQUAL(keyList.size(), keysFromSettings.size());
		for (register unsigned i = 0 ; i <  keyList.size() ; i++) {
			BOOST_CHECK_EQUAL(keyList[i], keysFromSettings[i]);
		}
	}
};


class CascadingSettingsTest {
public:
	void testGet() {
		CascadingSettings settings;
		CascadingSettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML, TEST_USER_XML);

		int ret1 = settings.getIntegerKeyValue(KEY1);
		BOOST_CHECK_EQUAL(ret1, USER_VALUE1);

		ret1 = boost::any_cast<int>( settings.getAny(KEY1) );
		BOOST_CHECK_EQUAL(ret1, USER_VALUE1);

		std::string ret2 = settings.getStringKeyValue(KEY2);
		BOOST_CHECK_EQUAL(ret2, VALUE2);
	}

	void testAllKeys() {
		CascadingSettings settings;
		CascadingSettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML, TEST_USER_XML);

		StringList keyList, keysFromSettings;

		keyList += KEY1;
		keyList += KEY2;
		keyList += RESOURCE_KEY;
		keyList.sort();

		//All key must be present and sorted
		keysFromSettings = settings.getAllKeys();

		BOOST_CHECK_EQUAL(keyList.size(), keysFromSettings.size());
		for (register unsigned i = 0 ; i <  keyList.size() ; i++) {
			BOOST_CHECK_EQUAL(keyList[i], keysFromSettings[i]);
		}
	}

	void testGetDefaultValue() {
		CascadingSettings settings;
		CascadingSettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML, TEST_USER_XML);

		int value = boost::any_cast<int>( settings.getDefaultValue(KEY1) );
		BOOST_CHECK_EQUAL(value, VALUE1);
	}

	void testReset() {
		CascadingSettings settings;
		CascadingSettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML, TEST_USER_XML);

		int ret1 = settings.getIntegerKeyValue(KEY1);
		BOOST_CHECK_EQUAL(ret1, USER_VALUE1);

		ValueChangedEventCounter<CascadingSettings> counter(settings);
		settings.resetToDefaultValue(KEY1);
		BOOST_CHECK_EQUAL(counter._keyCount[KEY1], 1);

		ret1 = settings.getIntegerKeyValue(KEY1);
		BOOST_CHECK_EQUAL(ret1, VALUE1);

		std::string data = serializer.serialize();
		BOOST_CHECK_EQUAL(data, "<settings>\n</settings>\n");
	}

	void testSetAndSerialize() {
		CascadingSettings settings;
		CascadingSettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML, "");

		settings.set(KEY1, VALUE1 + 1);

		std::string data = serializer.serialize();

		Settings userSettings;
		SettingsXMLSerializer userSerializer(userSettings);
		userSerializer.unserialize(data);

		BOOST_CHECK_EQUAL(userSettings.getIntegerKeyValue(KEY1), VALUE1 + 1);
		BOOST_CHECK_EQUAL(int( userSettings.size() ), 1);
	}

	void testStickyKeys() {
		CascadingSettings settings;
		CascadingSettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML, "");
		BOOST_CHECK(!settings.isKeySticky(KEY1));
		settings.addStickyKey(KEY1);
		BOOST_CHECK(settings.isKeySticky(KEY1));

		std::string data = serializer.serialize();

		Settings userSettings;
		SettingsXMLSerializer userSerializer(userSettings);
		userSerializer.unserialize(data);
		BOOST_CHECK(userSettings.contains(KEY1));

		settings.removeStickyKey(KEY1);
		BOOST_CHECK(!settings.isKeySticky(KEY1));

		data = serializer.serialize();

		userSerializer.unserialize(data);
		BOOST_CHECK(!userSettings.contains(KEY1));
	}

	void testStickyKeysBeforeUnserializing() {
		CascadingSettings settings;
		BOOST_CHECK(!settings.isKeySticky(KEY1));
		settings.addStickyKey(KEY1);
		BOOST_CHECK(settings.isKeySticky(KEY1));

		CascadingSettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML, "");

		std::string data = serializer.serialize();

		Settings userSettings;
		SettingsXMLSerializer userSerializer(userSettings);
		userSerializer.unserialize(data);
		BOOST_CHECK(userSettings.contains(KEY1));
	}

	void testResourceGetSet() {
		CascadingSettings settings;
		CascadingSettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML, "");

		const std::string RESOURCE_DIR = File::convertPathSeparators("/some/place");
		const std::string PATH_OUTSIDE_RESOURCE_DIR = File::convertPathSeparators("/my/custom/resource");
		const std::string RELATIVE_USER_RESOURCE = "relative/user/resource";
		const std::string PATH_INSIDE_RESOURCE_DIR = File::convertPathSeparators(RESOURCE_DIR + "/" + RELATIVE_USER_RESOURCE);

		settings.setResourcesDir(RESOURCE_DIR);
		std::string ret;

		// Read system value
		ret = settings.getResourceKeyValue(RESOURCE_KEY);
		BOOST_CHECK_EQUAL(ret, File::convertPathSeparators(RESOURCE_DIR + "/" + RESOURCE_VALUE));

		// Set a value which is outside the resources dir
		settings.setResource(RESOURCE_KEY, PATH_OUTSIDE_RESOURCE_DIR);
		ret = settings.getStringKeyValue(RESOURCE_KEY);
		BOOST_CHECK_EQUAL(ret, File::convertToUnixPathSeparators(PATH_OUTSIDE_RESOURCE_DIR));

		ret = settings.getResourceKeyValue(RESOURCE_KEY);
		BOOST_CHECK_EQUAL(ret, PATH_OUTSIDE_RESOURCE_DIR);

		// Set a value which is inside the resources dir
		settings.setResource(RESOURCE_KEY, PATH_INSIDE_RESOURCE_DIR);
		ret = settings.getStringKeyValue(RESOURCE_KEY);
		BOOST_CHECK_EQUAL(ret, RELATIVE_USER_RESOURCE);

		ret = settings.getResourceKeyValue(RESOURCE_KEY);
		BOOST_CHECK_EQUAL(ret, PATH_INSIDE_RESOURCE_DIR);
	}

	void testResourceWindowsPathSeparators() {
		CascadingSettings settings;
		CascadingSettingsXMLSerializer serializer(settings);
		serializer.unserialize(TEST_SYSTEM_XML, "");

		const std::string WIN_RESOURCE_DIR = "c:\\some\\place";
		const std::string NATIVE_RESOURCE_DIR = File::convertPathSeparators(WIN_RESOURCE_DIR + "\\");
		const std::string ABSOLUTE_USER_RESOURCE = "c:/absolute/user/resource";
		const std::string PATH_OUTSIDE_RESOURCE_DIR = File::convertPathSeparators(ABSOLUTE_USER_RESOURCE);

		settings.setResourcesDir(WIN_RESOURCE_DIR);
		BOOST_CHECK_EQUAL(settings.getResourcesDir(), NATIVE_RESOURCE_DIR);

		std::string ret;
		settings.setResource(RESOURCE_KEY, PATH_OUTSIDE_RESOURCE_DIR);
		ret = settings.getStringKeyValue(RESOURCE_KEY);
		BOOST_CHECK_EQUAL(ret, ABSOLUTE_USER_RESOURCE);

		ret = settings.getResourceKeyValue(RESOURCE_KEY);
		BOOST_CHECK_EQUAL(ret, PATH_OUTSIDE_RESOURCE_DIR);
	}

	void testResourceOddPaths() {
		CascadingSettings settings;
		CascadingSettingsXMLSerializer serializer(settings);
		serializer.unserialize("<settings><a.key><string></string></a.key></settings>", "");

		const std::string RESOURCE_DIR = File::convertPathSeparators("/some/place/ending/with/a/slash/");
		settings.setResourcesDir(RESOURCE_DIR);

		std::string ret;
		ret = settings.getResourceKeyValue("a.key");
		BOOST_CHECK(ret.empty());

		// Check we don't end up with double slashes because RESOURCE_DIR ends
		// with a slash
		settings.setResource("a.key", "value");
		ret = settings.getResourceKeyValue("a.key");
		BOOST_CHECK_EQUAL(ret, File::convertPathSeparators(RESOURCE_DIR + "value"));
	}
};

class SettingsTestSuite : public test_suite {
public:

	SettingsTestSuite() : test_suite("SettingsTestSuite") {
		{
		boost::shared_ptr<SettingsTest> instance(new SettingsTest());
		add(BOOST_CLASS_TEST_CASE(&SettingsTest::testSettings, instance));
		add(BOOST_CLASS_TEST_CASE(&SettingsTest::testAllKeys, instance));
		}

		{
		boost::shared_ptr<CascadingSettingsTest> instance(new CascadingSettingsTest());
		add(BOOST_CLASS_TEST_CASE(&CascadingSettingsTest::testGet, instance));
		add(BOOST_CLASS_TEST_CASE(&CascadingSettingsTest::testGetDefaultValue, instance));
		add(BOOST_CLASS_TEST_CASE(&CascadingSettingsTest::testAllKeys, instance));
		add(BOOST_CLASS_TEST_CASE(&CascadingSettingsTest::testReset, instance));
		add(BOOST_CLASS_TEST_CASE(&CascadingSettingsTest::testSetAndSerialize, instance));
		add(BOOST_CLASS_TEST_CASE(&CascadingSettingsTest::testStickyKeys, instance));
		add(BOOST_CLASS_TEST_CASE(&CascadingSettingsTest::testStickyKeysBeforeUnserializing, instance));
		add(BOOST_CLASS_TEST_CASE(&CascadingSettingsTest::testResourceGetSet, instance));
		add(BOOST_CLASS_TEST_CASE(&CascadingSettingsTest::testResourceWindowsPathSeparators, instance));
		add(BOOST_CLASS_TEST_CASE(&CascadingSettingsTest::testResourceOddPaths, instance));
		}
	}
};

#endif	//SETTINGSTEST_H
