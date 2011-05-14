/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef NETLIB_H
#define NETLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define NETLIB_BOOLEAN int
#define NETLIB_TRUE 1
#define NETLIB_FALSE 0

/**
 * @file netlib.h
 *
 * Detects the network configuration of a computer.
 *
 * @author Julien Bossart
 */

#define HTTP_PORT 80
#define HTTPS_PORT 443
#define SIP_PORT 5060

/**
 * log type.
 */
typedef enum {
	NETLIB_LOG_UNK = 0,
	NETLIB_LOG_IN,
	NETLIB_LOG_OUT,
	NETLIB_LOG_INFO,
	NETLIB_LOG_ERR
} NETLIB_LOG_TYPE;

typedef void (*NetlibLogCbk)(NETLIB_LOG_TYPE type, const char * buffer);

void setLogCbk( NetlibLogCbk );		//VOXOX - JRT - 2009.12.23 
/**
 * NAT type.
 */
typedef enum  {
	StunTypeUnknown=0,
	StunTypeOpen,
	StunTypeConeNat,
	StunTypeRestrictedNat,
	StunTypePortRestrictedNat,
	StunTypeSymNat,
	StunTypeSymFirewall,
	StunTypeBlocked,
	StunTypeFailure
} NatType;

/**
 * Gets the NAT type of the computer.
 *
 * @param stun_server STUN server IP address used to detect the NAT type
 * @return NAT type or -1 if the STUN server is incorrect
 */
NatType get_nat_type(const char * stun_server);

/**
 * Checks if a specific UDP port is opened.
 *
 * @param stun_server STUN server IP address
 * @param port port number to test
 * @param ntype filled with proxy type
 * @return true if the port is opened; false otherwise
 */
NETLIB_BOOLEAN is_udp_port_opened(const char *stun_server, int port, NatType *ntype);

/**
 * Tries to send a SIP request and wait for a response.
 *
 * @param sip_server Sip server address we try to SIPping
 * @param sip_port Sip server port we try to SIPping
 * @param local_port local port to use
 * @param ping_timeout in seconds
 * @param sip_login login used to create sip packet
 * @param sip_realm realm used to create sip packet
 * @return true if a response is received; false otherwise
 */
NETLIB_BOOLEAN udp_sip_ping(const char *sip_server, unsigned short sip_port, unsigned short local_port, int ping_timeout, const char *sip_login, const char *sip_realm);

/**
 * Checks if a specific UDP local port is used.
 *
 * @param itf IP address of an interface. If NULL, default address is used.
 * @param port port number to test
 * @return true if the port is used; false otherwise
 */
NETLIB_BOOLEAN is_local_udp_port_used(const char *itf, int port);

/**
 * Gets a random unused local port.
 *
 * @param itf IP address of an interface. If NULL, default address is used.
 * @return unused local port or -1 if an error occurred
 */
int get_local_free_udp_port(const char *itf);


/**
 * Gets the local HTTP proxy IP address.
 *
 * @return local HTTP proxy address or NULL
 */
char *get_local_http_proxy_address();


/**
 * Gets the local HTTP proxy port number.
 *
 * @return local HTTP proxy port or 0
 */
int get_local_http_proxy_port();


/**
 * Checks if an authentication with proxy is needed.
 *
 * @param proxy_addr IP address of the proxy we want to connect through
 * @param proxy_port port of the proxy we want to connect through
 * @param timeout in seconds
 * @return true if an authentication is needed; false otherwise
 */
NETLIB_BOOLEAN is_proxy_auth_needed(const char *proxy_addr, int proxy_port, int timeout);


typedef enum {
	proxyAuthUnknown,
	proxyAuthBasic,
	proxyAuthDigest,
	proxyAuthNTLM
}	EnumAuthType;

/**
 * Gets the proxy authentication type.
 *
 * @param proxy_addr IP address of the proxy we want to connect through
 * @param proxy_port port of the proxy we want to connect through
 * @param timeout in seconds
 * @return proxy authentication type
 */
EnumAuthType get_proxy_auth_type(const char *proxy_addr, int proxy_port, int timeout);


/**
 * Tries to authenticate with a proxy.
 *
 * @param proxy_addr IP address of the proxy we want to connect through
 * @param proxy_port port of the proxy we want to connect through
 * @param proxy_login proxy authentication login
 * @param proxy_passwd proxy authentication password
 * @param timeout in seconds
 * @return true if the authentication succeeds; false otherwise
 */
NETLIB_BOOLEAN is_proxy_auth_ok(const char *proxy_addr, int proxy_port,
								const char *proxy_login, const char *proxy_passwd,
								int timeout);


/**
 * Tries to find a url in the proxyless exceptions list.
 *
 * @param url url we want to check
 * @return true if url is in exception list; false otherwise
 */
NETLIB_BOOLEAN is_url_proxyless_exception(const char *url);


/**
 * is_http_conn_allowed() or is_tunnel_conn_allowed() return.
 */
typedef enum
{
	HTTP_OK = 0,
	HTTP_NOK,
	HTTP_AUTH
}	HttpRet;

/**
 * Checks if a http request can be sent.
 *
 * @param url url we try to request
 * @param proxy_addr proxy address we want to connect through
 * @param proxy_port proxy port we want to connect through
 * @param proxy_login if proxy authentication needed
 * @param proxy_passwd if proxy authentication needed
 * @param ssl use ssl or not
 * @param timeout in seconds
 * @return State of HTTP response
 */
HttpRet is_http_conn_allowed(const char *url,
							const char *proxy_addr, int proxy_port,
							const char *proxy_login, const char *proxy_passwd,
							NETLIB_BOOLEAN ssl, int timeout);

/**
 * Checks if a http tunnel can be created.
 *
 * @param http_gate_addr http gateway address
 * @param http_gate_port http gateway port
 * @param sip_addr sip server address we want to join
 * @param sip_port sip server port we want to join
 * @param proxy_addr proxy address we want to connect through
 * @param proxy_port proxy port we want to connect through
 * @param proxy_login if proxy authentication needed
 * @param proxy_passwd if proxy authentication needed
 * @param ssl use ssl or not
 * @param timeout in seconds
 * @param sip_realm realm used to create SIP packet
 * @param sip_ping try to send a SIPping
 * @param ping_timeout in milliseconds
 * @return State of HTTP response or SIPping result if it's enabled
 */
HttpRet is_tunnel_conn_allowed(const char *http_gate_addr, int http_gate_port,
								const char *sip_addr, int sip_port,
								const char *proxy_addr, int proxy_port,
								const char *proxy_login, const char *proxy_passwd,
								NETLIB_BOOLEAN ssl, int timeout, const char *sip_realm,
								NETLIB_BOOLEAN sip_ping, int ping_timeout);

/**
 * Check the connection state.
 *
 * @return true if a connection is available otherwise false
 */
NETLIB_BOOLEAN is_connection_available();

#ifdef OS_WIN32
const char* getRandomFileName();			//VOXOX - JRT - 2009.09.30 - Improve SSL initialization.
#endif
	
#ifdef __cplusplus
}
#endif

#endif	//NETLIB_H
