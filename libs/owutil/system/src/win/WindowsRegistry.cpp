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

#include <system/WindowsRegistry.h>

bool WindowsRegistry::keyExists(HKEY rootKey, const std::string & key) {
	HKEY hKey;
	if (::RegOpenKeyExA(rootKey, key.c_str(), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
		::RegCloseKey(hKey);
		return true;
	}
	::RegCloseKey(hKey);
	return false;
}

bool WindowsRegistry::entryExists(HKEY rootKey, const std::string & subkey, const std::string & entry) {
	HKEY hKey;
	if (::RegOpenKeyExA(rootKey, subkey.c_str(), 0, KEY_QUERY_VALUE, &hKey)
		== ERROR_SUCCESS) {
		DWORD dwDataType = REG_SZ;
		DWORD dwSize = 255;
		if (ERROR_SUCCESS == ::RegQueryValueExA(hKey, entry.c_str(), 0, &dwDataType,
			(BYTE *) NULL, &dwSize)) {
			::RegCloseKey(hKey);
			return true;
		}
	}
	::RegCloseKey(hKey);
	return false;
}

std::string WindowsRegistry::getValue(HKEY rootKey, const std::string & subkey, const std::string & entry) {
	HKEY hKey;
	if (::RegOpenKeyExA(rootKey, subkey.c_str(), 0, KEY_QUERY_VALUE, &hKey)
		== ERROR_SUCCESS) {
		DWORD dwDataType = REG_SZ;
		DWORD dwSize = 255;
		char * entryValue = new char[dwSize];
		if (ERROR_SUCCESS == ::RegQueryValueExA(hKey, entry.c_str(), 0, &dwDataType,
			(BYTE *) entryValue, &dwSize)) {
			::RegCloseKey(hKey);
			return std::string(entryValue);
		}
	}
	::RegCloseKey(hKey);
	return "";
}

bool WindowsRegistry::createKey(HKEY rootKey, const std::string & subkey, const std::string & key, const std::string & defaultValue) {
	HKEY hKey;
	if (::RegOpenKeyExA(rootKey, subkey.c_str(), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
		if (::RegCreateKeyA(hKey, key.c_str(), &hKey) == ERROR_SUCCESS) {

			if (!defaultValue.empty()) {
				if (::RegSetValueExA(hKey, "", 0, REG_SZ,
					(const BYTE *) defaultValue.c_str(), defaultValue.length()) == ERROR_SUCCESS) {
					::RegCloseKey(hKey);
					return true;
				} else {
					::RegCloseKey(hKey);
					return false;
				}
			}
			::RegCloseKey(hKey);
			return true;
		}
	}
	::RegCloseKey(hKey);
	return false;
}

bool WindowsRegistry::deleteKey(HKEY rootKey, const std::string & subkey, const std::string & key) {
	HKEY hKey;
	if (::RegOpenKeyExA(rootKey, subkey.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
		if (::RegDeleteKeyA(hKey, key.c_str()) == ERROR_SUCCESS) {
			::RegCloseKey(hKey);
			return true;
		}
	}
	::RegCloseKey(hKey);
	return false;
}

bool WindowsRegistry::createEntry(HKEY rootKey, const std::string & subkey, const std::string & entry, const std::string & value) {
	HKEY hKey;
	::RegCreateKeyA(rootKey, subkey.c_str(), &hKey);
	if (::RegSetValueExA(hKey, entry.c_str(), 0, REG_SZ,
		(const BYTE *) value.c_str(), value.length()) == ERROR_SUCCESS) {
		::RegCloseKey(hKey);
		return true;
	} else {
		::RegCloseKey(hKey);
		return false;
	}
}

bool WindowsRegistry::removeEntry(HKEY rootKey, const std::string & subkey, const std::string & entry) {
	HKEY hKey;
	if (::RegOpenKeyExA(rootKey, subkey.c_str(), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
		if (::RegDeleteValueA(hKey, entry.c_str()) != ERROR_SUCCESS) {
			::RegCloseKey(hKey);
			return false;
		} else {
			::RegCloseKey(hKey);
			return true;
		}
	}
	return false;
}
