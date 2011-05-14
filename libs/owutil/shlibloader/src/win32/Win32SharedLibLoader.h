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

#ifndef OWWIN32SHAREDLIBLOADER_H
#define OWWIN32SHAREDLIBLOADER_H

#include <shlibloader/ISharedLibLoader.h>

#include <windows.h>

/**
 * Windows implementation for shlibloader.
 *
 * @author Tanguy Krotoff
 */
class Win32SharedLibLoader : public ISharedLibLoader {
public:

	Win32SharedLibLoader(const std::string & fileName);

	bool load();

	bool unload();

	void * resolve(const std::string & symbol);

private:

	HINSTANCE _pHnd;
};

#endif	//OWWIN32SHAREDLIBLOADER_H
