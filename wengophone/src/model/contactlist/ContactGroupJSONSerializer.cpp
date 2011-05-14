/*	
	VOXOX !!!
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

#include "stdafx.h"
#include "ContactGroupJSONSerializer.h"
#include "ContactList.h"

//-----------------------------------------------------------------------------

ContactGroupSetJSONSerializer::ContactGroupSetJSONSerializer( ContactGroupSet& groupSet )
	: _groupSet(groupSet)
{
}

//-----------------------------------------------------------------------------

std::string ContactGroupSetJSONSerializer::serialize() 
{
	int nCount		   = 0;
	int totalContacts  = 0;
	int onlineContacts = 0;

	openArray( "" );

	for (ContactGroupSet::const_iterator it = _groupSet.begin(); it != _groupSet.end(); ++it) 
	{
		if ( (*it).second.isUser() )
		{
			openObject();

			const_cast<ContactGroup&>((*it).second).getCounts( totalContacts, onlineContacts );

			addValue( "id",			(*it).second.getId()   );
			addValue( "groupid",	(*it).second.getKey()  );
			addValue( "groupName",  (*it).second.getName() );
			addValue( "count",		totalContacts		   );

			closeObject();
		}
	}

	closeArray();

	std::string result = toString( false );

	return result;
}

//-----------------------------------------------------------------------------

bool ContactGroupSetJSONSerializer::unserialize(const std::string & data) 
{
	bool result = true;
//	TiXmlDocument doc;
//
//	doc.Parse(data.c_str());
//
//	TiXmlHandle docHandle(&doc);
//	TiXmlNode * contactlist = docHandle.FirstChild("contactlist").Node();
//
//	if (contactlist) {
//		//Retrieving Contacts
//		TiXmlNode * lastChild = NULL;
//		while (result && (lastChild = contactlist->IterateChildren("wgcard", lastChild))) {
//			string nodeData;
//			nodeData << *lastChild;
//			//VOXOX - JRT - 2009.04.26 - Changes needed due to map-based ContactList
////			ContactXMLSerializer serializer(_contactList.createContact(), _contactList, _imAccountList);
////			result = serializer.unserialize(nodeData);
//			Contact* contact = _contactList.createContact();
//			ContactXMLSerializer serializer( *contact, _contactList, _imAccountList);
//			result = serializer.unserialize(nodeData);
//			_contactList.addContact( contact );
//		}
//		////
//	}

	return result;
}
