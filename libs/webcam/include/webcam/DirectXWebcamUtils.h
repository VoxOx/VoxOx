/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

/**
 * @file DirectXWebcamUtils.h
 *
 * DirectX utility functions.
 *
 * @author Mathieu Stute
 */

#ifndef OWDIRECTXWEBCAMUTILS_H
#define OWDIRECTXWEBCAMUTILS_H

#include <tchar.h>
#include <dshow.h>
#include <atlbase.h>

//VOXOX - JRT - 2009.08.28 - Fix for problem with Windows DirectX SDK
#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
//END fix.

#include <qedit.h>
#include <comutil.h> //for _bstr_t

HRESULT FindMyCaptureDevice(IBaseFilter ** pF, BSTR bstrName);

/**
 * Takes a base filter and try to return a pointer to its IAMStreamConfig.
 *
 * @return a NULL pointer in the case it fails
 */
IAMStreamConfig * GetIAMStreamConfig(IBaseFilter * pFilter);

#endif	//OWDIRECTXWEBCAMUTILS_H
