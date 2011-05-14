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

#ifndef OWQUICKTIME_PIXERTOOL_H
#define OWQUICKTIME_PIXERTOOL_H

#include <pixertool/pixertool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Palette identifier conversion.
 *
 * Converts from v4l palette identifier to
 * pixer palette identifier
 *
 * @param pix palette identifier to convert
 * @return equivalent pixosi identifier
 */
pixosi pix_quicktime_to_pix_osi(int pix);

/**
 * Palette identifier conversion.
 *
 * Converts from pixer palette identifier to
 * v4l palette identifier
 *
 * @param pix palette identifier to convert
 * @return equivalent v4l identifier
 */
int pix_quicktime_from_pix_osi(pixosi pix);

/**
 * Gets a string describing the format.
 *
 * @param pix pixel format quicktime id
 * @return the string describing the format
 */
const char * pix_quicktime_to_string(unsigned pix);

#ifdef __cplusplus
}
#endif

#endif	//OWQUICKTIME_PIXERTOOL_H
