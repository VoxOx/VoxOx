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

#include <pixertool/directx-pixertool.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	GUID pix_ms;
	pixosi pix_osi;
} pixms_pixosi;

pixms_pixosi _ms_table[] = {
	{ MEDIASUBTYPE_YUYV, PIX_OSI_YUYV },
	{ MEDIASUBTYPE_Y411, PIX_OSI_YUV411 },
	{ MEDIASUBTYPE_Y41P, PIX_OSI_YUV41P },
	{ MEDIASUBTYPE_YUY2, PIX_OSI_YUY2 },
	{ MEDIASUBTYPE_YVYU, PIX_OSI_YVYU },
	{ MEDIASUBTYPE_UYVY, PIX_OSI_UYVY },
	{ MEDIASUBTYPE_YV12, PIX_OSI_YV12 },
	{ MEDIASUBTYPE_RGB1, PIX_OSI_RGB1 },
	{ MEDIASUBTYPE_RGB4, PIX_OSI_RGB4 },
	{ MEDIASUBTYPE_RGB8, PIX_OSI_RGB8 },
	{ MEDIASUBTYPE_RGB565, PIX_OSI_RGB565 },
	{ MEDIASUBTYPE_RGB555, PIX_OSI_RGB555 },
	{ MEDIASUBTYPE_RGB24, PIX_OSI_RGB24 },
	{ MEDIASUBTYPE_RGB32, PIX_OSI_RGB32 },
	{ MEDIASUBTYPE_ARGB32, PIX_OSI_ARGB32 },
	{ OUR_MEDIASUBTYPE_I420, PIX_OSI_YUV420P }, //FIXME: OUR_MEDIASUBTYPE_I420 is actually OUR_MEDIASUBTYPE_IYUV
	{ OUR_MEDIASUBTYPE_Y422, PIX_OSI_YUV422 },
	{ OUR_MEDIASUBTYPE_IYUV, PIX_OSI_YUV420P },
	{ OUR_MEDIASUBTYPE_NV12, PIX_OSI_NV12 }
};

pixosi pix_directx_to_pix_osi(GUID pix) {
	register unsigned i;
	pixosi palette = PIX_OSI_UNSUPPORTED;

	for (i = 0; i < sizeof(_ms_table) / sizeof(pixms_pixosi); i++) {
		if (memcmp(&_ms_table[i].pix_ms, &pix, sizeof(GUID)) == 0) {
			palette = _ms_table[i].pix_osi;
			break;
		}
	}
	return palette;
}

GUID pix_directx_from_pix_osi(pixosi pix) {
	register unsigned i;
	GUID palette = MEDIASUBTYPE_NULL;

	for (i = 0; i < sizeof(_ms_table) / sizeof(pixms_pixosi); i++) {
		if (_ms_table[i].pix_osi == pix) {
			return _ms_table[i].pix_ms;
		}
	}
	return palette;
}
