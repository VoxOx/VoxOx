/*
	VOXOX !!!!!
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

#include <serializer/VoxXMLSerializerBase.h>

#include <assert.h>
#include <util/String.h>
#include <util/StringList.h>
#include <tinyxml.h>

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::serializeTag( const char* tag, const std::string& value, bool bSkipEmpty, bool useCData )
{
	std::string result  = "";

	if ( shouldHandle( bSkipEmpty, value ) )
	{
		result += "<";
		result += tag;
		result += ">";

		if ( useCData )
		{
			result += "<![CDATA[";
		}
		
		result += value;
		
		if ( useCData )
		{
			result += "]]>";
		}
		
		result += "</";
		result += tag;
		result += ">";
	}

	return result;
}

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::serializeTag( const char* tag, const char* value, bool bSkipEmpty, bool useCData )
{
	std::string strTemp(value);
	return serializeTag( tag, strTemp, bSkipEmpty, useCData );
}

//-----------------------------------------------------------------------------
	
std::string XMLSerializerBase::serializeTag( const char* tag, const int value, bool bSkipEmpty, bool useCData )
{
	String str = String::fromNumber( value );

	return serializeTag( tag, str, bSkipEmpty, useCData );
}

//-----------------------------------------------------------------------------
	
std::string XMLSerializerBase::serializeTag( const char* tag, const Date value, bool bSkipEmpty, bool useCData )
{
	std::string str = value.toString();
	return serializeTag( tag, str, bSkipEmpty, useCData );
}

//-----------------------------------------------------------------------------
	
std::string XMLSerializerBase::serializeTag( const char* tag  )
{
	std::string result = "";

	result += "<";
	result += tag;
	result += "/>";

	return result;
}

//-----------------------------------------------------------------------------


std::string XMLSerializerBase::serializeTagAndType( const char* tag, const char* type, const std::string& value,	
												    bool bSkipEmpty, bool useCData )
{
	std::string result  = "";

	if ( shouldHandle( bSkipEmpty, value ) )
	{
		result += "<";
		result += tag;

		result += " type=\"";
		result += type;

		result += "\">";

		if ( useCData )
		{
			result += "<![CDATA[";
		}
		
		result += value;
		
		if ( useCData )
		{
			result += "]]>";
		}
		
		result += "</";
		result += tag;
		result += ">";
	}

	return result;
}

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::serializeTagAndType( const char* tag, const char* type, const char* value, 
												    bool bSkipEmpty, bool useCData )
{
	std::string strTemp(value);
	return serializeTagAndType( tag, type, strTemp, bSkipEmpty, useCData );
}

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::serializeTagAndType( const char* tag, const char* type, const int value, 
												    bool bSkipEmpty, bool useCData )
{
	String str = String::fromNumber( value );

	return serializeTagAndType( tag, type, str, bSkipEmpty, useCData );
}

//-----------------------------------------------------------------------------
	
std::string XMLSerializerBase::generateKeyTag( int nCount )
{
	char str[50];
	sprintf( str, "key_%d", nCount );

	std::string result = str;
	return result;
}

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::openTag( const char* tag )
{
	_mainTags.push_front(tag);

	return formatTag( tag, true );
}

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::closeTag()
{
	assert( _mainTags.size() > 0 );
	std::string result = formatTag( _mainTags.front(), false );
	_mainTags.pop_front();

	return result;
}

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::formatTag( const char* tag, bool bOpen )
{
	std::string ret = "";
	
	ret += "<";

	if ( !bOpen )
	{
		ret += "/";
	}

	ret += tag;
	ret += ">";

	return ret;
}

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::fromNode( TiXmlNode* node )
{
	std::string result = "";
	TiXmlNode* subNode = NULL;

	if ( node != NULL )
	{
		switch( node->Type() )
		{
		case TiXmlNode::ELEMENT:
			subNode = node->FirstChild();
			if ( subNode )
			{
				result = subNode->Value();
			}
			break;

		case TiXmlNode::TEXT:
			result = node->Value();
			break;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::fromNode( TiXmlNode* rootNode, const char* tag )
{
	std::string result = "";

	if ( rootNode )
	{
		TiXmlNode* node = rootNode->FirstChild( tag );
		result = fromNode( node );
	}

	return result;
}

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::fromNode( TiXmlHandle& rootElt, const char* tag )
{
	TiXmlNode* node = rootElt.FirstChild( tag ).FirstChild().Node();
	return fromNode( node );
}

//-----------------------------------------------------------------------------

std::string XMLSerializerBase::fromAttr( TiXmlNode* rootNode, const char* tag )
{
	std::string result = "";

	if ( rootNode )
	{
		TiXmlElement* element = rootNode->ToElement();

		if ( element )
		{
			result = element->Attribute( tag );
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
//Transformations
//-----------------------------------------------------------------------------
	
bool XMLSerializerBase::toBool( const char*  val )
{
	bool bRet = false;

	if ( (strcmp( val, "1" ) == 0 ) ||
		 (strcmp( val, "Y" ) == 0 ) ||
		 (strcmp( val, "y" ) == 0 ) ||
		 (strcmp( val, "T" ) == 0 ) ||
		 (strcmp( val, "t" ) == 0 ) ||
		 (strcmp( val, "TRUE" ) == 0 ) ||
		 (strcmp( val, "true" ) == 0 ) )
	{
		bRet = true;
	}

	return bRet;
}

//-----------------------------------------------------------------------------

int XMLSerializerBase::toInt( const char*  val )
{
	return atoi( val );
}

//-----------------------------------------------------------------------------

double XMLSerializerBase::toDouble( const char*  val )
{
	return atof( val );
}

//-----------------------------------------------------------------------------

Date XMLSerializerBase::toDate( const char*  val )
{
	std::string delimiter = "-";
	Date result;

	StringList tokens = String(val).split( delimiter );

	int year  = String(tokens[0]).toInteger();
	int month = String(tokens[1]).toInteger();
	int day   = String(tokens[2]).toInteger();

	if ( month > 0 && day > 0 )
	{
		result.setYear ( year  );
		result.setMonth( month );
		result.setDay  ( day   );
	}

	return result;
}

//-----------------------------------------------------------------------------

bool XMLSerializerBase::shouldHandle( bool bSkipEmpty, const std::string& value )
{
	bool bHandle = true;

	if ( bSkipEmpty )
	{
		bHandle = !value.empty();
	}

	return bHandle;
}

//-----------------------------------------------------------------------------
