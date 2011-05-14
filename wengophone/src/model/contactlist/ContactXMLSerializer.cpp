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
#include "ContactXMLSerializer.h"

#include "Contact.h"
#include "ContactGroup.h"
#include "ContactList.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/ProfileXMLSerializer.h>

#include <imwrapper/IMContactXMLSerializer.h>
#include <imwrapper/IMContactSet.h>

#include <util/String.h>

#include <tinyxml.h>

using namespace std;

ContactXMLSerializer::ContactXMLSerializer(Contact & contact,
	ContactList & contactList, IMAccountList & imAccountList)
	: ProfileXMLSerializer(contact),
	_contact(contact),
	_contactList(contactList),
	_imAccountList(imAccountList) {
}

string ContactXMLSerializer::serialize() {
	string result;

	//VOXOX CHANGE by Rolando 01-13-09
	result += "<wgcard version=\"1.0\" xmlns=\"http://www.voxox.com/wgcard/1.0\">\n";

	result += ProfileXMLSerializer::serialize();

	//Serializing IMContacts
	for (IMContactSet::const_iterator it = _contact._imContactSet.begin();
		it != _contact._imContactSet.end();
		++it) 
	{
		IMContactXMLSerializer imContactSerializer((IMContact &)*it, _imAccountList);
		result += imContactSerializer.serialize();
	}
	////

	//Serializing group
	result += openTag( "groups" );

	////TODO: iterate once we support multiple user groups.
	//ContactGroup * contactGroup = _contactList.getContactGroup( _contact.getFirstUserGroupId() );	//VOXOX - JRT - 2009.05.06 - JRT-GRPS
	//if (contactGroup) {
	//	assert( contactGroup->isUser() );
	//	result += "<group><![CDATA[" + contactGroup->getName() + "]]></group>\n";
	//}

	//VOXOX - JRT - 2009.08.07 - TODO: Serialize ContactGroupInfoSet
	ContactGroupInfoSet& cgis = _contact.getContactGroupInfoSet();

	for ( ContactGroupInfoSet::const_iterator it = cgis.begin(); it != cgis.end(); it++ )
	{
		if ( (*it).second.isUser() )
		{
			std::string key = (*it).second.getKey();
			ContactGroup* group = _contactList.getContactGroup( key );

			if ( group )
			{
				result += serializeTag( "group", group->getName(), true, true );
			}
		}
	}

	result += closeTag();

	//serializing UUID
	result += serializeTag( "UUID",		 _contact.getKey() );
	result += serializeTag( "TIMESTAMP", _contact.getTimestamp() );

	result += "</wgcard>\n";

	return result;
}

bool ContactXMLSerializer::unserialize(const string & data) {
	TiXmlDocument doc;
	EnumIMProtocol imProtocol;
	bool result = true;
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string defaultRealm = config.getWengoRealm();

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle wgCard = docHandle.FirstChild("wgcard");

	ProfileXMLSerializer::unserializeContent(wgCard);

	//retrieving UUID
	//VOXOX - JRT - 2009.04.21 - UUID is controlled by ContactList.
//	TiXmlNode * uuid = wgCard.FirstChild("UUID").Node();
//	if (uuid) {
//		_contact.setUUID(string(uuid->FirstChild()->Value()));
//	}
	////

	//Retrieving Groups
	//VOXOX - JRT - 2009.08.07 - Support multiple groups.
	TiXmlNode * groups = wgCard.FirstChild("groups").Node();

	if ( groups )
	{
		TiXmlNode * lastGroup = NULL;
		while (result && (lastGroup = groups->IterateChildren("group", lastGroup))) 
		{
			if (lastGroup) 
			{
				Group grp;
				grp.setName( lastGroup->FirstChild()->Value() );
				_contact.getGroups().Add( &grp );
			}
		}
	}
	else		//For backward compatibility
	{
		TiXmlNode * group = wgCard.FirstChild("group").Node();
		if (group) 
		{
			Group grp;
			grp.setName( group->FirstChild()->Value() );
			_contact.getGroups().Add( &grp );
		}
	}
	////

	//Retrieving IMContacts
	TiXmlNode * imLastChild = NULL;
	while (result && (imLastChild = wgCard.Node()->IterateChildren("im", imLastChild))) {
		string imData;
		IMContact imContact(EnumIMProtocol::IMProtocolUnknown, String::null);
		IMContactXMLSerializer imContactSerializer(imContact, _imAccountList);
		imContactSerializer.setDefaultRealm(defaultRealm);

		imData << *imLastChild;
		result = imContactSerializer.unserialize(imData);

		_contact._addIMContact(imContact);
	}

	_contact.setTimestamp( toInt( fromNode( wgCard, "TIMESTAMP" ) ) );

	//This is NOT serialization.
	_contact.updatePresenceState();

	return result;
}
