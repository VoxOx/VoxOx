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

#include <pixertool/pixertool.h>

piximage * pix_alloc(pixosi pix, unsigned width, unsigned height) {
	return NULL;
}

void pix_free(piximage * ptr) {
}

unsigned pix_size(pixosi pix, unsigned width, unsigned height) {
	return 0;
}

pixerrorcode pix_convert(int flags, piximage * img_dst, piximage * img_src) {
	return PIX_NOK;
}

pixosi pix_ffmpeg_to_pix_osi(int pix) {
	return PIX_OSI_UNSUPPORTED;
}

int pix_ffmpeg_from_pix_osi(pixosi pix) {
	return (int) PIX_OSI_UNSUPPORTED;
}
