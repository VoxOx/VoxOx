/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#ifndef OWIMACCOUNTXMLSERIALIZER1_H
#define OWIMACCOUNTXMLSERIALIZER1_H

#include <serialization/Serializable.h>

class IMAccount;

/**
 * Serializes an IMAccount object.
 *
 * @author Philippe Bernery
 */
class IMAccountXMLSerializer1 : public Serializable {
public:

	IMAccountXMLSerializer1(IMAccount & imAccount);

	void setIMAccountParametersData(const std::string & imAccountParametersData);

	std::string serialize();

	bool unserialize(const std::string & data);

private:

	IMAccount & _imAccount;
	std::string _imAccountParametersData;
};

#endif	//OWIMACCOUNTXMLSERIALIZER1_H
