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
#include "WengoAccount.h"

//#include "WengoPhoneBuildId.h"		//VOXOX - JRT - 2009.10.02 - Not used.
#include "WengoAccountParser.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/network/NetworkObserver.h>
#include <model/network/NetworkProxyDiscovery.h>

#include <thread/Thread.h>
#include <util/StringList.h>
#include <util/Logger.h>
#include <http/HttpRequest.h>

#include <sstream>
#include <exception>

using namespace std;
static const std::string STATUS_CODE_OK = "200";//VOXOX CHANGE by Rolando - 2009.09.10 
static const std::string UNAUTHORIZED = "401";//VOXOX CHANGE by Rolando - 2009.09.10 
static const std::string UNACTIVATED = "402";//VOXOX CHANGE by Rolando - 2009.09.10 
static const std::string NOTFOUND = "404";//VOXOX CHANGE by Rolando - 2009.09.10 
static const std::string STATUS_CODE_OTHER = "444";//VOXOX CHANGE by Rolando - 2009.09.10 

unsigned short WengoAccount::_testStunRetry = 2;


WengoAccount::WengoAccount()
	: SipAccount(), WengoWebService(this) 
{
	initVars();
}

WengoAccount::WengoAccount(const std::string & login, const std::string & password, bool rememberPassword)
	: SipAccount(),
	WengoWebService(this) 
{
	initVars();

	_wengoLogin		  = login;
	_wengoPassword	  = password;
	_rememberPassword = rememberPassword;
	_statusCode		  = NOTFOUND;

	//Moved to initVars()
//	Config & config = ConfigManager::getInstance().getCurrentConfig();
//	setProtocol( EnumIMProtocol::IMProtocolWengo );
//	_ssoRequestOk = false;
//	_wengoLoginOk = false;
//	_ssoWithSSL = false;
//	_needsHttpTunnel = false;
//	_useEncryption = false;
//	_stunServer = config.getNetworkStunServer();
//	_discoveringNetwork = false;
//	_lastNetworkDiscoveryState = NetworkDiscoveryStateError;

//	_ssoTimer.timeoutEvent += boost::bind(&WengoAccount::ssoTimeoutEventHandler, this);
//	_ssoTimer.lastTimeoutEvent += boost::bind(&WengoAccount::ssoLastTimeoutEventHandler, this);
}

WengoAccount::WengoAccount(const WengoAccount & wengoAccount)
	: SipAccount(*this),
	WengoWebService(this) 
{
	initVars();

	copy(wengoAccount);
}

void WengoAccount::initVars()
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	_wengoLogin		  = "";
	_wengoPassword	  = "";
	_rememberPassword = false;
	_statusCode		  = "";

	setProtocol( EnumIMProtocol::IMProtocolWengo );

	_ssoRequestOk		= false;
	_wengoLoginOk		= false;
	_ssoWithSSL			= false;
	_discoveringNetwork = false;
	_needsHttpTunnel	= false;
	_useEncryption		= true;	//VOXOX - JRT - 2009.09.28 
	_stunServer			= config.getNetworkStunServer();
	_lastNetworkDiscoveryState = NetworkDiscoveryStateError;

	_ssoTimer.timeoutEvent	   += boost::bind(&WengoAccount::ssoTimeoutEventHandler, this);
	_ssoTimer.lastTimeoutEvent += boost::bind(&WengoAccount::ssoLastTimeoutEventHandler, this);
}

WengoAccount & WengoAccount::operator = (const WengoAccount & wengoAccount) {
	copy(wengoAccount);

	return *this;
}

bool WengoAccount::operator == (const SipAccount & other) const {
	if (other.getType() != SipAccount::SipAccountTypeWengo) {
		return false;
	}

	const WengoAccount & gother = dynamic_cast<const WengoAccount &>(other);

	return (	_wengoLogin == gother._wengoLogin &&
				_wengoPassword == gother._wengoPassword);
}

WengoAccount * WengoAccount::clone() const {
	return new WengoAccount(*this);
}

void WengoAccount::copy(const WengoAccount & wengoAccount) {
	SipAccount::copy(wengoAccount);

	_wengoLogin			= wengoAccount._wengoLogin;
	_wengoPassword		= wengoAccount._wengoPassword;
	_rememberPassword	= wengoAccount._rememberPassword;
	_ssoRequestOk		= wengoAccount._ssoRequestOk;
	_wengoLoginOk		= wengoAccount._wengoLoginOk;
	_statusCode 		= wengoAccount._statusCode;
	_ssoWithSSL			= wengoAccount._ssoWithSSL;
	_stunServer			= wengoAccount._stunServer;
	_discoveringNetwork = false;

	_lastNetworkDiscoveryState = wengoAccount._lastNetworkDiscoveryState;
}

WengoAccount::~WengoAccount() {
	Mutex::ScopedLock lock(_mutex);

	_ssoTimer.stop();
}

EnumSipLoginState::SipLoginState WengoAccount::discoverNetwork() {
	static const unsigned LOGIN_TIMEOUT = 10000;
	static const unsigned LIMIT_RETRY = 5;

	if (!discoverForSSO()) {
		LOG_ERROR("error while discovering network for SSO");
		_lastNetworkDiscoveryState = NetworkDiscoveryStateHTTPError;
		return EnumSipLoginState::SipLoginStateNetworkError;
	}

	_ssoTimerFinished = false;
	_ssoTimer.start(0, LOGIN_TIMEOUT, LIMIT_RETRY);
	while (!_ssoTimerFinished) {
		Thread::msleep(100);
	}

	if (!_ssoRequestOk) {
		LOG_ERROR("error while doing SSO request");
		_lastNetworkDiscoveryState = NetworkDiscoveryStateError;
		return EnumSipLoginState::SipLoginStateNetworkError;
	} else if (_ssoRequestOk && !_wengoLoginOk) {
		LOG_ERROR("SSO request Ok but login/password are invalid or server has problems with user info");
		_lastNetworkDiscoveryState = NetworkDiscoveryStateOk;
		if (_statusCode == UNACTIVATED){//VOXOX CHANGE by Rolando - 2009.09.10 
			return EnumSipLoginState::SipLoginStateUnActivatedError;
		}
		else{
			if(_statusCode == STATUS_CODE_OTHER){//VOXOX CHANGE by Rolando - 2009.09.10 
				return EnumSipLoginState::SipLoginStateOtherError;
			}
			else{
				return EnumSipLoginState::SipLoginStatePasswordError;//VOXOX CHANGE by Rolando - 2009.09.10 
			}
		}
	}

	if (!discoverForSIP()) {
		LOG_ERROR("error while discovering network for SIP");
		_lastNetworkDiscoveryState = NetworkDiscoveryStateSIPError;
		return EnumSipLoginState::SipLoginStateNetworkError;
	}

	LOG_DEBUG("initialization Ok");
	_lastNetworkDiscoveryState = NetworkDiscoveryStateOk;
	return EnumSipLoginState::SipLoginStateReady;
}

bool WengoAccount::discoverForSSO() {
	//
	// Please contact network@openwengo.com before any modifications.
	//
	Config& config = ConfigManager::getInstance().getCurrentConfig();
	string  url    = config.getWengoServerHostname() + ":" + String::fromNumber(80) + config.getWengoSSOPath();
	LOG_DEBUG("discovering network parameters for SSO connection: " + url);

	//VOXOX - JRT - 2009.09.30 - Re-implemented SSL check.
	string  url2 = config.getWengoServerHostname() + ":" + String::fromNumber(443) + config.getWengoSSOPath();

	if (_networkDiscovery.testHTTP(url2, true)) {
		_ssoWithSSL = true;
		LOG_DEBUG("SSO can connect on port 443 with SSL");
		return true;
	}


	if (_networkDiscovery.testHTTP(url, false)) {
		_ssoWithSSL = false;
		LOG_DEBUG("SSO can connect on port 80 without SSL");
		return true;
	}

	LOG_ERROR("SSO cannot connect");
	return false;
}

bool WengoAccount::discoverForSIP() {
	//
	// Please contact network@openwengo.com before any modifications.
	//

	LOG_DEBUG("discovering network parameters for SIP connection");

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	_localSIPPort = _networkDiscovery.getFreeLocalPort();
	
	if (!config.getNetWorkTunnelNeeded()) {
		LOG_DEBUG("SIP will use " + String::fromNumber(_localSIPPort) + " as local SIP port");

		// Stun test
		unsigned short iTestStun;
		for (iTestStun = 0; iTestStun < _testStunRetry; iTestStun++) {
			LOG_DEBUG("testUDP (Stun): " + String::fromNumber(iTestStun + 1));
			if (_networkDiscovery.testUDP(_stunServer)) {
				break;
			}
		}

		if (iTestStun == _testStunRetry) {
			// Stun test failed
			_networkDiscovery.setNatConfig(EnumNatType::NatTypeFullCone);
		}
		////

		// SIP test with UDP
		for (unsigned short i = 0; i < _testSIPRetry; i++) {
			LOG_DEBUG("testSIP test number: " + String::fromNumber(i + 1));

			if (_networkDiscovery.testSIP(_sipProxyServerHostname,_username, _sipProxyServerPort, _localSIPPort)) {
				LOG_DEBUG("SIP can connect via UDP");
				_needsHttpTunnel = false;
				return true;
			}
		}
		////
	}

	LOG_DEBUG("cannot connect via UDP");

	// SIP test with Http
	if (_networkDiscovery.testSIPHTTPTunnel(_httpTunnelServerHostname, 80, false,
		_sipProxyServerHostname, _sipProxyServerPort)) {

		_needsHttpTunnel = true;
		_httpTunnelServerPort = 80;
		_httpTunnelWithSSL = false;

		LOG_DEBUG("SIP can connect via a tunnel on port 80 without SSL");
		return true;
	}

	if (_networkDiscovery.testSIPHTTPTunnel(_httpTunnelServerHostname, 443, false,
		_sipProxyServerHostname, _sipProxyServerPort)) {

		_needsHttpTunnel = true;
		_httpTunnelServerPort = 443;
		_httpTunnelWithSSL = false;

		LOG_DEBUG("SIP can connect via a tunnel on port 443 without SSL");
		return true;
	}

	if (_networkDiscovery.testSIPHTTPTunnel(_httpsTunnelServerHostname, 443, true,
		_sipProxyServerHostname, _sipProxyServerPort)) {

		_needsHttpTunnel = true;
		_httpsTunnelServerPort = 443;
		_httpTunnelWithSSL = true;

		LOG_DEBUG("SIP can connect via a tunnel on port 443 with SSL");
		return true;
	}
	////

	LOG_ERROR("SIP cannot connect");
	return false;
}

void WengoAccount::ssoTimeoutEventHandler() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	setHostname(config.getWengoServerHostname());
	setGet(false);
	setHttps(_ssoWithSSL);
	setServicePath(config.getWengoSSOPath());
	setWengoAuthentication(true);

	LOG_DEBUG("setting proxy settings for SSO request");
	NetworkProxy networkProxy = NetworkProxyDiscovery::getInstance().getNetworkProxy();

	HttpRequest::setProxy(networkProxy.getServer(), networkProxy.getServerPort(),
		networkProxy.getLogin(), networkProxy.getPassword());

	if (_ssoWithSSL) {
		setPort(443);
		LOG_DEBUG("sending SSO request with SSL");
	} else {
		setPort(80);
		LOG_DEBUG("sending SSO request without SSL");
	}

	//VOXOX - JRT - 2009.09.28 - Make passwords more secure.
	if ( usingEncryption() )
	{
		setParameters( "enc=true" );
	}

	call(this);
}

void WengoAccount::ssoLastTimeoutEventHandler() {
	_ssoTimerFinished = true;
}

void WengoAccount::answerReceived(const std::string & answer, int requestId) {
	if (!answer.empty()) {
		LOG_DEBUG("SSO request has been processed successfully");
		_ssoRequestOk = true;
		WengoAccountParser parser(*this, answer);
		_statusCode = parser.getStatusCode();
		_loginErrorMessage = parser.getLoginErrorMessage();//VOXOX CHANGE by Rolando - 2009.09.10 
		if (parser.isLoginPasswordOk()) {
			LOG_DEBUG("login/password Ok");
			_wengoLoginOk = true;
			_ssoTimer.stop();
			_ssoTimerFinished = true;
			//SIP connection test can now be launched
		} else {
			LOG_DEBUG("Problem with user info or server sent a custom error message");//VOXOX CHANGE by Rolando - 2009.09.10 
			_wengoLoginOk = false;
			_ssoTimer.stop();
			_ssoTimerFinished = true;
		}
	}
}

bool WengoAccount::isEmpty() const {
	return (_wengoLogin.empty() || _wengoPassword.empty());
}
