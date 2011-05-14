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

#ifndef OWSTORAGE_H
#define OWSTORAGE_H

#include <serialization/owserializationdll.h>

#include <util/Interface.h>

#include <string>

/**
 * Storage interface that loads and saves data to a given format (File, RPC...).
 *
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class Storage : Interface {
public:

	OWSERIALIZATION_API virtual ~Storage() {
	}

	/**
	 * Loads the datas from a media.
	 *
	 * @param url the url where to load the data from
	 * @return true if success
	 */
	OWSERIALIZATION_API virtual bool load(const std::string & url) = 0;

	/**
	 * Saves the datas to a media.
	 *
	 * @param url the url where to save the data to
	 * @return true if success
	 */
	OWSERIALIZATION_API virtual bool save(const std::string & url) = 0;
};

#endif //OWSTORAGE_H
