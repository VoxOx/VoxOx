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

//#define _JRT_TEST_DATA_	//Uncomment to add test data.

//==============================================================

#include "stdafx.h"
#include "ContactProfileJSONSerializer.h"

#include "ContactProfile.h"
#include "ContactList.h"

using namespace std;

ContactProfileJSONSerializer::ContactProfileJSONSerializer( ContactList& contactList, ContactProfile& profile)
	: _contactList(contactList),
	  _profile(profile) 
{
//	_method = "";
}

//-----------------------------------------------------------------------------

std::string ContactProfileJSONSerializer::serialize()
{
	IMContact* imContact = _profile.getPreferredIMContact();
	std::string imContactId = (imContact ? imContact->getContactId() : "no Id");
	std::string networkKey  = (imContact ? imContact->getIMAccountId() : "no Id");	//VOXOX - JRT - 2009.09.18 

	//Some needed conversions.
	string gender = EnumSex::toServerString( _profile.getSex() );

	openObject();	//Main
	openObject( "pinfo" );
	
	addValue( "id",				_profile.getId()			);
	addValue( "contactid",		imContactId					);
	addValue( "networkkey",		networkKey					);	//VOXOX - JRT - 2009.09.18 

	addValue( "fname",			_profile.getFirstName()		);
//	addValue( "mname",			_profile.getMiddleName()	);
	addValue( "lname",			_profile.getLastName()		);
	addValue( "company",		_profile.getCompany()		);
	addValue( "title",			_profile.getTitle()			);

	addValue( "fullname",		_profile.getCompleteName()	);
	addValue( "nickname",		_profile.getAlias()			);
	addValue( "gender",			gender						);	
	addValue( "bithday",		_profile.getBirthdate()		);	
	addValue( "timezone",		_profile.getTimeZone()		);
	addValue( "description",	_profile.getDescription()	);
	addValue( "interests",		_profile.getInterests()		);

	addValue( "language",		_profile.getLanguage()		);
	addValue( "systementry",	 _profile.isSystemEntry()	);
	addValue( "searchable",		_profile.isSearchable()		);
	addValue( "smssignature",	_profile.getSmsSignature()	);
//	addValue( "statusmsg",		_profile.getStatusMessage()	);

	closeObject();	//pinfo


	serializeGroup();
	serializePhone();
	serializeEmail();
	serializeAddress();
	serializeWebSite();
	serializeNetwork();

//	serializeAvatar();
	serializeMergedContacts();
	serializeWorkPlace();

	closeObject();	//Main

	string result = toString ( false );

//	unserialize( result );	//JRT-XXX testing.

	return result;
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::serializeGroup()
{
	std::string groupId   = "";
	std::string groupName = "";
	ContactGroupInfo* contactGroupInfo = NULL;
	ContactGroup*	  contactGroup     = NULL;

	ContactGroupInfoSet& cgis = _profile.getContactGroupInfoSet();

	int nCount = 0;
	openArray( "groups" );

	for ( ContactGroupInfoSet::iterator it = cgis.begin(); it != cgis.end(); it++ )
	{
		groupId		 = (*it).second.getKey();
		contactGroup = _contactList.getContactGroup( groupId  );

		if ( contactGroup )
		{
			if ( contactGroup->isUser() )
			{
				openObject();

				addValue( "id",			contactGroup->getId()   );
				addValue( "groupid",	contactGroup->getKey()  );
				addValue( "groupName",  contactGroup->getName() );
				addValue( "type",		contactGroup->getType() );

				closeObject();
			}
		}
	}

	closeArray();
}

//-----------------------------------------------------------------------------

//void ContactProfileJSONSerializer::serializeAvatar()
//{
//	const OWPicture& avatar = _profile.getIcon();
//
//	openArray( "PHOTO" );
//
//	addValue( "TY",  avatar.getType().c_str()		);
//	addValue( "FP",  avatar.getFilename().c_str() );
//	addValue( "MD5", avatar.getMd5().c_str()		);
//	addValue( "BV",  avatar.getData().c_str()		);
//
////	serializeVis( (*it).getVisibility() );
//
//	closeArray();
//}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::serializeAddress()
{
	StreetAddresses& rAddrs = _profile.getStreetAddresses();

	
#ifdef _JRT_TEST_DATA_	//JRT-XXX
	if ( !rAddrs.hasValid() )
	{
		StreetAddress addr;
		addr.setType	( "home");
		addr.setStreet1 ( "street1xxx" );
		addr.setStreet2 ( "street2yyy" );
		addr.setStreet3 ( "street3zzz" );
		addr.setCity	( "my town" );
		addr.setStateProvince( "my state" );
		addr.setCountry	     ( "USA" );
		addr.setPostalCode   ( "92057" );

		rAddrs.Add( &addr );
	}
#endif

//	if ( !rAddrs.hasValid() )
//		return result;

	int nCount = 0;
	openArray( "addresses" );

	for ( StreetAddresses::iterator it = rAddrs.begin(); it != rAddrs.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			openObject();

			addValue( "id",	 ++nCount );
			addValue( "label",		(*it).getType().c_str()    );
			addValue( "street1",	(*it).getStreet1().c_str() );
			addValue( "street2",	(*it).getStreet2().c_str() );
			addValue( "street3",	(*it).getStreet3().c_str() );

			addValue( "city",		(*it).getCity().c_str()		  );
			addValue( "state",	    (*it).getStateProvince().c_str() );
			addValue( "country",	(*it).getCountry().c_str()		  );
			addValue( "postalcode", (*it).getPostalCode().c_str()	  );

			//if ( (*it).isPreferred() )
			//{
			//	result += addValue( "PREF" );
			//}

	//		serializeVis( (*it).getVisibility() );

			closeObject();
		}
	}

	closeArray();
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::serializeEmail()
{
	EmailAddresses& rEmails = _profile.getEmailAddresses();

#ifdef _JRT_TEST_DATA_	//JRT-XXX
	if ( !rEmails.hasValid() )
	{
		EmailAddress email;
		email.setType( "home" );
		email.setAddress( "jrt@j.com" );

		rEmails.Add( &email );
	}
#endif

//	if ( !rEmails.hasValid() )
//		return result;

	int nCount = 0;
	openArray( "emails" );

	for ( EmailAddresses::iterator it = rEmails.begin(); it != rEmails.end(); it++ )
	{
//		if ( (*it).isValid() )
		{
			openObject();

			addValue( "id",    ++nCount );
			addValue( "label", (*it).getType().c_str()    );
			addValue( "email", (*it).getAddress().c_str() );

			closeObject();
		}
	}

	closeArray();
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::serializeNetwork()
{
	IMContactSet& rContacts = _profile.getIMContactSetNonConst();

//	if ( !rPhones.hasValid() )
//		return result;

	std::string	network;

	int nCount = 0;
	openArray( "networks" );

	for ( IMContactSet::iterator it = rContacts.begin(); it != rContacts.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			openObject();

			network = QtEnumIMProtocolMap::getInstance().toString( (*it).getQtProtocol() );

			addValue( "id",		 ++nCount );
			addValue( "label",   (*it).getDisplayContactId()  );
			addValue( "service", network );

			closeObject();
		}
	}

	closeArray();
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::serializePhone()
{
	Telephones& rPhones = _profile.getTelephones();


#ifdef _JRT_TEST_DATA_	//JRT-XXX
	if ( !rPhones.hasValid() )
	{
		Telephone phone;
		phone.setType( "home" );
		phone.setNumber( "1112223333" );

		rPhones.Add( &phone );
	}
#endif
//	if ( !rPhones.hasValid() )
//		return result;

	int nCount = 0;
	openArray( "phones" );

	for ( Telephones::iterator it = rPhones.begin(); it != rPhones.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			openObject();

			addValue( "id",		++nCount );
			addValue( "label", (*it).getType().c_str()   );
			addValue( "phone", (*it).getNumber().c_str() );

			closeObject();
		}
	}

	closeArray();
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::serializeWebSite()
{
	Urls& rUrls = _profile.getUrls();


#ifdef _JRT_TEST_DATA_	//JRT-XXX
	if ( !rUrls.hasValid() )
	{
		Url url;
		url.setType( "personal" );
		url.setUrl ( "jeff.com" );

		rUrls.Add( &url );
	}
#endif
//	if ( !rUrls.hasValid() )
//		return result;

	int nCount = 0;
	openArray( "websites" );

	for ( Urls::iterator it = rUrls.begin(); it != rUrls.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			openObject();

			addValue( "id",		++nCount );
			addValue( "label", (*it).getType().c_str()  );
			addValue( "url",   (*it).getUrl().c_str()	);
	//		serializeVis( (*it).getVisibility() );

			closeObject();
		}
	}

	closeArray();
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::serializeWorkPlace()
{
//	WorkPlaces& rWorkPlaces = _profile.getWorkPlaces();
//
//	for ( WorkPlaces::iterator it = rWorkPlaces.begin(); it != rWorkPlaces.end(); it++ )
//	{
//		openArray( "WKPLACE" );
//
//		addValue( "N",     (*it).getName().c_str()		);
//		addValue( "POS",   (*it).getPosition().c_str()	);
//		addValue( "DESC",  (*it).getDescription().c_str() );
//		addValue( "START", (*it).getStartDate().c_str()	);
//		addValue( "END",   (*it).getEndDate().c_str()		);
//
////		serializeVis( (*it).getVisibility() );
//
//		closeArray();
//	}
}

//-----------------------------------------------------------------------------
//End Serialize methods
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//Unserialize methods
//-----------------------------------------------------------------------------

bool ContactProfileJSONSerializer::unserialize( const std::string& data )
{
	json_spirit::Value value;
	json_spirit::read( data, value );

	json_spirit::Object baseObj = value.get_obj();

	int			size = baseObj.size();
	std::string name = "";

	for ( int x = 0; x < size; x++ )
	{
		const json_spirit::Pair& pair = baseObj[x];

		name = pair.name_;

		if		( name == "pinfo" )
		{
			unserializeInfo( pair.value_.get_obj() );
		}
		else if ( name == "groups" )
		{
			unserializeGroups( pair.value_.get_array() );
		}
		else if ( name == "phones" )
		{
			unserializePhones( pair.value_.get_array() );
		}
		else if ( name == "emails" )
		{
			unserializeEmails( pair.value_.get_array() );
		}
		else if ( name == "addresses" )
		{
			unserializeAddresses( pair.value_.get_array() );
		}
		else if ( name == "websites" )
		{
			unserializeWebSites( pair.value_.get_array() );
		}
		else if ( name == "networks" )
		{
			unserializeNetworks( pair.value_.get_array() );
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::unserializeInfo( const json_spirit::Object& baseObj )
{
	int			size = baseObj.size();
	std::string contactId  = "";
	std::string networkKey = "";

	for ( int x = 0; x < size; x++ )
	{
		const json_spirit::Pair& pair = baseObj[x];

		const std::string&		  name  = pair.name_;
		const json_spirit::Value& value = pair.value_;

		if		( name == "id" )
		{
			_profile.setId( toInt( value ) );
		}
		else if ( name == "contactid" )
		{
			contactId = value.get_str();
		}
		else if ( name == "networkkey" )
		{
			if ( value.type() == json_spirit::str_type )
			{
				networkKey = value.get_str();
			}
		}
		else if ( name == "fname" )
		{
			_profile.setFirstName( value.get_str() );
		}
//		else if ( name == "mname" )
//		{
//			_profile.setMiddleName( value.get_str() );
//		}
		else if ( name == "lname" )
		{
			_profile.setLastName( value.get_str() );
		}
		else if ( name == "company" )
		{
			_profile.setCompany( value.get_str() );
		}
		else if ( name == "title" )
		{
			_profile.setTitle( value.get_str() );
		}
//		else if ( name == "fullname" )
//		{
//			_profile.setFullName( toString( value ) );
//		}
		else if ( name == "nickname" )
		{
			_profile.setAlias( value.get_str() );
		}
		else if ( name == "gender" )
		{
			_profile.setSex( EnumSex::toSex( value.get_str() ) );
//			string gender   = EnumSex::toServerString( _profile.getSex() );
		}
		else if ( name == "birthday" )
		{
			_profile.setBirthdate( toDate( value.get_str() ) );
		}
		else if ( name == "timezone" )
		{
			_profile.setTimeZone( toInt( value ) );
		}
		else if ( name == "description" )
		{
			_profile.setDescription( value.get_str() );
		}
		else if ( name == "interests" )
		{
			_profile.setInterests( value.get_str() );
		}
		else if ( name == "language" )
		{
			_profile.setLanguage( value.get_str() );
		}
		else if ( name == "systementry" )
		{
			_profile.setIsSystemEntry( toBool( value ) );
		}
		else if ( name == "searchable" )
		{
			_profile.setIsSearchable( toBool( value )  );
		}
		else if ( name == "smssignature" )
		{
			_profile.setSmsSignature( value.get_str() );
		}
//		else if ( name == "statusmsg" )
//		{
//			_profile.setStatusMessage( value.get_str() );
//		}
	}

	//If this is an add, we need to create the IMContact entry.
	if ( _profile.getId() <= 0 )
	{
		IMAccount* imAccount = _contactList.getIMAccount( networkKey );

		if ( imAccount )
		{
			IMContact imContact( *imAccount, contactId );
			_profile.addIMContact( imContact );

			delete imAccount;
		}
	}
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::unserializeGroups( const json_spirit::Array& arrayBase )
{
	int			gid		  = 0;
	std::string groupId   = "";
	std::string groupName = "";
	EnumGroupType::GroupType groupType = EnumGroupType::GroupType_User;

	int objectCount = arrayBase.size();

	for ( int x = 0; x < objectCount; x++ )
	{
		const json_spirit::Object& obj = arrayBase[x].get_obj();
		int	size = obj.size();

		for ( int y = 0; y < size; y++ )
		{
			const json_spirit::Pair& pair = obj[y];

			const std::string&		  name  = pair.name_;
			const json_spirit::Value& value = pair.value_;

			if		( name == "id" )
			{
				gid = toInt( value );
			}
			else if ( name == "groupid" )
			{
				groupId = value.get_str();
			}
			else if ( name == "groupName" )
			{
				groupName = value.get_str();
			}
			else if ( name == "type" )
			{
				groupType = (EnumGroupType::GroupType)toInt( value );
			}
		}

		//TODO: how to handle NEW groups - blank GroupId, GID = 0?
		ContactGroupInfo cgi( groupId, groupType );
		_profile.getContactGroupInfoSet().Add( &cgi );
	}
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::unserializePhones( const json_spirit::Array& arrayBase )
{
	int objectCount = arrayBase.size();
	Telephone phone;

	for ( int x = 0; x < objectCount; x++ )
	{
		const json_spirit::Object& obj = arrayBase[x].get_obj();
		int	size = obj.size();

		for ( int y = 0; y < size; y++ )
		{
			const json_spirit::Pair& pair = obj[y];

			const std::string&		  name  = pair.name_;
			const json_spirit::Value& value = pair.value_;

			if		( name == "id" )
			{
//				phone.setId( toInt( value  );
			}
			else if ( name == "label" )
			{
				phone.setType( value.get_str() );
			}
			else if ( name == "phone" )
			{
				phone.setNumber( value.get_str() );
			}
		}

		_profile.getTelephones().Add( &phone );
	}
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::unserializeEmails( const json_spirit::Array& arrayBase )
{
	int objectCount = arrayBase.size();
	EmailAddress email;

	for ( int x = 0; x < objectCount; x++ )
	{
		const json_spirit::Object& obj = arrayBase[x].get_obj();
		int	size = obj.size();

		for ( int y = 0; y < size; y++ )
		{
			const json_spirit::Pair& pair = obj[y];

			const std::string&		  name  = pair.name_;
			const json_spirit::Value& value = pair.value_;

			if		( name == "id" )
			{
//				email.setId( toInt( value ) );
			}
			else if ( name == "label" )
			{
				email.setType( value.get_str() );
			}
			else if ( name == "email" )
			{
				email.setAddress( value.get_str() );
			}
		}

		_profile.getEmailAddresses().Add( &email );
	}
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::unserializeAddresses( const json_spirit::Array& arrayBase )
{
	StreetAddress addr;
	int objectCount = arrayBase.size();

	for ( int x = 0; x < objectCount; x++ )
	{
		const json_spirit::Object& obj = arrayBase[x].get_obj();
		int	size = obj.size();

		for ( int y = 0; y < size; y++ )
		{
			const json_spirit::Pair& pair = obj[y];

			const std::string&		  name  = pair.name_;
			const json_spirit::Value& value = pair.value_;

			if		( name == "id" )
			{
//				addr.setId( toInt( value ) );
			}
			else if ( name == "label" )
			{
				addr.setType( value.get_str() );
			}
			else if ( name == "street1" )
			{
				addr.setStreet1( value.get_str() );
			}
			else if ( name == "street2" )
			{
				addr.setStreet2( value.get_str() );
			}
			else if ( name == "street3" )
			{
				addr.setStreet3( value.get_str() );
			}
			else if ( name == "city" )
			{
				addr.setCity( value.get_str() );
			}
			else if ( name == "state" )
			{
				addr.setStateProvince( value.get_str() );
			}
			else if ( name == "country" )
			{
				addr.setCountry( value.get_str() );
			}
			else if ( name == "postalcode" )
			{
				addr.setPostalCode( value.get_str() );
			}
		}

		_profile.getStreetAddresses().Add( &addr );
	}
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::unserializeWebSites ( const json_spirit::Array& arrayBase )
{
	Url url;
	int objectCount = arrayBase.size();

	for ( int x = 0; x < objectCount; x++ )
	{
		const json_spirit::Object& obj = arrayBase[x].get_obj();
		int	size = obj.size();

		for ( int y = 0; y < size; y++ )
		{
			const json_spirit::Pair& pair = obj[y];

			const std::string&		  name  = pair.name_;
			const json_spirit::Value& value = pair.value_;

			if		( name == "id" )
			{
//				url.setId( toInt( value ) );
			}
			else if ( name == "label" )
			{
				url.setType( value.get_str() );
			}
			else if ( name == "url" )
			{
				url.setUrl( value.get_str() );
			}
		}

		_profile.getUrls().Add( &url );
	}
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::unserializeNetworks ( const json_spirit::Array& arrayBase )
{
	//Merged contacts?
}

//-----------------------------------------------------------------------------

void ContactProfileJSONSerializer::serializeMergedContacts()
{
//	std::string temp   = "";
//
//	MergedContacts& rMCs = _profile.getMergedContacts();
//
////	if ( !rMCs.hasValid() )
////		return result;
//
//	//TODO:
//	for ( MergedContacts::iterator it = rMCs.begin(); it != rMCs.end(); it++ )
//	{
//		if ( (*it).isValid() )
//		{
//			//TODO: we need a base class for this, but do it manually for now.
//
//			temp += "<MERCON ";
//
//			temp += "UID=";
//			temp += (*it).getUsername();
//
//			temp += " NID=";
//			temp += (*it).getNetworkId();
//
//			temp += "/>";
//
//			result += temp;
//		}
//	}
}

//-----------------------------------------------------------------------------
