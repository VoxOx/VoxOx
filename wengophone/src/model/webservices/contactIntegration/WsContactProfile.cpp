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
#include "WsContactProfile.h"

#include <model/profile/UserProfile.h>
#include <model/contactlist/ContactProfileXMLSerializer.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/contactlist/ContactProfile.h>

#include <util/Logger.h>
#include <string.h>
#include <tinyxml.h>

#ifdef _WIN32
#include <windows.h>
#endif


std::string WsContactProfile::_saveUserProfileMethod    = "saveUserProfile";
std::string WsContactProfile::_getUserProfileMethod     = "getUserProfile";
std::string WsContactProfile::_saveContactProfileMethod = "saveContactProfile";
std::string WsContactProfile::_getContactProfileMethod  = "getContactProfile";

//=============================================================================

ProfileRequest::ProfileRequest()
{
	initVars();
}

//-----------------------------------------------------------------------------

ProfileRequest::~ProfileRequest()
{
}

//-----------------------------------------------------------------------------

ProfileRequest& ProfileRequest::operator=( const ProfileRequest& src )
{
	if ( this != &src )
	{
		setMethod		 ( src.getMethod()			);
		setContactKey	 ( src.getContactKey()		);
		setXml			 ( src.getXml()				);
		setAvatarChecksum( src.getAvatarCheckSum()	);
		setUserNetworkId ( src.getUserNetworkId()   );
		setTimestamp	 ( src.getTimestamp()		);
	}
	
	return *this;
}

//-----------------------------------------------------------------------------

bool ProfileRequest::operator==( const ProfileRequest& src )
{
	bool bEqual = false;

	if ( getMethod()		== src.getMethod()		  &&
		 getContactKey()	== src.getContactKey()	  &&
		 getUserNetworkId() == src.getUserNetworkId() &&
		 getTimestamp()		== src.getTimestamp() )
	{
		bEqual = true;
	}

	return bEqual;
}

//-----------------------------------------------------------------------------

void ProfileRequest::initVars()
{
	setMethod		 ( QueryResult::EnumMethod_None );
	setContactKey	 ( "" );
	setXml			 ( "" );
	setAvatarChecksum( "" );
	setUserNetworkId ( 0  );
	setTimestamp	 ( 0  );
}

//=============================================================================


//=============================================================================

void ProfileRequestList::addSetContactProfile( const std::string& contactKey, const std::string& xml )
{
	ProfileRequest pr;

	pr.setMethod	( QueryResult::EnumMethod_SaveContactProfile );
	pr.setXml		( xml );
	pr.setContactKey( contactKey );

	Add( &pr );
}

//-----------------------------------------------------------------------------

void ProfileRequestList::addGetContactProfile( const std::string& contactKey, int userNetworkId, int timestamp )
{
	ProfileRequest pr;

	pr.setMethod       ( QueryResult::EnumMethod_GetContactProfile );
	pr.setContactKey   ( contactKey );
	pr.setUserNetworkId( userNetworkId );
	pr.setTimestamp    ( timestamp );

	Add( &pr );
}

//-----------------------------------------------------------------------------

void ProfileRequestList::addSetUserProfile( const std::string& contactKey, const std::string& contactXml)
{
	ProfileRequest pr;

	pr.setMethod	( QueryResult::EnumMethod_SaveUserProfile );
	pr.setContactKey( contactKey );
	pr.setXml		( contactXml );

	Add( &pr );
}

//-----------------------------------------------------------------------------

void ProfileRequestList::addGetUserProfile( int timestamp, const std::string& avatarChecksum )
{
	ProfileRequest pr;

	pr.setMethod		( QueryResult::EnumMethod_GetUserProfile );
	pr.setTimestamp		( timestamp		 );
	pr.setAvatarChecksum( avatarChecksum );

	Add( &pr );
}

//-----------------------------------------------------------------------------

ProfileRequest ProfileRequestList::getFirst()
{
	lock();
	ProfileRequest pr = front();
	unlock();

	return pr;
}

//-----------------------------------------------------------------------------

void ProfileRequestList::removeFirst()
{
	lock();
	pop_front();
	unlock();
}

//=============================================================================


//=============================================================================

WsContactProfile::WsContactProfile(UserProfile & userProfile)
	: WengoWebService(userProfile.getWengoAccount()),
	_userProfile(userProfile) 
{
	Config& config = ConfigManager::getInstance().getCurrentConfig();

//	setHostname(config.getWengoServerHostname());

#if 1
//	setHostname( "api.telcentris.com" );
	setHostname( config.getApiHostname() );
	setServicePath( config.getVoxOxSyncWSPath() );
	setHttps(true);
	setPort(443);
#else
	setHostname( "jrt.voxox.com" );
	setServicePath( "/api.telcentris.com/public_html/sync/Rest" );
	setHttps(false);
	setPort( 80 );
#endif

	setGet(true);	//true = POST in Curl lib.
	setWengoAuthentication(false);

	//Init thread
	_thread = new WsContactProfileThread( *this, _reqList );
	_thread->init();

	_apiTimer = NULL;
}

//-----------------------------------------------------------------------------

WsContactProfile::~WsContactProfile()
{
	if ( _thread )
	{
		delete _thread;
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::setContactProfile( const ContactProfile& contactProfile) 
{
	std::string method = _saveContactProfileMethod;		//Needed to generate proper XML.
	std::string xml = toXml( contactProfile, method );
	std::string contactKey = contactProfile.getContactId();

	setContactProfile( contactKey, xml );
}

//-----------------------------------------------------------------------------

void WsContactProfile::setContactProfile( const std::string& contactKey, const std::string& contactXml ) 
{
	if( getWengoAccount() )
	{
		_reqList.addSetContactProfile( contactKey, contactXml );
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::getContactProfile( const std::string& contactKey, int userNetworkId, int timestamp )
{
	if( getWengoAccount() )
	{
		if ( userNetworkId == 0 )
		{
			LOG_WARN( " Request profile for " + contactKey + " has invalid userNetworkId." );
		}
		else
		{
			_reqList.addGetContactProfile( contactKey, userNetworkId, timestamp );
		}
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::setUserProfile( const UserProfile& userProfile) 
{
	std::string method	   = _saveUserProfileMethod;		//Needed to generate proper XML.
	std::string xml		   = toXml( userProfile, method );
	std::string contactKey = userProfile.getVoxOxAccount()->getKey();

	setUserProfile( contactKey, xml );
}

//-----------------------------------------------------------------------------

void WsContactProfile::setUserProfile( const std::string& contactKey, const std::string& contactXml ) 
{
	if( getWengoAccount() )
	{
		_reqList.addSetUserProfile( contactKey, contactXml );
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::getUserProfile( int timestamp, const std::string& avatarChecksum )
{
	if( getWengoAccount() )
	{
		_reqList.addGetUserProfile( timestamp, avatarChecksum );
	}
}

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//Private calls
//-----------------------------------------------------------------------------

void WsContactProfile::executeRequest( ProfileRequest& pr )
{
	switch( pr.getMethod() )
	{
	case QueryResult::EnumMethod_GetContactProfile:
		getContactProfilePrivate( pr.getContactKey(), pr.getUserNetworkId(), pr.getTimestamp() );
		break;

	case QueryResult::EnumMethod_SaveContactProfile:
		setContactProfilePrivate( pr.getContactKey(), pr.getXml() );
		break;

	case QueryResult::EnumMethod_GetUserProfile:
		getUserProfilePrivate( pr.getTimestamp(), pr.getAvatarCheckSum() );
		break;

	case QueryResult::EnumMethod_SaveUserProfile:
		setUserProfilePrivate( pr.getXml() );
		break;

	case QueryResult::EnumMethod_None:
		break;				//Failed or new EnumMethod.
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::setContactProfilePrivate( const std::string& contactKey, const std::string& contactXml ) 
{
	if( getWengoAccount() )
	{
		std::string method = _saveContactProfileMethod;
		std::string parms  = "";
		
		parms += initParameters( method );
		parms += "&uri="	   + contactKey;
		parms += "&xmlstring=" + contactXml;
		
		setParameters( parms );
		
		execute( method );
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::getContactProfilePrivate( const std::string& contactKey, int userNetworkId, int timeStamp )
{
	if( getWengoAccount() )
	{
		if ( userNetworkId == 0 )
		{
			assert(false);		//Should not get here.
			LOG_WARN( " Request profile for " + contactKey + " has invalid userNetworkId." );
			_thread->removeCurrentProfileRequest();
		}
		else
		{
			std::string method = _getContactProfileMethod;
			std::string parms  = "";
			
			parms += initParameters( method );
			parms += "&uri="			+ contactKey;
			parms += "&unid="			+ String::fromNumber( userNetworkId );
			parms += "&timestamp="		+ String::fromNumber( timeStamp);

			setParameters( parms );
			
//			LOG_INFO(" Request profile for " + contactKey );	//Too Verbose now that it works.
			execute( method );
		}
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::setUserProfilePrivate( const std::string& contactXml) 
{
	if( getWengoAccount() )
	{
		std::string method = _saveUserProfileMethod;
		std::string parms  = "";
		
		parms += initParameters( method );
		parms += "&xmlstring=" + contactXml;
		
		setParameters( parms );
		
		execute( method );
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::getUserProfilePrivate( int timeStamp, const std::string& avatarChecksum )
{
	if( getWengoAccount() )
	{
		LOG_INFO(" Request User Profile" );

		std::string method = _getUserProfileMethod;
		std::string parms  = "";
		
		parms += initParameters( method );
		parms += "&timestamp="		+ String::fromNumber( timeStamp );
		parms += "&avatarChecksum=" + avatarChecksum;		//VOXOX - JRT - 2009.08.14 

		setParameters( parms );
		
		execute( method );
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::execute( const std::string& method )
{
	call(this);		//Calls the web service

	assert( _apiTimer == NULL );

	_apiTimer = new VoxApiTimer( "WsContactProfile", method, false );
	_apiTimer->Start();
}

//-----------------------------------------------------------------------------

void WsContactProfile::answerReceived( const std::string & answer, int requestId ) 
{
	std::string strSize = "Size: " + String::fromNumber( answer.size() );
	_apiTimer->Stop( strSize );
	delete _apiTimer;
	_apiTimer = NULL;

	if (answer.empty()) 
	{
		LOG_WARN( "Server XML response is empty." );
#ifdef _WIN32
		OutputDebugString( "Server XML response is empty.\n" );	//JRT-XXX
#endif
	}
	else
	{
		std::string		contactId = "";
		ContactProfile	contactProfile;
		ProfileData		userProfile;

		_queryResult.init();	//VOXOX - JRT - 2009.09.11 

		fromXml( answer, contactProfile, userProfile );

		switch( _queryResult.getEnumMethod() )
		{
		case QueryResult::EnumMethod_GetContactProfile:
			if ( contactProfile.isValid() )				//There may be no Profile saved.
			{
				//VOXOX - JRT - 2009.07.13 - HACK for domain change.
				if ( applyContactProfile( contactProfile ) )	//VOXOX - JRT - 2009.07.22 
				{
					LOG_INFO(" Received profile for " + contactProfile.getContactId() );	//VOXOX - JRT - 2009.07.22 
					wsContactProfileGetEvent(*this, requestId, contactProfile);
				}
			}
			break;

		case QueryResult::EnumMethod_SaveContactProfile:
			wsContactProfileSetEvent(*this, requestId, _queryResult.getContactId(), _queryResult.getUserNetworkId(), _queryResult.getTimestamp() );
			break;

		case QueryResult::EnumMethod_GetUserProfile:
			wsUserProfileGetEvent( *this, requestId, userProfile ) ;
			break;

		case QueryResult::EnumMethod_SaveUserProfile:
			wsUserProfileSetEvent( *this, requestId, _queryResult.getTimestamp() );
			break;

		case QueryResult::EnumMethod_None:
			LOG_ERROR( "Invalid/Unknown method called" );
			break;				//Failed or new EnumMethod.
		}
	}

	std::string responseInfo = "Server reports this query took " + String::fromDouble( _queryResult.getQueryTime() ) + " seconds.";
	LOG_INFO( responseInfo );

	_thread->removeCurrentProfileRequest();
}

//-----------------------------------------------------------------------------

bool WsContactProfile::applyContactProfile( const ContactProfile& profile )
{
	bool use = true;

	IMContact* imContact = const_cast<IMContact*>(profile.getPreferredIMContact());

	if ( imContact )
	{
		IMAccount* imAccount = const_cast<IMAccount*>(imContact->getIMAccount());

		if ( imAccount )
		{
			use = ! imAccount->isOldVoxOxDomain();
		}
	}

	return use;
}

//-----------------------------------------------------------------------------

std::string WsContactProfile::initParameters( const std::string& method )
{
	std::string parms = "";

	parms += "key=" + getSecurityKey();
	parms += "&method=";
	parms += method;
	parms += "&userId=" + _userProfile.getVoxOxAccount()->getDisplayAccountId();

	return parms;
}

//-----------------------------------------------------------------------------

std::string WsContactProfile::getSecurityKey()
{
	return _userProfile.getSecurityKey();
}

//-----------------------------------------------------------------------------

void WsContactProfile::fromXml( const std::string& xml, ContactProfile& contactProfile, ProfileData& userProfile )
{
	bool result = true;

	TiXmlDocument doc;

	doc.Parse(xml.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlNode * syncNode = NULL;
	
	syncNode = findSyncNode( docHandle );

	QueryResult::EnumMethod eMethod = _queryResult.getEnumMethod();

	if (syncNode) 
	{
		assert( eMethod != QueryResult::EnumMethod_None );

		fromXmlGeneric( syncNode, eMethod );

		//Main <PROFILE> 
		TiXmlNode* profileNode = syncNode->FirstChild( "PROFILE" );
		profileFromXml( profileNode, contactProfile, userProfile, eMethod );

		//<PHOTO>
		TiXmlNode* photoNode = syncNode->FirstChild( "PHOTO" );
		photoFromXml( photoNode, contactProfile, userProfile, eMethod );
	}
	else
	{
		int xxx = 1;		//Did not find node.  Did XML change?
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::profileFromXml( TiXmlNode* profileNode, ContactProfile& contactProfile, ProfileData& userProfile, QueryResult::EnumMethod eMethod )
{
	if ( profileNode )
	{
//		nodeData = "";
//		nodeData << *profileNode;

		switch( eMethod )
		{
		case QueryResult::EnumMethod_GetContactProfile:
		{
			ContactProfileXMLSerializer2 serializer( contactProfile );

			TiXmlHandle handle( profileNode );
			serializer.unserialize( handle );
			break;
		}

		case QueryResult::EnumMethod_GetUserProfile:
		{
			UserProfileXMLSerializer2 serializer( userProfile);

			TiXmlHandle handle( profileNode );
			serializer.unserialize( handle );
			break;
		}

		default:
			assert(false);		//Should not be here
		}
	}
	else
	{
		if ( ( eMethod == QueryResult::EnumMethod_GetUserProfile) ||
			 ( eMethod == QueryResult::EnumMethod_GetContactProfile) )
		{
			if (! _queryResult.succeeded() )
			{
				int xxx = 1;		//TODO: We should have empty <PROFILE> if no profile has been saved?
			}
		}
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::photoFromXml( TiXmlNode* photoNode, ContactProfile& contactProfile, ProfileData& userProfile, QueryResult::EnumMethod eMethod )
{
	if ( photoNode )
	{
//		std::string nodeData = "";
//		nodeData << *photoNode;

		switch( eMethod )
		{
		case QueryResult::EnumMethod_GetContactProfile:
		{
			ContactProfileXMLSerializer2 serializer( contactProfile );

			TiXmlHandle handle( photoNode );
			serializer.unserializeAvatar2( handle );
			break;
		}

		case QueryResult::EnumMethod_GetUserProfile:
		{
			UserProfileXMLSerializer2 serializer( userProfile);

			TiXmlHandle handle( photoNode );
			serializer.unserializeAvatar2( handle );
			break;
		}

		default:
			assert(false);		//Should not be here
		}
	}
	else
	{
		if ( ( eMethod == QueryResult::EnumMethod_GetUserProfile) ||
			 ( eMethod == QueryResult::EnumMethod_GetContactProfile) )
		{
			if (! _queryResult.succeeded() )
			{
				int xxx = 1;		//TODO: We should have empty <PROFILE> if no profile has been saved?
			}
		}
	}
}

//-----------------------------------------------------------------------------

void WsContactProfile::fromXmlGeneric( TiXmlNode * syncNode, QueryResult::EnumMethod eMethod )
{
//		std::string nodeData;
//		nodeData << *syncNode;

	//Results
	QueryResultXMLSerializer resultSerializer( _queryResult );

	//Query node may or may not exist, depending on timestamp.
	TiXmlNode* queryNode = syncNode->FirstChild( "QUERY" );

	if ( queryNode )
	{
		resultSerializer.unserializeQuery( queryNode );
	}

	//Response - We should get this:
	//				1) if no update required due to timestamp. value == 0
	//				2) on all gets.  value = 1
	TiXmlNode* responseNode = syncNode->FirstChild( "response" );

	if ( responseNode )
	{
		resultSerializer.unserializeResponse( responseNode );
	}

	//Status - success or failure of overall action
	TiXmlNode* statusNode = syncNode->FirstChild( "status" );

	if ( statusNode )
	{
		resultSerializer.unserializeStatus( statusNode );
	}

	//Result of set.
	TiXmlNode* tempNode = NULL;

	switch( eMethod )
	{
	case QueryResult::EnumMethod_SaveContactProfile:
		tempNode = syncNode->FirstChild( "CID" );
		if ( tempNode )
		{
			_queryResult.setContactId( tempNode->FirstChild()->Value() );
		}

		tempNode = syncNode->FirstChild( "UNID" );
		if ( tempNode )
		{
			String temp = tempNode->FirstChild()->Value();
			_queryResult.setUserNetworkId( temp.toInteger() );
		}

		tempNode = syncNode->FirstChild( "timestamp" );
		if ( tempNode )
		{
			String temp = tempNode->FirstChild()->Value();
			_queryResult.setTimestamp( temp.toInteger() );
		}

		break;

	case QueryResult::EnumMethod_SaveUserProfile:
		//VOXOX - JRT - 2009.08.07 - TODO: Ideally we should be some ID info to ensure server is returning proper info.
		tempNode = syncNode->FirstChild( "timestamp" );
		if ( tempNode )
		{
			String temp = tempNode->FirstChild()->Value();
			_queryResult.setTimestamp( temp.toInteger() );
		}

		break;
	}
}

//-----------------------------------------------------------------------------

TiXmlNode* WsContactProfile::findSyncNode( TiXmlHandle docHandle )
{
	TiXmlNode* syncNode = NULL;
	QueryResult::EnumMethod eMethod = QueryResult::EnumMethod_None;

	if ( syncNode == NULL )
	{
		syncNode = docHandle.FirstChild("Sync").FirstChild( _saveUserProfileMethod ).Node();
		eMethod  = (syncNode ? QueryResult::EnumMethod_SaveUserProfile : QueryResult::EnumMethod_None);
	}

	if ( syncNode == NULL )
	{
		syncNode = docHandle.FirstChild("Sync").FirstChild( _getUserProfileMethod ).Node();
		eMethod  = (syncNode ? QueryResult::EnumMethod_GetUserProfile : QueryResult::EnumMethod_None);
	}

	if ( syncNode == NULL )
	{
		syncNode = docHandle.FirstChild("Sync").FirstChild( _saveContactProfileMethod ).Node();
		eMethod  = (syncNode ? QueryResult::EnumMethod_SaveContactProfile : QueryResult::EnumMethod_None);
	}

	if ( syncNode == NULL )
	{
		syncNode = docHandle.FirstChild("Sync").FirstChild( _getContactProfileMethod ).Node();
		eMethod  = (syncNode ? QueryResult::EnumMethod_GetContactProfile : QueryResult::EnumMethod_None);
	}

	_queryResult.setEnumMethod( eMethod );

	return syncNode;
}

//-----------------------------------------------------------------------------
	
std::string WsContactProfile::toXml( const ContactProfile& profile, const std::string& method )
{
	std::string result = "";

	ContactProfileXMLSerializer2 serializer( const_cast<ContactProfile&>(profile) );
	serializer.setMethod( method );
	result = serializer.serialize();

	return result;
}

//-----------------------------------------------------------------------------

std::string WsContactProfile::toXml( const UserProfile& userProfile, const std::string& method )
{
	std::string result = "";

	ProfileData profileData;
	
	const_cast<UserProfile&>(userProfile).toProfileData( profileData );

	UserProfileXMLSerializer2 serializer( profileData );
	serializer.setMethod( method );
	result = serializer.serialize();

	return result;
}

//=============================================================================



//=============================================================================

WsContactProfileThread::WsContactProfileThread( WsContactProfile& wsContactProfile, ProfileRequestList& reqList )
	: _wsContactProfile( wsContactProfile ), _reqList( reqList )
{
	_started   = false;
	_reqActive = false;

	_startCount = 0;
	_endCount   = 0;
}

//-----------------------------------------------------------------------------

void WsContactProfileThread::init() 
{
	_started = true;
	start();
}

//-----------------------------------------------------------------------------

WsContactProfileThread::~WsContactProfileThread() 
{
	if (_started) 
	{
		_started = false;
	}
}

//-----------------------------------------------------------------------------

void WsContactProfileThread::run() 
{
	while (_started) 
	{
		if ( canProcess() )
		{
			ProfileRequest pr = _reqList.getFirst();

			_reqActive = true;
			_startCount++;
			_wsContactProfile.executeRequest( pr );
		}
		else
		{
			Thread::msleep( 10 );
		}
	}
}

//-----------------------------------------------------------------------------

bool WsContactProfileThread::canProcess()
{
	bool result = false;

	if ( _reqList.GetCount() > 0 )
	{
		if ( ! _reqActive )
		{
			result = true;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------

void WsContactProfileThread::removeCurrentProfileRequest()
{
	_reqList.removeFirst();

	_reqActive = false;
	_endCount++;
}

//=============================================================================
