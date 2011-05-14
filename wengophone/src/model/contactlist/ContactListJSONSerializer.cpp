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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "ContactListJSONSerializer.h"

#include "ContactXMLSerializer.h"
#include "ContactList.h"
#include "Contact.h"

#include <tinyxml.h>

//-----------------------------------------------------------------------------

ContactListJSONSerializer::ContactListJSONSerializer(ContactList & contactList )	//, IMAccountList & imAccountList)
	: _contactList(contactList)
//	_imAccountList(imAccountList) {
{
}

//-----------------------------------------------------------------------------

std::string ContactListJSONSerializer::serialize() 
{
	int nCount = 0;
	int			groupIdJ  = 0;
	std::string groupId   = "";
	std::string groupName = "";
	ContactGroupInfo* contactGroupInfo = NULL;
	ContactGroup*	  contactGroup     = NULL;

	_contactList.lock();

	const Contacts& rContacts = _contactList.getContacts();

	openArray( "" );

	for (Contacts::const_iterator it = rContacts.begin(); it != rContacts.end(); ++it) 
	{
		openObject();

		contactGroupInfo = const_cast<ContactGroupInfoSet&>((*it).second.getContactGroupInfoSetConst()).FindFirstUserGroupInfo();
		groupId			 = (contactGroupInfo ? contactGroupInfo->getKey() : "");

		contactGroup     = _contactList.getContactGroup( groupId  );
		groupName		 = (contactGroup ? contactGroup->getName() : "" );
		groupIdJ		 = (contactGroup ? contactGroup->getId()   : 0  );

		addValue( "id",			(*it).second.getId()		);
		addValue( "contactid",	(*it).second.getKey()		);
		addValue( "jid",		(*it).second.getContactId()	);
		addValue( "groupid",	groupIdJ					);
		addValue( "fname",		(*it).second.getFirstName() );
		addValue( "lname",		(*it).second.getLastName()  );
		addValue( "company",	(*it).second.getCompany()   );
		addValue( "groupId",	groupId   );
		addValue( "groupName",  groupName );

		closeObject();
	}

	closeArray();

	_contactList.unlock();

	std::string result = toString( false );

	return result;
}

//-----------------------------------------------------------------------------

bool ContactListJSONSerializer::unserialize(const std::string & data) 
{
	bool result = true;

	json_spirit::Value value;

	if ( json_spirit::read( data, value ) )
	{

	}

	return result;
}
