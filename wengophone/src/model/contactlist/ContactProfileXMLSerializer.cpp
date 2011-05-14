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

//==============================================================
//NOTES: JRT - 2009.05.27 
//	#ifdef _JRT1_ is for serialize methods.  
//	#ifdef _JRT_  is for unserialize methods.  
//		If defined, we use <ADR> <ADR>
//		If not defined we use <ADR> <key_x>
//
// It is expected that server XML may change in near future, so I kept the code.
//==============================================================

#include "stdafx.h"
#include "ContactProfileXMLSerializer.h"

#include "ContactProfile.h"
#include "../profile/UserProfile.h"	//VOXOX - JRT - 2009.08.03 

#include <imwrapper/IMAccount.h>

#include <serializer/DateXMLSerializer.h>
#include <util/Base64.h>
#include <tinyxml.h>

#include "WengoPhoneBuildId.h"

//#ifdef _WIN32		//JRT-XXX
//#include <Windows.h>
//#endif

using namespace std;

//=============================================================================

ProfileXMLSerializer2::ProfileXMLSerializer2( Profile & profile)
	: _profile(profile) 
{
	_method = "";
}

//-----------------------------------------------------------------------------

std::string ProfileXMLSerializer2::serialize()
{
	string result    = "";

	assert( !getMethod().empty() );

	//Some needed conversions.
	string gender   = EnumSex::toServerString( _profile.getSex() );
	string buildId  = WengoPhoneBuildId::getVersion();
	
	
//	result += "<Sync generator=\"voxox\" version=\"" + buildId  + "\">";		//TODO: need method for tag w/ attributes.
	result += "<Sync>";			//TODO: the double-quoted attributes above are causing a problem with Apache server.
	result += openTag( getMethod().c_str() );
	result += openTag( "PROFILE" );
	
	
	result += serializeIdentity();		//Virtual to be overridden for ContactProfile and UserProfile

	result += serializeTag( "FN",		_profile.getCompleteName()	);
	result += serializeTag( "NN",		_profile.getAlias()			);
	result += serializeTag( "SEX",		gender						);	
	result += serializeTag( "BDAY",		_profile.getBirthdate()		);	
	result += serializeTag( "TZ",		_profile.getTimeZone()		);
	result += serializeTag( "DESC",		_profile.getDescription()	);
	result += serializeTag( "INTS",		_profile.getInterests()		);

	result += serializeTag( "LANG",		_profile.getLanguage()		);
	result += serializeTag( "SYSENT",	_profile.isSystemEntry()	);
	result += serializeTag( "SRCH",		_profile.isSearchable()		);
	result += serializeTag( "ORG",		_profile.getCompany()		);
	result += serializeTag( "SMSSIGN",	_profile.getSmsSignature()	);

//	result += serializeTag( "STATMSG", _profile.getStatusMessage()	);

	result += serializeName();
	result += serializeAddress();
//	result += serializeAvatar();		//VOXOX - JRT - 2009.08.14 
	result += serializeEmail();
	result += serializePhone();
	result += serializeWebSite();
	result += serializeWorkPlace();

//	result += serializeMergedContacts();

	result += closeTag();	//PROFILE
	result += closeTag();	//syncToServe
	result += "</Sync>";	

	return result;
}

//-----------------------------------------------------------------------------

std::string ProfileXMLSerializer2::serializeName()
{
	string result = "";

	result += openTag( "N" );

	result += serializeTag( "F", _profile.getFirstName(),  true, false );
//	result += serializeTag( "M", _profile.getMiddleName(), true, false );
	result += serializeTag( "L", _profile.getLastName(),   true, false );

//	result += serializeVis( _profile.getNameVis() );

	result += closeTag();

	return result;
}

//-----------------------------------------------------------------------------

std::string ProfileXMLSerializer2::serializeAvatar()
{
	std::string result = "";

	const OWPicture& avatar = _profile.getIcon();

	result += openTag( "PHOTO" );

	result += serializeTag( "TY",  avatar.getType().c_str()		);
	result += serializeTag( "FP",  avatar.getFilename().c_str() );
	result += serializeTag( "MD5", avatar.getMd5().c_str()		);
	result += serializeTag( "BV",  Base64::encode(avatar.getData())	);

//	result += serializeVis( (*it).getVisibility() );

	result += closeTag();

	return result;
}

//-----------------------------------------------------------------------------

std::string ProfileXMLSerializer2::serializeAddress()
{
	std::string result = "";

	StreetAddresses& rAddrs = _profile.getStreetAddresses();

	if ( !rAddrs.hasValid() )
		return result;

#ifdef _JRT1_
	for ( StreetAddresses::iterator it = rAddrs.begin(); it != rAddrs.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += openTag( "ADR" );

			result += serializeTag( "TY",  (*it).getType().c_str()    );
			result += serializeTag( "ST",  (*it).getStreet1().c_str() );
			result += serializeTag( "ST2", (*it).getStreet2().c_str() );
			result += serializeTag( "ST3", (*it).getStreet3().c_str() );

			result += serializeTag( "CITY",  (*it).getCity().c_str()			);
			result += serializeTag( "STATE", (*it).getStateProvince().c_str()	);
			result += serializeTag( "CTRY",	 (*it).getCountry().c_str()			);
			result += serializeTag( "PCODE", (*it).getPostalCode().c_str()		);

			//if ( (*it).isPreferred() )
			//{
			//	result += serializeTag( "PREF" );
			//}

	//		result += serializeVis( (*it).getVisibility() );

			result += closeTag();
		}
	}
#else
	int nCount = 0;
	result += openTag( "ADR" );

	for ( StreetAddresses::iterator it = rAddrs.begin(); it != rAddrs.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += openTag( generateKeyTag( nCount++ ).c_str() );

			result += serializeTag( "TY",  (*it).getType().c_str()    );
			result += serializeTag( "ST",  (*it).getStreet1().c_str() );
			result += serializeTag( "ST2", (*it).getStreet2().c_str() );
			result += serializeTag( "ST3", (*it).getStreet3().c_str() );

			result += serializeTag( "CITY",  (*it).getCity().c_str()		  );
			result += serializeTag( "STATE", (*it).getStateProvince().c_str() );
			result += serializeTag( "CTRY",	 (*it).getCountry().c_str()		  );
			result += serializeTag( "PCODE", (*it).getPostalCode().c_str()	  );

			//if ( (*it).isPreferred() )
			//{
			//	result += serializeTag( "PREF" );
			//}

	//		result += serializeVis( (*it).getVisibility() );

			result += closeTag();
		}
	}

	result += closeTag();
#endif
	return result;
}

//-----------------------------------------------------------------------------

std::string ProfileXMLSerializer2::serializeEmail()
{
	std::string result = "";

	EmailAddresses& rEmails = _profile.getEmailAddresses();

	if ( !rEmails.hasValid() )
		return result;

#ifdef _JRT1_
	for ( EmailAddresses::iterator it = rEmails.begin(); it != rEmails.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += openTag( "EMAIL" );

			result += serializeTag( "TY",  (*it).getType().c_str()    );
			result += serializeTag( "UID", (*it).getAddress().c_str() );

			if ( (*it).isPreferred() )
			{
				result += serializeTag( "PREF" );
			}

	//		result += serializeVis( (*it).getVisibility() );

			result += closeTag();
		}
	}
#else
	int nCount = 0;
	result += openTag( "EMAIL" );

	for ( EmailAddresses::iterator it = rEmails.begin(); it != rEmails.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += openTag( generateKeyTag( nCount++ ).c_str() );

			result += serializeTag( "TY",  (*it).getType().c_str()    );
			result += serializeTag( "UID", (*it).getAddress().c_str() );

			if ( (*it).isPreferred() )
			{
				result += serializeTag( "PREF" );
			}

	//		result += serializeVis( (*it).getVisibility() );

			result += closeTag();
		}
	}

	result += closeTag();
#endif

	return result;
}

//-----------------------------------------------------------------------------

std::string ProfileXMLSerializer2::serializePhone()
{
	std::string result = "";

	Telephones& rPhones = _profile.getTelephones();

	if ( !rPhones.hasValid() )
		return result;

#ifdef _JRT1_
	for ( Telephones::iterator it = rPhones.begin(); it != rPhones.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += openTag( "TEL" );

			result += serializeTag( "NUM", (*it).getNumber().c_str() );
			result += serializeTag( "TY",  (*it).getType().c_str()   );

			if ( (*it).isPreferred() )
			{
				result += serializeTag( "PREF" );
			}

	//		result += serializeVis( (*it).getVisibility() );

			result += closeTag();
		}
	}
#else
	int nCount = 0;
	result += openTag( "TEL" );

	for ( Telephones::iterator it = rPhones.begin(); it != rPhones.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += openTag( generateKeyTag( nCount++ ).c_str() );

			result += serializeTag( "NUM", (*it).getNumber().c_str() );
			result += serializeTag( "TY",  (*it).getType().c_str()   );

			if ( (*it).isPreferred() )
			{
				result += serializeTag( "PREF" );
			}

	//		result += serializeVis( (*it).getVisibility() );

			result += closeTag();
		}
	}

	result += closeTag();
#endif
	return result;
}

//-----------------------------------------------------------------------------

std::string ProfileXMLSerializer2::serializeWebSite()
{
	std::string result = "";

	Urls& rUrls = _profile.getUrls();

	if ( !rUrls.hasValid() )
		return result;

#ifdef _JRT1_
	for ( Urls::iterator it = rUrls.begin(); it != rUrls.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += openTag( "WEBSITE" );

			result += serializeTag( "TY",  (*it).getType().c_str()  );
			result += serializeTag( "URL", (*it).getUrl().c_str()	);
	//		result += serializeVis( (*it).getVisibility() );

			result += closeTag();
		}
	}
#else
	int nCount = 0;
	result += openTag( "WEBSITE" );

	for ( Urls::iterator it = rUrls.begin(); it != rUrls.end(); it++ )
	{
		if ( (*it).isValid() )
		{
			result += openTag( generateKeyTag( nCount++ ).c_str() );

			result += serializeTag( "TY",  (*it).getType().c_str()  );
			result += serializeTag( "URL", (*it).getUrl().c_str()	);
	//		result += serializeVis( (*it).getVisibility() );

			result += closeTag();
		}
	}

	result += closeTag();
#endif
	return result;
}

//-----------------------------------------------------------------------------

std::string ProfileXMLSerializer2::serializeWorkPlace()
{
	std::string result = "";

//	WorkPlaces& rWorkPlaces = _profile.getWorkPlaces();
//
//	for ( WorkPlaces::iterator it = rWorkPlaces.begin(); it != rWorkPlaces.end(); it++ )
//	{
//		result += openTag( "WKPLACE" );
//
//		result += serializeTag( "N",     (*it).getName().c_str()		);
//		result += serializeTag( "POS",   (*it).getPosition().c_str()	);
//		result += serializeTag( "DESC",  (*it).getDescription().c_str() );
//		result += serializeTag( "START", (*it).getStartDate().c_str()	);
//		result += serializeTag( "END",   (*it).getEndDate().c_str()		);
//
////		result += serializeVis( (*it).getVisibility() );
//
//		result += closeTag();
//	}

	return result;
}

//-----------------------------------------------------------------------------
//
//std::string ProfileXMLSerializer2::serializeMergedContacts()
//{
//	std::string result = "";
//	std::string temp   = "";
//
//	MergedContacts& rMCs = _profile.getMergedContacts();
//
//	if ( !rMCs.hasValid() )
//		return result;
//
//#ifdef _JRT1_
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
//#else
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
//#endif
//
//	return result;
//}

//-----------------------------------------------------------------------------
//End Serialize methods
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//Unserialize methods
//-----------------------------------------------------------------------------

bool ProfileXMLSerializer2::unserialize( const std::string& xml )
{
	return false;
}

//-----------------------------------------------------------------------------

bool ProfileXMLSerializer2::unserialize( TiXmlHandle& rootElt )
{
	unserializeIdentity( rootElt );

	//Some needed conversions.
	EnumSex::Sex sex = EnumSex::fromServerString( fromNode( rootElt, "SEX"  ) );

//	_profile.setFullName     (         fromNode( rootElt, "FN"   ) );				//TODO: profile creates this from First/Last.
	_profile.setAlias        (         fromNode( rootElt, "NN"   ) );
	_profile.setSex          (  	   sex						 );	
	_profile.setBirthdate    ( toDate( fromNode( rootElt, "BDAY" ) ) );
	_profile.setTimeZone     (  toInt( fromNode( rootElt, "TZ" ) ) );
	_profile.setDescription  (         fromNode( rootElt, "DESC" ) );
	_profile.setInterests    (         fromNode( rootElt, "INTS" ) );

	_profile.setLanguage     (         fromNode( rootElt, "LANG"	) );
	_profile.setCompany	     (         fromNode( rootElt, "ORG"		) );
	_profile.setSmsSignature (         fromNode( rootElt, "SMSSIGN" ) );
	_profile.setStatusMessage(         fromNode( rootElt, "STATMSG" ) );
	_profile.setIsSystemEntry( toBool( fromNode( rootElt, "SYSENT"  ) ) );
	_profile.setIsSearchable ( toBool( fromNode( rootElt, "SRCH"    ) ) );

//	unserializeAvatar		 ( rootElt );	//VOXOX - JRT - 2009.08.14 - Server returns this outside <PROFILE> node
	unserializeName			 ( rootElt );

	unserializeAddress		 ( rootElt );
	unserializeEmail		 ( rootElt );
	unserializePhone		 ( rootElt );
	unserializeWebSite		 ( rootElt );
	unserializeWorkPlace	 ( rootElt );

//	unserializeMergedContacts( rootElt );

	return true;
}

//-----------------------------------------------------------------------------

void ProfileXMLSerializer2::unserializeName(TiXmlHandle & rootElt )
{
	TiXmlHandle nameElt = rootElt.FirstChild("N");

	_profile.setFirstName ( fromNode( nameElt, "F" ) );
//	_profile.setMiddleName( fromNode( nameElt, "M" ) );
	_profile.setLastName  ( fromNode( nameElt, "L" ) );
}

//-----------------------------------------------------------------------------

void ProfileXMLSerializer2::unserializeAvatar(TiXmlHandle & rootElt )
{
//	OWPicture avatar;

	TiXmlHandle avatarElt = rootElt.FirstChild("PHOTO");

	unserializeAvatar2( avatarElt );
}

//-----------------------------------------------------------------------------

void ProfileXMLSerializer2::unserializeAvatar2(TiXmlHandle & avatarElt )
{
//	std::string nodeData = "";
//	nodeData << *avatarElt.ToNode();

	std::string data = Base64::decode( fromNode( avatarElt, "BV"  ) );
	OWPicture avatar = OWPicture::pictureFromData( data );	//This will calc and store MD5.

//#ifdef _WIN32		//JRT-XXX
//	char msg[200];
//	sprintf_s( msg, 200, "unserializeAvatar2: data len = %d\n\n", data.size() );
//	OutputDebugString( msg );
//#endif

	avatar.setType		  ( fromNode( avatarElt, "TY"  ) );
	avatar.setFilename	  ( fromNode( avatarElt, "FP"  ) );
	avatar.setProvidedMd5 ( fromNode( avatarElt, "MD5" ) );	

	_profile.updateIcon( avatar );
}

//-----------------------------------------------------------------------------

void ProfileXMLSerializer2::unserializeAddress(TiXmlHandle & rootElt )
{
	StreetAddress addr;
	
#ifdef _JRT_
	TiXmlNode* addrNode = NULL;

	while ((rootElt.Node()) && (addrNode = rootElt.Node()->IterateChildren("ADR", addrNode))) 
	{
		addr.setType		 ( fromNode( addrNode, "TY"    ) );

		addr.setStreet1		 ( fromNode( addrNode, "ST"    ) );
		addr.setStreet2		 ( fromNode( addrNode, "ST2"   ) );
		addr.setStreet3		 ( fromNode( addrNode, "ST3"   ) );

		addr.setCity		 ( fromNode( addrNode, "CITY"  ) );
		addr.setStateProvince( fromNode( addrNode, "STATE" ) );
		addr.setCountry      ( fromNode( addrNode, "CTRY"  ) );
		addr.setPostalCode   ( fromNode( addrNode, "PCODE" ) );

//		addr.setIsPreferred( toBool( fromNode( addrNode, "PREF" ) ) );
		addr.setVisibility ( EnumVisibility::toVisibility( fromNode( addrNode, "VIS" ) ) );

		_profile.getStreetAddresses().Add( &addr );
	}
#else
	TiXmlHandle addrElt = rootElt.FirstChildElement( "ADR" );

	if ( addrElt.Node() )
	{
		TiXmlNode* addrNode = NULL;

		while ( addrNode = addrElt.Node()->IterateChildren(addrNode) )
		{
			addr.setType		 ( fromNode( addrNode, "TY"    ) );

			addr.setStreet1		 ( fromNode( addrNode, "ST"    ) );
			addr.setStreet2		 ( fromNode( addrNode, "ST2"   ) );
			addr.setStreet3		 ( fromNode( addrNode, "ST3"   ) );

			addr.setCity		 ( fromNode( addrNode, "CITY"  ) );
			addr.setStateProvince( fromNode( addrNode, "STATE" ) );
			addr.setCountry      ( fromNode( addrNode, "CTRY"  ) );
			addr.setPostalCode   ( fromNode( addrNode, "PCODE" ) );

	//		addr.setIsPreferred( toBool( fromNode( addrNode, "PREF" ) ) );
			addr.setVisibility ( EnumVisibility::toVisibility( fromNode( addrNode, "VIS" ) ) );

			_profile.getStreetAddresses().Add( &addr );
		}
	}
#endif
}

//-----------------------------------------------------------------------------

void ProfileXMLSerializer2::unserializeEmail(TiXmlHandle & rootElt )
{
	EmailAddress email;
	
#ifdef _JRT_
	TiXmlNode* emailNode = NULL;

	while ((rootElt.Node()) && (emailNode = rootElt.Node()->IterateChildren("EMAIL", emailNode))) 
	{
		email.setType       (         fromNode( emailNode, "TY"   ) );
		email.setAddress    (         fromNode( emailNode, "UID"  ) );
		email.setIsPreferred( toBool( fromNode( emailNode, "PREF" ) ) );
		email.setVisibility ( EnumVisibility::toVisibility( fromNode( emailNode, "VIS" ) ) );

		_profile.getEmailAddresses().Add( &email );
	}
#else
	TiXmlHandle emailElt = rootElt.FirstChildElement( "EMAIL" );

	if ( emailElt.Node() )
	{
		TiXmlNode* emailNode = NULL;

		while ( emailNode = emailElt.Node()->IterateChildren( emailNode) )
		{
			email.setType       (         fromNode( emailNode, "TY"   ) );
			email.setAddress    (         fromNode( emailNode, "UID"  ) );
			email.setIsPreferred( toBool( fromNode( emailNode, "PREF" ) ) );
			email.setVisibility ( EnumVisibility::toVisibility( fromNode( emailNode, "VIS" ) ) );

			_profile.getEmailAddresses().Add( &email );
		}
	}
#endif
}

//-----------------------------------------------------------------------------

void ProfileXMLSerializer2::unserializePhone(TiXmlHandle& rootElt )
{
	Telephone phone;
	
#ifdef _JRT_
	TiXmlNode* telNode = NULL;

	while ((rootElt.Node()) && (telNode = rootElt.Node()->IterateChildren("TEL", telNode))) 
	{
		phone.setType		(		  fromNode( telNode, "TY"   ) );
		phone.setNumber		(		  fromNode( telNode, "NUM"  ) );
		phone.setIsPreferred( toBool( fromNode( telNode, "PREF" ) ) );
		phone.setVisibility ( EnumVisibility::toVisibility( fromNode( telNode, "VIS" ) ) );

		_profile.getTelephones().Add( &phone );
	}
#else
	TiXmlHandle phoneElt = rootElt.FirstChildElement( "TEL" );

	if ( phoneElt.Node() )
	{
		TiXmlNode* telNode = NULL;

		while ( telNode = phoneElt.Node()->IterateChildren(telNode) ) 
		{
			phone.setType		(		  fromNode( telNode, "TY"   ) );
			phone.setNumber		(		  fromNode( telNode, "NUM"  ) );
			phone.setIsPreferred( toBool( fromNode( telNode, "PREF" ) ) );
			phone.setVisibility ( EnumVisibility::toVisibility( fromNode( telNode, "VIS" ) ) );

			_profile.getTelephones().Add( &phone );
		}
	}
#endif
}

//-----------------------------------------------------------------------------

void ProfileXMLSerializer2::unserializeWebSite(TiXmlHandle & rootElt )
{
	Url url;
	
#ifdef _JRT_
	TiXmlNode* urlNode = NULL;

	while ((rootElt.Node()) && (urlNode = rootElt.Node()->IterateChildren("WEBSITE", urlNode))) 
	{
		url.setType( fromNode( urlNode, "TY"  ) );
		url.setUrl ( fromNode( urlNode, "URL" ) );
//		url.setIsPreferred( toBool( fromNode( urlNode, "PREF" ) ) );
		url.setVisibility ( EnumVisibility::toVisibility( fromNode( urlNode, "VIS" ) ) );

		_profile.getUrls().Add( &url );
	}
#else
	TiXmlHandle websiteElt = rootElt.FirstChildElement( "WEBSITE" );

	if ( websiteElt.Node() )
	{
		TiXmlNode* urlNode = NULL;

		while ( urlNode = websiteElt.Node()->IterateChildren( urlNode ) ) 
		{
			url.setType( fromNode( urlNode, "TY"  ) );
			url.setUrl ( fromNode( urlNode, "URL" ) );
	//		url.setIsPreferred( toBool( fromNode( urlNode, "PREF" ) ) );
			url.setVisibility ( EnumVisibility::toVisibility( fromNode( urlNode, "VIS" ) ) );

			_profile.getUrls().Add( &url );
		}
	}
#endif
}

//-----------------------------------------------------------------------------

void ProfileXMLSerializer2::unserializeWorkPlace(TiXmlHandle & rootElt )
{
	//TODO: we don't have/need this class yet.
	//WorkPlace wp;
	//
	//TiXmlNode* wpNode = NULL;

	//while ((rootElt.Node()) && (wpNode = rootElt.Node()->IterateChildren("WKPLACE", wpNode))) 
	//{
	//	wp.setName       ( fromNode( wpNode, "N"     ) );
	//	wp.setPosition   ( fromNode( wpNode, "POS"   ) );
	//	wp.setDescription( fromNode( wpNode, "DESC"  ) );
	//	wp.setStartDate  ( fromNode( wpNode, "START" ) );
	//	wp.setEndDate    ( fromNode( wpNode, "END"   ) );
	//	wp.setVisibility ( EnumVisibility::toVisibility( fromNode( wpNode, "VIS" ) ) );

	//	_profile.getWorkPlaces().Add( &wp );
	//}
}

//-----------------------------------------------------------------------------

//void ProfileXMLSerializer2::unserializeMergedContacts(TiXmlHandle & rootElt )
//{
//	MergedContact mc;
//	
//#ifdef _JRT_
//	TiXmlNode* mcNode = NULL;
//
//	while ((rootElt.Node()) && (mcNode = rootElt.Node()->IterateChildren("MERCON", mcNode))) 
//	{
//		mc.setUsername (        fromAttr( mcNode, "UID" ) );
//		mc.setNetworkId( toInt( fromAttr( mcNode, "NID" ) ) );
//
//		_profile.getMergedContacts().Add( &mc );
//	}
//#else
//	TiXmlHandle merconElt = rootElt.FirstChildElement( "MERCON" );
//
//	if ( merconElt.Node() )
//	{
//		TiXmlNode* mcNode = NULL;
//
//		while ( mcNode = merconElt.Node()->IterateChildren(mcNode) ) 
//		{
//			mc.setUsername (        fromAttr( mcNode, "UID" ) );
//			mc.setNetworkId( toInt( fromAttr( mcNode, "NID" ) ) );
//
//			_profile.getMergedContacts().Add( &mc );
//		}
//	}
//#endif
//}

//=============================================================================


//=============================================================================

ContactProfileXMLSerializer2::ContactProfileXMLSerializer2( ContactProfile& profile)
	: ProfileXMLSerializer2( profile ), 
	 _profile(profile) 
{
}

//-----------------------------------------------------------------------------

std::string ContactProfileXMLSerializer2::serializeIdentity()
{
	std::string result = "";
	IMContact* imContact = _profile.getPreferredIMContact();

	assert( imContact );

	int unid = imContact->getIMAccount()->getUserNetworkId();
	int nid  = imContact->getIMAccount()->getServerProtocolId();

	result += serializeTag( "CID",		imContact->getContactId()	);
	result += serializeTag( "UNID",		unid );	
	result += serializeTag( "NID",		nid	 );
//	result += serializeTag( "GRP",		_profile.getGroup()			);

	return result;
}

//-----------------------------------------------------------------------------

void ContactProfileXMLSerializer2::unserializeIdentity( TiXmlHandle& rootElt )
{
	std::string							contactId		 = fromNode( rootElt, "CID" );
	QtEnumIMProtocol::ServerProtocolId	serverProtocolId = (QtEnumIMProtocol::ServerProtocolId) toInt( fromNode( rootElt, "NID" ) );
	int									userNetworkId	 = toInt( fromNode( rootElt, "UNID" ) );

	//IMContact is key to connecting to ContactList.
	EnumIMProtocol::IMProtocol protocol = QtEnumIMProtocolMap::getInstance().toModelIMProtocol( serverProtocolId );	//TODO: Assumes we have ONE IMAccount per network protocol.

	IMContact imContact( protocol, contactId);
	imContact.setServerProtocolId( serverProtocolId );
	imContact.setUserNetworkId   ( userNetworkId );

	_profile.addIMContact( imContact );
}

//=============================================================================


//=============================================================================

UserProfileXMLSerializer2::UserProfileXMLSerializer2( ProfileData& profile)
	: ProfileXMLSerializer2( profile ), 
	 _profile(profile) 
{
}

//-----------------------------------------------------------------------------

std::string UserProfileXMLSerializer2::serializeIdentity()
{
	std::string result = "";

	IMAccount& imAccount = _profile.getIMAccount();

	int nid  = imAccount.getServerProtocolId();
//	int unid = imAccount.getUserNetworkId();

	result += serializeTag( "CID",		imAccount.getLogin() );
	result += serializeTag( "NID",		nid	 );
//	result += serializeTag( "UNID",		unid );

	return result;
}

//-----------------------------------------------------------------------------

void UserProfileXMLSerializer2::unserializeIdentity( TiXmlHandle& rootElt )
{
	std::string							login			 = fromNode( rootElt, "CID" );
	QtEnumIMProtocol::ServerProtocolId	serverProtocolId = (QtEnumIMProtocol::ServerProtocolId) toInt( fromNode( rootElt, "NID" ) );
//	int									userNetworkId	 = toInt( fromNode( rootElt, "UNID" ) );

	EnumIMProtocol::IMProtocol protocol = QtEnumIMProtocolMap::getInstance().toModelIMProtocol( serverProtocolId );

	IMAccount imAccount( protocol );
	imAccount.setLogin ( login    );
	imAccount.setServerProtocolId( serverProtocolId );
//	imAccount.setUserNetworkId   ( userNetworkId );

	_profile.getIMAccount() = imAccount;
}

//=============================================================================
