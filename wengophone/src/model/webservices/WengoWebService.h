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

#ifndef OWWENGOWEBSERVICE_H
#define OWWENGOWEBSERVICE_H

#include <http/HttpRequest.h>
#include <util/Trackable.h>

class WengoAccount;

/**
 * WengoWebService.
 *
 * Class helper to create web services easily.
 *
 * @author Mathieu Stute
 */
class WengoWebService : public Trackable {
public:

	/**
	 * Callback to check if the SMS was received or not.
	 *
	 * @param sender this class
	 * @param requestId request unique identifier
	 * @param answer the response returned by the web service
	 */
	Event<void (WengoWebService & sender, int requestId, const std::string & answer)> answerReceivedEvent;

	/**
	 * Default constructor.
	 *
	 * @param wengoAccount the WengoAccount used for web services
	 */
	WengoWebService(WengoAccount * wengoAccount = NULL);

	virtual ~WengoWebService();

	/**
	 * Set/unset https mode.
	 *
	 * @param https if true https else http
	 */
	void setHttps(bool https);

	/**
	 * Set/unset get mode
	 * @param get if true GET else POST
	 */
	 void setGet(bool get)									{ _httpRequestParms.setPostMethod( get );		}

	/**
	 * Set hostname
	 * @param hostname hostname
	 */
	void setHostname(const std::string & hostname)			{ _httpRequestParms.setHostName( hostname );	}

	/**
	 * Set port
	 * @param port port
	 */
	void setPort(int port)									{ _httpRequestParms.setHostPort( port );		}

	/**
	 * Set service path
	 * @param servicePath the path to the service
	 */
	void setServicePath(const std::string & servicePath)	{ _httpRequestParms.setPath( servicePath );		}

	/**
	 * Sets additionnal parameter.
	 *
	 * A string like : "name=toto&city=paris",
	 * no '?' at the begining of the string
	 *
	 * @param servicePath the path to the service
	 */
	void setParameters(const std::string & param)			{ _httpRequestParms.setData( param );			}

	void setCurlLogCbk( void* val )							{ _httpRequestParms.setCurlLogCbk( val );		}	//VOXOX - JRT - 2009.12.24 
	/**
	 * Set/unset wengo authentication.
	 *
	 * @param auth activate authentication if true
	 */
	void setWengoAuthentication(bool auth);


	/**
	 * Calls the web service.
	 *
	 * @param caller caller
	 */
	int call(WengoWebService * caller);


protected:
	//VOXOX - JRT - 2009.06.04 - Improve OpenSSL initialization.
//	const char*	getSslRandomFile()	const				{ return _sslRandomFile.c_str();	}
//	void		setSslRandomFile( const char* val )		{ _sslRandomFile = val;	}
	const char*	getSslRandomFile()	const				{ return _httpRequestParms.getSslRandomFile().c_str();	}
	void		setSslRandomFile( const char* val )		{ _httpRequestParms.setSslRandomFile( val );	}

	std::string	getParameters() const					{ return _httpRequestParms.getData();	}

	/**
	 * Sends a request to a web service.
	 *
	 * @param https if true this is a https request
	 * @param get if true this is a get request
	 * @param hostname hostname of the webservice
	 * @param port port
	 * @param servicePath path to the service
	 * @param additionalParameters additional parameters to the request
	 * @return unique request ID
	 */
	int sendRequest();

	/**
	 * @see IHttpRequest::answerReceivedEvent
	 */
	void answerReceivedEventHandler(IHttpRequest * sender, int requestId, const std::string & answer, HttpRequest::Error error);

	WengoAccount* getWengoAccount() const		{ return _wengoAccount;	}

private:

	void		setParametersPrivate( const std::string& val )	{ _httpRequestParms.setData( val );		}

	/**
	 * Answer received callback.
	 *
	 * @param answer the answer to the request
	 * @param requestId the request id
	 */
	virtual void answerReceived(const std::string & answer, int requestId) = 0;

	/** WengoAccount: to get login & password */
	WengoAccount * _wengoAccount;

	CurlHttpRequestParms	_httpRequestParms;

	/** use authentication mode */
	bool _auth;

	/** caller */
	WengoWebService * _caller;

	/** https/http mode */
//	bool _https;

	/** GET/POST mode */
//	bool _get;

	/** hostname of the web service */
//	std::string _hostname;

	/** port of the web service */
//	int _port;

	/** path of the web service */
//	std::string _servicePath;

	/** parameters of the web service */
//	std::string _parameters;

//	std::string			_sslRandomFile;		//VOXOX - JRT - 2009.06.04 - Improve OpenSSL init for HTTPS
};

#endif	//OWWENGOWEBSERVICE_H

