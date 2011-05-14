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

#include "PurpleIMConnect.h"

#include "PurpleEnumIMProtocol.h"
#include "PurpleEnumPresenceState.h"	//VOXOX - JRT - 2009.09.10 

extern "C" 
{
#include <libpurple/account.h>
#include <libpurple/connection.h>
#include <libpurple/core.h>
#include <libpurple/proxy.h>
}

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountParameters.h>

#include <util/Logger.h>
#include <thread/Thread.h>

#ifdef _WIN32
#include <windows.h>
#endif

static const char * MAIL_NOTIFICATION_KEY		= "check-mail";

static const char * YAHOO_IS_JAPAN_KEY			= "yahoojp";
static const char * YAHOO_SERVER_KEY			= "server";
static const char * YAHOO_JAPAN_SERVER_KEY		= "serverjp";
static const char * YAHOO_PORT_KEY				= "port";
static const char * YAHOO_XFER_HOST_KEY			= "xfer_host";
static const char * YAHOO_JAPAN_XFER_HOST_KEY	= "xferjp_host";
static const char * YAHOO_XFER_PORT_KEY			= "xfer_port";
static const char * YAHOO_ROOM_LIST_LOCALE_KEY	= "room_listlocale";

static const char * MSN_SERVER_KEY				= "server";
static const char * MSN_PORT_KEY				= "port";
static const char * MSN_HTTP_SERVER_KEY			= "http_method_server";
static const char * MSN_USE_HTTP_KEY			= "http_method";

static const char * MYSPACE_SERVER_KEY			= "server";
static const char * MYSPACE_PORT_KEY			= "port";

static const char * OSCAR_SERVER_KEY			= "server";
static const char * OSCAR_PORT_KEY				= "port";
static const char * OSCAR_ENCODING_KEY			= "encoding";
static const char * OSCAR_USE_RV_PROXY_KEY		= "always_use_rv_proxy";

static const char * JABBER_SERVER_KEY			= "server";
static const char * JABBER_PORT_KEY				= "port";
static const char * JABBER_RESOURCE_KEY			= "resource";
static const char * JABBER_USE_TLS_KEY			= "use_tls";
static const char * JABBER_REQUIRE_TLS_KEY		= "require_tls";
static const char * JABBER_USE_OLD_SSL_KEY		= "old_ssl";
static const char * JABBER_AUTH_PLAIN_IN_CLEAR_KEY = "auth_plain_in_clear";
static const char * JABBER_CONNECTION_SERVER_KEY = "connect_server";

//typedef struct misc_s
//{
//	PurpleIMConnect * instance;
//	int cbk_id;
//}	misc_t;

Mutex PurpleIMConnect::_mutex;

//-----------------------------------------------------------------------------

PurpleIMConnect::PurpleIMConnect(IMAccount account) : IMConnect(account)
{
	_connected = false;
}

//-----------------------------------------------------------------------------

PurpleIMConnect::~PurpleIMConnect()
{
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::AddMSNAccountParams(void *purpleAccount, const IMAccountParameters &mParams)
{
	PurpleAccount *gAccount = (PurpleAccount *)purpleAccount;

	purple_account_set_bool(gAccount, MSN_USE_HTTP_KEY, mParams.isMSNHttpUsed());
	if (mParams.isMSNHttpUsed())
	{
		purple_account_set_string(gAccount, MSN_SERVER_KEY, mParams.getMSNHttpServer().c_str());
		purple_account_set_int(gAccount, MSN_PORT_KEY, 80);
	}
	else
	{
		purple_account_set_string(gAccount, MSN_SERVER_KEY, mParams.getMSNServer().c_str());
		purple_account_set_int(gAccount, MSN_PORT_KEY, mParams.getMSNServerPort());
	}
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::AddMySpaceAccountParams(void *purpleAccount, const IMAccountParameters &mParams)
{
	PurpleAccount *gAccount = (PurpleAccount *)purpleAccount;

	purple_account_set_string(gAccount, MYSPACE_SERVER_KEY, mParams.getMySpaceServer().c_str());
	purple_account_set_int   (gAccount, MYSPACE_PORT_KEY,   mParams.getMySpaceServerPort());
	
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::AddFacebookAccountParams(void *purpleAccount, const IMAccountParameters &mParams)
{
	//TODO ADD FACEBOOK PARAMS
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::AddTwitterAccountParams(void *purpleAccount, const IMAccountParameters &mParams)
{
	//TODO ADD TWITTER PARAMS, NO SERVER OR PORT NEEDED, NOT SURE IF THIS IS NEEDED
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::AddSkypeAccountParams(void *purpleAccount, const IMAccountParameters &mParams)
{
	//TODO ADD TWITTER PARAMS, NO SERVER OR PORT NEEDED, NOT SURE IF THIS IS NEEDED. SKYPE PLUGGING DONT EVEN NEED PASSWORD
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::AddYahooAccountParams(void *purpleAccount, const IMAccountParameters &mParams)
{
	PurpleAccount *gAccount = (PurpleAccount *)purpleAccount;

	purple_account_set_bool  (gAccount, YAHOO_IS_JAPAN_KEY,			mParams.isYahooJapan());
	purple_account_set_string(gAccount, YAHOO_SERVER_KEY,			mParams.getYahooServer().c_str());
	purple_account_set_string(gAccount, YAHOO_JAPAN_SERVER_KEY,		mParams.getYahooJapanServer().c_str());
	purple_account_set_int   (gAccount, YAHOO_PORT_KEY,				mParams.getYahooServerPort());
	purple_account_set_string(gAccount, YAHOO_XFER_HOST_KEY,		mParams.getYahooXferHost().c_str());
	purple_account_set_string(gAccount, YAHOO_JAPAN_XFER_HOST_KEY,	mParams.getYahooJapanXferHost().c_str());
	purple_account_set_int   (gAccount, YAHOO_XFER_PORT_KEY,		mParams.getYahooXferPort());
	purple_account_set_string(gAccount, YAHOO_ROOM_LIST_LOCALE_KEY, mParams.getYahooRoomListLocale().c_str());
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::AddOscarAccountParams(void *purpleAccount, const IMAccountParameters &mParams)
{
	PurpleAccount *gAccount = (PurpleAccount *)purpleAccount;

	purple_account_set_string(gAccount, OSCAR_SERVER_KEY,		mParams.getOscarServer().c_str());
	purple_account_set_int   (gAccount, OSCAR_PORT_KEY,			mParams.getOscarServerPort());
	purple_account_set_string(gAccount, OSCAR_ENCODING_KEY,		mParams.getOscarEncoding().c_str());
	purple_account_set_bool  (gAccount, OSCAR_USE_RV_PROXY_KEY, FALSE);
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::AddJabberAccountParams(void *purpleAccount, const IMAccountParameters &mParams)
{
	PurpleAccount *gAccount = (PurpleAccount *)purpleAccount;

	purple_account_set_string(gAccount, JABBER_SERVER_KEY,				mParams.getJabberServer().c_str());
	purple_account_set_int   (gAccount, JABBER_PORT_KEY,				mParams.getJabberServerPort());
	purple_account_set_string(gAccount, JABBER_RESOURCE_KEY,			mParams.getJabberResource().c_str());
	purple_account_set_bool  (gAccount, JABBER_USE_TLS_KEY,				mParams.isJabberTLSUsed());
	purple_account_set_bool  (gAccount, JABBER_REQUIRE_TLS_KEY,			mParams.isJabberTLSRequired());
	purple_account_set_bool  (gAccount, JABBER_USE_OLD_SSL_KEY,			mParams.isJabberOldSSLUsed());
	purple_account_set_bool  (gAccount, JABBER_AUTH_PLAIN_IN_CLEAR_KEY, mParams.isJabberAuthPlainInClearUsed());
	purple_account_set_string(gAccount, JABBER_CONNECTION_SERVER_KEY,	mParams.getJabberConnectionServer().c_str());
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::AddAccountParams(void *purpleAccount, const IMAccount & account)
{
	PurpleAccount *gAccount = (PurpleAccount *)purpleAccount;
	const IMAccountParameters & mParams = const_cast<IMAccount &>(account).getIMAccountParameters();
	
	/* Proxy configuration if it's available*/
	PurpleProxyInfo *proxyInfo = purple_account_get_proxy_info(gAccount);
	if (!proxyInfo)
	{
		proxyInfo = purple_proxy_info_new();
		purple_account_set_proxy_info(gAccount, proxyInfo);
	}

	if (mParams.isHttpProxyUsed())
	{
		purple_proxy_info_set_type	  (proxyInfo, PURPLE_PROXY_HTTP);
		purple_proxy_info_set_host	  (proxyInfo, mParams.getHttpProxyServer().c_str());
		purple_proxy_info_set_port	  (proxyInfo, mParams.getHttpProxyPort());
		purple_proxy_info_set_username(proxyInfo, mParams.getHttpProxyLogin().c_str());
		purple_proxy_info_set_password(proxyInfo, mParams.getHttpProxyPassword().c_str());
	}

	purple_account_set_bool(gAccount, MAIL_NOTIFICATION_KEY, mParams.isMailNotified());

	switch (account.getProtocol())
	{
		case EnumIMProtocol::IMProtocolMSN:
			AddMSNAccountParams(gAccount, mParams);
			break;

		case EnumIMProtocol::IMProtocolMYSPACE:
			AddMySpaceAccountParams(gAccount, mParams);
			break;

		case EnumIMProtocol::IMProtocolFacebook:
			AddFacebookAccountParams(gAccount, mParams);
			break;

		case EnumIMProtocol::IMProtocolTwitter:
			AddTwitterAccountParams(gAccount, mParams);
			break;

		case EnumIMProtocol::IMProtocolSkype:
			AddSkypeAccountParams(gAccount, mParams);
			break;

		case EnumIMProtocol::IMProtocolYahoo:
			AddYahooAccountParams(gAccount, mParams);
			break;

		case EnumIMProtocol::IMProtocolAIM:
		case EnumIMProtocol::IMProtocolICQ:
			AddOscarAccountParams(gAccount, mParams);
			break;

		case EnumIMProtocol::IMProtocolJabber:
			AddJabberAccountParams(gAccount, mParams);
			break;

		default:
			LOG_FATAL("unknown protocol=" + String::fromNumber(account.getProtocol()));
			break;
	}
}

//-----------------------------------------------------------------------------

void *PurpleIMConnect::CreateAccount(const IMAccount & account)
{
	char *PrclId = (char *)PurpleIMPrcl::GetPrclId(account.getProtocol());

	PurpleAccount *gAccount = purple_account_new(account.getLogin().c_str(), PrclId);

	if (gAccount)
	{
		if (!account.getPassword().empty()) 
		{
			purple_account_set_password(gAccount, account.getPassword().c_str());
		}

		purple_accounts_add(gAccount);
	}

	return gAccount;
}

//-----------------------------------------------------------------------------

bool PurpleIMConnect::connectCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMConnect::_mutex);

	PurpleIMConnectCallbackData* cbData = (PurpleIMConnectCallbackData*) dataIn;

	PurpleIMConnect* imConnect = cbData->getPurpleIMConnect();
	PurpleAccount*   gAccount  = getPurpleAccount(imConnect->getIMAccount() );
	
	if (!gAccount) 
	{
		if ((gAccount = (PurpleAccount *)PurpleIMConnect::CreateAccount(imConnect->getIMAccount()))) 
		{
			PurpleIMConnect::AddAccountParams(gAccount, imConnect->getIMAccount());
		} 
		else 
		{
			timeoutRemove( cbData );
			delete cbData;

			return FALSE;
		}
	}

	std::string password = imConnect->getIMAccount().getPassword();
	if (!password.empty()) 
	{
		purple_account_set_password(gAccount, password.c_str());
	}

	// FIXME: must publish chosen status before connecting
	const char* initialPresence = PurplePreState::GetStatusId( imConnect->getIMAccount().getInitialPresenceState(),
															   imConnect->getIMAccount().getProtocol() );

	purple_account_set_status (gAccount, initialPresence, TRUE, NULL);	//VOXOX - JRT - 2009.09.10 
//	purple_account_set_status (gAccount, "available", TRUE, NULL);		//VOXOX - JRT - 2009.08.12 - The presence should be a parameter.
	purple_account_set_enabled(gAccount, purple_core_get_ui(), TRUE);

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::connect()
{
	PurpleIMConnectCallbackData* cbData = new PurpleIMConnectCallbackData;
	
	cbData->setPurpleIMConnect( this );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMConnect::connectCbk ) );
//	Mutex::ScopedLock lock(PurpleIMConnect::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMConnect::connectCbk, data);
}

//-----------------------------------------------------------------------------

bool PurpleIMConnect::disconnectCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMConnect::_mutex);

	PurpleIMConnectCallbackData* cbData = (PurpleIMConnectCallbackData*) dataIn;

	PurpleIMConnect* imConnect = cbData->getPurpleIMConnect();
	PurpleAccount*   gAccount  = getPurpleAccount( imConnect->getIMAccount() );

	purple_account_set_enabled(gAccount, purple_core_get_ui(), FALSE);

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::disconnect(bool force)
{
	PurpleIMConnectCallbackData* cbData = new PurpleIMConnectCallbackData;
	
	cbData->setPurpleIMConnect( this );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMConnect::disconnectCbk ) );
//	Mutex::ScopedLock lock(PurpleIMConnect::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMConnect::disconnectCbk, data);
}

//-----------------------------------------------------------------------------
//static
PurpleAccount* PurpleIMConnect::getPurpleAccount( const IMAccount& imAccount )
{
	return purple_accounts_find( imAccount.getLogin().c_str(), PurpleIMPrcl::GetPrclId( imAccount.getProtocol() ) );
}

//-----------------------------------------------------------------------------

int PurpleIMConnect::timeoutAdd( void* cbData, GSourceFunc callbackFunc )
{
	Mutex::ScopedLock lock(PurpleIMConnect::_mutex);
	return purple_timeout_add(0, callbackFunc, cbData);
}

//-----------------------------------------------------------------------------

void PurpleIMConnect::timeoutRemove( PurpleIMConnectCallbackData* cbData )
{
	int waitTime = 1;
	int totalWaitTime = 0;
	while ( cbData->getCallbackId() == 0 )
	{
		Thread::msleep( waitTime );
		totalWaitTime += waitTime;

#ifdef _WIN32
		char msg[200];
		sprintf_s( msg, "PurpleIMConnect::timeoutRemove - waiting for valid callbackId. %d ms\n", totalWaitTime );
		OutputDebugString( msg );
#endif
	}

#ifdef _WIN32
	if ( totalWaitTime > 0 )
	{
		char msg[200];
		sprintf_s( msg, "\n" );
		OutputDebugString( msg );
	}
#endif

	purple_timeout_remove( cbData->getCallbackId() );
}

//-----------------------------------------------------------------------------
