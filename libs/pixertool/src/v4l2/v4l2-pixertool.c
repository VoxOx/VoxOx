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

#include <pixertool/v4l2-pixertool.h>

#include <sys/types.h>

#include <asm/types.h>

#include <linux/videodev2.h>

pixosi pix_v4l2_to_pix_osi(unsigned int pix) {
	pixosi palette;

	switch (pix) {
	case V4L2_PIX_FMT_YUV420:
		palette = PIX_OSI_YUV420P;
		break;
	case V4L2_PIX_FMT_YUYV:
		palette = PIX_OSI_YUYV;
		break;
	case V4L2_PIX_FMT_UYVY:
		palette = PIX_OSI_UYVY;
		break;
	case V4L2_PIX_FMT_YUV422P:
		palette = PIX_OSI_YUV422P;
		break;
	case V4L2_PIX_FMT_YUV411P:
		palette = PIX_OSI_YUV411;
		break;
	case V4L2_PIX_FMT_NV12:
		palette = PIX_OSI_NV12;
		break;
	case V4L2_PIX_FMT_RGB555:
		palette = PIX_OSI_RGB555;
		break;
	case V4L2_PIX_FMT_RGB565:
		palette = PIX_OSI_RGB565;
		break;
	case V4L2_PIX_FMT_BGR32:
		palette = PIX_OSI_RGB32;
		break;
	case V4L2_PIX_FMT_BGR24:
		palette = PIX_OSI_RGB24;
		break;
	default:
		palette = PIX_OSI_UNSUPPORTED;
	}

	return (palette);
}

unsigned int pix_v4l2_from_pix_osi(pixosi pix) {
	unsigned int pixfmt;

	switch (pix) {
	case PIX_OSI_YUV420P:
		pixfmt = V4L2_PIX_FMT_YUV420;
		break;
	case PIX_OSI_YUV422:
	case PIX_OSI_YUYV:
	case PIX_OSI_YUY2:
		pixfmt = V4L2_PIX_FMT_YUYV;
		break;
	case PIX_OSI_UYVY:
		pixfmt = V4L2_PIX_FMT_UYVY;
		break;
	case PIX_OSI_YUV422P:
		pixfmt = V4L2_PIX_FMT_YUV422P;
		break;
	case PIX_OSI_YUV411:
		pixfmt = V4L2_PIX_FMT_YUV411P;
		break;
	case PIX_OSI_NV12:
		pixfmt = V4L2_PIX_FMT_NV12;
		break;
	case PIX_OSI_RGB555:
		pixfmt = V4L2_PIX_FMT_RGB555;
		break;
	case PIX_OSI_RGB565:
		pixfmt = V4L2_PIX_FMT_RGB565;
		break;
	case PIX_OSI_RGB32:
		pixfmt = V4L2_PIX_FMT_BGR32;
		break;
	case PIX_OSI_RGB24:
		pixfmt = V4L2_PIX_FMT_BGR24;
		break;
	default:
		pixfmt = 0;
	}

	return pixfmt;
}
