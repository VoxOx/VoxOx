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

#include <shlibloader/SharedLibLoader.h>

#include <util/String.h>
#include <util/SafeDelete.h>

#include <cutil/global.h>

#if defined(OS_WINDOWS)
	#include "win32/Win32SharedLibLoader.h"
#elif defined(OS_POSIX)
	#include "posix/PosixSharedLibLoader.h"
#else
	#include "null/NullSharedLibLoader.h"
#endif

SharedLibLoader::SharedLibLoader(const std::string & fileName) {
	_loaderPrivate = NULL;
	String tmp(fileName);

	if (!tmp.contains(SharedLibLoader::getSharedLibExtension(), false)) {
		tmp += SharedLibLoader::getSharedLibExtension();
	}

#if defined(OS_WINDOWS)
	_loaderPrivate = new Win32SharedLibLoader(tmp);
#elif defined(OS_POSIX)
	_loaderPrivate = new PosixSharedLibLoader(tmp);
#else
	_loaderPrivate = new NullSharedLibLoader(tmp);
#endif
}

SharedLibLoader::~SharedLibLoader() {
	OWSAFE_DELETE(_loaderPrivate);
}

bool SharedLibLoader::load() {
	return _loaderPrivate->load();
}

bool SharedLibLoader::unload() {
	return _loaderPrivate->unload();
}

void * SharedLibLoader::resolve(const std::string & symbol) {
	return _loaderPrivate->resolve(symbol);
}

void * SharedLibLoader::resolve(const std::string & fileName, const std::string & symbol) {
	SharedLibLoader loader(fileName);
	if (loader.load()) {
		return loader.resolve(symbol);
	}
	return NULL;
}

std::string SharedLibLoader::getSharedLibExtension() {
#if defined(OS_WINDOWS)
	return ".dll";
#elif defined(OS_LINUX)
	return ".so";
#elif defined(OS_MACOSX)
	return ".dylib";
#endif
}
