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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "NetworkProxyDiscovery.h"
#include "NetworkObserver.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <util/Logger.h>
#include <thread/ThreadEvent.h>

#include <netlib.h>
#include <owsl.h>

NetworkProxy::NetworkProxy() {
	_proxyAuthType = ProxyAuthTypeUnknown;
}

NetworkProxy::NetworkProxy(const NetworkProxy & networkProxy) {
	_login = networkProxy._login;
	_password = networkProxy._password;
	_server = networkProxy._server;
	_serverPort = networkProxy._serverPort;
	_proxyAuthType = networkProxy._proxyAuthType;
}

NetworkProxyDiscovery * NetworkProxyDiscovery::_networkProxyDiscoveryInstance = NULL;

NetworkProxyDiscovery::NetworkProxyDiscovery() {
	if (owsl_initialize()) {
		LOG_FATAL("network transport library (OWSL) initialization failed");
	}

	_state = NetworkProxyDiscoveryStateUnknown;

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	_networkProxy.setServer(config.getNetworkProxyServer());
	_networkProxy.setServerPort(config.getNetworkProxyPort());
	_networkProxy.setLogin(config.getNetworkProxyLogin());
	_networkProxy.setPassword(config.getNetworkProxyPassword());

	NetworkObserver::getInstance().connectionIsUpEvent +=
		boost::bind(&NetworkProxyDiscovery::connectionIsUpEventHandler, this, _1);
	NetworkObserver::getInstance().connectionIsDownEvent +=
		boost::bind(&NetworkProxyDiscovery::connectionIsDownEventHandler, this, _1);

	if (!NetworkObserver::getInstance().isConnected()) {
		LOG_DEBUG("no connection available");
		_state = NetworkProxyDiscoveryStateUnknown;
		return;
	}

	int ret = 0;
	if (config.getNetworkProxyDetected()
		&& (ret = is_http_conn_allowed("www.google.com:80", _networkProxy.getServer().c_str(),
			_networkProxy.getServerPort(), _networkProxy.getLogin().c_str(),
			_networkProxy.getPassword().c_str(), NETLIB_FALSE, 10)) == HTTP_OK) {

		// Protect against crash due to IE Digest authentication management bug
		EnumAuthType proxyAuthType = get_proxy_auth_type(_networkProxy.getServer().c_str(),
			_networkProxy.getServerPort(), PROXY_TIMEOUT);
		if (proxyAuthType == proxyAuthDigest) {
			_networkProxy.setProxyAuthType(NetworkProxy::ProxyAuthTypeDigest);
		}
		_state = NetworkProxyDiscoveryStateDiscovered;
	} else if (ret == HTTP_AUTH) {
		_state = NetworkProxyDiscoveryStateNeedsAuthentication;
		if (_networkProxy.getLogin().empty()) {
			proxyNeedsAuthenticationEvent(*this, _networkProxy);
		}
		else {
			wrongProxyAuthenticationEvent(*this, _networkProxy);
		}
	} else {
		discoverProxy();
	}
}

NetworkProxyDiscovery::~NetworkProxyDiscovery() {
	owsl_terminate();
	terminate();
}

NetworkProxyDiscovery & NetworkProxyDiscovery::getInstance() {
	if (!_networkProxyDiscoveryInstance) {
		_networkProxyDiscoveryInstance = new NetworkProxyDiscovery();
	}

	return *_networkProxyDiscoveryInstance;
}

void NetworkProxyDiscovery::discoverProxy() {
	start();
}

void NetworkProxyDiscovery::run() {
	Mutex::ScopedLock lock(_mutex);

	if (!NetworkObserver::getInstance().isConnected()) {
		LOG_DEBUG("no connection available");
		_state = NetworkProxyDiscoveryStateUnknown;
		return;
	}

	// Trying to ping an http server. We do this because sometimes 
	// a system can be set to use a proxy whereas it could connect directly.
	if (is_http_conn_allowed("www.google.com",
		NULL, 0, NULL, NULL,
		NETLIB_FALSE, 10) == HTTP_OK) {

		LOG_DEBUG("can connect without proxy");

		_networkProxy.setServer(String::null);
		_networkProxy.setServerPort(0);
		_networkProxy.setLogin(String::null);
		_networkProxy.setPassword(String::null);

		saveProxySettings();
		_state = NetworkProxyDiscoveryStateDiscovered;
		_condition.notify_all();
		return;
	}

	//See below for explaination about this test
	if (_state != NetworkProxyDiscoveryStateNeedsAuthentication) {
		LOG_DEBUG("discovering network proxy...");
		_state = NetworkProxyDiscoveryStateDiscovering;

		LOG_DEBUG("searching for proxy...");

		char * localProxyUrl = get_local_http_proxy_address();
		int localProxyPort = get_local_http_proxy_port();

		_networkProxy.setServer(localProxyUrl ? String(localProxyUrl) : String(String::null));
		_networkProxy.setServerPort(localProxyPort);
		_networkProxy.setLogin(String::null);
		_networkProxy.setPassword(String::null);

		if (!localProxyUrl) {
			LOG_DEBUG("no proxy found");
			saveProxySettings();
			_state = NetworkProxyDiscoveryStateDiscovered;
			_condition.notify_all();
			return;
		}
	}

	LOG_DEBUG("proxy found");

	if (is_proxy_auth_needed(_networkProxy.getServer().c_str(),
		_networkProxy.getServerPort(), PROXY_TIMEOUT)) {
		LOG_DEBUG("proxy authentication needed");

		EnumAuthType proxyAuthType = get_proxy_auth_type(_networkProxy.getServer().c_str(),
				_networkProxy.getServerPort(), PROXY_TIMEOUT);
		if (proxyAuthType == proxyAuthDigest) {
			_networkProxy.setProxyAuthType(NetworkProxy::ProxyAuthTypeDigest);
		}

		if (_networkProxy.getLogin().empty()) {
			LOG_DEBUG("proxy needs login/password");

			_state = NetworkProxyDiscoveryStateNeedsAuthentication;
			proxyNeedsAuthenticationEvent(*this, _networkProxy);
			return;
		}

		if (!is_proxy_auth_ok(_networkProxy.getServer().c_str(), _networkProxy.getServerPort(),
			_networkProxy.getLogin().c_str(), _networkProxy.getPassword().c_str(), PROXY_TIMEOUT)) {

			LOG_DEBUG("proxy needs valid login/password");

			_state = NetworkProxyDiscoveryStateNeedsAuthentication;
			wrongProxyAuthenticationEvent(*this, _networkProxy);
			return;
		}
	}

	saveProxySettings();
	_state = NetworkProxyDiscoveryStateDiscovered;
	_condition.notify_all();
}

void NetworkProxyDiscovery::saveProxySettings() {
 	Config & config = ConfigManager::getInstance().getCurrentConfig();

	config.set(Config::NETWORK_PROXY_DETECTED_KEY, true);
	config.set(Config::NETWORK_PROXY_SERVER_KEY, _networkProxy.getServer());
	config.set(Config::NETWORK_PROXY_PORT_KEY, _networkProxy.getServerPort());
	config.set(Config::NETWORK_PROXY_LOGIN_KEY, _networkProxy.getLogin());
	config.set(Config::NETWORK_PROXY_PASSWORD_KEY, _networkProxy.getPassword());
}

NetworkProxyDiscovery::NetworkProxyDiscoveryState NetworkProxyDiscovery::getState() const {
	return _state;
}

void NetworkProxyDiscovery::setProxySettings(NetworkProxy networkProxy) {
	Mutex::ScopedLock lock(_mutex);

	_networkProxy = networkProxy;
	discoverProxy();
}

NetworkProxy NetworkProxyDiscovery::getNetworkProxy() const {
	Mutex::ScopedLock lock(_mutex);

	if (_state == NetworkProxyDiscoveryStateDiscovering) {
		//Waiting for the end of the discovery
		_condition.wait(lock);
	}

	return _networkProxy;
}

void NetworkProxyDiscovery::connectionIsUpEventHandler(NetworkObserver & sender) {
	Mutex::ScopedLock lock(_mutex);

	if (_state != NetworkProxyDiscoveryStateDiscovered) {
		discoverProxy();
	}
}

void NetworkProxyDiscovery::connectionIsDownEventHandler(NetworkObserver & sender) {
	terminate();
}
