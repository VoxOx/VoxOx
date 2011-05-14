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

#ifndef OWCONTACTLISTXMLSERIALIZER1_H
#define OWCONTACTLISTXMLSERIALIZER1_H

#include <serialization/Serializable.h>

#include <string>

class ContactList;
class IMAccountList;

/**
 * 1st version of ContactListXMLSerializer1. Uses ContactXMLSerializer1.
 *
 * @author Philippe Bernery
 */
class ContactListXMLSerializer1 : public Serializable {
public:

	ContactListXMLSerializer1(ContactList & contactList, IMAccountList & _imAccountList);

	std::string serialize();

	bool unserialize(const std::string & data);

private:

	ContactList & _contactList;

	IMAccountList & _imAccountList;

};

#endif //OWCONTACTLISTXMLSERIALIZER1_H
