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

#ifndef OWSHAREDLIBLOADER_H
#define OWSHAREDLIBLOADER_H

#include <shlibloader/owshlibloaderdll.h>

#include <shlibloader/ISharedLibLoader.h>

/**
 * Loads shared libraries at runtime.
 *
 * The aim is to be multiplatform using different backends.
 *
 * @see QLibrary
 * @author Tanguy Krotoff
 */
class SharedLibLoader : public ISharedLibLoader {
public:

	OWSHLIBLOADER_API SharedLibLoader(const std::string & fileName);

	OWSHLIBLOADER_API ~SharedLibLoader();

	OWSHLIBLOADER_API bool load();

	OWSHLIBLOADER_API bool unload();

	OWSHLIBLOADER_API void * resolve(const std::string & symbol);

	/**
	 * @see ISharedLibLoader::resolve()
	 */
	OWSHLIBLOADER_API static void * resolve(const std::string & fileName, const std::string & symbol);

	/**
	 * Gets the system shared library extension.
	 *
	 * Windows: .dll
	 * MacOSX: .dylib
	 * UNIX: .so
	 */
	OWSHLIBLOADER_API static std::string getSharedLibExtension();

private:

	/** Private implementation. */
	ISharedLibLoader * _loaderPrivate;
};

#endif	//OWSHAREDLIBLOADER_H
