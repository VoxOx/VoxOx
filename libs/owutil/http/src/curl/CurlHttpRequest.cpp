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

#include "CurlHttpRequest.h"

#include <http/HttpRequest.h>

#include <util/String.h>
#include <util/Logger.h>
#include <util/Uuid.h>

#include <iostream>
using namespace std;

size_t curlHttpHeaderWrite(void * ptr, size_t size, size_t nmemb, void * curlHttpRequestInstance);
int curlHTTPProgress(void * curlHttpRequestInstance, double dltotal, double dlnow, double ultotal, double ulnow);
size_t curlHTTPWrite(void * ptr, size_t size, size_t nmemb, void * curlHttpRequestInstance);
size_t curlHTTPRead(void * ptr, size_t size, size_t nmemb, void * curlHttpRequestInstance);

#include <stdio.h>

char * getstr(std::string str) {
	char * tmp = (char *) malloc((sizeof(char) * str.size()) + 1);
	strcpy(tmp, str.data());
	strncat(tmp, "\0", 1 );
	return tmp;
}

bool CurlHttpRequest::_verbose;
bool CurlHttpRequest::_proxyAuthenticationDetermine;
long CurlHttpRequest::_proxyAuthentication;

CurlHttpRequest::CurlHttpRequest() {
	_curl = NULL;
	_proxyAuthenticationDetermine = false;
	_verbose		= false;
	abortTransfer	= false;
	downloadDone	= 0;
	downloadTotal	= 0;

//	_sslProtocol	= false;
//	_hostname		= String::null;
//	_hostPort		= 0;
//	_path			= String::null;
//	_data			= String::null;
//	_postMethod		= false;
//	_sslRandomFile	= "";
}

//int CurlHttpRequest::sendRequest(bool sslProtocol, const std::string & hostname, unsigned int hostPort,
//								 const std::string & path, const std::string & data, bool postMethod,
//								 const char* sslRandomFile ) 		//VOXOX - JRT - 2009.06.04 
int CurlHttpRequest::sendRequest( const CurlHttpRequestParms& httpRequestParms )	//VOXOX - JRT - 2009.12.23 
{
	//TODO: do not call this when running

	if (!_proxyAuthenticationDetermine) 
	{
		_proxyAuthenticationDetermine = true;
		_proxyAuthentication = getProxyAuthenticationType( httpRequestParms.getHostName() );	//VOXOX - JRT - 2009.12.23 
	}

//	_sslProtocol = sslProtocol;
//	_hostname	 = hostname;
//	_hostPort	 = hostPort;
//	_path		 = path;
//	_data		 = data;
//	_postMethod  = postMethod;
//	_sslRandomFile = (sslRandomFile ? sslRandomFile : "");		//VOXOX - JRT - 2009.06.04 //VOXOX - JRT - 2009.08.18 
	_httpRequestParms = httpRequestParms;		//VOXOX - JRT - 2009.12.23 

	// Since we set autoDelete to true, 'this' may not be available when we
	// return from start(), so we keep a copy of _lastRequestId in a local
	// variable to avoid reading freed memory.
	int id = Uuid::generateInteger();
	_lastRequestId = id;

	setAutoDelete(true);
	start();

	return id;
}

void CurlHttpRequest::run() {
	CURLcode res;

	_curl = curl_easy_init();

	if (_curl) {
		setCurlParam();
		setProxyParam();
		setProxyUserParam();
		setSSLParam();
		setUrl();
		curl_easy_setopt(_curl, CURLOPT_PRIVATE, _lastRequestId);

		res = curl_easy_perform(_curl);
		if (res != CURLE_OK) {
			LOG_WARN(std::string(curl_easy_strerror(res)));
			answerReceivedEvent(NULL, _lastRequestId, String::null, getReturnCode(res));
		} else {
			answerReceivedEvent(NULL, _lastRequestId, entireResponse, HttpRequest::NoError);
		}
		
		/*
		curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &response);
		if (!response) {
			LOG_DEBUG("no server response code has been received");
		} else {
			LOG_DEBUG("server response code=" + String::fromNumber(response));
		}
		*/

		if (_verbose) {
			char * effectiveUrl = NULL;
			curl_easy_getinfo(_curl, CURLINFO_EFFECTIVE_URL, &effectiveUrl);
			if (effectiveUrl) {
				LOG_DEBUG("CURLINFO_EFFECTIVE_URL: " + std::string(effectiveUrl));
			}

			long tmp = -1;

			curl_easy_getinfo(_curl, CURLINFO_HEADER_SIZE, &tmp);
			if (tmp >= 0) {
				LOG_DEBUG("CURLINFO_HEADER_SIZE: " + String::fromNumber(tmp));
			}

			curl_easy_getinfo(_curl, CURLINFO_REQUEST_SIZE, &tmp);
			if (tmp >= 0) {
				LOG_DEBUG("CURLINFO_REQUEST_SIZE: " + String::fromNumber(tmp));
			}

			curl_easy_getinfo(_curl, CURLINFO_SSL_VERIFYRESULT, &tmp);
			if (tmp >= 0) {
				LOG_DEBUG("CURLINFO_SSL_VERIFYRESULT: " + String::fromNumber(tmp));
			}

			double tmpDouble = -1;	//VOXOX - JRT - 2009.12.28 - CURLINFO_SIZE_DOWNLOAD needs a double.
			curl_easy_getinfo(_curl, CURLINFO_SIZE_DOWNLOAD, &tmpDouble);
			if (tmpDouble >= 0) {	//VOXOX - JRT - 2009.12.28 
				LOG_DEBUG("CURLINFO_SIZE_DOWNLOAD: " + String::fromNumber((long)tmpDouble));	//VOXOX - JRT - 2009.12.28 
			}
		}
	} else {
		LOG_WARN("cURL initialization failed");
		return;
	}

	curl_easy_cleanup(_curl);
}

void CurlHttpRequest::setUrl() {
	string url;

//	if (_sslProtocol) {
//		url = HttpRequest::HTTPS_PROTOCOL;
//	} else {
//		url = HttpRequest::HTTP_PROTOCOL;
//	}
	url = getSslProtocol() ? HttpRequest::HTTPS_PROTOCOL : HttpRequest::HTTP_PROTOCOL;	//VOXOX - JRT - 2009.12.23 

//	if (!_hostname.empty()) {
//		url.append(_hostname);
//	}
	//VOXOX - JRT - 2009.12.23 
	if ( !getHostName().empty() )
	{
		url.append( getHostName() );
	}

	if ( getHostPort() > 0)		//VOXOX - JRT - 2009.12.23 
	{
		url.append(HttpRequest::HTTP_PORT_SEPARATOR);
		url.append(String::fromNumber( getHostPort() ));	//VOXOX - JRT - 2009.12.23 
	}

//	if (!_path.empty()) {
//		url.append(_path);
//	}

	//VOXOX - JRT - 2009.12.23 
	if ( !getPath().empty() )
	{
		url.append( getPath() );
	}

//	if (!_data.empty()) 
	if (!getData().empty() ) //VOXOX - JRT - 2009.12.23 
	{
//		if (!_postMethod) 
		if (!getPostMethod() )	//VOXOX - JRT - 2009.12.23 
		{
			curl_easy_setopt(_curl, CURLOPT_POST, 0);
			url.append("?");
			url.append( getData() );	//VOXOX - JRT - 2009.12.23 
		} 
		else 
		{
			curl_easy_setopt(_curl, CURLOPT_POST, 1);

			if (!getData().empty())		//VOXOX - JRT - 2009.12.23 
			{
				setPostData( getData() );	//VOXOX - JRT - 2009.12.23 
			}
		}
	}

	curl_easy_setopt(_curl, CURLOPT_URL, getstr(url));
}

void CurlHttpRequest::setCurlParam() 
{

	//VOXOX - JRT - 2009.12.24 - To get CURL logging.  Do this first so _verbose can be set.
	if ( _httpRequestParms.getCurlLogCbk() != NULL )
	{
		_verbose = true;
		curl_easy_setopt(_curl, CURLOPT_DEBUGFUNCTION, _httpRequestParms.getCurlLogCbk() );
	}

	curl_easy_setopt(_curl, CURLOPT_VERBOSE, _verbose);
	curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, false);
	curl_easy_setopt(_curl, CURLOPT_READFUNCTION, curlHTTPRead);
	curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, curlHTTPWrite);
	curl_easy_setopt(_curl, CURLOPT_PROGRESSFUNCTION, curlHTTPProgress);
	//curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, curlHttpHeaderWrite);
	//curl_easy_setopt(_curl, CURLOPT_WRITEHEADER, this);
	curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(_curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 0);
	curl_easy_setopt(_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

	//Maximum time in seconds that you allow the libcurl transfer operation to take
	curl_easy_setopt(_curl, CURLOPT_TIMEOUT, 0);

	//Limits the connection phase, once it has connected, this option is of no more use
	curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, 10);

	//Sets the user agent
	curl_easy_setopt(_curl, CURLOPT_USERAGENT, HttpRequest::getUserAgent().c_str());
}

void CurlHttpRequest::setSSLParam() {
	curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(_curl, CURLOPT_SSLVERSION, 3);

	curl_easy_setopt(_curl, CURLOPT_RANDOM_FILE, getstr( getSslRandomFile() ) );	//VOXOX - JRT - 2009.12.23 
}

void CurlHttpRequest::setPostData(std::string _data) {
	char * data = getstr(_data);

	if (data) {
		//struct POST_DATA pooh;
		pooh.readptr = data;
		pooh.sizeleft = strlen(data);
//VoxOxChange CJC Support for bigger string in the post

		//curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, (long)pooh.sizeleft);
		curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, data);
	}
}

void CurlHttpRequest::setProxyUserParam() {
	if (useProxyAuthentication() && _proxyAuthentication) {
		if ((_proxyAuthentication & CURLAUTH_BASIC) == CURLAUTH_BASIC) {
			LOG_DEBUG("set proxy authentication: BASIC");
			curl_easy_setopt(_curl, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);
		}
		else if ((_proxyAuthentication & CURLAUTH_DIGEST) == CURLAUTH_DIGEST) {
			LOG_DEBUG("set proxy authentication: DIGEST");
			curl_easy_setopt(_curl, CURLOPT_PROXYAUTH, CURLAUTH_DIGEST);
		}
		else if ((_proxyAuthentication & CURLAUTH_NTLM) == CURLAUTH_NTLM) {
			LOG_DEBUG("set proxy authentication: NTLM");
			curl_easy_setopt(_curl, CURLOPT_PROXYAUTH, CURLAUTH_NTLM);
		}
		string proxyUserIDPassword = HttpRequest::getProxyUsername() + ":" + HttpRequest::getProxyPassword();
		curl_easy_setopt(_curl, CURLOPT_PROXYUSERPWD, getstr(proxyUserIDPassword));
	}
}

void CurlHttpRequest::setProxyParam() {
	if (useProxy()) {
		string proxy = HttpRequest::getProxyHost();
		proxy += ":" + String::fromNumber(HttpRequest::getProxyPort());
		curl_easy_setopt(_curl, CURLOPT_PROXY, getstr(proxy));
		curl_easy_setopt(_curl, CURLOPT_HTTPPROXYTUNNEL, 0);
	}
}

/**
 * Finds the proxy authentication method (BASIC, DIGEST, NTLM).
 *
 * Thanks to Julien Bossart
 */
long CurlHttpRequest::getProxyAuthenticationType(const string& hostname) {
	CURL * curl_tmp;
	long AuthMask = 0;
	int ret;

	if (useProxy()) {
		string proxy = HttpRequest::getProxyHost();
		proxy += ":" + String::fromNumber(HttpRequest::getProxyPort());

		curl_tmp = curl_easy_init();
		curl_easy_setopt(curl_tmp, CURLOPT_URL, getstr(hostname));
		curl_easy_setopt(curl_tmp, CURLOPT_VERBOSE, _verbose);
		curl_easy_setopt(curl_tmp, CURLOPT_NOBODY, 1);
		curl_easy_setopt(curl_tmp, CURLOPT_PROXY, getstr(proxy));
		ret = curl_easy_perform(curl_tmp);
		curl_easy_getinfo(curl_tmp, CURLINFO_PROXYAUTH_AVAIL, & AuthMask);
		curl_easy_cleanup(curl_tmp);

		if ((AuthMask & CURLAUTH_BASIC) == CURLAUTH_BASIC) {
			LOG_DEBUG("proxy authentication find: BASIC");
		}
		else if ((AuthMask & CURLAUTH_DIGEST) == CURLAUTH_DIGEST) {
			LOG_DEBUG("proxy authentication find: DIGEST");
		}
		else if ((AuthMask & CURLAUTH_NTLM) == CURLAUTH_NTLM) {
			LOG_DEBUG("proxy authentication find: NTLM");
		}
	}
	return AuthMask;
}

bool CurlHttpRequest::useProxy() const {
	if (!HttpRequest::getProxyHost().empty() && HttpRequest::getProxyPort() != 0) {
		return true;
	}
	return false;
}

bool CurlHttpRequest::useProxyAuthentication() const {
	if (!HttpRequest::getProxyUsername().empty() && !HttpRequest::getProxyPassword().empty()) {
		return true;
	}
	return false;
}

IHttpRequest::Error CurlHttpRequest::getReturnCode(int curlcode) {
	Error error;

	switch (curlcode) {
	case CURLE_OK:
		error = IHttpRequest::NoError;
		break;
	case CURLE_COULDNT_RESOLVE_HOST:
		error = IHttpRequest::HostNotFound;
		break;
	case CURLE_COULDNT_CONNECT:
		error = IHttpRequest::ConnectionRefused;
		break;
	case CURLE_COULDNT_RESOLVE_PROXY:
		error = IHttpRequest::ProxyConnectionError;
		break;
	case CURLE_LOGIN_DENIED:
		error = IHttpRequest::ProxyAuthenticationError;
		break;
	case CURLE_OPERATION_TIMEOUTED:
		error = IHttpRequest::TimeOut;
		break;
	default:
		error = IHttpRequest::UnknownError;
		break;
	}

	return error;
}

size_t curlHTTPWrite(void * ptr, size_t size, size_t nmemb, void * curlHttpRequestInstance) {
	if (curlHttpRequestInstance && ptr) {
		CurlHttpRequest * instance = (CurlHttpRequest *) curlHttpRequestInstance;

		long requestId;
		curl_easy_getinfo(instance->_curl, CURLINFO_PRIVATE, &requestId);

		//Appends the data received to the entire response content
		instance->entireResponse.append((const char *) ptr, nmemb);
		/*LOG_DEBUG("download done=" + String::fromNumber(instance->downloadDone) +
			" download total=" + String::fromNumber(instance->downloadTotal));*/

		if (instance->abortTransfer) {
			return 0;
		} else {
			return nmemb;
		}
	}
	else {
		return 0;
	}
}

size_t curlHTTPRead(void * ptr, size_t size, size_t nmemb, void * userp) {
	struct POST_DATA * pooh = (struct POST_DATA *) userp;

	if (size * nmemb < 1) {
		return 0;
	}

	if (pooh->sizeleft) {
		//Copy one single byte
		* (char *) ptr = pooh->readptr[0];

		//Advance pointer
		pooh->readptr++;

		//Less data left
		pooh->sizeleft--;

		//We return 1 byte at a time!
		return 1;
	}

	//No more data left to deliver
	return 0;
}

size_t curlHttpHeaderWrite(void * ptr, size_t size, size_t nmemb, void * curlHttpRequestInstance) {
	if (curlHttpRequestInstance && ptr) {
		CurlHttpRequest * instance = (CurlHttpRequest *) curlHttpRequestInstance;
		if (instance->abortTransfer) {
			return 0;
		} else {
			return nmemb;
		}
	} else {
		return 0;
	}
}

int curlHTTPProgress(void * curlHttpRequestInstance, double dltotal, double dlnow, double ultotal, double ulnow) {
	if (curlHttpRequestInstance) {
		CurlHttpRequest * instance = (CurlHttpRequest *) curlHttpRequestInstance;
		instance->downloadDone = dlnow;
		instance->downloadTotal = dltotal;
		long requestId;
		curl_easy_getinfo(instance->_curl, CURLINFO_PRIVATE, &requestId);
		instance->dataReadProgressEvent(requestId, dlnow, dltotal);
		instance->dataSendProgressEvent(requestId, ulnow, ultotal);

		//Launches answerReceivedEvent only if the entire response content has been received
		if ((dlnow >= dltotal) && (dlnow != 0 && dltotal != 0)) {
			//instance->answerReceivedEvent(NULL, requestId, instance->entireResponse, HttpRequest::NoError);
		}

		if (instance->abortTransfer) {
			return 1;
		} else {
			return 0;
		}

	} else {
		return 1;
	}
}
