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

#ifndef OWUUID_H
#define OWUUID_H

#include <util/owutildll.h>

#include <string>

/**
 * Universally Unique IDentifier Generator.
 *
 * @author Philippe Bernery
 * @author Tanguy Krotoff
 */
class OWUTIL_API Uuid {
public:

	/**
	 * FIXME: Generates a pseudo-unique ID.
	 *
	 * Current implementation works with less that 10 ID generated per second,
	 * above 10 an id already generated will be obtain.
	 * Every 4 years there is a risk to get the same ID.
	 * This is the price to fit the ID inside a integer.
	 *
	 * @return unique ID generated
	 */
	 static int generateInteger();

	/**
	 * Generates a UUID.
	 *
	 * @return a string containing a UUID.
	 */
	 static std::string generateString();
};

#endif	//OWUUID_H
