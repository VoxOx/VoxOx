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

#include <system/RegisterProtocol.h>

#include <util/String.h>
#include <cutil/global.h>

#ifdef OS_WINDOWS
	#include <system/WindowsRegistry.h>
#endif

RegisterProtocol::RegisterProtocol(const std::string & protocol)
	: _protocol(protocol) {
}

bool RegisterProtocol::bind(const std::string & command, const std::string & icon, const std::string & url) {
#ifdef OS_WINDOWS
	return (
		WindowsRegistry::createKey(HKEY_CLASSES_ROOT, String::null, _protocol) &&
		WindowsRegistry::createKey(HKEY_CLASSES_ROOT, _protocol, "DefaultIcon", icon) &&
		WindowsRegistry::createEntry(HKEY_CLASSES_ROOT, _protocol , "URL Protocol", url) &&
		WindowsRegistry::createKey(HKEY_CLASSES_ROOT, _protocol, "shell\\open\\command", command)
	);
#else
	return false;
#endif
}

bool RegisterProtocol::isBinded() const {
#ifdef OS_WINDOWS
	return WindowsRegistry::keyExists(HKEY_CLASSES_ROOT, _protocol);
#else
	return false;
#endif
}

bool RegisterProtocol::unbind() {
#ifdef OS_WINDOWS
	return (
		WindowsRegistry::deleteKey(HKEY_CLASSES_ROOT, _protocol, "DefaultIcon") &&
		WindowsRegistry::deleteKey(HKEY_CLASSES_ROOT, _protocol, "shell\\open\\command") &&
		WindowsRegistry::deleteKey(HKEY_CLASSES_ROOT, _protocol, "shell\\open") &&
		WindowsRegistry::deleteKey(HKEY_CLASSES_ROOT, _protocol, "shell") &&
		WindowsRegistry::deleteKey(HKEY_CLASSES_ROOT, String::null, _protocol)
	);
#else
	return false;
#endif
}
