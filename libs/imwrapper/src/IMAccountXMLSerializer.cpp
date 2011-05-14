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

#include <imwrapper/IMAccountXMLSerializer.h>

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountParametersXMLSerializer.h>

#include <util/Base64.h>

#include <tinyxml.h>

using namespace std;

IMAccountXMLSerializer::IMAccountXMLSerializer(IMAccount & imAccount)
	: _imAccount(imAccount) 
{
}

void IMAccountXMLSerializer::setIMAccountParametersData(const std::string & data) 
{
	_imAccountParametersData = data;
}

std::string IMAccountXMLSerializer::serialize() 
{
	string result;
	EnumIMProtocol enumIMProtocol;

	result += "<account protocol=\"" + enumIMProtocol.toString(_imAccount.getProtocol()) + "\">\n";	//VOXOX - JRT - 2009.04.24

	result += serializeTag( "login",			_imAccount.getLogin() );
	result += serializeTag( "password",			Base64::encode(_imAccount.getPassword()) );
	result += serializeTag( "statusMessage",	_imAccount.getStatusMessage() );		//VOXOX CHANGE by Rolando - 2009.07.01 
//	result += serializeTag( "presence",			EnumPresenceState::toString(_imAccount._presenceState) );	
	result += serializeTag( "initialPresence",	EnumPresenceState::toString(_imAccount.getInitialPresenceState()) );	
	result += serializeTag( "voxoxAccount",		_imAccount.isVoxOxAccount()		 );
	result += serializeTag( "serverPID",		_imAccount.getServerProtocolId() );
	result += serializeTag( "userNID",			_imAccount.getUserNetworkId()	 );

	IMAccountParametersXMLSerializer serializer(_imAccount.getIMAccountParameters());
	result += serializer.serialize();
	result += "</account>\n";

	return result;
}

bool IMAccountXMLSerializer::unserialize(const std::string & data) 
{
	TiXmlDocument doc;
	EnumIMProtocol imProtocol;

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle account = docHandle.FirstChild("account");

	// Retrieving protocol
	TiXmlElement * lastChildElt = account.Element();
	if (lastChildElt) 
	{
		_imAccount.setProtocol( imProtocol.toIMProtocol(lastChildElt->Attribute("protocol") ) );
	} 
	else 
	{
		return false;
	}

	//Retrieving server Network ID //VOXOX - JRT - 2009.06.01 
	_imAccount.setServerProtocolId( (QtEnumIMProtocol::ServerProtocolId)toInt( fromNode( account, "serverPID" ) ) );
	_imAccount.setUserNetworkId   (                                     toInt( fromNode( account, "userNID"   ) ) );

	//Retrieving login
	TiXmlText * login = account.FirstChild("login").FirstChild().Text();
	if (login) 
	{
		_imAccount.setLogin( login->Value() );
	} 
	else 
	{
		return false;
	}

	//Retrieving password
	TiXmlText * password = account.FirstChild("password").FirstChild().Text();
	if (password) 
	{
		_imAccount.setPassword( Base64::decode(password->Value()) );
	}

	//Retrieving status message
	TiXmlText * statusMessage = account.FirstChild("statusMessage").FirstChild().Text();
	if (statusMessage) 
	{
		_imAccount.setStatusMessage( statusMessage->Value() );
	}

	//Retrieving Presence state
	//TiXmlText * presence = account.FirstChild("presence").FirstChild().Text();
	//if (presence) 
	//{
	//	_imAccount._presenceState = EnumPresenceState::fromString(presence->Value());
	//}

	//Retrieving Presence state
	TiXmlText * presence = account.FirstChild("initialPresence").FirstChild().Text();
	if (presence) 
	{
		_imAccount.setInitialPresenceState( EnumPresenceState::fromString(presence->Value()) );
	}

	TiXmlText * voxoxAcc = account.FirstChild("voxoxAccount").FirstChild().Text();
	if (voxoxAcc) 
	{
		_imAccount.setIsVoxoxAccount( String(voxoxAcc->Value()).toBoolean() );
	}
	else
	{
		_imAccount.setIsVoxoxAccount( false );
	}

	//Retrieving IMAccountParameters
	IMAccountParameters imAccountParameters;
	IMAccountParametersXMLSerializer serializer(imAccountParameters);
	serializer.unserializeSystemSettings(_imAccountParametersData);

	TiXmlNode * settingsNode = account.FirstChild("settings").Node();
	if (settingsNode) 
	{
		string nodeData;
		nodeData << *settingsNode;
		serializer.unserializeUserSettings(nodeData);
	}

	_imAccount.setIMAccountParameters( imAccountParameters );

	return true;
}


//VOXOX - JRT - 2009.06.01 
std::string IMAccountXMLSerializer::serializeForWebService( std::string& buildId ) 
{
	std::string result = "";

	assert( !getMethod().empty() );

	//Some needed transformations.
	QtEnumIMProtocol::ServerProtocolId pid = QtEnumIMProtocolMap::getInstance().toServerProtocolId( _imAccount.getQtProtocol() );
	std::string uri = "";
	
	switch ( pid )
	{
	case QtEnumIMProtocol::ServerProtocolIdYahoo:
		uri = _imAccount.getLogin() + "@yahoo.com";
		break;

	case QtEnumIMProtocol::ServerProtocolIdJabber:
	case QtEnumIMProtocol::ServerProtocolIdGoogleTalk:
		uri = _imAccount.getIMAccountParameters().getJabberConnectionServer();
		break;

	default:
		uri = _imAccount.getLogin();
	}


//	result += "<Sync generator=\"voxox\" version=\"" + buildId  + "\">";		//TODO: need method for tag w/ attributes.
	result += "<Sync>";		//TODO: need method for tag w/ attributes.
//	result += "<Sync generator='voxox' version='" + buildId  + "'>";		//TODO: need method for tag w/ attributes.
	result += openTag( getMethod().c_str() );
	result += openTag( "NETWORK" );
	
	result += serializeTag( "UNID",		_imAccount.getUserNetworkId()	);
	result += serializeTag( "NID",		pid								);
	result += serializeTag( "URI",		uri								);

	result += serializeTag( "LGN",		_imAccount.getLogin()			);
	result += serializeTag( "PSWD",		_imAccount.getPassword()		);
	result += serializeTag( "STM",		_imAccount.getStatusMessage()	);
	result += serializeTag( "PRES",		_imAccount.getInitialPresenceState() );

	result += closeTag();	//NETWORK
	result += closeTag();	//syncToServe
	result += "</Sync>";	

	return result;
}

//-----------------------------------------------------------------------------

//VOXOX - JRT - 2009.06.01 
bool IMAccountXMLSerializer::unserializeWebService( TiXmlHandle& accountElt, IMAccountParameters& imAccountParameters ) 
{
	//VOXOX - JRT - 2009.06.06 - TODO: at some point we can use expanded EnumImprotocol values for VoxOx and Gtalk,
	//								but it will take some review to determine impact.
	//Some needed transformations
	EnumIMProtocol::IMProtocol			imProtocol		 = EnumIMProtocol::IMProtocolUnknown;
	QtEnumIMProtocol::IMProtocol		qtImProtocol	 = QtEnumIMProtocol::IMProtocolUnknown;
	QtEnumIMProtocol::ServerProtocolId	serverProtocolId = (QtEnumIMProtocol::ServerProtocolId) toInt( fromNode( accountElt, "NID"  ) );

	getIMProtocols( fromNode(accountElt, "NID"), imProtocol, qtImProtocol );

	//Set IMAccount
	_imAccount.setUserNetworkId   (	toInt( fromNode( accountElt, "UNID" ) ) );
	_imAccount.setServerProtocolId( serverProtocolId	);
	_imAccount.setProtocol        (	imProtocol			);
	
	_imAccount.setLogin			  ( fromNode( accountElt, "LGN"   ) );
	_imAccount.setPassword		  (	fromNode( accountElt, "PSWD"  ) );
	_imAccount.setStatusMessage	  ( fromNode( accountElt, "STM"   ) );	//VOXOX - JRT - 2009.09.14 

	//VOXOX - JRT - 2009.09.14 - If not does not exist, then do NOT override default.
	TiXmlNode * presenceNode = accountElt.FirstChild("PRES").Node();
	if ( presenceNode )
	{
		EnumPresenceState::PresenceState initPresence = (EnumPresenceState::PresenceState) toInt( fromNode( accountElt, "PRES" ) );
		_imAccount.setInitialPresenceState( initPresence );
	}

	_imAccount.setServer		  (	fromNode( accountElt, "URI"   ) );
	
	_imAccount.setIsVoxoxAccount( qtImProtocol == QtEnumIMProtocol::IMProtocolVoxOx);

	//Retrieving IMAccountParameters
//	initImAccountParameters( _imAccount );
//	IMAccountParameters imAccountParameters;
//	IMAccountParametersXMLSerializer serializer(imAccountParameters);
//	serializer.unserializeSystemSettings(_imAccountParametersData);

//	TiXmlNode * settingsNode = account.FirstChild("settings").Node();
//	if (settingsNode) 
//	{
//		string nodeData;
//		nodeData << *settingsNode;
//		serializer.unserializeUserSettings(nodeData);
//	}

	//IMAccountParameters
	switch( serverProtocolId )
	{
	case QtEnumIMProtocol::ServerProtocolIdJabber:
//		_imAccount._imAccountParameters.setByProtocol( _imAccount.getProtocol(), false, _imAccount.getServer() );
		imAccountParameters.setByProtocol( _imAccount.getProtocol(), false, _imAccount.getServer() );	//VOXOX - JRT - 2009.09.09 
		_imAccount.setIMAccountParameters( imAccountParameters );
		break;

	case QtEnumIMProtocol::ServerProtocolIdGoogleTalk:
//		_imAccount._imAccountParameters.setByProtocol( _imAccount.getProtocol(), true, _imAccount.getServer() );
		imAccountParameters.setByProtocol( _imAccount.getProtocol(), true, _imAccount.getServer() );	//VOXOX - JRT - 2009.09.09 
		_imAccount.setIMAccountParameters( imAccountParameters );
		break;
	}

	//	_imAccount._imAccountParameters = imAccountParameters;

	return true;
}

//-----------------------------------------------------------------------------

void IMAccountXMLSerializer::getIMProtocols( const std::string& protocolId,
										    EnumIMProtocol::IMProtocol& imProtocol, QtEnumIMProtocol::IMProtocol& qtImProtocol)
{
	qtImProtocol = getQtProtocolFromId(protocolId);
	imProtocol   = getModelProtocolFromId(protocolId);
}

//-----------------------------------------------------------------------------

void IMAccountXMLSerializer::initImAccountParameters( IMAccount& imAccount )
{
	IMAccountParameters& parameters = imAccount.getIMAccountParameters();
	IMAccountParametersXMLSerializer serializer( parameters );
	serializer.unserializeSystemSettings(_imAccountParametersData);
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.06.06 
QtEnumIMProtocol::IMProtocol IMAccountXMLSerializer::getQtProtocolFromId(std::string id) 
{
	QtEnumIMProtocol::ServerProtocolId pid  = (QtEnumIMProtocol::ServerProtocolId)atoi( id.c_str() );
	QtEnumIMProtocol::IMProtocol qtProtocol = QtEnumIMProtocol::IMProtocolUnknown;

	qtProtocol = QtEnumIMProtocolMap::getInstance().fromServerProtocolId( pid );

	return  qtProtocol;
}

//-----------------------------------------------------------------------------

EnumIMProtocol::IMProtocol IMAccountXMLSerializer::getModelProtocolFromId(std::string id) 
{
	QtEnumIMProtocol::IMProtocol qtProtocol  = getQtProtocolFromId(id);
	EnumIMProtocol::IMProtocol modelProtocol = EnumIMProtocol::IMProtocolUnknown;

	modelProtocol = QtEnumIMProtocolMap::getInstance().toModelIMProtocol( qtProtocol );

	return  modelProtocol;
}

//-----------------------------------------------------------------------------
