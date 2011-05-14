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

#include <serializer/VoxJSONSerializerBase.h>

#include <assert.h>
#include <util/String.h>
#include <util/StringList.h>

//-----------------------------------------------------------------------------
	
JSONSerializerBase::JSONSerializerBase()
{
	_baseValue	 = NULL;
	_currentValue = NULL;
}

//-----------------------------------------------------------------------------

void JSONSerializerBase::addValue( const char* tag, const std::string& value )
{
	json_spirit::Pair p( tag, value );

	addPair( p );
}

//-----------------------------------------------------------------------------

void JSONSerializerBase::addValue( const char* tag, const char* value )
{
	json_spirit::Pair p( tag, value );

	addPair( p );
}

//-----------------------------------------------------------------------------
	
void JSONSerializerBase::addValue( const char* tag, const int value )
{
	json_spirit::Pair p( tag, value );

	addPair( p );
}

//-----------------------------------------------------------------------------
	
void JSONSerializerBase::addValue( const char* tag, const Date value )
{
	std::string str = value.toString();
	addValue( tag, str );
}

//-----------------------------------------------------------------------------

std::string JSONSerializerBase::toString( bool bFmt )
{
	std::string result = "";
	
	if ( bFmt )
	{
		result = json_spirit::write_formatted( _baseValue );
	}
	else
	{
		result = json_spirit::write( _baseValue );
	}

	return result;
}

//-----------------------------------------------------------------------------
	
std::string JSONSerializerBase::generateKeyTag( int nCount )
{
	char str[50];
	sprintf( str, "key_%d", nCount );

	std::string result = str;
	return result;
}

//-----------------------------------------------------------------------------

void JSONSerializerBase::addPair( const json_spirit::Pair& pair )
{
	json_spirit::Value_type currType = getCurrentType();
	json_spirit::Value_type newType  = pair.value_.type();

	switch ( newType )
	{
	case json_spirit::obj_type:
		switch( currType )
		{
		case json_spirit::obj_type:			//Can add Object to Object
			_currentValue->get_obj().push_back( pair );
			_values.push_back( &(_currentValue->get_obj().back().value_) );
			_currentValue = _values.back();
			break;

		case json_spirit::array_type:		//Can add Object to Array
			_currentValue->get_array().push_back( pair.value_ );
			_values.push_back( &(_currentValue->get_array().back() ) );
			_currentValue = _values.back();
			break;

		case json_spirit::null_type:		//If CurrPair == Null, this is BasePair
			_baseValue = pair.value_;
			_values.push_back( &_baseValue );
			_currentValue = _values.back();
			break;

		default:
			assert(false);
			break;
		}
		break;

	case json_spirit::array_type:
		switch( currType )
		{
		case json_spirit::obj_type:			//Can add Array to Object
			_currentValue->get_obj().push_back( pair );
			_values.push_back( &(_currentValue->get_obj().back().value_) );
			_currentValue = _values.back();
			break;

		case json_spirit::null_type:		//If CurrPair == Null, this is BasePair
			_baseValue = pair.value_;
			_values.push_back( &_baseValue );
			_currentValue = _values.back();
			break;

		default:
			assert(false);
			break;
		}
		break;

	case json_spirit::str_type:
	case json_spirit::bool_type:
	case json_spirit::int_type:
	case json_spirit::real_type:
	case json_spirit::null_type:
		switch( currType )
		{
		case json_spirit::obj_type:		//Can only add these, and only these, to Object
			_currentValue->get_obj().push_back( pair );
			break;

		default:
			assert(false);		//Can only add value pairs to Object.
			break;
		}
		break;

	default:
		assert( false );		//New type
	}
}

//-----------------------------------------------------------------------------
	
json_spirit::Value_type JSONSerializerBase::getCurrentType()
{
	json_spirit::Value_type type = json_spirit::null_type;

	if ( _currentValue )
	{
		type = _currentValue->type();
	}

	return type;
}

//-----------------------------------------------------------------------------

void JSONSerializerBase::openObject( bool intoObject )
{
	openObject( "", intoObject );
}

//-----------------------------------------------------------------------------

void JSONSerializerBase::openObject( const char* tag, bool intoObject )
{
	json_spirit::Object obj;

	json_spirit::Pair p(tag, obj );
	addPair( p );
}

//-----------------------------------------------------------------------------

void JSONSerializerBase::closeObject()
{
	assert( getCurrentType() == json_spirit::obj_type);
	assert( _values.size() > 0 );
	_values.pop_back();

	_currentValue = (_values.size() == 0 ? NULL : _values.back() );
}

//-----------------------------------------------------------------------------

void JSONSerializerBase::openArray( const char* tag )
{
	json_spirit::Array array1;
	json_spirit::Pair p(tag, array1 );

	addPair( p );
}

//-----------------------------------------------------------------------------

void JSONSerializerBase::closeArray()
{
	assert( getCurrentType() == json_spirit::array_type);
	assert( _values.size() > 0 );
	_values.pop_back();

	_currentValue = (_values.size() == 0 ? NULL : _values.back() );
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//Transformations
//-----------------------------------------------------------------------------
			
bool JSONSerializerBase::toBool( const json_spirit::Value& value )
{
	bool result = false;

	switch( value.type() )
	{
	case json_spirit::str_type:
		result = toBool( value.get_str() );
		break;

	case json_spirit::bool_type:
		result = value.get_bool();
		break;

	case json_spirit::int_type:
		result = (value.get_int() != 0);
		break;

	case json_spirit::real_type:
		result = (value.get_real() != 0);
		break;

	case json_spirit::null_type:
	case json_spirit::obj_type:
	case json_spirit::array_type:
	default:
		break;
	}

	return result;
}

//-----------------------------------------------------------------------------
	
bool JSONSerializerBase::toBool( const char*  val )
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
			
int JSONSerializerBase::toInt( const json_spirit::Value& value )
{
	int result = 0;

	switch( value.type() )
	{
	case json_spirit::str_type:
		result = toInt( value.get_str() );
		break;

	case json_spirit::bool_type:
		result = (int)(value.get_bool() );
		break;

	case json_spirit::int_type:
		result = value.get_int();
		break;

	case json_spirit::real_type:
		result = (int)(value.get_real() );
		break;

	case json_spirit::null_type:
	case json_spirit::obj_type:
	case json_spirit::array_type:
	default:
		break;
	}

	return result;
}

//-----------------------------------------------------------------------------

int JSONSerializerBase::toInt( const char*  val )
{
	return atoi( val );
}

//-----------------------------------------------------------------------------

double JSONSerializerBase::toDouble( const char*  val )
{
	return atof( val );
}

//-----------------------------------------------------------------------------

Date JSONSerializerBase::toDate( const char*  val )
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
