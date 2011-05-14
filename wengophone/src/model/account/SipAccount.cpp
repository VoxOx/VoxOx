/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "SipAccount.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/network/NetworkObserver.h>
#include <model/WengoPhone.h>

#include <thread/Thread.h>
#include <thread/ThreadEvent.h>
#include <util/Logger.h>

SipAccount		SipAccount::empty;
unsigned short	SipAccount::_testSIPRetry		=    4;
int				SipAccount::_initTimerMaximum	=    4;
int				SipAccount::_initTimerInterval	= 5000;

SipAccount::SipAccount()
	: Account(EnumIMProtocol::IMProtocolSIP) 
{
	initVars();
}

SipAccount::SipAccount(const std::string & username,const std::string & userid,
	const std::string & password, const std::string & realm,
	const std::string & registerservername, unsigned registerserverport,
	const std::string & SIPproxyservername, unsigned SIPproxyserverport,
	bool rememberPasswod, bool enablePIM, bool rememberUsername, bool autoLoginVoxOx) 
{
	initVars();

	setProtocol( EnumIMProtocol::IMProtocolSIP );
	_identity					= userid;
	_username					= username;
	_realm						= realm;
	_password					= password;
	_imPassword					= password;			//VOXOX - JRT - 2009.09.25 - SIP/Jabber password separation. Will get actual password from SSO.
	_registerServerHostname		= registerservername;
	_registerServerPort			= registerserverport;
	_sipProxyServerHostname		= SIPproxyservername;
	_sipProxyServerPort			= SIPproxyserverport;
	_enablePIM					= enablePIM;
	_rememberPassword			= rememberPasswod;
	_rememberUsername			= rememberUsername;
	_autoLoginVoxOx				= autoLoginVoxOx;

	//VOXOX - JRT - 2009.09.07 - These are already set in new initVars().
//	_voicemail					= "*98";
//	_discoveringNetwork			= false;
//	_lastNetworkDiscoveryState	= NetworkDiscoveryStateError;
//	_initSuccessful				= false;

//	_initTimerCounter			= 0;
//	_initTimer.lastTimeoutEvent += boost::bind(&SipAccount::initLastTimeoutEventHandler, this);
}

void SipAccount::initVars()
{
	_registerServerPort			= 0;
	_sipProxyServerPort			= 0;
	_httpTunnelServerPort		= 0;
	_httpsTunnelServerPort		= 0;
	_needsHttpTunnel			= false;
	_httpTunnelWithSSL			= false;
	_localSIPPort				= 0;
	_isConnected				= false;
	_rememberPassword			= true;
	_rememberUsername			= true;
	_autoLoginVoxOx				= false;
	_visibleName				= "";
	_voicemail					= "*98";
	_enablePIM					= true;
	_initSuccessful				= false;
	_vLineId					= 0;
	_discoveringNetwork			= false;

	_lastNetworkDiscoveryState	= NetworkDiscoveryStateError;
	_lastLoginState				= EnumSipLoginState::SipLoginStateUnknown;

	_initTimerCounter			= 0;
	_initTimer.lastTimeoutEvent += boost::bind(&SipAccount::initLastTimeoutEventHandler, this);
}

SipAccount::SipAccount(const SipAccount & sipAccount)
	: Account(sipAccount) {
	copy(sipAccount);
}

SipAccount & SipAccount::operator = (const SipAccount & sipAccount) {
	copy(sipAccount);

	return *this;
}

SipAccount * SipAccount::clone() const {
	return new SipAccount(*this);
}

SipAccount::~SipAccount() {
	Mutex::ScopedLock lock(_mutex);

	NetworkObserver::getInstance().connectionIsDownEvent -= boost::bind(&SipAccount::connectionIsDownEventHandler, this, _1);
	NetworkObserver::getInstance().connectionIsUpEvent   -= boost::bind(&SipAccount::connectionIsUpEventHandler, this, _1);

	_initTimer.stop();
}

void SipAccount::copy(const SipAccount & sipAccount) 
{
	Account::copy(sipAccount);

	_registerServerHostname		= sipAccount._registerServerHostname;
	_registerServerPort			= sipAccount._registerServerPort;
	_sipProxyServerHostname		= sipAccount._sipProxyServerHostname;
	_sipProxyServerPort			= sipAccount._sipProxyServerPort;
	_httpTunnelServerHostname	= sipAccount._httpTunnelServerHostname;
	_httpTunnelServerPort		= sipAccount._httpTunnelServerPort;
	_httpsTunnelServerHostname	= sipAccount._httpsTunnelServerHostname;
	_httpsTunnelServerPort		= sipAccount._httpsTunnelServerPort;
	_needsHttpTunnel			= sipAccount._needsHttpTunnel;
	_httpTunnelWithSSL			= sipAccount._httpTunnelWithSSL;
	_localSIPPort				= sipAccount._localSIPPort;
	_isConnected				= sipAccount._isConnected;
	_rememberPassword			= sipAccount._rememberPassword;
	_rememberUsername			= sipAccount._rememberUsername;
	_autoLoginVoxOx				= sipAccount._autoLoginVoxOx;
	_displayName				= sipAccount._displayName;
	_identity					= sipAccount._identity;
	_username					= sipAccount._username;
	_password					= sipAccount._password;
	_imPassword					= sipAccount._imPassword;
	_realm						= sipAccount._realm;
	_lastNetworkDiscoveryState	= sipAccount._lastNetworkDiscoveryState;
	_lastLoginState				= sipAccount._lastLoginState;
	_visibleName				= sipAccount._visibleName;
	_voicemail					= sipAccount._voicemail;
	_enablePIM					= sipAccount._enablePIM;
	_initSuccessful				= sipAccount._initSuccessful;
	_vLineId					= sipAccount._vLineId;		//VOXOX - JRT - 2009.09.07 

	_discoveringNetwork			= false;

	_initTimerCounter			= sipAccount._initTimerCounter;
	_initTimer.lastTimeoutEvent += boost::bind(&SipAccount::initLastTimeoutEventHandler, this);
}

bool SipAccount::operator == (const SipAccount & other) const 
{
	if (other.getType() != SipAccountTypeBasic) 
	{
		return false;
	}
	else
	{
		loginStateChangedEvent(*(SipAccount *)this, EnumSipLoginState::SipLoginStateNetworkError, "");//VOXOX CHANGE by Rolando - 2009.09.10 
	}

	return (	_identity				== other._identity &&
				_username				== other._username &&
				_realm					== other._realm &&
				_password				== other._password &&
				_imPassword				== other._imPassword &&		//VOXOX - JRT - 2009.09.25 
				_registerServerHostname == other._registerServerHostname &&
				_registerServerPort		== other._registerServerPort &&
				_sipProxyServerHostname == other._sipProxyServerHostname &&
				_sipProxyServerPort		== other._sipProxyServerPort );
}

void SipAccount::setConnected(bool connected) 
{
	Mutex::ScopedLock lock(_mutex);

	if (_isConnected != connected) 
	{
		_isConnected = connected;

		if (_isConnected) 
		{
			loginStateChangedEvent(*this, EnumSipLoginState::SipLoginStateConnected, "");//VOXOX CHANGE by Rolando - 2009.09.10 
		} 
		else 
		{
			loginStateChangedEvent(*this, EnumSipLoginState::SipLoginStateDisconnected, "");//VOXOX CHANGE by Rolando - 2009.09.10 
		}
	}
}

void SipAccount::phoneLineAuthenticationError() 
{
	loginStateChangedEvent(*this, EnumSipLoginState::SipLoginStatePasswordError, "");//VOXOX CHANGE by Rolando - 2009.09.10 
}

void SipAccount::init() 
{
	Mutex::ScopedLock lock(_mutex);

	_initSuccessful = false;

	if (NetworkObserver::getInstance().isConnected()) 
	{
		_initTimerCounter = 0;
		_initTimer.start(0, _initTimerInterval, 1);
	}
	else
	{
		loginStateChangedEvent(*this, EnumSipLoginState::SipLoginStateNetworkError, "");//VOXOX CHANGE by Rolando - 2009.09.10 
	}

	NetworkObserver::getInstance().connectionIsDownEvent += boost::bind(&SipAccount::connectionIsDownEventHandler, this, _1);
	NetworkObserver::getInstance().connectionIsUpEvent   += boost::bind(&SipAccount::connectionIsUpEventHandler, this, _1);
}

void SipAccount::initLastTimeoutEventHandler() 
{
	typedef ThreadEvent0<void()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&SipAccount::initLastTimeoutEventHandlerThreadSafe, this));
	WengoPhone::getInstance().postEvent(event);
}

void SipAccount::initLastTimeoutEventHandlerThreadSafe() 
{
	Mutex::ScopedLock lock(_mutex);

	_initTimer.stop();

	if (++_initTimerCounter >= _initTimerMaximum) 
	{
		loginStateChangedEvent(*this, EnumSipLoginState::SipLoginStateNetworkError, "");//VOXOX CHANGE by Rolando - 2009.09.10 
		networkDiscoveryStateChangedEvent(*this, _lastNetworkDiscoveryState);
	} 
	else 
	{
		if (!_discoveringNetwork) 
		{
			_discoveringNetwork = true;
 			_lastLoginState = discoverNetwork();
			std::string loginErrorMessage = getLoginErrorMessage();//VOXOX CHANGE by Rolando - 2009.09.10 

			if (_lastLoginState != EnumSipLoginState::SipLoginStateNetworkError) 
			{

				loginStateChangedEvent(*this, _lastLoginState, loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
				networkDiscoveryStateChangedEvent(*this, _lastNetworkDiscoveryState);

				/*NetworkObserver::getInstance().connectionIsDownEvent -= boost::bind(&SipAccount::connectionIsDownEventHandler, this, _1);
				NetworkObserver::getInstance().connectionIsUpEvent     -= boost::bind(&SipAccount::connectionIsUpEventHandler, this, _1);*/
				_initSuccessful = true;
			} 
			else 
			{
				// restart timer
				_initTimer.start(_initTimerInterval, 0, 1);
			}

			_discoveringNetwork = false;
		}
	}
}

void SipAccount::connectionIsUpEventHandler(NetworkObserver & sender) 
{
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&SipAccount::connectionIsUpEventHandlerThreadSafe, this));

	WengoPhone::getInstance().postEvent(event);
}

void SipAccount::connectionIsUpEventHandlerThreadSafe() 
{
	Mutex::ScopedLock lock(_mutex);

	if (!_initSuccessful) 
	{
		_initTimerCounter = 0;
		_initTimer.start(0, _initTimerInterval, 1);
	}
}

void SipAccount::connectionIsDownEventHandler(NetworkObserver & sender) //VOXOX - JRT - 2009.09.07 - TODO: does this normal ThreadSafe calls?
{
	//typedef ThreadEvent0<void ()> MyThreadEvent;
	//MyThreadEvent * event = new MyThreadEvent(boost::bind(&SipAccount::connectionIsDownEventHandlerThreadSafe, this));

	//WengoPhone::getInstance().postEvent(event);
	connectionIsDownEventHandlerThreadSafe();
}

void SipAccount::connectionIsDownEventHandlerThreadSafe() 
{
	Mutex::ScopedLock lock(_mutex);

	_initTimer.stop();
}

bool SipAccount::discoverForSIP() 
{
	//
	// Please contact network@openwengo.com before any modifications.
	//

	LOG_DEBUG("discovering network parameters for SIP connection");

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	_localSIPPort = _networkDiscovery.getFreeLocalPort();

	if (!config.getNetWorkTunnelNeeded()) 
	{
		LOG_DEBUG("SIP will use " + String::fromNumber(_localSIPPort) + " as local SIP port");

		// SIP test with UDP
		for (unsigned short i = 0; i < _testSIPRetry; i++) 
		{
			LOG_DEBUG("testSIP test number: " + String::fromNumber(i + 1));

			if (_networkDiscovery.testSIP(_sipProxyServerHostname,_username, (unsigned short) _sipProxyServerPort, (unsigned short) _localSIPPort)) 
			{
				LOG_DEBUG("SIP can connect via UDP");
				_needsHttpTunnel = false;
				return true;
			}
		}
	}

	LOG_ERROR("cannot connect via UDP");

	return false;
}

EnumSipLoginState::SipLoginState SipAccount::discoverNetwork() 
{
	if (!discoverForSIP()) 
	{
		LOG_ERROR("error while discovering network for SIP");
		_lastNetworkDiscoveryState = NetworkDiscoveryStateSIPError;
		return EnumSipLoginState::SipLoginStateNetworkError;
	}

	LOG_DEBUG("initialization Ok");
	_lastNetworkDiscoveryState = NetworkDiscoveryStateOk;
	return EnumSipLoginState::SipLoginStateReady;
}

bool SipAccount::isEmpty() const 
{
	return (	_identity.empty() ||
				_username.empty() ||
				_realm.empty() ||
				_registerServerHostname.empty() ||
				_registerServerPort == 0 ||
				_sipProxyServerHostname.empty() ||
				_sipProxyServerPort == 0 );
}
