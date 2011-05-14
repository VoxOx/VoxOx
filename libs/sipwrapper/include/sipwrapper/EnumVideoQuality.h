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

#ifndef OWENUMVIDEOQUALITY_H
#define OWENUMVIDEOQUALITY_H

#include <util/NonCopyable.h>

#include <string>

/**
 * Video quality.
 *
 * Video quality depends on your Internet connection bandwidth.
 *
 * @author Tanguy Krotoff
 */
class EnumVideoQuality : NonCopyable {
public:

	enum VideoQuality {

		/** Down=0-512kbit/s up=0-128kbit/s. */
		VideoQualityNormal,

		/** Down=512-2048kbit/s up=128-256kbit/s. */
		VideoQualityGood,

		/** Down=+2048kbit/s up=+256kbit/s. */
		VideoQualityVeryGood,

		/** Down=+8192kbit/s up=+1024kbit/s. */
		VideoQualityExcellent
	};

	/**
	 * Converts a VideoQuality into a string.
	 *
	 * @return the string
	 */
	static std::string toString(VideoQuality videoQuality);

	/**
	 * Converts a string into a VideoQuality.
	 *
	 * @return the VideoQuality
	 */
	static VideoQuality toVideoQuality(const std::string & videoQuality);
};

#endif	//OWENUMVIDEOQUALITY_H
