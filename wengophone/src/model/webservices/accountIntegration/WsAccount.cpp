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
#include "WsAccount.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/account/wengo/WengoAccount.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/IMAccountXMLSerializer.h>
#include <imwrapper/IMAccountListXMLSerializer.h>

#include <util/Logger.h>
#include <tinyxml.h>

#include "WengoPhoneBuildId.h"

//-----------------------------------------------------------------------------

WsAccount::WsAccount( const WsAccount& src )
	: WengoWebService(src._userProfile.getWengoAccount()),
	_userProfile(src._userProfile)
{
	*this = src;
}

//-----------------------------------------------------------------------------

WsAccount::WsAccount(UserProfile & userProfile )
	: WengoWebService(userProfile.getWengoAccount()),
	_userProfile(userProfile)
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	//Setup the web service
//	setHostname(config.getWengoServerHostname());

#if 1
//	setHostname( "" );//VOXOX - CJC - 2009.07.05 Set server address
	setHostname( config.getApiHostname() );
	setServicePath( config.getVoxOxSyncWSPath() );
	setHttps(true);
	setPort(443);
#else
	setHostname( "" );
	setServicePath( "" );//VOXOX - CJC - 2009.07.05  Ser server address
	setHttps(false);
	setPort( 80 );
#endif

	setGet(true);	//true = POST in Curl lib.
	setWengoAuthentication(false);

	_userNetworkId = 0;
	_bDeleted	   = false;
}

//-----------------------------------------------------------------------------

void WsAccount::getUserNetworks()	//Gets all userNetworks
{
	if( getWengoAccount() )
	{
		std::string method = "getUserNetwork";
		std::string parms  = "";
		
		parms += initParameters( method );

		setParameters( parms );
		
		call(this);		//Calls the web service
	}
}

//-----------------------------------------------------------------------------

void WsAccount::setUserNetwork( const IMAccount& imAccount ) 
{
	std::string method = "saveUserNetwork";
	std::string xml = toXml( imAccount, method );

	setUserNetwork( method, xml );
}

//-----------------------------------------------------------------------------

void WsAccount::setUserNetwork( const std::string& method, const std::string& xml ) 
{
	if( getWengoAccount() )
	{
		std::string parms = "";
		
		parms += initParameters( method );
		parms += "&xmlstring=" + xml;
		
		setParameters( parms );
		
		call(this);		//Calls the web service
	}
}

//-----------------------------------------------------------------------------

void WsAccount::removeUserNetwork( int serverKey )
{
	if( getWengoAccount() )
	{
		std::string method = "deleteUserNetwork";
		std::string parms  = "";
		
		parms += initParameters( method );
		parms += "&userNetworkId=" + String::fromNumber(serverKey);

		setParameters( parms );
		
		call(this);		//Calls the web service
	}
}

//-----------------------------------------------------------------------------

void WsAccount::answerReceived(const std::string & answer, int requestId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	if (!answer.empty()) 
	{
		bool fromServer = false;
		bool isValid    = false;
		IMAccountList imAccountList;

		fromXml( imAccountList, answer );

		switch( _queryResult.getEnumMethod() )
		{
		case QueryResult::EnumMethod_SaveUserNetwork:
			handleSet( imAccountList );
			break;

		case QueryResult::EnumMethod_GetUserNetwork:
			handleGet( imAccountList );
			break;

		case QueryResult::EnumMethod_RemoveUserNetwork:
			handleRemove();
			break;

		default:
			LOG_WARN( "Unexpected method" );
			assert(false);
		}
	}
}

//-----------------------------------------------------------------------------

std::string WsAccount::initParameters( const std::string& method )
{
	std::string parms = "";

	parms += "key=" + getSecurityKey();
	parms += "&method=";
	parms += method;
//	parms += "&userId=" + _userProfile.getVoxOxAccount()->getCleanLogin();	//Strips /VoxOx.
	parms += "&userId=" + _userProfile.getVoxOxAccount()->getDisplayAccountId();

	return parms;
}

//-----------------------------------------------------------------------------

std::string WsAccount::getSecurityKey()
{
	return _userProfile.getSecurityKey();
}

//-----------------------------------------------------------------------------
	
std::string WsAccount::toXml( const IMAccount& imAccount, const std::string& method )
{
	std::string result = "";
	std::string buildId = WengoPhoneBuildId::getVersion();

	IMAccountXMLSerializer serializer( const_cast<IMAccount&>(imAccount) );
//	serializer.setIMAccountParametersData( this->_userProfile._imAccountParametersData);
//	serializer.unserialize(data);

	serializer.setMethod( method );
	result = serializer.serializeForWebService( buildId );

	return result;
}

//-----------------------------------------------------------------------------

void WsAccount::fromXml( IMAccountList& imAccountList, const std::string& xml )
{
	bool result = true;

	TiXmlDocument doc;

	doc.Parse(xml.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlNode*  syncNode = NULL;
	
	syncNode = findSyncNode( docHandle );

	QueryResult::EnumMethod eMethod = _queryResult.getEnumMethod();

	if (syncNode) 
	{
		assert( eMethod != QueryResult::EnumMethod_None );

		std::string nodeData;
		nodeData << *syncNode;

		switch( eMethod )
		{
		case QueryResult::EnumMethod_GetUserNetwork:
			parseGet( syncNode, imAccountList );
			break;

		case QueryResult::EnumMethod_SaveUserNetwork:
			parseSet( syncNode, imAccountList );
			break;

		case QueryResult::EnumMethod_RemoveUserNetwork:
			parseRemove( syncNode, _userNetworkId, _bDeleted );
			break;

		default:
			assert(false);		//New method?
		}

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
	}
	else
	{
		assert(false);		//Did not find node.  Did XML change?
	}
}

//-----------------------------------------------------------------------------
			
void WsAccount::parseGet( TiXmlNode* syncNode, IMAccountList& imAccountList )
{
	TiXmlNode* networkNode = syncNode->FirstChild( "NTWK" );

	if ( networkNode )
	{
		std::string nodeData = "";
		nodeData << *networkNode;
	
		IMAccount imAccount;
		_userProfile.initImAccountParameters( &imAccount );

		IMAccountListXMLSerializer serializer( const_cast<IMAccountList&>(imAccountList));
		TiXmlHandle h(networkNode);
		serializer.unserializeWebService(h, imAccount.getIMAccountParameters() );
	}
}

//-----------------------------------------------------------------------------
			
void WsAccount::parseSet( TiXmlNode* syncNode, IMAccountList& imAccountList )
{
	TiXmlNode*  networkNode = NULL;

	IMAccount imAccount;
	_userProfile.initImAccountParameters( &imAccount );

	IMAccountXMLSerializer serializer( imAccount );
	TiXmlHandle h(syncNode);
	serializer.unserializeWebService(h, imAccount.getIMAccountParameters() );

	//VOXOX - JRT - 2009.06.02  we currently get empty node if no UserNetworks exist on server.
	if ( imAccount.getProtocol() != EnumIMProtocol::IMProtocolUnknown )
	{
		//VOXOX - JRT - 2009.07.13 - HACK to remove old voxox domain accounts.
		if ( !imAccount.isOldVoxOxDomain() )	//Just throw this away.
		{
			imAccountList.Add( &imAccount );
		}
	}
	else
	{
		int xxx = 1;	//TODO: we get here is network already exists and was 'saved' with NID == 0.
	}
}

//-----------------------------------------------------------------------------
				
void WsAccount::parseRemove( TiXmlNode* syncNode, int&unid, bool& bDeleted )
{
	unid	 = 0;
	bDeleted = false;

	TiXmlNode* networkNode = syncNode->FirstChild( "NTWK" );

	if ( networkNode )
	{
		std::string nodeData = "";
		nodeData << *networkNode;

		TiXmlNode* node = networkNode->FirstChild( "UNID" );
		if ( node )
		{
			unid = String( node->FirstChild()->Value() ).toInteger();
		}

		node = networkNode->FirstChild("DLTD");
		if ( node )
		{
			bDeleted = (strcmp( node->FirstChild()->Value(), "TRUE" ) == 0);
		}
	}
}

//-----------------------------------------------------------------------------

TiXmlNode* WsAccount::findSyncNode( TiXmlHandle docHandle )
{
	TiXmlNode* syncNode = NULL;
	QueryResult::EnumMethod eMethod = QueryResult::EnumMethod_None;

	if ( syncNode == NULL )
	{
		syncNode = docHandle.FirstChild("Sync").FirstChild("getUserNetwork").Node();
		eMethod  = (syncNode ? QueryResult::EnumMethod_GetUserNetwork : QueryResult::EnumMethod_None);
	}

	if ( syncNode == NULL )
	{
		syncNode = docHandle.FirstChild("Sync").FirstChild("saveUserNetwork").Node();
		eMethod  = (syncNode ? QueryResult::EnumMethod_SaveUserNetwork : QueryResult::EnumMethod_None);
	}

	if ( syncNode == NULL )
	{
		syncNode = docHandle.FirstChild("Sync").FirstChild("deleteUserNetwork").Node();
		eMethod  = (syncNode ? QueryResult::EnumMethod_RemoveUserNetwork : QueryResult::EnumMethod_None);
	}

	_queryResult.setEnumMethod( eMethod );

	return syncNode;
}

//-----------------------------------------------------------------------------

void WsAccount::handleSet( const IMAccountList& imAccountList )
{
	IMAccount* imAccount = const_cast<IMAccountList&>(imAccountList).getFirstIMAccount();

	if ( imAccount )
	{
		wsAccountAddedEvent( *this, *imAccount );				//Need to handle to get server NID.
	}
	else
	{
//		assert(false);	//Should not be empty
	}
}

//-----------------------------------------------------------------------------

void WsAccount::handleGet( const IMAccountList& imAccountList )
{
	wsAccountFetchedEvent( *this, imAccountList );
}

//-----------------------------------------------------------------------------

void WsAccount::handleRemove()
{
	if ( !_bDeleted )
	{
		wsAccountRemovedEvent( *this, _userNetworkId );
	}
}

//-----------------------------------------------------------------------------

std::string WsAccount::getServerProtocolId( const IMAccount& imAccount )
{
	QtEnumIMProtocol::ServerProtocolId pid = QtEnumIMProtocolMap::getInstance().toServerProtocolId( imAccount.getQtProtocol() );

	String networkId = String::fromNumber( pid );
	return networkId;
}

//-----------------------------------------------------------------------------
	
WsAccount& WsAccount::operator=( const WsAccount& src )
{
	if ( this != &src )
	{
//		_userProfile	= src._userProfile;
		_queryResult	= src._queryResult;
		_bDeleted		= src._bDeleted;
		_userNetworkId	= src._userNetworkId;
	}

	return *this;
}

//=============================================================================
