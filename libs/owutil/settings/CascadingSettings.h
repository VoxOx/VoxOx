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

#ifndef OWCASCADINGSETTINGS_H
#define OWCASCADINGSETTINGS_H

#include <settings/owsettingsdll.h>

#include <thread/RecursiveMutex.h>

#include <util/StringList.h>
#include <util/Event.h>

#include <boost/any.hpp>
#include <set>
#include <map>

#include <settings/Settings.h>

/**
 * Cascading settings, allowing the user to override system settings.
 */
class CascadingSettings {
	friend class CascadingSettingsXMLSerializer;
public:

	/**
	 * A value has changed inside CascadingSettings.
	 *
	 * @param key key whose value has changed
	 */
	Event<void (const std::string & key)> valueChangedEvent;

	OWSETTINGS_API CascadingSettings();

	OWSETTINGS_API CascadingSettings(const CascadingSettings & settings);

	OWSETTINGS_API CascadingSettings & operator=(const CascadingSettings & settings);

	OWSETTINGS_API virtual ~CascadingSettings();

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
	OWSETTINGS_API bool toggle(const std::string & key);	//VOXOX - JRT - 2009.05.03 - Convenience method

	/**
	 * @see set()
	 */
	OWSETTINGS_API void set(const std::string & key, int value);

	/**
	 * Sets the value of a resource key
	 */
	OWSETTINGS_API void setResource(const std::string & key, const std::string & value);

	OWSETTINGS_API boost::any getAny(const std::string & key) const;

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

	OWSETTINGS_API std::string getResourceKeyValue(const std::string & key) const;

	OWSETTINGS_API void resetToDefaultValue(const std::string & key);

	OWSETTINGS_API boost::any getDefaultValue(const std::string & key) const;

	/**
	 * Gets all the keys.
	 *
	 * @return list of all keys, including subkeys
	 */
	OWSETTINGS_API StringList getAllKeys() const;

	/**
	 * Marks a key as sticky.
	 *
	 * A sticky key is always saved in the user file, even if it's the same as
	 * the system key. This is useful for version for example.
	 */
	OWSETTINGS_API void addStickyKey(const std::string& key);

	OWSETTINGS_API bool isKeySticky(const std::string& key);

	OWSETTINGS_API void removeStickyKey(const std::string& key);

	OWSETTINGS_API void setResourcesDir(const std::string& dir);

	/**
	 * Returns the resources dir, as a system native path. The path will always
	 * ends with a path separator.
	 */
	OWSETTINGS_API std::string getResourcesDir() const;

protected:

	OWSETTINGS_API virtual void copy(const CascadingSettings & settings);

	Settings _systemSettings;
	Settings _userSettings;
	std::set<std::string> _stickyKeySet;

private:
	template<class T, T (Settings::*getter)(const std::string&) const>
	void internalSet(const std::string& key, const T& value);

	template<class T, T (Settings::*getter)(const std::string&) const>
	T internalGet(const std::string & key) const;

	/**
	 * Set both system and user key, then emit valueChangedEvent.
	 * This method should only be used by unserializers.
	 */
	void addKey(const std::string & key, const boost::any& systemValue, const boost::any& userValue);

	std::string _resourcesDir;
};

#endif	//OWCASCADINGSETTINGS_H
