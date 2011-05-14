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

#ifndef OWSERIALIZABLE_H
#define OWSERIALIZABLE_H

#include <serialization/owserializationdll.h>

#include <string>

/**
 * Serializability of a class is enabled by the class implementing this interface.
 *
 * @author Tanguy Krotoff
 */
class Serializable {
public:

	OWSERIALIZATION_API virtual ~Serializable() {
	}

	/**
	 * Cannot be const.
	 */
	OWSERIALIZATION_API virtual std::string serialize() /*const*/ = 0;

	OWSERIALIZATION_API virtual bool unserialize(const std::string & data) = 0;
};

#endif	//OWSERIALIZABLE_H
