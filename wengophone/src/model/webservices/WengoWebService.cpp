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
#include "WengoWebService.h"

#include <model/account/wengo/WengoAccount.h>
#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <WengoPhoneBuildId.h>

#include <util/Platform.h>	//VOXOX - JRT - 2009.12.16 
#include <util/Logger.h>

#include <openssl/rand.h>	//VOXOX - JRT - 2009.06.04 

//-----------------------------------------------------------------------------

WengoWebService::WengoWebService(WengoAccount * wengoAccount)
	: _wengoAccount(wengoAccount) 
{
	_httpRequestParms.initVars();
//	_hostname		= "";
//	_servicePath	= "";
//	_param/eters		= "";
//	_https			= false;
//	_get			= false;
//	_port			= 0;
//	setSslRandomFile( "" );	//VOXOX - JRT - 2009.06.04 

	setWengoAuthentication( false );
	_caller		= NULL;
}

//-----------------------------------------------------------------------------

WengoWebService::~WengoWebService() 
{
}

//-----------------------------------------------------------------------------

void WengoWebService::setHttps(bool https) 
{
//	_https = https;
	_httpRequestParms.setSslProtocol( https );

//	if ( _https )
	if ( _httpRequestParms.getSslProtocol() )
	{
		char buffer[256];
		setSslRandomFile( RAND_file_name(buffer, sizeof(buffer) ) );
	}
}

//-----------------------------------------------------------------------------

//void WengoWebService::setGet(bool get) 
//{
//	_get = get;
//}

//void WengoWebService::setHostname(const std::string & hostname) 
//{
//	_hostname= hostname;
//}

//void WengoWebService::setPort(int port) 
//{
//	_port = port;
//}

//void WengoWebService::setServicePath(const std::string & servicePath) 
//{
//	_servicePath = servicePath;
//}

//void WengoWebService::setParameters(const std::string & parameters) 
//{
//	_parameters = parameters;
//}

//-----------------------------------------------------------------------------

void WengoWebService::setWengoAuthentication(bool auth) 
{
	_auth = auth;
}

//-----------------------------------------------------------------------------

int WengoWebService::sendRequest() 
{
	HttpRequest * httpRequest = new HttpRequest();
	httpRequest->answerReceivedEvent += boost::bind(&WengoWebService::answerReceivedEventHandler, this, _1, _2, _3, _4);

//	CurlHttpRequestParms parms;

//	parms.setSslProtocol  ( _https			);
//	parms.setHostName     ( _hostname		);
//	parms.setHostPort	  ( _port			);
//	parms.setPath		  ( _servicePath	);
//	parms.setData		  ( _parameters		);
//	parms.setPostMethod   ( _get			);
//	parms.setSslRandomFile( getSslRandomFile() );

//	return httpRequest->sendRequest( parms );	//VOXOX - JRT - 2009.12.23 
	return httpRequest->sendRequest( _httpRequestParms );	//VOXOX - JRT - 2009.12.23 
}

//-----------------------------------------------------------------------------

int WengoWebService::call(WengoWebService * caller) 
{
	//Set caller
	_caller = caller;

	//Add wengo parameters
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string language = config.getLanguage();

	std::string data = "lang=" + language;
	data += "&wl=" + std::string(WengoPhoneBuildId::getSoftphoneName());

	//Add authentication parameters
	if ((_auth) && (_wengoAccount)) 
	{
		String login = String::encodeUrl(_wengoAccount->getWengoLogin());
		login.replace("%2e", ".", false);

		String password = String::encodeUrl(_wengoAccount->getWengoPassword());
		password.replace("%2e", ".", false);
		data += "&login=" + login + "&password=" + password;
	}

	//VOXOX - JRT - 2009.11.04 
	String version = String::encodeUrl( WengoPhoneBuildId::getNameAndFullVersion() );
	data += "&v=" + version;

	//VOXOX - JRT - 2009.12.16 - Add OS parameter
	data += "&os=" + Platform::getPlatformCode();

	if (!getParameters().empty()) 
	{
//		_parameters = data + "&" + getParameters();
		setParametersPrivate( data + "&" + getParameters() );	//VOXOX - JRT - 2009.12.23 
	} 
	else 
	{
//		_parameters = data;
		setParameters( data );	//VOXOX - JRT - 2009.12.23 
	}

	return sendRequest();
}

//-----------------------------------------------------------------------------

void WengoWebService::answerReceivedEventHandler(IHttpRequest * sender, int requestId, const std::string & answer, HttpRequest::Error error) 
{
	if (_caller) 
	{
		if (error == HttpRequest::NoError) 
		{
			_caller->answerReceived(answer, requestId);
		} 
		else 
		{
			_caller->answerReceived(String::null, requestId);
		}
	}
}

//-----------------------------------------------------------------------------
