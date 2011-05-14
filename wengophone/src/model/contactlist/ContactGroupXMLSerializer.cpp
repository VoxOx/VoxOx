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
#include "ContactGroupXMLSerializer.h"

using namespace std;

//=============================================================================

ContactGroupSetXMLSerializer::ContactGroupSetXMLSerializer( ContactGroupSet& groupSet )	//VOXOX - JRT - 2009.05.05 
	: _groupSet( groupSet ) 
{
}

//-----------------------------------------------------------------------------

string ContactGroupSetXMLSerializer::serialize() 
{
	string result = "";
	
	result += openTag("groups");

	for ( ContactGroupSet::iterator it = _groupSet.begin(); it != _groupSet.end(); ++it)	//VOXOX - JRT - 2009.05.05 
	{
		ContactGroup *temp;
		temp = const_cast<ContactGroup*>(&(*it).second);	//VOXOX - JRT - 2009.05.05 

		ContactGroupXMLSerializer contactGroupSerializer( (*temp) );
		result += contactGroupSerializer.serialize();
	}

	result += closeTag();

	return result;
}

//-----------------------------------------------------------------------------

bool ContactGroupSetXMLSerializer::unserialize( TiXmlNode* groupsNode )
{
	bool bRet = false;

	if ( groupsNode )
	{
		TiXmlElement* groupsElt = groupsNode->ToElement();
		std::string groupsData;
		groupsData << *groupsElt;

		bRet = unserialize( groupsData );
	}

	return bRet;
}

//-----------------------------------------------------------------------------

bool ContactGroupSetXMLSerializer::unserialize(const string & data ) 
{
	TiXmlDocument doc;
	bool result = true;

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle groupsNode = docHandle.FirstChild("groups");

	//Retrieving groups
	TiXmlNode * imLastChild = NULL;
	while (result && (imLastChild = groupsNode.Node()->IterateChildren("group", imLastChild))) 
	{
		string data;
		ContactGroup contactGroup( "", EnumGroupType::GroupType_User );	//VOXOX - JRT - 2009.08.12 - GroupType will be changed in unserialize().
		ContactGroupXMLSerializer contactGroupSerializer( contactGroup );

		data << *imLastChild;
		result = contactGroupSerializer.unserialize(data);

		_groupSet.Add1( contactGroup, NULL );		//VOXOX - JRT - 2009.05.05 
	}

	return result;
}

//=============================================================================


//=============================================================================

ContactGroupXMLSerializer::ContactGroupXMLSerializer( ContactGroup& group )
	: _group( group ) 
{
}

//-----------------------------------------------------------------------------

string ContactGroupXMLSerializer::serialize() 
{
	string result = "";

	result += openTag( "group" );

	result += serializeTag( "name",  _group.getName(),	    false, true  );
	result += serializeTag( "open",  _group.isOpen(),       false, false );
	result += serializeTag( "order", _group.getSortOrder(), false, false );
	result += serializeTag( "type",  _group.getType(),		false, false );

	result += closeTag();
	result += "\n";

	return result;
}

//-----------------------------------------------------------------------------

bool ContactGroupXMLSerializer::unserialize(const string& data) 
{
	TiXmlDocument doc;
	bool result = true;

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle groupNode = docHandle.FirstChild("group");

	if (groupNode.ToNode()) 
	{
		_group.setName	   (		 parseElement( groupNode, "name"  ) );
		_group.setIsOpen   ( toBool( parseElement( groupNode, "open"  ) ) );
		_group.setSortOrder( toInt ( parseElement( groupNode, "order" ) ) );
		_group.setType     ( (EnumGroupType::GroupType)toInt( parseElement( groupNode, "type" ) ) );
	}

	return result;
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.05.01 - TODO: this should be part of a base class
std::string ContactGroupXMLSerializer::parseElement( TiXmlHandle& rootElt, const char* childTag )
{
	std::string strValue = "";
	TiXmlNode* node = rootElt.FirstChild(childTag).FirstChild().Node();

	if ( node )
	{
		strValue = node->Value();
	}

	return strValue;
}

//=============================================================================
