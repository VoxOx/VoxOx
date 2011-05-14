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
//#include "stdafx.h"
#include <serializer/VoxQueryResultXMLSerializer.h>
#include <tinyxml.h>

//=============================================================================

QueryResultXMLSerializer::QueryResultXMLSerializer( QueryResult& queryResult )
	: _queryResult(queryResult) 
{
}

//-----------------------------------------------------------------------------

bool QueryResultXMLSerializer::unserializeQuery( const std::string& xml )
{
	TiXmlDocument doc;

	doc.Parse(xml.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlNode*  queryNode = docHandle.FirstChild("QUERY").Node();

	return unserializeQuery( queryNode );
}

//-----------------------------------------------------------------------------

bool QueryResultXMLSerializer::unserializeQuery( TiXmlNode* queryNode )
{
	bool result = true;

	if (queryNode) 
	{
		_queryResult.setQuestion   (		   fromNode( queryNode, "question"    ) );
		_queryResult.setFilter     (		   fromNode( queryNode, "filterField" ) );
		_queryResult.setFilterValue(		   fromNode( queryNode, "filterValue" ) );
//		_queryResult.setQueryTime  ( toDouble( fromNode( queryNode, "queryTook" ) ) );
		_queryResult.setQueryTime  ( toDouble( fromNode( queryNode, "T"			) ) );
	}

	return result;
}

//-----------------------------------------------------------------------------

bool QueryResultXMLSerializer::unserializeResponse( const std::string& xml )
{
	TiXmlDocument doc;

	doc.Parse(xml.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlNode*  node = docHandle.FirstChild("response").Node();

	return unserializeStatus( node );
}

//-----------------------------------------------------------------------------

bool QueryResultXMLSerializer::unserializeResponse( TiXmlNode* responseNode )
{
	bool result = true;

	if ( responseNode )
	{
		_queryResult.setResponse( toInt( fromNode( responseNode ) ) );
	}

	return result;
}

//-----------------------------------------------------------------------------

bool QueryResultXMLSerializer::unserializeStatus( const std::string& xml )
{
	TiXmlDocument doc;

	doc.Parse(xml.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlNode*  node = docHandle.FirstChild("status").Node();

	return unserializeStatus( node );
}

//-----------------------------------------------------------------------------

bool QueryResultXMLSerializer::unserializeStatus( TiXmlNode* statusNode )
{
	bool result = true;

	if ( statusNode )
	{
		std::string value = fromNode( statusNode );

		_queryResult.setSucceeded( value == "success" );
	}

	return result;
}

//-----------------------------------------------------------------------------
