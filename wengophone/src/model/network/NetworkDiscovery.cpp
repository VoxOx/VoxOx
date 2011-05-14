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
#include "NetworkDiscovery.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/network/NetworkProxyDiscovery.h>

#include <thread/Thread.h>
#include <util/Logger.h>
#include <util/StringList.h>

#include <netlib.h>
#include <owsl.h>

using namespace std;

NetworkDiscovery::NetworkDiscovery() {
	if (owsl_initialize()) {
		LOG_FATAL("network transport library (OWSL) initialization failed");
	}
}

NetworkDiscovery::~NetworkDiscovery() {
	owsl_terminate();
}

bool NetworkDiscovery::testHTTP(const std::string & url, bool ssl) {
	NETLIB_BOOLEAN sslActivated = ((ssl) ? NETLIB_TRUE : NETLIB_FALSE);
	bool result = false;
	NetworkProxy networkProxy = NetworkProxyDiscovery::getInstance().getNetworkProxy();

	if (is_http_conn_allowed(url.c_str(),
		networkProxy.getServer().c_str(), networkProxy.getServerPort(),
		networkProxy.getLogin().c_str(), networkProxy.getPassword().c_str(),
		sslActivated, HTTP_TIMEOUT) == HTTP_OK) {

		result = true;
	} else {
		LOG_DEBUG("cannot connect to " + url + (ssl ? " with" : " without") + " SSL");
	}

	return result;
}

bool NetworkDiscovery::testUDP(const string & stunServer) {
	LOG_DEBUG("testing UDP connection and discovering NAT type with STUN server " + stunServer);
	// By getting the NetworkProxy object, we wait for Proxy detection
	// See NetworProxyDisocvery::getNetworkProxy
	NetworkProxy networkProxy = NetworkProxyDiscovery::getInstance().getNetworkProxy();

	NatType natType;
	bool opened = (is_udp_port_opened(stunServer.c_str(), SIP_PORT, &natType) == NETLIB_TRUE ? true : false);

	EnumNatType::NatType nat = EnumNatType::NatTypeUnknown;

	switch(natType) {
	case StunTypeUnknown:
		nat = EnumNatType::NatTypeUnknown;
		break;
	case StunTypeOpen:
		nat = EnumNatType::NatTypeOpen;
		break;
	case StunTypeConeNat:
		nat = EnumNatType::NatTypeFullCone;
		break;
	case StunTypeRestrictedNat:
		nat = EnumNatType::NatTypeRestrictedCone;
		break;
	case StunTypePortRestrictedNat:
		nat = EnumNatType::NatTypePortRestrictedCone;
		break;
	case StunTypeSymNat:
		nat = EnumNatType::NatTypeSymmetric;
		break;
	case StunTypeSymFirewall:
		nat = EnumNatType::NatTypeSymmetricFirewall;
		break;
	case StunTypeBlocked:
		nat = EnumNatType::NatTypeBlocked;
		break;
	case StunTypeFailure:
		nat = EnumNatType::NatTypeFailure;
		break;
	default:
		LOG_FATAL("unknown NAT type=" + String::fromNumber(natType));
	}

	setNatConfig(nat);

	return opened;
}

bool NetworkDiscovery::testSIP(const string & server,const string & login, unsigned short port, unsigned short localPort) {
	LOG_DEBUG("pinging SIP server " + server + " on port " + String::fromNumber(port) 
		+ " from port " + String::fromNumber(localPort));
	return (udp_sip_ping(server.c_str(), port, localPort, 
		PING_TIMEOUT, login.c_str(), server.c_str()) == NETLIB_TRUE ? true : false);
}

bool NetworkDiscovery::testSIPHTTPTunnel(const string & tunnelServer, unsigned tunnelPort, bool ssl,
	const string & sipServer, unsigned sipServerPort) {
	bool result = false;
	NetworkProxy networkProxy = NetworkProxyDiscovery::getInstance().getNetworkProxy();

	LOG_DEBUG("testing SIP tunnel connection");
	if (is_tunnel_conn_allowed(tunnelServer.c_str(), tunnelPort,
		sipServer.c_str(), sipServerPort,
		networkProxy.getServer().c_str(), networkProxy.getServerPort(),
		networkProxy.getLogin().c_str(), networkProxy.getPassword().c_str(),
		(ssl ? NETLIB_TRUE : NETLIB_FALSE), HTTP_TIMEOUT, sipServer.c_str(),
		NETLIB_TRUE, PING_TIMEOUT) == HTTP_OK) {

		result = true;
	} else {
		LOG_DEBUG("cannot create a tunnel to " + tunnelServer + ":" + String::fromNumber(tunnelPort)
			+ " for SIP server " + sipServer + ":" + String::fromNumber(sipServerPort)
			+ (ssl ? " with" : " without") + " SSL");
	}

	return result;
}

unsigned NetworkDiscovery::getFreeLocalPort() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	int localPort = SIP_PORT;

	if (!is_local_udp_port_used(NULL, config.getNetworkSipLocalPort())) {
		LOG_DEBUG("UDP port configured, will use port number : "
			+ String::fromNumber(config.getNetworkSipLocalPort()));
		return config.getNetworkSipLocalPort();
	}
	if (!is_local_udp_port_used(NULL, localPort)) {
		LOG_DEBUG("UDP port 5060 is free");
		return localPort;
	} else if (!is_local_udp_port_used(NULL, localPort + 1)) {
		LOG_DEBUG("UDP port 5060 is busy, will use 5061");
		return localPort + 1;
	} else {
		localPort = get_local_free_udp_port(NULL);
		if (localPort == -1) {
			LOG_DEBUG("cannot get a free local port");
			localPort = 0;
		}
		LOG_DEBUG("UDP port 5061 is busy, will use random port number : " + String::fromNumber(localPort));
		return localPort;
	}
}

void NetworkDiscovery::setNatConfig(EnumNatType::NatType natType) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	std::string tmp = EnumNatType::toString(natType);
	LOG_DEBUG("NAT type=" + tmp);
	config.set(Config::NETWORK_NAT_TYPE_KEY, tmp);
}
