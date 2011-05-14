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

#include "stdafx.h"
#include "ContactListXMLSerializer.h"

#include "ContactXMLSerializer.h"
#include "ContactList.h"
#include "Contact.h"

#include <imwrapper/IMAccount.h>		//VOXOX - JRT - 2009.07.13 

#include <tinyxml.h>

using namespace std;

ContactListXMLSerializer::ContactListXMLSerializer(ContactList & contactList, IMAccountList & imAccountList)
	: _contactList(contactList),
	_imAccountList(imAccountList) {
}

string ContactListXMLSerializer::serialize() {
	string result;

//JRT_XXX	_contactList.lock();		//VOXOX - JRT - 2009.07.13 
	
	result += "<contactlist>\n";

	for (Contacts::const_iterator it = _contactList._contacts.begin(); it != _contactList._contacts.end(); ++it) 
	{
		ContactXMLSerializer serializer( const_cast<Contact&>((*it).second), _contactList, _imAccountList);
		result += serializer.serialize();
	}

	result += "</contactlist>\n";

//JRT-XXX	_contactList.unlock();	//VOXOX - JRT - 2009.07.13 

	return result;
}

bool ContactListXMLSerializer::unserialize(const std::string & data) {
	TiXmlDocument doc;
	bool result = true;

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlNode * contactlist = docHandle.FirstChild("contactlist").Node();

	if (contactlist) {
		//Retrieving Contacts
		TiXmlNode * lastChild = NULL;
		while (result && (lastChild = contactlist->IterateChildren("wgcard", lastChild))) {
			string nodeData;
			nodeData << *lastChild;

			Contact* contact = _contactList.createContact();
			ContactXMLSerializer serializer( *contact, _contactList, _imAccountList);
			result = serializer.unserialize(nodeData);

			//VOXOX - JRT - 2009.07.13 - HACK to remove old VoxOx domains
			IMContact* imContact = const_cast<IMContact*>(contact->getPreferredIMContact());

			if ( imContact )
			{
				IMAccount* imAccount = const_cast<IMAccount*>(imContact->getIMAccount());

				if ( imAccount )
				{
					if ( ! imAccount->isOldVoxOxDomain() )
					{
						_contactList.addContact( contact );	//VOXOX - JRT - 2009.08.10 - Multiple group support.
					}
				}
			}
		}
		////
	}

	return result;
}

//VOXOX - JRT - 2009.03.30 - Port from v1.1
string ContactListXMLSerializer::serializeForWebservice() 
{
	string result ="";

	_contactList.lock();

	for (Contacts::const_iterator it = _contactList._contacts.begin(); it != _contactList._contacts.end(); ++it) 
	{
		Contact & contact = const_cast<Contact&>((*it).second);

		if(!contact.getIsIMAccountVoxox())
		{
			string contactID = contact.getContactId();	//OK use of getContactId().
								
			string contactProtocol;

			if(!contact.getIsIMAccountGtalk())
			{
				contactProtocol = EnumIMProtocol::toString(contact.getIMProtocol());
			}
			else
			{
				contactProtocol = "Gtalk";
			}

			//Add voxox.com at the end for requirement purposes
			contactProtocol = contactProtocol+".voxox.com";
			result+="<contact>";
			result+="<uri>"+contactID+"</uri>";
			result+="<network>"+contactProtocol+"</network>";
			result+="</contact>";	
		}
	}

	_contactList.unlock();


	return result;
}
//END VOXOX

