/*
	VOXOX!!!!

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

#ifndef CONTACT_PROFILE_JSON_SERIALIZER_H
#define CONTACT_PROFILE_JSON_SERIALIZER_H

#include <serializer/VoxJSONSerializerBase.h>
#include <string>

class ContactList;
class ContactProfile;
//class TiXmlHandle;

/**
 * Serialize a ContactProfile object.
 *
 * @author Jeff Theinert
 */
class ContactProfileJSONSerializer : public JSONSerializerBase
{
public:
	ContactProfileJSONSerializer( ContactList & contactList, ContactProfile& profile );

	virtual std::string serialize();

//	bool unserialize( TiXmlHandle& rootElt   );
	bool unserialize( const std::string& data );

//	void setMethod( const std::string& method  )		{ _method = method;	}

protected:
//	std::string	getMethod() const						{ return _method;	}

	void serializeGroup();
	void serializeAddress();
//	void serializeAvatar();
	void serializeEmail();
	void serializeMergedContacts();
	void serializePhone();
	void serializeWebSite();
	void serializeWorkPlace();
	void serializeNetwork();

	void unserializeInfo	 ( const json_spirit::Object& objBase   );
	void unserializeGroups   ( const json_spirit::Array&  arrayBase );
	void unserializePhones   ( const json_spirit::Array&  arrayBase );
	void unserializeEmails   ( const json_spirit::Array&  arrayBase );
	void unserializeAddresses( const json_spirit::Array&  arrayBase );
	void unserializeWebSites ( const json_spirit::Array&  arrayBase );
	void unserializeNetworks ( const json_spirit::Array&  arrayBase );

	ContactList&	_contactList;
	ContactProfile& _profile;
};

#endif //CONTACT_PROFILE_JSON_SERIALIZER_H
