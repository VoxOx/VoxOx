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

#ifndef OWNETWORKPROXYDISCOVERY_H
#define OWNETWORKPROXYDISCOVERY_H

#include <thread/Condition.h>
#include <thread/Mutex.h>
#include <thread/Thread.h>

#include <util/Event.h>

#include <string>

class NetworkObserver;

/**
 * Network proxy data container.
 */
class NetworkProxy {
public:

	NetworkProxy();

	NetworkProxy(const NetworkProxy & networkProxy);

	std::string getLogin() const { return _login; }
	void setLogin(const std::string & login) { _login = login; }

	std::string getPassword() const { return _password; }
	void setPassword(const std::string & password) { _password = password; }

	std::string getServer() const { return _server; }
	void setServer(const std::string & server) { _server = server; }

	int getServerPort() const { return _serverPort; }
	void setServerPort(int port) { _serverPort = port; }

	enum ProxyAuthType {
		ProxyAuthTypeUnknown,
		ProxyAuthTypeBasic,
		ProxyAuthTypeDigest,
		ProxyAuthTypeNTLM
	};

	ProxyAuthType getProxyAuthType() const { return _proxyAuthType; }
	void setProxyAuthType(ProxyAuthType proxyAuthType) { _proxyAuthType = proxyAuthType; }

private:

	std::string _login;

	std::string _password;

	std::string _server;

	int _serverPort;

	ProxyAuthType _proxyAuthType;
};

/**
 * Network proxy discovery.
 *
 * TODO: add a proxyHasChangedEvent to notify registered classes that the proxy settings have changed.
 *
 * @author Julien Bossart
 * @author Philippe Bernery
 */
class NetworkProxyDiscovery : public Thread {
public:

	enum NetworkProxyDiscoveryState {
		/** Initial state. */
		NetworkProxyDiscoveryStateUnknown,

		/** NetworkProxyDiscovery is discovering the proxy settings. */
		NetworkProxyDiscoveryStateDiscovering,

		/** NetworkProxyDiscovery needs authentication parameters to continue discovery. */
		NetworkProxyDiscoveryStateNeedsAuthentication,

		/** NetworkProxyDiscovery has discovered the proxy settings. */
		NetworkProxyDiscoveryStateDiscovered
	};

	/**
	 * Emitted when a proxy has been detected and needs a login/password.
	 *
	 * @param sender this class
	 * @param networkProxy the current result of the discovery
	 */
	Event< void (NetworkProxyDiscovery & sender, NetworkProxy networkProxy) > proxyNeedsAuthenticationEvent;

	/**
	 * Emitted when given login/password are wrong.
	 *
	 * @param sender this class
	 * @param networkProxy the current result of the discovery
	 */
	Event< void (NetworkProxyDiscovery & sender, NetworkProxy networkProxy)> wrongProxyAuthenticationEvent;

	static NetworkProxyDiscovery & getInstance();

	NetworkProxyDiscoveryState getState() const;

	/**
	 * Sets the NetworProxy settings.
	 *
	 * Will try to detect the Proxy with the given parameters.
	 * It it works, parameters will be saved in Config.
	 */
	void setProxySettings(NetworkProxy networkProxy);

	/**
	 * Returns proxy parameters. This method is blocking while proxy settings have not been detected.
	 */
	NetworkProxy getNetworkProxy() const;

private:

	NetworkProxyDiscovery();

	~NetworkProxyDiscovery();

	/**
	 * Sets the proxy settings in Config.
	 */
	void saveProxySettings();

	void discoverProxy();

	virtual void run();

	/**
	 * @see NetworkObserver::connectionIsUpEvent
	 */
	void connectionIsUpEventHandler(NetworkObserver & sender);

	/**
	 * @see NetworkObserver::connectionIsDownEvent
	 */
	void connectionIsDownEventHandler(NetworkObserver & sender);

	static NetworkProxyDiscovery * _networkProxyDiscoveryInstance;

	NetworkProxyDiscoveryState _state;

	NetworkProxy _networkProxy;

	mutable Mutex _mutex;

	mutable Condition _condition;

	static const unsigned PROXY_TIMEOUT = 10;
};

#endif	//OWNETWORKPROXYDISCOVERY_H
