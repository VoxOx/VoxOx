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

#include <owlibc.h>
#include <owsl.h>

#ifdef OS_WIN32
	#include <wininet.h>
	#ifndef CC_MINGW
		#include <urlmon.h>
		#ifndef CC_MSVC8
			#include "bugfix.h"
		#endif
	#endif // CC_MINGW
#else
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <errno.h>
	#include <net/if.h>
	#include <sys/ioctl.h>
	#include <netdb.h>
	#include <strings.h>
#endif //OS_WIN32
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "udp_func.h"
#include "stun_func.h"

#include <curl/curl.h>
#include "netlib.h"
#include "netlib_util.h"

#ifdef OS_WIN32
	#include <openssl/rand.h>	//VOXOX - JRT - 2009.09.30 - Improve OpenSSL initialization.
#endif

/* TODO: replace by owsl_reuse_set() */
/**
 * Set the SO_REUSEADDR option on sock, to avoid "Address already in use"
 * errors.
 */
static int allow_address_reuse(Socket sock) {
	int tmp = 1;
#ifdef OS_WIN32
	// On Win32, prototype is setsockopt(SOCKET, int, int, const char*, int)
	return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&tmp, sizeof(tmp));
#else
	int optval;

	// SO_REUSEPORT is only defined on BSD
	// See Qt implementation: qnativesocketengine_unix.cpp
#ifdef SO_REUSEPORT
	optval = SO_REUSEPORT;
#else
	optval = SO_REUSEADDR;
#endif
	// On Unix, prototype is setsockopt(int, int, int, const void*, socklen_t)
	return setsockopt(sock, SOL_SOCKET, optval, (void*)&tmp, sizeof(tmp));
#endif
}

typedef struct	HttpProxy_s
{
	int			port;
	char		*address;
	long		auth_type;
	long		proxy_auth_type;
	char		**proxy_exceptions;
}				HttpProxy_t;

HttpProxy_t	_LocalProxy = {0, NULL, 0, 0, NULL};


static NetlibLogCbk writeLogCbk = 0;

void setLogCbk( NetlibLogCbk cb )		//VOXOX - JRT - 2009.12.23 
{
	writeLogCbk = cb;
}

#define NETLIB_LOG_ERROR(buff) {if (writeLogCbk) writeLogCbk(NETLIB_LOG_ERR, buff); }

int _curloutputcbk(CURL *mycurl, curl_infotype type, char *buffer, size_t size, void * arg)
{
	if (writeLogCbk) {
		NETLIB_LOG_TYPE netlib_type = NETLIB_LOG_UNK;

		switch (type) {
			case CURLINFO_HEADER_IN:
				netlib_type = NETLIB_LOG_IN;
				break;

			case CURLINFO_HEADER_OUT:
				netlib_type = NETLIB_LOG_OUT;
				break;

			case CURLINFO_TEXT:
				netlib_type = NETLIB_LOG_INFO;
				break;

			default:
				return 0;
		}

		writeLogCbk(netlib_type, buffer);
	}

	return 0;
}

/**
 * Empty callback to silence libcurl
 * follows curl_write_callback prototype
 */
static size_t _curlsilentwritecbk(char* /*ptr*/, size_t size, size_t nitems, void* /*outstream*/) {
	return size * nitems;
}

char *_cleanStr(char *str)
{
	int i;

	if (str && *str) {
		for (i = 0; str[i]; i++) {
			if (str[i] == '=') {
				return (&str[i+1]);
			}
		}

		return str;
	}

	return 0;
}

int _parseProxyUrl(char *url)
{
	char * tmp;

	for (tmp = _cleanStr(url), url = tmp; *tmp && *tmp != ':'; tmp++);

	if (tmp && *tmp) {
		_LocalProxy.address = (char *) malloc(tmp - url + 1);
		memcpy(_LocalProxy.address, url, tmp - url);
		_LocalProxy.address[tmp - url] = 0;

		for (url = ++tmp; *tmp && *tmp >= '0' && * tmp <= '9'; tmp++);
		*tmp = 0;
		_LocalProxy.port = atoi(url);
		return 0;
	}

	return -1;
}

int _getProxyAddress()
{
#if defined OS_WIN32 && defined CC_MSVC
	long ret;
	HKEY result;
	char url[1024];
	DWORD size = 1024;
	char TempPath[MAX_PATH];
	char TempFile[MAX_PATH];
	HMODULE hModJS;
	DWORD enable;
	DWORD enablesize = sizeof(DWORD);

	/* MSDN EXAMPLE */

	char url1[1025] = "http://www.google.fr/about.html";
	char host[256] = "http://www.google.fr";
	char proxyBuffer[1024];
	char * proxy = proxyBuffer;
	ZeroMemory(proxy, 1024);
	DWORD dwProxyHostNameLength = 1024;
	DWORD returnVal;
	// Declare function pointers for the three autoproxy functions
	pfnInternetInitializeAutoProxyDll pInternetInitializeAutoProxyDll;
	pfnInternetDeInitializeAutoProxyDll pInternetDeInitializeAutoProxyDll;
	pfnInternetGetProxyInfo pInternetGetProxyInfo;

	/* ************ */

	ret = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_QUERY_VALUE, & result);
	ret = RegQueryValueExA(result, "ProxyEnable", 0, 0, (LPBYTE) & enable, & enablesize);
	ret = RegQueryValueExA(result, "ProxyServer", 0, 0, (LPBYTE)url, & size);

	if (ret == ERROR_SUCCESS && enable == 1) {
		if (_parseProxyUrl(url) == 0) {
			RegCloseKey(result);
			return 0;
		}
	}

	ZeroMemory(url, 1024);
	size = 1024;
	ret = RegQueryValueExA(result, "AutoConfigURL", 0, 0, (LPBYTE)url, & size);
	RegCloseKey(result);

	if (ret != ERROR_SUCCESS) {
		if (DetectAutoProxyUrl(url, size, PROXY_AUTO_DETECT_TYPE_DHCP | PROXY_AUTO_DETECT_TYPE_DNS_A) == false) {
			NETLIB_LOG_ERROR(make_error_string("_getProxyAddress : DetectAutoProxyUrl"));
			return -1;
		}
	}

	GetTempPathA(sizeof(TempPath), TempPath);
	GetTempFileNameA(TempPath, NULL, 0, TempFile);
	if (URLDownloadToFileA(NULL, url, TempFile, NULL, NULL) != S_OK) {
		NETLIB_LOG_ERROR(make_error_string("_getProxyAddress : URLDownloadToFileA"));
		return -1;
	}

	if (!(hModJS = LoadLibraryA("jsproxy.dll"))) {
		NETLIB_LOG_ERROR(make_error_string("_getProxyAddress : LoadLibrary"));
		return -1;
	}

	if (!(pInternetInitializeAutoProxyDll = (pfnInternetInitializeAutoProxyDll)
		GetProcAddress(hModJS, "InternetInitializeAutoProxyDll")) ||
		!(pInternetDeInitializeAutoProxyDll = (pfnInternetDeInitializeAutoProxyDll)
		GetProcAddress(hModJS, "InternetDeInitializeAutoProxyDll")) ||
		!(pInternetGetProxyInfo = (pfnInternetGetProxyInfo)
		GetProcAddress(hModJS, "InternetGetProxyInfo"))) {
		NETLIB_LOG_ERROR(make_error_string("_getProxyAddress : GetProcAddress"));
		return -1;
	}

	if (!(returnVal = pInternetInitializeAutoProxyDll(0, TempFile, NULL, 0, NULL))) {
		NETLIB_LOG_ERROR(make_error_string("_getProxyAddress : pInternetInitializeAutoProxyDll"));
		return -1;
	}

	DeleteFileA(TempFile);

	if (!pInternetGetProxyInfo((LPSTR)url1, sizeof(url1),
								(LPSTR)host, sizeof(host),
								&proxy, &dwProxyHostNameLength)) {
		NETLIB_LOG_ERROR(make_error_string("_getProxyAddress : pInternetGetProxyInfo"));
		return -1;
	}

	if (strncmp("PROXY ", proxy, 6) == 0) {
		if (_parseProxyUrl(proxy + 6) != 0) {
			NETLIB_LOG_ERROR(make_error_string("_getProxyAddress : _parseProxyUrl"));
			return -1;
		}
	}
	else {
		NETLIB_LOG_ERROR("_getProxyAddress : strncmp : PROXY doesn't match\n");
		return -1;
	}

	if (!pInternetDeInitializeAutoProxyDll(NULL, 0)) {
		NETLIB_LOG_ERROR(make_error_string("_getProxyAddress : pInternetDeInitializeAutoProxyDll"));
		return -1;
	}
#endif

	return 0;
}

EnumAuthType get_proxy_auth_type(const char *proxy_addr, int proxy_port, int timeout)
{
	EnumAuthType authType = proxyAuthUnknown;

	is_proxy_auth_needed(proxy_addr, proxy_port, timeout);

	if ((_LocalProxy.proxy_auth_type & CURLAUTH_BASIC) == CURLAUTH_BASIC) {
		authType = proxyAuthBasic;
	}
	else if ((_LocalProxy.proxy_auth_type & CURLAUTH_DIGEST) == CURLAUTH_DIGEST) {
		authType = proxyAuthDigest;
	}
	else if ((_LocalProxy.proxy_auth_type & CURLAUTH_NTLM) == CURLAUTH_NTLM) {
		authType = proxyAuthNTLM;
	}

	return authType;
}

void _get_proxy_auth_type2(const char *url, const char *proxy_addr, int proxy_port, int timeout)
{
	CURL *curl_tmp;
	char url_buf[1024];
	char proxy_buf[1024];
	int ret;

	ret = 0;
	curl_tmp = curl_easy_init();

	curl_easy_setopt(curl_tmp, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl_tmp, CURLOPT_WRITEFUNCTION, _curlsilentwritecbk);
	curl_easy_setopt(curl_tmp, CURLOPT_DEBUGFUNCTION, _curloutputcbk);

	snprintf(url_buf, sizeof(url_buf), "http://%s", url);
	curl_easy_setopt(curl_tmp, CURLOPT_URL, url_buf);

	snprintf(proxy_buf, sizeof(proxy_buf), "%s:%d", proxy_addr, proxy_port);
	curl_easy_setopt(curl_tmp, CURLOPT_PROXY, proxy_buf);

	if (timeout > 0) {
		curl_easy_setopt(curl_tmp, CURLOPT_TIMEOUT, timeout);
	}

	curl_easy_setopt(curl_tmp, CURLOPT_HTTPPROXYTUNNEL, 1);
	ret = curl_easy_perform(curl_tmp);

	curl_easy_getinfo(curl_tmp, CURLINFO_PROXYAUTH_AVAIL, &(_LocalProxy.proxy_auth_type));

	curl_easy_cleanup(curl_tmp);
}

void _get_auth_type(const char *url, int timeout)
{
	CURL *curl_tmp;
	char url_buf[1024];
	int ret;

	ret = 0;
	curl_tmp = curl_easy_init();

	curl_easy_setopt(curl_tmp, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl_tmp, CURLOPT_WRITEFUNCTION, _curlsilentwritecbk);
	curl_easy_setopt(curl_tmp, CURLOPT_DEBUGFUNCTION, _curloutputcbk);

	snprintf(url_buf, sizeof(url_buf), "http://%s", url);
	curl_easy_setopt(curl_tmp, CURLOPT_URL, url_buf);

	if (timeout > 0) {
		curl_easy_setopt(curl_tmp, CURLOPT_TIMEOUT, timeout);
	}

	ret = curl_easy_perform(curl_tmp);

	curl_easy_getinfo(curl_tmp, CURLINFO_HTTPAUTH_AVAIL, &(_LocalProxy.auth_type));

	curl_easy_cleanup(curl_tmp);
}

char **internet_explorer_proxyless_exception_list()
{
#ifdef OS_WIN32
	char **list;
	long ret;
	HKEY result;
	char buff[1024];
	DWORD buff_size = sizeof(buff);

	ret = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_QUERY_VALUE, &result);
	ret = RegQueryValueExA(result, "ProxyOverride", 0, 0, (LPBYTE)buff, &buff_size);
	RegCloseKey(result);

	list = my_split(buff, ';');
	return list;
#else
	return 0;
#endif
}

NETLIB_BOOLEAN is_url_proxyless_exception(const char *url)
{
	int i;

	if (!url) {
		return NETLIB_FALSE;
	}

	if (!_LocalProxy.proxy_exceptions) {
		_LocalProxy.proxy_exceptions = internet_explorer_proxyless_exception_list();
	}

	if (!_LocalProxy.proxy_exceptions) {
		return NETLIB_FALSE;
	}

	for (i = 0; _LocalProxy.proxy_exceptions[i]; i++) {
		if (domain_url_cmp((char *)url, _LocalProxy.proxy_exceptions[i]) == false) {
			return NETLIB_TRUE;
		}
	}

	return NETLIB_FALSE;
}

NETLIB_BOOLEAN is_udp_port_opened(const char *stun_server, int port, NatType *nType)
{
	*nType = get_nat_type(stun_server);
	return (*nType > StunTypeUnknown && *nType < StunTypeBlocked ?
		NETLIB_TRUE : NETLIB_FALSE);
}

NETLIB_BOOLEAN is_local_udp_port_used(const char *itf, int port)
{
	struct sockaddr_in  raddr;
	Socket localsock;

	if (!itf) {
		raddr.sin_addr.s_addr = htons(INADDR_ANY);
	} else {
		raddr.sin_addr.s_addr = inet_addr(itf);
	}
	raddr.sin_port = htons((short)port);
	raddr.sin_family = AF_INET;

	if ((localsock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
		NETLIB_LOG_ERROR(make_error_string("is_local_udp_port_used : socket"));
		return NETLIB_TRUE;
	}
	if (allow_address_reuse(localsock) < 0) {
		NETLIB_LOG_ERROR(make_error_string("is_local_udp_port_used : setsockopt"));
		closesocket(localsock);
		return NETLIB_TRUE;
	}

	if (bind(localsock, (struct sockaddr *)&raddr, sizeof (raddr)) < 0) {
		NETLIB_LOG_ERROR(make_error_string("is_local_udp_port_used : bind"));
		closesocket(localsock);
		return NETLIB_TRUE;
	}

	closesocket(localsock);
	return NETLIB_FALSE;
}

int get_local_free_udp_port(const char *itf)
{
	struct sockaddr_in raddr;
	struct sockaddr_in name;
	int name_size = sizeof (struct sockaddr_in);
	Socket localsock;

	if (!itf) {
		raddr.sin_addr.s_addr = htons(INADDR_ANY);
	} else {
		raddr.sin_addr.s_addr = inet_addr(itf);
	}
	raddr.sin_port = htons(0);
	raddr.sin_family = AF_INET;

	if ((localsock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
		NETLIB_LOG_ERROR(make_error_string("get_local_free_udp_port : socket"));
		return -1;
	}

	if (allow_address_reuse(localsock) < 0) {
		NETLIB_LOG_ERROR(make_error_string("is_local_udp_port_used : setsockopt"));
		closesocket(localsock);
		return -1;
	}

	if (bind(localsock, (struct sockaddr *)&raddr, sizeof (raddr)) < 0) {
		NETLIB_LOG_ERROR(make_error_string("get_local_free_udp_port : bind"));
		closesocket(localsock);
		return -1;
	}

	if (getsockname(localsock, (struct sockaddr *) &name, (socklen_t *)&name_size) < 0) {
		NETLIB_LOG_ERROR(make_error_string("get_local_free_udp_port : getsockname"));
		closesocket(localsock);
		return -1;
	}

	closesocket(localsock);
	return ntohs(name.sin_port);
}

static char *
ping_request_new
(
	const char * sip_login,
	const char * sip_realm,
	OWSLSocket socket
)
{
	int return_code ;
	char * request ;
	struct sockaddr * bound_address ;
	char bound_ip [OWSL_IP_MAX_SIZE] ;
	unsigned short bound_port ;
	int i ;
	int int_max_decimal_digits ;

	bound_address = owsl_bound_address_get (socket) ;
	if (bound_address == NULL)
	{
		return NULL ;
	}

	return_code = owsl_address_parse (bound_address, NULL, bound_ip, sizeof (bound_ip), & bound_port) ;
	if (return_code != 0)
	{
		return NULL ;
	}

	int_max_decimal_digits = 0 ;
	i = INT_MAX ;
	while (i > 0)
	{
		int_max_decimal_digits ++ ;
		i /= 10 ;
	}

	request = (char *) malloc
	(
		210 + /* static text */
		strlen (sip_login) * 3 +
		strlen (sip_realm) * 4 +
		strlen (bound_ip) * 2 +
		5 /* bound_port */ +
		5 /* rand() */ +
		int_max_decimal_digits * 4 +
		1 /* '\0' */
	) ;

	srand ((unsigned int) time (NULL)) ;
	sprintf
	(
		request,
		"REGISTER sip:%s SIP/2.0\r\n"
		"Via: SIP/2.0/UDP %s:%d;rport;branch=%d\r\n"
		"Route: <sip:%s;lr>\r\n"
		"From: nobody <sip:%s@%s>;tag=%d\r\n"
		"To: <sip:%s@%s>\r\n"
		"Call-ID: %d\r\n"
		"Contact: <sip:%s@%s>\r\n"
		"CSeq: %d REGISTER\r\n"
		"X-Wengo-Ping: network test\r\n"
		"Content-Length: 0\r\n\r\n",
		sip_realm,       /* request uri */
		bound_ip,        /* via ip */
		bound_port,      /* via port */
		(int) rand (),   /* via branch */
		sip_realm,       /* route uri */
		sip_login,       /* from uri user */
		sip_realm,       /* from uri domain */
		(int) rand (),   /* from tag */
		sip_login,       /* to uri user */
		sip_realm,       /* to uri domain */
		(int) rand (),   /* call id */
		sip_login,       /* contact uri user */
		bound_ip,        /* contact uri domain */
		(int) rand ()    /* cseq number */
	) ;

	return request ;
}

static int
sip_ping
(
	OWSLSocket socket,
	int ping_timeout,
	const char * sip_login,
	const char * sip_realm
)
{
	int return_code ;
	char * request ;
	struct timeval timeout ;
	fd_set read_sockets ;
	char response [1024] ;

	request = ping_request_new (sip_login, sip_realm, socket) ;
	if (request == NULL)
	{
		return -1 ;
	}

	return_code = owsl_send (socket, request, strlen (request), 0) ;
	if (return_code <= 0)
	{
		free (request) ;
		return -1 ;
	}

	free (request) ;

	timeout.tv_sec = ping_timeout ;
	timeout.tv_usec = 0 ;
	FD_ZERO (& read_sockets) ;
	FD_SET (socket, & read_sockets) ;
	return_code = owsl_select (socket + 1, & read_sockets, NULL, NULL, & timeout) ;
	if (return_code < 1 || ! FD_ISSET (socket, & read_sockets))
	{
		if (return_code == 0)
		{
			NETLIB_LOG_ERROR ("sip_ping : select : timeout reached\n") ;
		}
		else
		{
			NETLIB_LOG_ERROR (make_error_string ("sip_ping : select")) ;
		}
		return -1 ;
	}

	return_code = owsl_recv (socket, response, sizeof (response), 0) ;
	if (return_code < 1)
	{
		if (return_code == 0)
		{
			NETLIB_LOG_ERROR ("sip_ping : recv : returns 0\n") ;
		}
		else
		{
			NETLIB_LOG_ERROR (make_error_string ("sip_ping : recv")) ;
		}
		return -1 ;
	}

	return 0 ;
}

NETLIB_BOOLEAN
udp_sip_ping
(
	const char * sip_server,
	unsigned short sip_port,
	unsigned short local_port,
	int ping_timeout,
	const char * sip_address,
	const char * sip_realm
)
{
	int return_code ;
	OWSLSocket udp_socket ;
	struct sockaddr * local_address ;
	struct sockaddr * remote_address ;
	char * ip = OWSL_LOCAL_ADDRESS ;
	char public_ip [OWSL_IP_MAX_SIZE] ;
	if (owsl_address_public_ip_get (OWSL_AF_IPV4, public_ip, sizeof (public_ip)) == 0)
	{
		ip = public_ip ;
	}

	/* TODO: IPv6 support in netlib */
	udp_socket = owsl_socket_by_type (OWSL_TYPE_IPV4_UDP) ;
	if (udp_socket <= 0)
	{
		return NETLIB_FALSE ;
	}

	return_code = owsl_reuse_set (udp_socket) ;
	if (return_code != 0)
	{
		owsl_close (udp_socket) ;
		return NETLIB_FALSE ;
	}

	/* TODO: IPv6 support in netlib */
	local_address = owsl_address_ip_port_new (OWSL_AF_IPV4, ip, local_port) ;
	if (local_address == NULL)
	{
		owsl_close (udp_socket) ;
		return NETLIB_FALSE ;
	}

	return_code = owsl_bind (udp_socket, local_address, /*OWSL_ADDRESS_SIZE*/sizeof(struct sockaddr)) ;
	free (local_address) ;
	if (return_code != 0)
	{
		owsl_close (udp_socket) ;
		return NETLIB_FALSE ;
	}

	/* TODO: IPv6 support in netlib */
	remote_address = owsl_address_port_new (OWSL_AF_IPV4, sip_server, sip_port) ;
	if (remote_address == NULL)
	{
		owsl_close (udp_socket) ;
		return NETLIB_FALSE ;
	}

	return_code = owsl_connect (udp_socket, remote_address, /*OWSL_ADDRESS_SIZE*/sizeof(struct sockaddr)) ;
	free (remote_address) ;
	if (return_code != 0)
	{
		NETLIB_LOG_ERROR (make_error_string ("udp_sip_ping : connect")) ;
		owsl_close (udp_socket) ;
		return NETLIB_FALSE ;
	}

	return_code = sip_ping (udp_socket, ping_timeout, sip_address, sip_realm) ;

	owsl_close (udp_socket) ;

	return return_code == 0 ? NETLIB_TRUE : NETLIB_FALSE ;
}

NETLIB_BOOLEAN is_https(const char *url)
{
	if (strncasecmp(url, "https", 5) == 0) {
		return NETLIB_TRUE;
	} else {
		return NETLIB_FALSE;
	}
}

HttpRet is_http_conn_allowed(const char *url,
							  const char *proxy_addr, int proxy_port,
							  const char *proxy_login, const char *proxy_passwd,
							  NETLIB_BOOLEAN ssl, int timeout)
{
	char url_buf[1024];
	char auth_buf[1024];
	char proxy_buf[1024];
	CURL *mcurl;
	int ret;
	long http_resp_code;

	mcurl = curl_easy_init();

	curl_easy_setopt(mcurl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(mcurl, CURLOPT_WRITEFUNCTION, _curlsilentwritecbk);
	curl_easy_setopt(mcurl, CURLOPT_DEBUGFUNCTION, _curloutputcbk);

	if (ssl) 
	{
		snprintf(url_buf, sizeof(url_buf), "https://%s", url);
		curl_easy_setopt(mcurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_SSLv3);
		
		curl_easy_setopt(mcurl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(mcurl, CURLOPT_SSL_VERIFYHOST, 0);

		#ifdef OS_WIN32
			curl_easy_setopt(mcurl, CURLOPT_RANDOM_FILE, getRandomFileName() );	//VOXOX - JRT - 2009.09.30 - Improve OpenSSL initialization time.
		#endif
		
	}
	else 
	{
		snprintf(url_buf, sizeof(url_buf), "http://%s", url);
	}

	curl_easy_setopt(mcurl, CURLOPT_URL, url_buf);

	if (timeout > 0) {
		curl_easy_setopt(mcurl, CURLOPT_TIMEOUT, timeout);
	}

	/* FOLLOW REDIRECTION */
	curl_easy_setopt(mcurl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(mcurl, CURLOPT_UNRESTRICTED_AUTH, 1);
	/* ****************** */

	if (proxy_addr && *proxy_addr != 0) {
		if (proxy_login && *proxy_login != 0) {
			if (!_LocalProxy.proxy_auth_type)
				_get_proxy_auth_type2(url, proxy_addr, proxy_port, timeout);

			snprintf(auth_buf, sizeof(auth_buf), "%s:%s", proxy_login, proxy_passwd);
			curl_easy_setopt(mcurl, CURLOPT_PROXYUSERPWD, auth_buf);

			if ((_LocalProxy.proxy_auth_type & CURLAUTH_BASIC) == CURLAUTH_BASIC) {
				curl_easy_setopt(mcurl, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);
			}
			else if ((_LocalProxy.proxy_auth_type & CURLAUTH_DIGEST) == CURLAUTH_DIGEST) {
				curl_easy_setopt(mcurl, CURLOPT_PROXYAUTH, CURLAUTH_DIGEST);
			}
			else if ((_LocalProxy.proxy_auth_type & CURLAUTH_NTLM) == CURLAUTH_NTLM) {
				curl_easy_setopt(mcurl, CURLOPT_PROXYAUTH, CURLAUTH_NTLM);
			}
		}
		snprintf(proxy_buf, sizeof(proxy_buf), "%s:%d", proxy_addr, proxy_port);
		curl_easy_setopt(mcurl, CURLOPT_PROXY, proxy_buf);
	}
	else {
		if (proxy_login && *proxy_login != 0) {
			if (!_LocalProxy.auth_type) {
				_get_auth_type(url, timeout);
			}

			snprintf(auth_buf, sizeof(auth_buf), "%s:%s", proxy_login, proxy_passwd);
			curl_easy_setopt(mcurl, CURLOPT_USERPWD, auth_buf);

			if ((_LocalProxy.proxy_auth_type & CURLAUTH_BASIC) == CURLAUTH_BASIC) {
				curl_easy_setopt(mcurl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
			}
			else if ((_LocalProxy.proxy_auth_type & CURLAUTH_DIGEST) == CURLAUTH_DIGEST) {
				curl_easy_setopt(mcurl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
			}
			else if ((_LocalProxy.proxy_auth_type & CURLAUTH_NTLM) == CURLAUTH_NTLM) {
				curl_easy_setopt(mcurl, CURLOPT_HTTPAUTH, CURLAUTH_NTLM);
			}
		}

	}

	ret = curl_easy_perform(mcurl);
	curl_easy_getinfo(mcurl, CURLINFO_RESPONSE_CODE, &http_resp_code);
	if (!_LocalProxy.proxy_auth_type) {
		curl_easy_getinfo(mcurl, CURLINFO_PROXYAUTH_AVAIL, &(_LocalProxy.proxy_auth_type));
	}

	HttpRet http_ret;
	/* If this is a redirection */
	if ((http_resp_code / 100) == 3) {
		NETLIB_BOOLEAN is_ssl;
		char *redir_tmp = 0;
		char *redir_url = 0;
		char *tmp;

		curl_easy_getinfo(mcurl, CURLINFO_EFFECTIVE_URL, &redir_tmp);
		if (redir_tmp) {
			redir_url = strdup(redir_tmp);
			curl_easy_cleanup(mcurl);

			if ((is_ssl = is_https(redir_url)) == NETLIB_TRUE) {
				tmp = redir_url + 8;
			}
			else {
				tmp = redir_url + 7;
			}

			if (is_url_proxyless_exception(tmp)) {
				http_ret = is_http_conn_allowed(tmp, NULL, 0, proxy_login, proxy_passwd, is_ssl, timeout);
			}
			else {
				http_ret = is_http_conn_allowed(tmp, proxy_addr, proxy_port, proxy_login, proxy_passwd, is_ssl, timeout);
			}
			free(redir_url);
		}
		else {
			curl_easy_cleanup(mcurl);
			http_ret = HTTP_NOK;
		}
		return http_ret;
	}

	if (http_resp_code == 200) {
		http_ret = HTTP_OK;
	}
	else if (http_resp_code != 404 && http_resp_code != 200 && http_resp_code != 0) {
		http_ret = HTTP_AUTH;
	}
	else {
		http_ret = HTTP_NOK;
	}

	curl_easy_cleanup(mcurl);
	return http_ret;
}

HttpRet is_tunnel_conn_allowed(const char *http_gate_addr, int http_gate_port,
								const char *sip_addr, int sip_port,
								const char *proxy_addr, int proxy_port,
								const char *proxy_login, const char *proxy_passwd,
								NETLIB_BOOLEAN ssl, int timeout, const char *sip_realm,
								NETLIB_BOOLEAN ping, int ping_timeout)
{
	int return_code = 0 ;
	OWSLSocket http_tunnel_socket ;
	int proxy_enabled = (proxy_addr != NULL && *proxy_addr ? 1 : 0) ;
	struct sockaddr * remote_address ;

	/* TODO: IPv6 support in netlib */
	http_tunnel_socket = owsl_socket_by_type
	(
		ssl == NETLIB_TRUE ? OWSL_TYPE_IPV4_UOHS : OWSL_TYPE_IPV4_UOH
	) ;
	if (http_tunnel_socket <= 0)
	{
		return HTTP_NOK ;
	}

	return_code |= owsl_parameter_set (http_tunnel_socket, "proxy.local.enable", & proxy_enabled) ;
	return_code |= owsl_parameter_set (http_tunnel_socket, "proxy.local.addr", proxy_addr) ;
	return_code |= owsl_parameter_set (http_tunnel_socket, "proxy.local.port", & proxy_port) ;
	return_code |= owsl_parameter_set (http_tunnel_socket, "proxy.local.login", proxy_login) ;
	return_code |= owsl_parameter_set (http_tunnel_socket, "proxy.local.passwd", proxy_passwd) ;
	return_code |= owsl_parameter_set (http_tunnel_socket, "gateway.http.addr", http_gate_addr) ;
	return_code |= owsl_parameter_set (http_tunnel_socket, "gateway.http.port", & http_gate_port) ;
	return_code |= owsl_parameter_set (http_tunnel_socket, "connection.timeout", & timeout) ;
	if (return_code != 0)
	{
		owsl_close (http_tunnel_socket) ;
		return HTTP_NOK ;
	}

	/* TODO: IPv6 support */
	remote_address = owsl_address_port_new (OWSL_AF_IPV4, sip_addr, sip_port) ;
	if (remote_address == NULL)
	{
		owsl_close (http_tunnel_socket) ;
		return HTTP_NOK ;
	}

	return_code = owsl_connect
	(
		http_tunnel_socket,
		remote_address,
		OWSL_ADDRESS_SIZE
	) ;
	free (remote_address) ;
	if (return_code != 0)
	{
		owsl_close (http_tunnel_socket) ;
		return HTTP_NOK ;
		/* TODO: manage HTTP_AUTH ? */
	}

	if (ping)
	{
		return_code = sip_ping (http_tunnel_socket, ping_timeout, "nobody", sip_realm) ;
		if (return_code == NETLIB_FALSE)
		{
			owsl_close (http_tunnel_socket) ;
			return HTTP_NOK ;
		}
	}

	owsl_close (http_tunnel_socket) ;

	return HTTP_OK ;
}


NETLIB_BOOLEAN is_proxy_auth_needed(const char *proxy_addr, int proxy_port, int timeout)
{
	HttpRet ret;

	if (_LocalProxy.auth_type) {
		return NETLIB_TRUE;
	}

	ret = is_http_conn_allowed("www.google.com:80", proxy_addr, proxy_port,
								NULL, NULL, NETLIB_FALSE, timeout);

	return (ret == HTTP_AUTH ? NETLIB_TRUE : NETLIB_FALSE);
}

NETLIB_BOOLEAN is_proxy_auth_ok(const char *proxy_addr, int proxy_port,
								const char *proxy_login, const char *proxy_passwd,
								int timeout)
{
	HttpRet ret;

	ret = is_http_conn_allowed("www.google.com:80", proxy_addr, proxy_port,
								proxy_login, proxy_passwd,
								NETLIB_FALSE, timeout);

	return (ret == HTTP_OK ? NETLIB_TRUE : NETLIB_FALSE);
}



char *get_local_http_proxy_address()
{
	if (_LocalProxy.address == NULL) {
		_getProxyAddress();
	}

	return _LocalProxy.address;
}

int get_local_http_proxy_port()
{
	return _LocalProxy.port;
}

NatType get_nat_type(const char *stun_server)
{
	static const int CTRL_PORT = 3478;

	initNetwork();

	StunAddress4 stunServerAddr;
	stunServerAddr.addr = 0;

	StunAddress4 sAddr[1];
	sAddr[0].addr = 0;
	sAddr[0].port = 0;

	bool ret = stunParseServerName((char *) stun_server, stunServerAddr, CTRL_PORT);
	if (!ret) {
		return StunTypeFailure;
	}

	bool presPort = false;
	bool hairpin = false;
	return stunNatType(stunServerAddr, true, & presPort, & hairpin, 0, & sAddr[0]);
}

NETLIB_BOOLEAN is_connection_available()
{
#if defined(OS_WIN32)
	DWORD flags;

	if(!InternetGetConnectedState(&flags, 0))
		return false;

	return (flags & INTERNET_CONNECTION_LAN
		|| flags & INTERNET_CONNECTION_MODEM);

#elif defined(OS_MACOSX)

	// TODO: to be implemented
	return NETLIB_TRUE;

#else
	NETLIB_BOOLEAN res = NETLIB_FALSE;
	int sock;
	char buf[BUFSIZ];
	struct ifconf ifc;

	memset(&ifc, 0, sizeof ifc);

	ifc.ifc_len = sizeof buf;
	ifc.ifc_buf = (char *)buf;

	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		NETLIB_LOG_ERROR(make_error_string("is_connection_available : socket"));
		return NETLIB_FALSE;
	}

	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
		closesocket(sock);
		NETLIB_LOG_ERROR(make_error_string("is_connection_available : ioctl"));
		return NETLIB_FALSE;
	}

	for (unsigned int i = 0; i < (ifc.ifc_len / sizeof(struct ifreq)); i++) {
		if (ioctl(sock, SIOCGIFFLAGS, &ifc.ifc_req[i]) == 0) {
			if ((ifc.ifc_req[i]).ifr_ifru.ifru_flags & IFF_LOOPBACK) {
				continue;
			}

			if ((ifc.ifc_req[i]).ifr_ifru.ifru_flags & IFF_RUNNING) {
				res = NETLIB_TRUE;
			}
		}
	}

	closesocket(sock);
	return res;

#endif
}

#ifdef OS_WIN32
//VOXOX - JRT - 2009.09.30 - Improve OpenSSL initialization.
const char* getRandomFileName()
{
	static bool initialized = false;
	static std::string s_randFileName = "";

	if ( !initialized )
	{
		char buffer[MAX_PATH+1];
		s_randFileName = RAND_file_name(buffer, sizeof(buffer) );
		initialized = true;
	}

	return s_randFileName.c_str();
}
#endif