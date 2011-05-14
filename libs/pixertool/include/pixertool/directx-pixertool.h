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

#ifndef OWDIRECTX_PIXERTOOL_H
#define OWDIRECTX_PIXERTOOL_H

//Warning: this file can only be included by a C++ file

#include <pixertool/pixertool.h>

#include <common.h>
#include <dshow.h>

/* I420 isn't defined in any of the headers but everyone seems to use it.... */
extern "C" const __declspec(selectany) GUID OUR_MEDIASUBTYPE_I420 =
	{0x30323449,0x0000,0x0010, {0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71}};

/* IYUV */
extern "C" const __declspec(selectany) GUID OUR_MEDIASUBTYPE_IYUV =
	{0x56555949,0x0000,0x0010, {0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71}};

/* Y444 */
extern "C" const __declspec(selectany) GUID OUR_MEDIASUBTYPE_Y444 =
	{0x34343459, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};

/* Y800 */
extern "C" const __declspec(selectany) GUID OUR_MEDIASUBTYPE_Y800 =
	{0x30303859, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};

/* Y422 */
extern "C" const __declspec(selectany) GUID OUR_MEDIASUBTYPE_Y422 =
	{0x32323459, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};

/* NV12 */
extern "C" const __declspec(selectany) GUID OUR_MEDIASUBTYPE_NV12 =
	{0x3231564E, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};

/**
 * Palette identifier conversion.
 *
 * Converts from directx palette identifier to
 * pixer palette identifier
 *
 * @param pix palette identifier to convert
 * @return equivalent pixosi identifier
 */
PIXERTOOL_API pixosi pix_directx_to_pix_osi(GUID pix);

/**
 * Palette identifier conversion.
 *
 * Converts from pixer palette identifier to
 * directx palette identifier
 *
 * @param pix palette identifier to convert
 * @return equivalent directx identifier
 */
PIXERTOOL_API GUID pix_directx_from_pix_osi(pixosi pix);

#endif	//OWDIRECTX_PIXERTOOL_H
