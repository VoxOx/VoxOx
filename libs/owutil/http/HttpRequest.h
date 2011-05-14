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

#ifndef OWHTTPREQUEST_H
#define OWHTTPREQUEST_H

#include <http/owhttpdll.h>
#include <http/IHttpRequest.h>

#include <util/Trackable.h>

/**
 * HttpRequest implementation.
 *
 * @author Tanguy Krotoff
 */
class HttpRequest : public IHttpRequest, public Trackable {
public:

	/** HTTP port separator tag (e.g : in wengo.fr:8080). */
	static const std::string HTTP_PORT_SEPARATOR;

	/** HTTP path separator tag (e.g / in wengo.fr/home/index.html). */
	static const std::string HTTP_PATH_SEPARATOR;

	/** HTTP default port number (e.g 80). */
	static const unsigned int HTTP_DEFAULT_PORT;

	/** HTTPS protocol tag (e.g https://). */
	static const std::string HTTPS_PROTOCOL;

	/** HTTP protocol tag (e.g http://). */
	static const std::string HTTP_PROTOCOL;

	/** HTTP GET method separator tag (e.g ? in wengo.fr/login.php?login=mylogin&password=mypassword). */
	static const std::string HTTP_GET_SEPARATOR;

	OWHTTP_API HttpRequest();

	OWHTTP_API virtual ~HttpRequest();

//	OWHTTP_API int sendRequest(bool sslProtocol,
//			const std::string & hostname,
//			unsigned int hostPort,
//			const std::string & path,
//			const std::string & data,
//			bool postMethod = false,
//			const char* sslRandomFile = NULL );	//VOXOX - JRT - 2009.06.04 

	OWHTTP_API int sendRequest( const CurlHttpRequestParms& httpRequestParms );	//VOXOX - JRT - 2009.12.23 

	/**
	 * Sends a HTTP request given a HTTP URL.
	 *
	 * A complete HTTP URL is something like:
	 * https://wengo.fr:8080/login.php?login=mylogin&password=mypassword
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function.
	 *
	 * @param url HTTP URL (e.g https://wengo.fr:8080/login.php)
	 * @param data HTTP data (e.g login=mylogin&password=mypassword)
	 * @param postMethod HTTP POST method if true, HTTP GET method if false
	 * @see sendRequest(bool, const std::string &, unsigned int, const std::string &, const std::string &, bool)
	 * @return HTTP request ID
	 */
	OWHTTP_API int sendRequest(const std::string & url, const std::string & data, bool postMethod = false, void* fnCurlLogCbk = NULL);

	/**
	 * Sets the local proxy settings.
	 * The proxy can be a SOCKS5 proxy,
	 *
	 * Static method so that it keeps the proxy informations once it is set.
	 *
	 * @param host HTTP proxy hostname
	 * @param port HTTP proxy port number
	 * @param username HTTP proxy username
	 * @param password HTTP proxy password
	 */
	OWHTTP_API static void setProxy(const std::string & host,
		unsigned int port,
		const std::string & username,
		const std::string & password) {

		_proxyHost = host;
		_proxyPort = port;
		_proxyUsername = username;
		_proxyPassword = password;
	}

	/**
	 * Gets the HTTP proxy hostname.
	 *
	 * @return HTTP proxy hostname
	 */
	OWHTTP_API static const std::string & getProxyHost() {
		return _proxyHost;
	}

	/**
	 * Gets the HTTP proxy port number.
	 *
	 * @return HTTP proxy port number
	 */
	OWHTTP_API static unsigned int getProxyPort() {
		return _proxyPort;
	}

	/**
	 * Gets the HTTP proxy username/login.
	 *
	 * @return HTTP proxy username
	 */
	OWHTTP_API static const std::string & getProxyUsername() {
		return _proxyUsername;
	}

	/**
	 * Gets the HTTP proxy password.
	 *
	 * @return HTTP proxy password
	 */
	OWHTTP_API static const std::string & getProxyPassword() {
		return _proxyPassword;
	}

	/**
	 * Sets the User Agent.
	 *
	 * @param userAgent the user agent
	 */
	OWHTTP_API static void setUserAgent(const std::string & userAgent) {
		_userAgent = userAgent;
	}

	/**
	 * Sets the User Agent.
	 *
	 * @param userAgent the user agent
	 */
	OWHTTP_API static const std::string & getUserAgent() {
		return _userAgent;
	}

	OWHTTP_API void abort();

private:

	//Inherited from Thread
	void run();
	////

	void answerReceivedEventHandler(IHttpRequest * sender, int requestId, const std::string & answer, Error error);

	/** System-dependant implementation. */
	IHttpRequest * _httpRequestPrivate;

	/** HTTP proxy hostname. */
	static std::string _proxyHost;

	/** HTTP proxy port number. */
	static unsigned int _proxyPort;

	/** HTTP proxy username. */
	static std::string _proxyUsername;

	/** HTTP proxy password. */
	static std::string _proxyPassword;

	/** HTTP user agent. */
	static std::string _userAgent;
};

#endif	//OWHTTPREQUEST_H
