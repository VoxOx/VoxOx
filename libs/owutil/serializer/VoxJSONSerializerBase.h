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

#ifndef VOX_JSON_SERIALIZER_H
#define VOX_JSON_SERIALIZER_H

#include <serialization/Serializable.h>
#include <string>
#include <list>
#include <json_spirit.h>
#include <cassert>
#include <util/Date.h>

/**
 * Provides convenience methods for JSON serialization.
 *
 * @author Jeff Theinert
 */
class JSONSerializerBase : public Serializable 
{
public:
	JSONSerializerBase();

	void addValue( const char* tag, const std::string& value );
	void addValue( const char* tag, const char*        value );
	void addValue( const char* tag, const int		   value );
	void addValue( const char* tag, const Date		   value );

	void openObject( bool intoObject = false );
	void openObject( const char* tag, bool intoObject = false );
	void closeObject();

	void openArray( const char* tag );
	void closeArray();

	std::string generateKeyTag( int nCount );
	std::string toString( bool bFmt = false );


	//Transformation methods
	bool   toBool( const std::string& val )				{ return toBool( val.c_str() );	}
	bool   toBool( const char*  val );
	bool   toBool( const json_spirit::Value& value );

	int	   toInt ( const std::string& val )				{ return toInt( val.c_str() );	}
	int    toInt ( const char*  val );
	int	   toInt ( const json_spirit::Value& value );

	double toDouble( const std::string& val )			{ return toDouble( val.c_str() );	}
	double toDouble( const char*  val );

	Date   toDate( const std::string& val )				{ return toDate( val.c_str() );	}
	Date   toDate( const char*  val );

protected:
	json_spirit::Value_type getCurrentType();
	void					addPair( const json_spirit::Pair& pair );

private:
	std::list<json_spirit::Value*>	_values;
	json_spirit::Value				_baseValue;			//Will contain initial Object/Array.
	json_spirit::Value*				_currentValue;
};

#endif //VOX_JSON_SERIALIZER_H
