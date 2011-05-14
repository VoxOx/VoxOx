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

#ifndef OWAVATAR_H
#define OWAVATAR_H

#include <string>

/**
 * Represents an Avatar.
 *
 * @author Philippe Bernery
 */
class Avatar {
public:

	/**
	 * @param fullpath Fullpath to the avatar file.
	 */
	Avatar(std::string fullpath);

	Avatar(const Avatar & avatar);

	~Avatar();

	/**
	 * @return the name of the Avatar. Usually the last part
	 * of the path to the Avatar.
	 */
	std::string getName() const;

	/**
	 * @return the full path to the Avatar.
	 */
	std::string getFullPath() const;

protected:

	std::string _fullpath;

};

#endif //OWAVATAR_H
