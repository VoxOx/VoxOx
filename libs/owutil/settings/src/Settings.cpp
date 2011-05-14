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

#include <settings/Settings.h>

#include <settings/StringListXMLSerializer.h>

#include <util/StringList.h>
#include <util/Logger.h>

using namespace std;

Settings::Settings() {
}

Settings::Settings(const Settings & settings) {
	copy(settings);
}

Settings & Settings::operator=(const Settings & settings) {
	copy(settings);
	return *this;
}

void Settings::copy(const Settings & settings) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);				//VOXOX - JRT - 2009.06.25 
	RecursiveMutex::ScopedLock scopedLock2(settings._mutex);	//VOXOX - JRT - 2009.06.25 

	_keyMap = settings._keyMap;
	//FIXME: should we copy the event? valueChangedEvent = settings.valueChangedEvent;
}

Settings::~Settings() {
}

StringList Settings::getAllKeys() const {
	StringList tmp;
	for (Keys::const_iterator it = _keyMap.begin(); it != _keyMap.end(); ++it) {
		tmp += it->first;
	}
	return tmp;
}

void Settings::remove(const std::string & key) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	Keys::iterator it = _keyMap.find(key);
	if (it != _keyMap.end()) {
		_keyMap.erase(it);
	}
}

bool Settings::contains(const std::string & key) const {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	Keys::const_iterator it = _keyMap.find(key);
	if (it != _keyMap.end()) {
		return true;
	}
	return false;
}

void Settings::set(const std::string & key, const std::string & value) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	if ((_keyMap.find(key) == _keyMap.end()) ||
		((boost::any_cast<std::string>(_keyMap[key]) != value))) {
		_keyMap[key] = value;
		valueChangedEvent(key);
	}
}

void Settings::set(const std::string & key, const StringList & value) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);
	
	if ((_keyMap.find(key) == _keyMap.end()) ||
		((boost::any_cast<StringList>(_keyMap[key]) != value))) {
		_keyMap[key] = value;
		valueChangedEvent(key);
	}
}

void Settings::set(const std::string & key, bool value) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	if ((_keyMap.find(key) == _keyMap.end()) ||
		((boost::any_cast<bool>(_keyMap[key]) != value))) {
		_keyMap[key] = value;
		valueChangedEvent(key);
	}
}

void Settings::set(const std::string & key, int value) {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	if ((_keyMap.find(key) == _keyMap.end()) ||
		((boost::any_cast<int>(_keyMap[key]) != value))) {
		_keyMap[key] = value;
		valueChangedEvent(key);
	}
}

void Settings::set(const std::string & key, boost::any value) {
	if (isBoolean(value)) {
		set(key, boost::any_cast<bool>(value));
	} else if (isInteger(value)) {
		set(key, boost::any_cast<int>(value));
	} else if (isString(value)) {
		set(key, boost::any_cast<std::string>(value));
	} else if (isStringList(value)) {
		set(key, boost::any_cast<StringList>(value));
	}
}

std::string Settings::getStringKeyValue(const std::string & key) const {
	boost::any value = getAny(key);
	if (!isString(value)) {
		LOG_FATAL("key '" + key + "' is not a string");
	}
	return boost::any_cast<std::string>(value);
}

StringList Settings::getStringListKeyValue(const std::string & key) const {
	boost::any value = getAny(key);
	if (!isStringList(value)) {
		LOG_FATAL("key '" + key + "' is not a string list");
	}
	return boost::any_cast<StringList>(value);
}

bool Settings::getBooleanKeyValue(const std::string & key) const {
	boost::any value = getAny(key);
	if (!isBoolean(value)) {
		LOG_FATAL("key '" + key + "' is not a boolean");
	}
	return boost::any_cast<bool>(value);
}

int Settings::getIntegerKeyValue(const std::string & key) const {
	boost::any value = getAny(key);
	if (!isInteger(value)) {
		LOG_FATAL("key '" + key + "' is not an integer");
	}
	return boost::any_cast<int>(value);
}

boost::any Settings::getAny(const std::string & key) const {
	//VOXOX - JRT - 2009.05.03 - Causing crash if key does not exist.
	//So check contains() for new keys before calling get.
	//Add the default value if it does not exist.
	Keys::const_iterator it = _keyMap.find(key);
	if (it == _keyMap.end()) {
		LOG_FATAL("key '" + key + "' does not exist");
	}
	return it->second;
}

bool Settings::isBoolean(const boost::any & value) {
	return (value.type() == typeid(bool));
}

bool Settings::isInteger(const boost::any & value) {
	return (value.type() == typeid(int));
}

bool Settings::isString(const boost::any & value) {
	return boost::any_cast<std::string>(&value);
}

bool Settings::isStringList(const boost::any & value) {
	return boost::any_cast<StringList>(&value);
}
