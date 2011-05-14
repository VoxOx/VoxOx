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
#include <settings/CascadingSettings.h>

#include <util/Logger.h>
#include <util/File.h>
#include <util/String.h>


// FIXME: This should go to File.h
static std::string cleanPathSeparators(const std::string& pathParam) {
	String path(pathParam);
	path.replace("//", "/");
	return path;
}

static bool isAbsolutePath(const std::string& path) {
	if (path[0] == '/') {
		return true;
	}
	if (path[1] == ':') {
		return true;
	}
	return false;
}

static bool anyAreEquals(const boost::any & any1, const boost::any & any2) {
	if (any1.type() != any2.type()) {
		return false;
	}

	if (Settings::isString(any1)) {
		return boost::any_cast<std::string>(any1) == boost::any_cast<std::string>(any2);
	}
	if (Settings::isStringList(any1)) {
		return boost::any_cast<StringList>(any1) == boost::any_cast<StringList>(any2);
	}
	if (Settings::isInteger(any1)) {
		return boost::any_cast<int>(any1) == boost::any_cast<int>(any2);
	}
	if (Settings::isBoolean(any1)) {
		return boost::any_cast<bool>(any1) == boost::any_cast<bool>(any2);
	}
	LOG_FATAL("Unknown 'any' type");
	return false;
}


CascadingSettings::CascadingSettings() {
}

CascadingSettings::CascadingSettings(const CascadingSettings & settings) {
	copy(settings);
}

CascadingSettings & CascadingSettings::operator=(const CascadingSettings & settings) {
	copy(settings);
	return *this;
}

CascadingSettings::~CascadingSettings() {
}

void CascadingSettings::copy(const CascadingSettings & settings) {
	_systemSettings = settings._systemSettings;
	_userSettings = settings._userSettings;
}

bool CascadingSettings::contains(const std::string & key) const {
	return _systemSettings.contains(key);
}

template<class T, T (Settings::*getter)(const std::string&) const>
void CascadingSettings::internalSet(const std::string& key, const T& value) {
	if (isKeySticky(key)) {
		if (value != internalGet<T, getter>(key)) {
			_userSettings.set(key, value);
			valueChangedEvent(key);
		}
		return;
	}
	if (_userSettings.contains(key)) {
		if (value == (_userSettings.*getter)(key)) {
			// Already set to this value, nothing to do
			return;
		} else {
			if (value == (_systemSettings.*getter)(key)) {
				// Come back to system value
				_userSettings.remove(key);
			} else {
				// Set a new user value
				_userSettings.set(key, value);
			}
			valueChangedEvent(key);
		}
	} else {
		if (value != (_systemSettings.*getter)(key)) {
			// Set a new user value
			_userSettings.set(key, value);
			valueChangedEvent(key);
		}
	}
}

void CascadingSettings::set(const std::string & key, const std::string & value) {
	String tmpValue = value;//VOXOX CHANGE by Rolando - 2009.07.29
	tmpValue.encodeToXMLSpecialCharacters();//VOXOX CHANGE by Rolando - 2009.08.04	
	internalSet<std::string, &Settings::getStringKeyValue>(key, tmpValue);
}

void CascadingSettings::set(const std::string & key, const StringList & value) {
	StringList tmpValue = value;//VOXOX CHANGE by Rolando - 2009.07.29
	StringList newValues;//VOXOX CHANGE by Rolando - 2009.07.30 
	StringList::iterator iter = tmpValue.begin();//VOXOX CHANGE by Rolando - 2009.07.29 
	String tmpString = "";//VOXOX CHANGE by Rolando - 2009.07.29 

	for(iter; iter != tmpValue.end(); iter++){//VOXOX CHANGE by Rolando - 2009.07.29 
		tmpString = *iter;//VOXOX CHANGE by Rolando - 2009.07.29
		tmpString.encodeToXMLSpecialCharacters();//VOXOX CHANGE by Rolando - 2009.08.04 		
		newValues.push_back(tmpString);//VOXOX CHANGE by Rolando - 2009.07.29  
	}
	
	internalSet<StringList, &Settings::getStringListKeyValue>(key, newValues);
}

void CascadingSettings::set(const std::string & key, bool value) {
	internalSet<bool, &Settings::getBooleanKeyValue>(key, value);
}

//VOXOX - JRT - 2009.05.03 
bool CascadingSettings::toggle(const std::string & key) 
{
	bool bNewVal = !(getBooleanKeyValue( key ));
	internalSet<bool, &Settings::getBooleanKeyValue>(key, bNewVal);

	return bNewVal;
}
//End VoxOx

void CascadingSettings::set(const std::string & key, int value) {
	internalSet<int, &Settings::getIntegerKeyValue>(key, value);
}

void CascadingSettings::setResource(const std::string & key, const std::string & paramValue) {
	std::string value = File::convertToUnixPathSeparators(paramValue);
	value = cleanPathSeparators(value);
	String str(value);

	if (str.beginsWith(_resourcesDir)) {
		str = str.substr(_resourcesDir.size());
		if (str[0] == '/') {
			str = str.substr(1);
		}
	}
	set(key, str);
}

boost::any CascadingSettings::getDefaultValue(const std::string & key) const {
	return _systemSettings.getAny(key);
}

template<class T, T (Settings::*getter)(const std::string&) const>
T CascadingSettings::internalGet(const std::string & key) const {
	if (_userSettings.contains(key)) {
		return (_userSettings.*getter)(key);
	} else {
		return (_systemSettings.*getter)(key);
	}
}

boost::any CascadingSettings::getAny(const std::string & key) const {
	return internalGet<boost::any, &Settings::getAny>(key);
}

std::string CascadingSettings::getStringKeyValue(const std::string & key) const {
	return internalGet<std::string, &Settings::getStringKeyValue>(key);
}

StringList CascadingSettings::getStringListKeyValue(const std::string & key) const {
	return internalGet<StringList, &Settings::getStringListKeyValue>(key);
}

bool CascadingSettings::getBooleanKeyValue(const std::string & key) const {
	return internalGet<bool, &Settings::getBooleanKeyValue>(key);
}

int CascadingSettings::getIntegerKeyValue(const std::string & key) const {
	return internalGet<int, &Settings::getIntegerKeyValue>(key);
}

std::string CascadingSettings::getResourceKeyValue(const std::string & key) const {
	std::string value = getStringKeyValue(key);
	if (value.empty()) {
		return value;
	}
	if (!isAbsolutePath(value)) {
		value = _resourcesDir + '/' + value;
	}
	value = cleanPathSeparators(value);
	return File::convertPathSeparators(value);
}

void CascadingSettings::resetToDefaultValue(const std::string & key) {
	if (_userSettings.contains(key)) {
		_userSettings.remove(key);
		valueChangedEvent(key);
	}
}

StringList CascadingSettings::getAllKeys() const {
	return _systemSettings.getAllKeys();
}

void CascadingSettings::addKey(const std::string & key, const boost::any& systemValue, const boost::any& userValue) {
	_systemSettings.set(key, systemValue);
	if (isKeySticky(key)) {
		if (userValue.empty()) {
			_userSettings.set(key, systemValue);
		} else {
			_userSettings.set(key, userValue);
		}
	} else {
		if (!userValue.empty() && !anyAreEquals(systemValue, userValue)) {
			_userSettings.set(key, userValue);
		}
	}
	valueChangedEvent(key);
}

void CascadingSettings::addStickyKey(const std::string& key) {
	_stickyKeySet.insert(key);
	// Make sure _userSettings contains a copy of the key
	if (_systemSettings.contains(key) && !_userSettings.contains(key)) {
		_userSettings.set(key, _systemSettings.getAny(key));
	}
}

bool CascadingSettings::isKeySticky(const std::string& key) {
	return _stickyKeySet.find(key) != _stickyKeySet.end();
}

void CascadingSettings::removeStickyKey(const std::string& key) {
	_stickyKeySet.erase(key);
	if (_userSettings.contains(key)) {
		if (anyAreEquals(_userSettings.getAny(key), _systemSettings.getAny(key))) {
			_userSettings.remove(key);
		}
	}
}

void CascadingSettings::setResourcesDir(const std::string& dir) {
	String tmp = File::convertToUnixPathSeparators(dir);
	if (!tmp.endsWith("/")) {
		tmp += "/";
	}
	_resourcesDir = tmp;
}

std::string CascadingSettings::getResourcesDir() const {
	return File::convertPathSeparators(_resourcesDir);
}
