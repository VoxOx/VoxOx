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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "ContactXMLSerializer1.h"

#include "Contact.h"
#include "ContactGroup.h"
#include "ContactList.h"

#include <model/profile/ProfileXMLSerializer.h>

#include <imwrapper/IMContactSet.h>
#include <imwrapper/IMContactXMLSerializer1.h>

#include <util/String.h>

#include <tinyxml.h>

using namespace std;

ContactXMLSerializer1::ContactXMLSerializer1(Contact & contact,
	ContactList & contactList, IMAccountList & imAccountList)
	: ProfileXMLSerializer(contact),
	_contact(contact),
	_contactList(contactList),
	_imAccountList(imAccountList) {
}

string ContactXMLSerializer1::serialize() {
	string result;
	//VOXOX CHANGE by Rolando 01-13-09
	result += "<wgcard version=\"1.0\" xmlns=\"http://www.voxox.com/wgcard/1.0\">\n";

	result += ProfileXMLSerializer::serialize();

	//Serializing IMContacts
	for (IMContactSet::const_iterator it = _contact._imContactSet.begin();
		it != _contact._imContactSet.end();
		++it) {
		IMContactXMLSerializer1 imContactSerializer((IMContact &)*it, _imAccountList);
		result += imContactSerializer.serialize();
	}
	////

	//Serializing group
	//JRT-GRPS
//	ContactGroup * contactGroup = _contactList.getContactGroup(_contact.getGroupId());
//	if (contactGroup) {
//		result += "<group><![CDATA[" + contactGroup->getName() + "]]></group>\n";
//	}
	ContactGroupSet& rGroups = _contactList.getContactGroupSetRef();
	for ( ContactGroupSet::const_iterator it = rGroups.begin(); it != rGroups.end(); it++ )
	{
		if ( (*it).second.isUser() )
		{
			result += "<UUID>"+ (*it).second.getKey() +"</UUID>\n";	
		}
	}
	//End JRT-GRPS

	//serializing UUID
//	result += "<UUID>"+ _contact.getUUID() +"</UUID>\n";
	result += "<UUID>"+ _contact.getKey() +"</UUID>\n";	//VOXOX - JRT - 2009.04.28 
	////

	result += "</wgcard>\n";

	return result;
}

bool ContactXMLSerializer1::unserialize(const string & data) {
	TiXmlDocument doc;
	EnumIMProtocol imProtocol;
	bool result = true;

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle wgCard = docHandle.FirstChild("wgcard");

	ProfileXMLSerializer::unserializeContent(wgCard);

	//retrieving UUID
	TiXmlNode * uuid = wgCard.FirstChild("UUID").Node();
	if (uuid) {
		_contact.setUUID(string(uuid->FirstChild()->Value()));
	}
	////

	//Retrieving Groups
	TiXmlNode * group = wgCard.FirstChild("group").Node();
	if (group) {
		ContactGroup contactGroup( string(group->FirstChild()->Value()), EnumGroupType::GroupType_User);
		_contactList._addToContactGroup( contactGroup, _contact);		//VOXOX - JRT - 2009.05.07 JRT-GRPS
//		_contactList._addToContactGroup(string(group->FirstChild()->Value()), _contact);
	}
	////

	//Retrieving IMContacts
	TiXmlNode * imLastChild = NULL;
	while (result && (imLastChild = wgCard.Node()->IterateChildren("im", imLastChild))) {
		string imData;
		IMContact imContact(EnumIMProtocol::IMProtocolUnknown, String::null);
		IMContactXMLSerializer1 imContactSerializer(imContact, _imAccountList);

		imData << *imLastChild;
		result = imContactSerializer.unserialize(imData);

		_contact._addIMContact(imContact);
	}
	////

	_contact.updatePresenceState();

	return result;
}
