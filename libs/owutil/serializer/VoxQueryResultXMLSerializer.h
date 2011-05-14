/*
 * VoxOx
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

#ifndef VOX_QUERY_RESULT_SERIALIZER_H
#define VOX_QUERY_RESULT_SERIALIZER_H

#include <thread/RecursiveMutex.h>

#include <serializer/VoxXMLSerializerBase.h>
#include <string>

class TiXmlHandle;

//=============================================================================

class QueryResult
{
public:
	enum EnumMethod
	{
		EnumMethod_None					= 0,
		EnumMethod_SaveUserProfile		= 1,
		EnumMethod_GetUserProfile		= 2,

		EnumMethod_SaveContactProfile	= 3,
		EnumMethod_GetContactProfile	= 4,
		EnumMethod_RemoveContactProfile	= 5,

		EnumMethod_SaveUserNetwork		= 6,
		EnumMethod_RemoveUserNetwork	= 7,
		EnumMethod_GetUserNetwork		= 8,
	};

	QueryResult()			{ initVars();	}
	virtual ~QueryResult()	{}

	void	init()										{ initVars();	}

	std::string getQuestion()	 const					{ return _question;		}
	std::string	getFilter()		 const					{ return _filter;		}
	std::string	getFilterValue() const					{ return _filterValue;	}
	double		getQueryTime()	 const					{ return _queryTime;	}
	bool		succeeded()		 const					{ return _succeeded;	}
	int			getResponse()	 const					{ return _response;		}
	EnumMethod	getEnumMethod()	 const					{ return _enumMethod;	}

	std::string	getContactId()	 const					{ return _contactId;	}
	int			getUserNetworkId() const				{ return _userNetworkId;}
	int			getTimestamp()	 const					{ return _timestamp;	}


	void setQuestion   ( const std::string& val )		{ _question		= val;	}
	void setFilter	   ( const std::string& val )		{ _filter		= val;	}
	void setFilterValue( const std::string& val )		{ _filterValue	= val;	}
	void setQueryTime  ( double		 val )				{ _queryTime	= val;	}
	void setSucceeded  ( bool		 val )				{ _succeeded	= val;	}
	void setResponse   ( int		 val )				{ _response		= val;	}
	void setEnumMethod ( EnumMethod  val )				{ _enumMethod	= val;	}

	void setContactId( const std::string& val )			{ _contactId	= val;	}
	void setUserNetworkId( int val )					{ _userNetworkId = val;	}
	void setTimestamp	 ( int val )					{ _timestamp	= val;	}

protected:
	void initVars()
	{
		std::string empty = "";

		setQuestion   ( empty );
		setFilter	  ( empty );
		setFilterValue( empty );
		setQueryTime  (	   0 );
		setSucceeded  ( false );
		setResponse   (     0 );
		setEnumMethod ( EnumMethod_None );

		setContactId    ( "" );
		setUserNetworkId( 0  );
		setTimestamp	( 0  );
	}

private:
	std::string	_question;
	std::string	_filter;
	std::string	_filterValue;
	double		_queryTime;
	bool		_succeeded;
	int			_response;
	EnumMethod  _enumMethod;

	std::string	_contactId;
	int			_userNetworkId;
	int			_timestamp;
};

//=============================================================================

//This is one-way.  We only UNserialize.
class QueryResultXMLSerializer : public XMLSerializerBase
{
public:
	QueryResultXMLSerializer( QueryResult& queryResult );

	bool unserializeQuery ( const std::string& xml );
	bool unserializeQuery ( TiXmlNode* queryNode   );

	bool unserializeStatus( const std::string& xml );
	bool unserializeStatus( TiXmlNode* statusNode  );

	bool unserializeResponse( const std::string& xml );
	bool unserializeResponse( TiXmlNode* responseNode  );

protected:
	std::string serialize()								{ return "";	}
	bool		unserialize(const std::string& str )	{ return true;	}

private:
	QueryResult& _queryResult;
};

#endif //VOX_QUERY_RESULT_SERIALIZER_H
