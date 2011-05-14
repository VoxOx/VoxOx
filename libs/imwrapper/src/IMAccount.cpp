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

#include <imwrapper/IMAccount.h>

#include <settings/SettingsXMLSerializer.h>

using namespace std;

std::string IMAccount::_oldVoxOxDomain = "im.voxox.com";//VOXOX - JRT - 2009.07.14 


IMAccount::IMAccount() : Account(EnumIMProtocol::IMProtocolUnknown) 
{
	initVars();
	updateKey();
}

IMAccount::IMAccount(EnumIMProtocol::IMProtocol protocol) 
	: Account(protocol) 
{
	initVars();

	//VOXOX - JRT - 2009.09.10 - These are set in initVars().
//	_connected		= false;
//	_voxoxAccount	= false;
//	_presenceState	= EnumPresenceState::PresenceStateOffline;	//PresenceStateOnline;	//VOXOX - JRT - 2009.09.08 - we should not assume online
//	_imAccountParameters.valueChangedEvent += boost::bind(&IMAccount::valueChangedEventHandler, this, _1);

	updateKey();
}
	
IMAccount::IMAccount(const std::string & login, const std::string & password, EnumIMProtocol::IMProtocol protocol, const std::string & statusMessage) 
	: Account(protocol) 
{
	initVars();

	_login			= correctedLogin(login, protocol);
	_password		= password;
	_statusMessage	= statusMessage;//VOXOX CHANGE by Rolando - 2009.07.01 

	updateKey();
}

IMAccount::IMAccount(const std::string & login, const std::string & password, EnumIMProtocol::IMProtocol protocol,const bool & voxoxAccount, const std::string & statusMessage) 
	: Account(protocol) 
{
	initVars();

	_login			= correctedLogin(login, protocol);	//TODO: this strips off Jabber ResourceID.
	_password		= password;
	_statusMessage  = statusMessage;//VOXOX CHANGE by Rolando - 2009.07.01 
	_voxoxAccount   = voxoxAccount;

	updateKey();
}

IMAccount::IMAccount(const IMAccount & imAccount) : Account(imAccount), Trackable() 
{
	copy(imAccount);
}

void IMAccount::initVars()
{
	_autoReconnect    = true;		//VOXOX - JRT - 2009.07.05 
	_connected		  = false;
	_needsSyncing	  = false;
	_voxoxAccount	  = false;
	_key			  = "";	
	_login			  = "";
	_password		  = "";
	_server			  = "";
	_statusMessage	  = "";
	_userNetworkId	  = 0;
	_serverProtocolId = QtEnumIMProtocol::ServerProtocolIdNone;

	_presenceState			= EnumPresenceState::PresenceStateOffline;	//PresenceStateOnline;	//VOXOX - JRT - 2009.09.08 - We should not assume online
	_initialPresenceState	= EnumPresenceState::PresenceStateOnline;

	_imAccountParameters.valueChangedEvent += boost::bind(&IMAccount::valueChangedEventHandler, this, _1);
}

IMAccount & IMAccount::operator = (const IMAccount & imAccount) 
{
	copy(imAccount);

	return *this;
}

IMAccount * IMAccount::clone() const 
{
	return new IMAccount(*this);
}

void IMAccount::copy(const IMAccount & imAccount) 
{
	Account::copy(imAccount);

	//VOXOX - JRT - 2009.09.10 - Some copy() calls maybe should NOT copy _presenceState since it is dynamic.
	//							 An example is when we sync from server.  Keep an eye out for this potential problem.
	_autoReconnect			= imAccount._autoReconnect;
	_connected				= imAccount._connected;
	_key					= imAccount.getKey();
	_login					= imAccount._login;
	_needsSyncing			= imAccount.needsSyncing();
	_password				= imAccount._password;
	_presenceState			= imAccount._presenceState;
	_initialPresenceState	= imAccount._initialPresenceState;
	_server					= imAccount.getServer();
	_serverProtocolId		= imAccount.getServerProtocolId();
	_statusMessage			= imAccount._statusMessage;
	_userNetworkId			= imAccount.getUserNetworkId();
	_voxoxAccount			= imAccount._voxoxAccount;

	_imAccountParameters	= imAccount._imAccountParameters;

	_imAccountParameters.valueChangedEvent += boost::bind(&IMAccount::valueChangedEventHandler, this, _1);
}
	
IMAccount::~IMAccount() 
{
}

//VOXOX - JRT - 2009.04.25 - override Identifiable class.
bool IMAccount::operator == (const IMAccount& imAccount) const
{
	return getKey() == imAccount.getKey();
}

bool IMAccount::empty() const 
{
	return _login.empty() 
		&& _password.empty() 
		&& ( getProtocol() == EnumIMProtocol::IMProtocolUnknown);
}

bool IMAccount::isVoxOxAccount() const 
{
	return ( getProtocol() == EnumIMProtocol::IMProtocolJabber && _voxoxAccount );
}

bool IMAccount::isGtalkAccount() const 
{
	bool isGtalk = false;

	if( getProtocol() == EnumIMProtocol::IMProtocolJabber )
	{
		std::string server = _imAccountParameters.getJabberConnectionServer();	//Debug
		
		if ( server == IMAccountParameters::GOOGLETALK_SERVER )
		{
			isGtalk = true;
		}
	}

	return isGtalk;
}

void IMAccount::setLogin(const std::string & login) 
{
	_login = correctedLogin(login, getProtocol() );
	setAutoReconnect( true );		//VOXOX - JRT - 2009.07.05 
	updateKey();
	imAccountChangedEvent(*this);
}

void IMAccount::setProtocol( EnumIMProtocol::IMProtocol val )
{
	Account::setProtocol( val );
	updateKey();
}

void IMAccount::setStatusMessage(const std::string & statusMessage)
{
	_statusMessage = statusMessage;
	imAccountChangedEvent(*this);
}

void IMAccount::setPassword(const string & password) 
{
	_password = password;
	setAutoReconnect( true );		//VOXOX - JRT - 2009.07.05 
	imAccountChangedEvent(*this);
}

void IMAccount::setIsVoxoxAccount(const bool & voxoxAccount) 
{
	_voxoxAccount = voxoxAccount;
	imAccountChangedEvent(*this);
}

string IMAccount::correctedLogin(const string & login, EnumIMProtocol::IMProtocol protocol) 
{
	string result = login;

	if (protocol == EnumIMProtocol::IMProtocolJabber) {
		string::size_type index = login.find('/');

		if (index == string::npos) {
			result += "/VoxOx";
		}
	}

	return result;
}

void IMAccount::valueChangedEventHandler(const std::string & key) 
{
	imAccountChangedEvent(*this);
}

void IMAccount::updateKey()
{
	_key = _login + EnumIMProtocol::toString( getProtocol() );
	_key = String::toLowerCase( _key );	//VOXOX - JRT - 2009.10.29 
}

//VOXOX CHANGE by Rolando - 2009.08.28 
void IMAccount::changeLogin(std::string newLogin)
{
	_login = newLogin;
	updateKey();
}

std::string IMAccount::getKey() const
{
	assert( !_key.empty() );
	return _key;
}

std::string IMAccount::getDisplayAccountId() const
{
	if (_voxoxAccount) 
	{
		size_t posFirstOf = _login.find("@");
		if (posFirstOf != _login.npos) 
		{
			return _login.substr(0, posFirstOf);
		}
	}

	return _login;
}

std::string IMAccount::getCleanLogin() const 
{
	size_t posFirstOf = _login.find("/");
		
	if (posFirstOf != _login.npos) 
	{
		return _login.substr(0, posFirstOf);
	}

	return _login;
}

QtEnumIMProtocol::IMProtocol IMAccount::getQtProtocol()	const
{
	QtEnumIMProtocol::IMProtocol qtProtocol;
	qtProtocol = QtEnumIMProtocolMap::getInstance().toQtIMProtocol( getProtocol(), isVoxOxAccount(), isGtalkAccount() );
	return qtProtocol;
}

QtEnumIMProtocol::ServerProtocolId IMAccount::toServerProtocolId() const
{
	QtEnumIMProtocol::ServerProtocolId protocolId;
	protocolId = QtEnumIMProtocolMap::getInstance().toServerProtocolId( getQtProtocol() );
	return protocolId;
}

//VOXOX - JRT - 2009.07.13 
std::string IMAccount::getDomain() const
{
	std::string domain = "";
	std::string login  = getLogin();

	if ( !login.empty() )
	{
		string::size_type pos1 = login.find_first_of("@");
		string::size_type pos2 = login.find_last_of("/");

		if ( pos1 == -1 && pos2 == -1 )
		{
			domain = login;
		}
		else
		{
			domain = login.substr( (pos1 == string::npos ? 1 : pos1+1), (pos2 == string::npos ? login.length() - 2 : pos2 - pos1 - 1 ) );
		}
	}

	return domain;
}

bool IMAccount::isOldVoxOxDomain()
{
	bool result = false;

	if ( isVoxOxAccount() )
	{
		String domain = getDomain();
		domain = domain.toLowerCase();

		if ( domain == _oldVoxOxDomain )
		{
			result = true;
		}
	}

	return result;
}

//VOXOX - JRT - 2009.07.14 
bool IMAccount::shouldSync() const
{
	bool result = false;

//	if ( getUserNetworkId() == 0 )
	{
		result = needsSyncing();
	}

	return result;
}

bool IMAccount::shouldConnect() const	//VOXOX - JRT - 2009.09.14
{
	bool result = false;

	if ( !EnumPresenceState::isOnlineEx( getPresenceState() ) )
	{
		result = (getInitialPresenceState() != EnumPresenceState::PresenceStateOffline);
	}

	return result;
}

std::string IMAccount::getUseableStatusMsg( const std::string newStatusMessage )
{
	std::string result = newStatusMessage;

	//VOXOX CHANGE CJC Just update status message if its not on Social networks accounts. 
	//This is trigger when the presence is change.
	//If the status message is especifically change, it should get trigger.
//	if(realIMAccount->getProtocol() != EnumIMProtocol::IMProtocolFacebook && realIMAccount->getProtocol() != EnumIMProtocol::IMProtocolTwitter && realIMAccount->getProtocol() != EnumIMProtocol::IMProtocolMYSPACE){
	if ( QtEnumIMProtocolMap::supportsPresenceNote( getQtProtocol() ) )
	{
		if(result == String::null)
		{
			result = getStatusMessage();//VOXOX CHANGE by Rolando - 2009.07.15 
		}
	}
	else
	{
		result = "";
	}

	return result;
}

		
bool IMAccount::isOkToChangePresence( EnumPresenceState::PresenceState newPresenceState )
{
	bool result = true;

	//VOXOX CHANGE by Rolando - 2009.07.22 - Fix to complete issue "Change: Completely remove status color circles from Twitter Account"
	if ( getProtocol() == EnumIMProtocol::IMProtocolTwitter )
	{
		switch ( newPresenceState )
		{
		case EnumPresenceState::PresenceStateOnline:
		case EnumPresenceState::PresenceStateOffline:
			result = true;
			break;

		default:
			result = false;
		}
	}

	return result;
}

void IMAccount::updateInitialPresenceState( EnumPresenceState::PresenceState ps )
{
	//Since not all presence statuses are supported by all protocols, let's get one that is.
	EnumPresenceState::PresenceState supportedPS = QtEnumIMProtocolMap::getMappedPresenceState( getQtProtocol(), ps );

	bool bChanged = (getInitialPresenceState() != supportedPS);

	setInitialPresenceState( supportedPS );

	if ( bChanged )
	{
		setNeedsSyncing( true );
	}
}
