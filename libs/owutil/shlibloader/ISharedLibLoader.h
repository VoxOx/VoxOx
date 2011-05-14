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

#ifndef OWISHAREDLIBLOADER_H
#define OWISHAREDLIBLOADER_H

#include <util/Interface.h>

#include <string>

/**
 * @see SharedLibLoader
 * @author Tanguy Krotoff
 */
class ISharedLibLoader : Interface {
public:

	virtual ~ISharedLibLoader() { }

	/**
	 * Loads the library.
	 *
	 * @return true if the library was loaded successfully; false otherwise
	 */
	virtual bool load() = 0;

	/**
	 * Unloads the library.
	 *
	 * @return true if the library could be unloaded; false otherwise
	 */
	virtual bool unload() = 0;

	/**
	 * Gets the address of the exported symbol of the shared library.
	 *
	 * Example:
	 * <pre>
	 * typedef int (*AvgFunction)(int, int);
	 * AvgFunction avg = (AvgFunction) library->resolve("avg");
	 * if (avg) {
	 * 	return avg(5, 8);
	 * } else {
	 * 	return -1;
	 * }
	 * </pre>
	 *
	 * @param symbol exported symbol
	 * @return address of the exported symbol
	 */
	virtual void * resolve(const std::string & symbol) = 0;

protected:

	std::string _fileName;
};

#endif	//OWISHAREDLIBLOADER_H
