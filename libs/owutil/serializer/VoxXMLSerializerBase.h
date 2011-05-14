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

#ifndef VOX_XML_SERIALIZER_H
#define VOX_XML_SERIALIZER_H

#include <serialization/Serializable.h>
#include <string>
#include <list>
//#include <tinyxml.h>
#include <util/Date.h>

class TiXmlNode;
class TiXmlHandle;

/**
 * Provides convenience methods for XML serialization.
 *
 * @author Jeff Theinert
 */
class XMLSerializerBase : public Serializable 
{
public:

	std::string serializeTag( const char* tag, const std::string& value, bool bSkipEmpty = true, bool useCData = false );
	std::string serializeTag( const char* tag, const char*        value, bool bSkipEmpty = true, bool useCData = false );
	std::string serializeTag( const char* tag, const int		  value, bool bSkipEmpty = true, bool useCData = false );
	std::string serializeTag( const char* tag, const Date		  value, bool bSkipEmpty = true, bool useCData = false );
	std::string serializeTag( const char* tag );

	std::string serializeTagAndType( const char* tag, const char* type, const std::string& value, bool bSkipEmpty = true, bool useCData = false );
	std::string serializeTagAndType( const char* tag, const char* type, const char*        value, bool bSkipEmpty = true, bool useCData = false );
	std::string serializeTagAndType( const char* tag, const char* type, const int		   value, bool bSkipEmpty = true, bool useCData = false );

	std::string openTag( const char* tag );
	std::string closeTag();
	std::string formatTag( const std::string& tag, bool bOpen )		{ return formatTag( tag.c_str(), bOpen );	}
	std::string formatTag( const char* tag, bool bOpen );
	std::string generateKeyTag( int nCount );

	std::string fromNode( TiXmlNode* node );
	std::string fromNode( TiXmlNode* rootNode,  const char* tag );
	std::string fromNode( TiXmlHandle& rootElt, const char* tag );

	std::string fromAttr( TiXmlNode* rootNode,  const char* tag );

	//Transformation methods
	bool   toBool( const std::string& val )				{ return toBool( val.c_str() );	}
	bool   toBool( const char*  val );

	int	   toInt ( const std::string& val )				{ return toInt( val.c_str() );	}
	int    toInt ( const char*  val );

	double toDouble( const std::string& val )			{ return toDouble( val.c_str() );	}
	double toDouble( const char*  val );

	Date   toDate( const std::string& val )				{ return toDate( val.c_str() );	}
	Date   toDate( const char*  val );

protected:
	bool shouldHandle( bool bSkipEmpty, const std::string& value );

private:
//	std::string	_mainTag;
	std::list<std::string>	_mainTags;
};

#endif //VOX_XML_SERIALIZER_H
