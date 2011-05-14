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

#ifndef OWIMCONTACTXMLSERIALIZER1_H
#define OWIMCONTACTXMLSERIALIZER1_H

#include <serialization/Serializable.h>

class IMContact;
class IMAccountList;

/**
 * First version of the IMContact serializer.
 *
 * This serializer expects to have an XML string with this format:
 * <im protocol="_protocol_">
 *   <id>_contactId_</id>
 *   <alias>_optional_alias_</alias>
 *   <account>_optional_linked_account_login_</account>
 *   <photo>_optional_photo_data_in_base64_</photo>
  * </im>
 *
 * Serializes an IMContact object.
 *
 * @author Philippe Bernery
 */
class IMContactXMLSerializer1 : public Serializable {
public:

	IMContactXMLSerializer1(IMContact & imContact, IMAccountList & imAccountList);

	std::string serialize();

	bool unserialize(const std::string & data);

private:

	IMContact & _imContact;

	IMAccountList & _imAccountList;
};

#endif	//OWIMCONTACTXMLSERIALIZER1_H
