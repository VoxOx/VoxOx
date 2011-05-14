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

#include "Win32SharedLibLoader.h"

#include <util/Logger.h>

Win32SharedLibLoader::Win32SharedLibLoader(const std::string & fileName) {
	_fileName = fileName;
}

bool Win32SharedLibLoader::load() {
	_pHnd = LoadLibraryA(_fileName.c_str());
	if (!_pHnd) {
		LOG_ERROR("couldn't load dll:" + String::fromNumber(GetLastError()));
		return false;
	}
	return true;
}

bool Win32SharedLibLoader::unload() {
	if (_pHnd) {
		if (!FreeLibrary(_pHnd)) {
			LOG_ERROR("couldn't free dll:" + String::fromNumber(GetLastError()));
			return false;
		}
	}
	return true;
}

void * Win32SharedLibLoader::resolve(const std::string & symbol) {
	void * address = (void *) GetProcAddress(_pHnd, symbol.c_str());
	if (!address) {
		LOG_ERROR("couldn't resolve symbol:" + String::fromNumber(GetLastError()));
		return NULL;
	}
	return address;
}
