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

#ifndef DLLTEST_H
#define DLLTEST_H

#include <dllexport.h>

/**
 * Creates a DLL for testing purpose.
 *
 * @file dlltest.h
 * @author Tanguy Krotoff
 */

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

#ifdef DLLTEST_DLL
	#ifdef BUILDING_DLL
		#define API DLLEXPORT
	#else
		#define API DLLIMPORT
	#endif
#else
	#define API
#endif

API void function_noparam();

API void function_param_int(int i);

API int function_return_int();

API int * function_return_pointer();

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif	//DLLTEST_H
