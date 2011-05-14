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
#include "WengoAccountParser.h"

#include <util/StringList.h>
#include <util/String.h>
#include <util/Logger.h>
#include <util/VoxMd5.h>
#include <util/VoxEncrypt.h>
#include <util/Base64.h>

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <tinyxml.h>

#include <stdlib.h>
#include <time.h>

#include <iostream>
using namespace std;

static const std::string STATUS_CODE_OK = "200";
static const std::string UNAUTHORIZED = "401";
static const std::string UNACTIVATED = "402";
static const std::string NOTFOUND = "404";
static const std::string STATUS_CODE_OTHER = "444";

static const std::string InitializationVector = "FLgaaqAH16pA70vh";
	
WengoAccountParser::WengoAccountParser(WengoAccount & account, const std::string & data) 
{
	TiXmlDocument doc;
	doc.Parse(data.c_str());

	TiXmlHandle docHandle(& doc);

	_statusCode			= NOTFOUND;
	_loginErrorMessage	= "";//VOXOX CHANGE by Rolando - 2009.09.10
	_loginPasswordOk	= false;//VOXOX CHANGE by Rolando - 2009.09.10 

	TiXmlHandle sso		= docHandle.FirstChild("sso");

	//sso status code
	TiXmlElement * elemStatus = sso.FirstChild("status").Element();//VOXOX CHANGE by Rolando - 2009.09.10 

	if (elemStatus) //VOXOX CHANGE by Rolando - 2009.09.10
	{
		if(elemStatus->Attribute("code"))	//VOXOX CHANGE by Rolando - 2009.09.10 
		{
			std::string statusCode = elemStatus->Attribute("code");
			LOG_DEBUG("SSO status code=" + statusCode);

			if (statusCode != STATUS_CODE_OK) 
			{
				_statusCode = statusCode;

				if(statusCode == STATUS_CODE_OTHER)
				{
					if(elemStatus->Attribute("message"))	//VOXOX CHANGE by Rolando - 2009.09.10 
					{
						_loginErrorMessage = elemStatus->Attribute("message");//VOXOX CHANGE by Rolando - 2009.09.10 
						LOG_DEBUG("SSO error Message=" + _loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
					}					
				}		

				_loginPasswordOk = false;
				return;
			}
			else 
			{
				_statusCode		 = statusCode;
				_loginPasswordOk = true;
			}
		}
	}
	
	//iterate over "d" element
	bool	haveImPassword = false;				//VOXOX - JRT - 2009.09.25
	bool	encrypted	   = false;				//VOXOX - JRT - 2009.09.29 

	TiXmlElement * element = sso.FirstChild("d").Element();

	while (element) 
	{
		std::string key = std::string(element->Attribute("k"));
		std::string value = "";
		const char* tmp   = element->Attribute("v");

		if (tmp) 
		{
			value = std::string(tmp);
		}

		if (key == "sip.auth.userid") 
		{
			account._identity = value;
		} 
		else if (key == "sip.auth.password") 
		{
			account._password = value;
		} 
		else if (key == "sip.auth.realm") 
		{
			account._realm = value;
		} 
		else if (key == "sip.address.name") 
		{
			account._username = value;
		} 
		else if (key == "sip.address.displayname") 
		{
			account._displayName = value;
		} 
		else if (key == "sip.address.server.host") 
		{
			account._registerServerHostname = value;
		} 
		else if (key == "sip.address.server.port") 
		{
			account._registerServerPort = String(value).toInteger();
		} 
		else if (key == "sip.outbound") 
		{
			//TODO: pass this parameter to phapi by phconfig
		} 
		else if (key == "sip.outbound.proxy.host") 
		{
			account._sipProxyServerHostname = value;
		} 
		else if (key == "sip.outbound.proxy.port") 
		{
			account._sipProxyServerPort = String(value).toInteger();
		} 
		else if (key == "netlib.stun.host") {
			account._stunServer = value;
		} 
//		else if (key == "membership.status") 
//		{
//			account._membershipStatus = value;
//		} 
		else if (key == "im.auth.password") 		//VOXOX - JRT - 2009.09.25 - SIP/Jabber password separation.
		{
			account._imPassword = value;			//Here for convenience.  We'll copy this to Jabber account.
			haveImPassword = true;
		} 
		else if ( key == "enc.enabled" )		//VOXOX - JRT - 2009.09.30 
		{
			encrypted = (value == "true");
		}
		else if (key == "im.host.name") 
		{
			Config & config = ConfigManager::getInstance().getCurrentConfig();//VOXOX - CJC - 2009.07.12 Check for voxox jabber server changes
			if(config.getJabberVoxoxServer() != value)
			{
				config.set(Config::VOXOX_JABBER_SERVER_KEY,value);
			}
		}
		//else if (key == "browser.default") 
		//{
		//	Config & config = ConfigManager::getInstance().getCurrentConfig();
		//	config.set(Config::BANNER_HOME_ADDRESS_KEY,value);
		//}
		else if (key == "netlib.tunnel.http") 
		{
			StringList httpTunnels;
			TiXmlElement * elt = element->FirstChildElement("l");

			while (elt) 
			{
				const char * tmp = elt->Attribute("v");

				if (tmp) 
				{
					httpTunnels += std::string(tmp);
				}

				elt = elt->NextSiblingElement("l");
			}

			account._httpTunnelServerHostname = chooseHttpTunnel(httpTunnels);
		} 
		else if (key == "netlib.tunnel.https") 
		{
			StringList httpsTunnels;
			TiXmlElement * elt = element->FirstChildElement("l");

			while (elt) 
			{
				const char * tmp = elt->Attribute("v");
				if (tmp) 
				{
					httpsTunnels += std::string(tmp);
				}

				elt = elt->NextSiblingElement("l");
			}

			account._httpsTunnelServerHostname = chooseHttpTunnel(httpsTunnels);
		}

		element = element->NextSiblingElement("d");
	}

	//VOXOX - JRT - 2009.09.25 - Let ensure we have the ImPassword populated.
	if ( !haveImPassword )
	{
		account._imPassword = account._password;
	}

	//VOXOX - JRT - 2009.09.28 - Decrypt passwords if needed. - TODO: better encapsulate the decode/decrypt
	if ( encrypted )
	{
		std::string key = createEncyptionKey( account._wengoLogin, account._identity );

		int			decodeLen	= 0;
		char*		decoded     = NULL;
		char*		decrypted   = NULL;
		std::string errMsg		= "";

		VoxEncrypt aes;

		aes.setInitializationVector( InitializationVector );


		//SIP password
		decoded = aes.Base16_Decode( account._password.c_str(), decodeLen );

		if ( aes.decrypt( decoded, decodeLen, key, &decrypted, errMsg ) )
		{
			account._password = decrypted;
		}

		delete decoded;
		delete decrypted;

		decoded	  = NULL;
		decrypted = NULL;

		//IM password
		decoded = aes.Base16_Decode( account._imPassword.c_str(), decodeLen );

		if ( aes.decrypt( decoded, decodeLen, key, &decrypted, errMsg ) )
		{
			account._imPassword = decrypted;
		}

		delete decoded;
		delete decrypted;
	}
}

std::string WengoAccountParser::chooseHttpTunnel(const StringList & httpTunnels) {
	srand(time(NULL));
	return httpTunnels[rand() % httpTunnels.size()];
}

std::string WengoAccountParser::createEncyptionKey( const std::string& userId, const std::string& did )
{
	VoxMd5 md5;

	String input = userId;
	input = input.toLowerCase();
	input += did;

	String tempMd5 = md5.toString( input.c_str(), input.size() );

	std::string key = tempMd5.substr( 0, 16 );

	return key;
}
