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

#ifndef OWWINDOWSREGISTRY_H
#define OWWINDOWSREGISTRY_H

#include <system/owsystemdll.h>

#include <windows.h>

#include <string>

/**
 * Class to manipulate the Windows registry database.
 *
 * @author Mathieu Stute
 */
class WindowsRegistry {
public:

	/**
	 * Tests the existence of a registry key.
	 *
	 * @param rootKey the root key of type HKEY (HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, ...).
	 * @param key the key to test.
	 * @return true if exists otherwise return false.
	 */
	OWSYSTEM_API static bool keyExists(HKEY rootKey, const std::string & key);

	/**
	 * Tests the existence of a registry entry.
	 *
	 * @param rootKey the root key of type HKEY (HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, ...).
	 * @param subkey the subkey where the search the entry.
	 * @param entry the entry to test.
	 * @return true if exists otherwise return false.
	 */
	OWSYSTEM_API static bool entryExists(HKEY rootKey, const std::string & subkey, const std::string & entry);

	/**
	 * Gets the value of a registry entry.
	 *
	 * @param rootKey the root key of type HKEY (HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, ...).
	 * @param subkey the subkey where the search the entry.
	 * @param entry the entry to test.
	 * @return the value of the entry if success otherwise an empty string.
	 */
	OWSYSTEM_API static std::string getValue(HKEY rootKey, const std::string & subkey, const std::string & entry);

	/**
	 * Creates a registry entry.
	 *
	 * @param rootKey the root key of type HKEY (HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, ...).
	 * @param subkey the subkey where the key must be created.
	 * @param entry the name of the key.
	 * @return true if success otherwise false.
	 */
	OWSYSTEM_API static bool createKey(HKEY rootKey, const std::string & subkey, const std::string & key, const std::string & defaultValue = "");

	/**
	 * Deletes a registry entry.
	 *
	 * @param rootKey the root key of type HKEY (HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, ...).
	 * @param subkey the subkey where the key is.
	 * @param entry the name of the key.
	 * @return true if success otherwise false.
	 */
	OWSYSTEM_API static bool deleteKey(HKEY rootKey, const std::string & subkey, const std::string & key);

	/**
	 * Creates an entry in a registry key.
	 *
	 * @param rootKey the root key of type HKEY (HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, ...).
	 * @param subkey the subkey where the entry will be.
	 * @param entry the name of the entry.
	 * @param value value of the entry.
	 * @return true if success otherwise false.
	 */
	OWSYSTEM_API static bool createEntry(HKEY rootKey, const std::string & subkey, const std::string & entry, const std::string & value);

	/**
	 * Removes an entry to a registry key.
	 *
	 * @param rootKey the root key of type HKEY (HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, ...).
	 * @param subkey the subkey where the entry is.
	 * @param entry the name of the entry.
	 * @return true if success otherwise false.
	 */
	OWSYSTEM_API static bool removeEntry(HKEY rootKey, const std::string & subkey, const std::string & entry);
};

#endif	//OWWINDOWSREGISTRY_H
