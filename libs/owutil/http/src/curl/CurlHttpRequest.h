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

#ifndef OWCURLHTTPREQUEST_H
#define OWCURLHTTPREQUEST_H

#include <http/IHttpRequest.h>

#include <curl/curl.h>

#include <string>
#include <list>

struct POST_DATA {
	const char * readptr;
	size_t sizeleft;
};

/**
 * HttpRequest implementation using libcurl.
 *
 * @see HttpRequest
 * @see http://curl.haxx.se/
 * @author Mathieu Stute
 * @author Tanguy Krotoff
 */
class CurlHttpRequest : public IHttpRequest {
public:

	CurlHttpRequest();

	//int sendRequest(bool sslProtocol,
	//		const std::string & hostname,
	//		unsigned int hostPort,
	//		const std::string & path,
	//		const std::string & data,
	//		bool postMethod = false,
	//		const char* sslRandomFile = NULL );	//VOXOX - JRT - 2009.06.04 
	int sendRequest( const CurlHttpRequestParms& httpRequestParms );

	void abort() {
		abortTransfer = true;
	}

	/** Libcurl component. */
	CURL * _curl;

	/** Current progress of the download. */
	double downloadDone;

	/** Total amount of data to download. */
	double downloadTotal;

	/** Entire response content. */
	std::string entireResponse;

	/** Abort or not the current HTTP transfer. */
	bool abortTransfer;

	void run();

private:

	/** Sets libcurl global parameters. */
	void setCurlParam();

	/**
	 * Sets libCurl URL.
	 *
	 */
	void setUrl();

	/** Sets libCurl proxy parameters. */
	void setProxyParam();

	/** Sets libCurl user proxy parameters : user id and password. */
	void setProxyUserParam();

	/** Sets libCurl SSL parameters. */
	void setSSLParam();

	/**
	 * Sets curl post options and data.
	 *
	 * @param post data
	 */
	void setPostData(std::string data);

	/**
	 * LibCurl error code to HttpRequest error code
	 *
	 * @param CurlCode
	 * @return the equivalent HttpRequest error code
	 */
	IHttpRequest::Error getReturnCode(int curlcode);

	/**
	 * Determines the proxy authentication method.
	 *
	 * @param hostname the hostname the method will try to contact to determine
	 * the proxy authentication method.
	 * @return a long representing one of following authentication method:
	 *         BASIC, DIGEST & NTLM
	 */
	long getProxyAuthenticationType(const std::string& hostname);

	bool useProxy() const;

	bool useProxyAuthentication() const;

	//VOXOX - JRT - 2009.12.23 
	bool			getSslProtocol()	const		{ return _httpRequestParms.getSslProtocol();	}
	std::string		getHostName()		const		{ return _httpRequestParms.getHostName();		}
	unsigned int	getHostPort()		const		{ return _httpRequestParms.getHostPort();		}
	std::string		getPath()			const		{ return _httpRequestParms.getPath();			}
	std::string		getData()			const		{ return _httpRequestParms.getData();			}
	bool			getPostMethod()		const		{ return _httpRequestParms.getPostMethod();		}
	std::string		getSslRandomFile()	const		{ return _httpRequestParms.getSslRandomFile();	}

	/** Tells curl to be verbose + additionnal information. */
	static bool _verbose;

	/** Is proxy authentication determined. */
	static bool _proxyAuthenticationDetermine;

	/** Proxy authentication mask. */
	static long _proxyAuthentication;

	/** Post data structure. */
	struct POST_DATA pooh;

	int _lastRequestId;

	//VOXOX - JRT - 2009.12.23 - Replace various parms with a pars class for easier maintenance
	CurlHttpRequestParms _httpRequestParms;
//	bool _sslProtocol;

//	std::string _hostname;

//	unsigned int _hostPort;

//	std::string _path;

//	std::string _data;

//	std::string	_sslRandomFile;		//VOXOX - JRT - 2009.06.04 

//	bool _postMethod;
};

#endif	//OWCURLHTTPREQUEST_H
