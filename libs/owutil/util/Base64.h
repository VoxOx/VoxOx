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

#ifndef OWBASE64_H
#define OWBASE64_H

#include <util/owutildll.h>
#include <util/NonCopyable.h>

#include <string>

/**
 * Base64 encoding and decoding.
 *
 * Base64 is commonly used in everyday SMTP mail messages, for example.
 * Note that encoding is not compression: it simply reduces a byte stream
 * to printable ASCII characters, actually increasing the size of the data
 * as a side effect (e.g. where the original data is a 100-byte stream
 * containing values 0-255, the base64 encoding will be a 136-byte stream
 * containing values 0-63, the effect being that these can be ASCII-encoded).
 *
 * @see http://www.ietf.org/rfc/rfc3548.txt
 * @author Tanguy Krotoff
 */
class OWUTIL_API Base64 : NonCopyable {
public:

	/**
	 * Encodes using base64.
	 *
	 * @param stringToEncode string to encode
	 * @return encoded string
	 */
	 static std::string encode(const std::string & stringToEncode);

	/**
	 * Decodes using base64.
	 *
	 * @param encodedString encoded string
	 * @return decoded string
	 */
	 static std::string decode(const std::string & encodedString);
};

#endif	//OWBASE64_H
