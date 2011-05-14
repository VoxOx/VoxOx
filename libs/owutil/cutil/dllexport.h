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

#ifndef OWDLLEXPORT_H
#define OWDLLEXPORT_H

#include <cutil/global.h>

/**
 * @file dllexport.h
 *
 * Header that export symbols for creating a .dll under Windows.
 * Example:
 * <pre>
 * #ifdef MY_DLL
 * 	#ifdef BUILDING_DLL
 * 		#define API DLLEXPORT
 * 	#else
 * 		#define API DLLIMPORT
 * 	#endif
 * #else
 * 	#define API
 * #endif
 *
 * class API MyClass
 * API int publicFunction()
 * class EXCEPTIONAPI(API) PublicThrowableClass
 * class DLLLOCAL MyClass
 * </pre>
 *
 * You should define DLL if you want to use the library as a shared one
 * + when you build the library you should define BUILDING_DLL
 * GCC > v4.0 support library visibility attributes.
 *
 * @see http://gcc.gnu.org/wiki/Visibility
 * @author Tanguy Krotoff
 */
//Shared library support
#ifdef OS_WIN32
	#define DLLIMPORT __declspec(dllimport)
	#define DLLEXPORT __declspec(dllexport)
	#define DLLLOCAL
	#define DLLPUBLIC
#else
	#define DLLIMPORT
	#ifdef CC_GCC4
		#define DLLEXPORT __attribute__ ((visibility("default")))
		#define DLLLOCAL __attribute__ ((visibility("hidden")))
		#define DLLPUBLIC __attribute__ ((visibility("default")))
	#else
		#define DLLEXPORT
		#define DLLLOCAL
		#define DLLPUBLIC
	#endif
#endif

//Throwable classes must always be visible on GCC in all binaries
#ifdef OS_WIN32
	#define EXCEPTIONAPI(api) api
#elif defined(GCC_HASCLASSVISIBILITY)
	#define EXCEPTIONAPI(api) DLLEXPORT
#else
	#define EXCEPTIONAPI(api)
#endif

#endif	//OWDLLEXPORT_H
