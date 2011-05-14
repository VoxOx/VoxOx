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

#ifndef OWCONTACTXMLSERIALIZER1_H
#define OWCONTACTXMLSERIALIZER1_H

#include <model/profile/ProfileXMLSerializer.h>

#include <string>

class Contact;
class ContactList;
class IMAccountList;

/**
 * This is the 1st version of ContactXMLSerializer1.
 *
 *  It uses IMContactXMLSerializer1
 *
 * @author Philippe Bernery
 */
class ContactXMLSerializer1 : public ProfileXMLSerializer {
public:

	ContactXMLSerializer1(Contact & contact, ContactList & contactList, 
		IMAccountList & imAccountList);

	std::string serialize();

	bool unserialize(const std::string & data);

private:

	Contact & _contact;

	ContactList & _contactList;

	IMAccountList & _imAccountList;
};

#endif //OWCONTACTXMLSERIALIZER1_H
