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

#include <pixertool/v4l-pixertool.h>

#include <sys/types.h>

#include <linux/videodev.h>

pixosi pix_v4l_to_pix_osi(int pix) {
	pixosi palette;

	switch (pix) {
	case VIDEO_PALETTE_YUV420P:
	case VIDEO_PALETTE_YUV420:
		palette = PIX_OSI_YUV420P;
		break;

	case VIDEO_PALETTE_YUV422:
	case VIDEO_PALETTE_YUYV:
		palette = PIX_OSI_YUV422;
		break;

	case VIDEO_PALETTE_UYVY:
		palette = PIX_OSI_UYVY;
		break;

	case VIDEO_PALETTE_YUV422P:
		palette = PIX_OSI_YUV422P;
		break;

	case VIDEO_PALETTE_RGB32:
		palette = PIX_OSI_RGB32;
		break;

	case VIDEO_PALETTE_RGB24:
		palette = PIX_OSI_RGB24;
		break;
	}

	return (palette);
}

int pix_v4l_from_pix_osi(pixosi pix) {
	int palette;

	switch (pix) {
	case PIX_OSI_YUV420P:
		palette = VIDEO_PALETTE_YUV420P;
		break;
	case PIX_OSI_YUV422:
	case PIX_OSI_YUYV:
	case PIX_OSI_YUY2:
		palette = VIDEO_PALETTE_YUV422;
		break;
	case PIX_OSI_UYVY:
		palette = VIDEO_PALETTE_UYVY;
		break;
	case PIX_OSI_YUV422P:
		palette = VIDEO_PALETTE_YUV422P;
		break;
	case PIX_OSI_RGB32:
		palette = VIDEO_PALETTE_RGB32;
		break;
	case PIX_OSI_RGB24:
		palette = VIDEO_PALETTE_RGB24;
	default:
		palette = PIX_OSI_UNSUPPORTED;
	}

	return palette;
}
