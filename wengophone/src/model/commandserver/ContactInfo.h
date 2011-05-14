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

#ifndef OWCONTACTINFO_H
#define OWCONTACTINFO_H

#include <string>

/**
 * Contact informations retrieved from the flash add contact button.
 *
 * Originaly done as a workaround for a f*c*i*g VS 2003 bug that produces an INTERNAL COMPILER ERROR.
 *
 * @author Mathieu Stute
 * @author Tanguy Krotoff
 */
class ContactInfo {
public:

	/** Wengo name= wengo pseudo. */
	std::string wengoName;

	/** Wdeal service title/name. */
	std::string wdealServiceTitle;

	/** Contact SIP address. */
	std::string sip;

	/** Contact first name. */
	std::string firstname;

	/** Contact last name. */
	std::string lastname;

	/** Contact country. */
	std::string country;

	/** Contact city. */
	std::string city;

	/** Contact state. */
	std::string state;

	/** Contact group. */
	std::string group;

	/** Contact website url. */
	std::string website;
};

#endif	//OWCONTACTINFO_H
