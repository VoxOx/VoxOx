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

#ifndef OWSETTINGS_H
#define OWSETTINGS_H

#include <settings/owsettingsdll.h>

#include <serialization/Serializable.h>

#include <thread/RecursiveMutex.h>

#include <util/StringList.h>
#include <util/Event.h>

#include <boost/any.hpp>
#include <map>

/**
 * Persistent platform-independent application settings.
 *
 * @see QSettings
 * @see java.util.Properties
 * @see java.util.Preferences
 * @see commoncpp.Keydata
 * @author Philippe Bernery
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class Settings {
	friend class SettingsXMLSerializer;
public:

	/**
	 * A value has changed inside Settings.
	 *
	 * @param key key whose value has changed
	 */
	Event<void (const std::string & key)> valueChangedEvent;

	OWSETTINGS_API Settings();

	OWSETTINGS_API Settings(const Settings & settings);

	OWSETTINGS_API Settings & operator=(const Settings & settings);

	OWSETTINGS_API virtual ~Settings();

	/**
	 * Gets all the keys.
	 *
	 * @return list of all keys, including subkeys
	 */
	OWSETTINGS_API StringList getAllKeys() const;

	/**
	 * Removes the setting key.
	 * @param key key to remove
	 */
	OWSETTINGS_API void remove(const std::string & key);

	/**
	 * Checks if the specified key exists.
	 *
	 * @return true if there exists a setting called key; returns false otherwise
	 */
	OWSETTINGS_API bool contains(const std::string & key) const;

	/**
	 * Sets the value of a key.
	 *
	 * @param key the key
	 * @param value key value
	 */
	OWSETTINGS_API void set(const std::string & key, const std::string & value);

	/**
	 * @see set()
	 */
	OWSETTINGS_API void set(const std::string & key, const StringList & value);

	/**
	 * @see set()
	 */
	OWSETTINGS_API void set(const std::string & key, bool value);

	/**
	 * @see set()
	 */
	OWSETTINGS_API void set(const std::string & key, int value);

	/**
	 * @see set()
	 */
	OWSETTINGS_API void set(const std::string & key, boost::any value);

	/**
	 * Gets the value for a given key.
	 *
	 * @param key the key
	 */
	OWSETTINGS_API std::string getStringKeyValue(const std::string & key) const;

	/**
	 */
	OWSETTINGS_API StringList getStringListKeyValue(const std::string & key) const;

	/**
	 */
	OWSETTINGS_API bool getBooleanKeyValue(const std::string & key) const;

	/**
	 */
	OWSETTINGS_API int getIntegerKeyValue(const std::string & key) const;

	/**
	 * Gets the value for a given key in a generic manner.
	 *
	 * @see get()
	 */
	OWSETTINGS_API boost::any getAny(const std::string & key) const;

	/**
	 * @return number of elements
	 */
	OWSETTINGS_API unsigned size() const {
		return _keyMap.size();
	}

	/**
	 * Checks if the value is a boolean.
	 *
	 * @param value to check
	 * @return true if value is a boolean; false otherwise
	 */
	OWSETTINGS_API static bool isBoolean(const boost::any & value);

	/**
	 * Checks if the value is an integer.
	 *
	 * @param value to check
	 * @return true if value is an integer; false otherwise
	 */
	OWSETTINGS_API static bool isInteger(const boost::any & value);

	/**
	 * Checks if the value is a std::string.
	 *
	 * @param value to check
	 * @return true if value is a std::string; false otherwise
	 */
	OWSETTINGS_API static bool isString(const boost::any & value);

	/**
	 * Checks if the value is a StringList.
	 *
	 * @param value to check
	 * @return true if value is a StringList; false otherwise
	 */
	OWSETTINGS_API static bool isStringList(const boost::any & value);

protected:

	OWSETTINGS_API virtual void copy(const Settings & settings);

	typedef std::map<const std::string, boost::any> Keys;
	Keys _keyMap;

private:

	/** Mutex for thread-safe. */
	mutable RecursiveMutex _mutex;
};

#endif	//OWSETTINGS_H
