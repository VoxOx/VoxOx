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

#ifndef OWNETWORKDISCOVERY_H
#define OWNETWORKDISCOVERY_H

#include <sipwrapper/EnumNatType.h>

#include <util/Event.h>

#include <string>

/**
 * Discover Network configuration.
 *
 * @ingroup model
 * @author Philippe Bernery
 */
class NetworkDiscovery {
public:

	NetworkDiscovery();

	~NetworkDiscovery();

	/**
	 * Tests if an url is joinable by Http.
	 *
	 * @param the url to test (e.g: "ws.wengo.fr:443/softphone-sso/sso.php")
	 * @param true if a SSL connection must be tested
	 * @return true if connection ok
	 */
	bool testHTTP(const std::string & url, bool ssl);

	/**
	 * Tests if a UDP connection is possible.
	 *
	 * @param stunServer the STUN server
	 * @return true if ok
	 */
	bool testUDP(const std::string & stunServer);

	/**
	 * Tests if a server respond to a SIP ping.
	 *
	 * @param server the server to test
	 * @param port the server port
	 * @param localPort the local port to use
	 * @return true if ok
	 */
	bool testSIP(const std::string & server,const std::string & login, unsigned short port, unsigned short localPort);

	/**
	 * Tests if a HttpTunnel can be create and if SIP can pass through this tunnel.
	 *
	 * @param tunnelServer the tunnel server
	 * @param tunnelPort the tunnel port
 	 * @param ssl true if tunnel must be done with SSL
	 * @param sipServer the SIP server to ping
	 * @param sipServerPort the SIP server port
	 * @return true if ok
	 */
	bool testSIPHTTPTunnel(const std::string & tunnelServer, unsigned tunnelPort, bool ssl,
		const std::string & sipServer, unsigned sipServerPort);

	/**
	 * @return a free local port.
	 */
	unsigned getFreeLocalPort();

	/**
	 * Sets the detected Nat Type.
	 */
	void setNatConfig(EnumNatType::NatType natType);

private:

	static const unsigned PING_TIMEOUT = 3;

	static const unsigned HTTP_TIMEOUT = 10;
};

#endif	//OWNETWORKDISCOVERY_H
