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

#include "GaimIMConnect.h"

#include "GaimEnumIMProtocol.h"

extern "C" {
#include <gaim/account.h>
#include <gaim/connection.h>
#include <gaim/core.h>
#include <gaim/proxy.h>
}

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountParameters.h>

#include <util/Logger.h>

static const char * MAIL_NOTIFICATION_KEY = "check-mail";
static const char * YAHOO_IS_JAPAN_KEY = "yahoojp";
static const char * YAHOO_SERVER_KEY = "server";
static const char * YAHOO_JAPAN_SERVER_KEY = "serverjp";
static const char * YAHOO_PORT_KEY = "port";
static const char * YAHOO_XFER_HOST_KEY = "xfer_host";
static const char * YAHOO_JAPAN_XFER_HOST_KEY = "xferjp_host";
static const char * YAHOO_XFER_PORT_KEY = "xfer_port";
static const char * YAHOO_ROOM_LIST_LOCALE_KEY = "room_listlocale";
static const char * MSN_SERVER_KEY = "server";
static const char * MSN_PORT_KEY = "port";
static const char * MSN_HTTP_SERVER_KEY = "http_method_server";
static const char * MSN_USE_HTTP_KEY = "http_method";
static const char * OSCAR_SERVER_KEY = "server";
static const char * OSCAR_PORT_KEY = "port";
static const char * OSCAR_ENCODING_KEY = "encoding";
static const char * OSCAR_USE_RV_PROXY_KEY = "always_use_rv_proxy";
static const char * JABBER_SERVER_KEY = "server";
static const char * JABBER_PORT_KEY = "port";
static const char * JABBER_RESOURCE_KEY = "resource";
static const char * JABBER_USE_TLS_KEY = "use_tls";
static const char * JABBER_REQUIRE_TLS_KEY = "require_tls";
static const char * JABBER_USE_OLD_SSL_KEY = "old_ssl";
static const char * JABBER_AUTH_PLAIN_IN_CLEAR_KEY = "auth_plain_in_clear";
static const char * JABBER_CONNECTION_SERVER_KEY = "connect_server";

typedef struct misc_s
{
	GaimIMConnect * instance;
	int cbk_id;
}	misc_t;

Mutex GaimIMConnect::_mutex;

GaimIMConnect::GaimIMConnect(IMAccount account)
	: IMConnect(account)
{
	_connected = false;
}

GaimIMConnect::~GaimIMConnect()
{
}

void GaimIMConnect::AddMSNAccountParams(void *gaimAccount, const IMAccountParameters &mParams)
{
	GaimAccount *gAccount = (GaimAccount *)gaimAccount;

	gaim_account_set_bool(gAccount, MSN_USE_HTTP_KEY, mParams.isMSNHttpUsed());
	if (mParams.isMSNHttpUsed())
	{
		gaim_account_set_string(gAccount, MSN_SERVER_KEY, mParams.getMSNHttpServer().c_str());
		gaim_account_set_int(gAccount, MSN_PORT_KEY, 80);
	}
	else
	{
		gaim_account_set_string(gAccount, MSN_SERVER_KEY, mParams.getMSNServer().c_str());
		gaim_account_set_int(gAccount, MSN_PORT_KEY, mParams.getMSNServerPort());
	}
}

void GaimIMConnect::AddYahooAccountParams(void *gaimAccount, const IMAccountParameters &mParams)
{
	GaimAccount *gAccount = (GaimAccount *)gaimAccount;

	gaim_account_set_bool(gAccount, YAHOO_IS_JAPAN_KEY, mParams.isYahooJapan());
	gaim_account_set_string(gAccount, YAHOO_SERVER_KEY, mParams.getYahooServer().c_str());
	gaim_account_set_string(gAccount, YAHOO_JAPAN_SERVER_KEY, mParams.getYahooJapanServer().c_str());
	gaim_account_set_int(gAccount, YAHOO_PORT_KEY, mParams.getYahooServerPort());
	gaim_account_set_string(gAccount, YAHOO_XFER_HOST_KEY, mParams.getYahooXferHost().c_str());
	gaim_account_set_string(gAccount, YAHOO_JAPAN_XFER_HOST_KEY, mParams.getYahooJapanXferHost().c_str());
	gaim_account_set_int(gAccount, YAHOO_XFER_PORT_KEY, mParams.getYahooXferPort());
	gaim_account_set_string(gAccount, YAHOO_ROOM_LIST_LOCALE_KEY, mParams.getYahooRoomListLocale().c_str());
}

void GaimIMConnect::AddOscarAccountParams(void *gaimAccount, const IMAccountParameters &mParams)
{
	GaimAccount *gAccount = (GaimAccount *)gaimAccount;

	gaim_account_set_string(gAccount, OSCAR_SERVER_KEY, mParams.getOscarServer().c_str());
	gaim_account_set_int(gAccount, OSCAR_PORT_KEY, mParams.getOscarServerPort());
	gaim_account_set_string(gAccount, OSCAR_ENCODING_KEY, mParams.getOscarEncoding().c_str());
	gaim_account_set_bool(gAccount, OSCAR_USE_RV_PROXY_KEY, FALSE);
}

void GaimIMConnect::AddJabberAccountParams(void *gaimAccount, const IMAccountParameters &mParams)
{
	GaimAccount *gAccount = (GaimAccount *)gaimAccount;

	gaim_account_set_string(gAccount, JABBER_SERVER_KEY, mParams.getJabberServer().c_str());
	gaim_account_set_int(gAccount, JABBER_PORT_KEY, mParams.getJabberServerPort());
	gaim_account_set_string(gAccount, JABBER_RESOURCE_KEY, mParams.getJabberResource().c_str());
	gaim_account_set_bool(gAccount, JABBER_USE_TLS_KEY, mParams.isJabberTLSUsed());
	gaim_account_set_bool(gAccount, JABBER_REQUIRE_TLS_KEY, mParams.isJabberTLSRequired());
	gaim_account_set_bool(gAccount, JABBER_USE_OLD_SSL_KEY, mParams.isJabberOldSSLUsed());
	gaim_account_set_bool(gAccount, JABBER_AUTH_PLAIN_IN_CLEAR_KEY, mParams.isJabberAuthPlainInClearUsed());
	gaim_account_set_string(gAccount, JABBER_CONNECTION_SERVER_KEY, mParams.getJabberConnectionServer().c_str());
}

void GaimIMConnect::AddAccountParams(void *gaimAccount, const IMAccount & account)
{
	GaimAccount *gAccount = (GaimAccount *)gaimAccount;
	const IMAccountParameters & mParams = const_cast<IMAccount &>(account).getIMAccountParameters();
	
	/* Proxy configuration if it's available*/
	GaimProxyInfo *proxyInfo = gaim_account_get_proxy_info(gAccount);
	if (!proxyInfo)
	{
		proxyInfo = gaim_proxy_info_new();
		gaim_account_set_proxy_info(gAccount, proxyInfo);
	}
	if (mParams.isHttpProxyUsed())
	{
		gaim_proxy_info_set_type(proxyInfo, GAIM_PROXY_HTTP);
		gaim_proxy_info_set_host(proxyInfo, mParams.getHttpProxyServer().c_str());
		gaim_proxy_info_set_port(proxyInfo, mParams.getHttpProxyPort());
		gaim_proxy_info_set_username(proxyInfo, mParams.getHttpProxyLogin().c_str());
		gaim_proxy_info_set_password(proxyInfo, mParams.getHttpProxyPassword().c_str());
	}

	gaim_account_set_bool(gAccount, MAIL_NOTIFICATION_KEY, mParams.isMailNotified());

	switch (account.getProtocol())
	{
		case EnumIMProtocol::IMProtocolMSN:
			AddMSNAccountParams(gAccount, mParams);
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

void *GaimIMConnect::CreateAccount(const IMAccount & account)
{
	char *PrclId = (char *)GaimIMPrcl::GetPrclId(account.getProtocol());

	GaimAccount *gAccount = gaim_account_new(account.getLogin().c_str(), PrclId);

	if (gAccount)
	{
		if (!account.getPassword().empty()) {
			gaim_account_set_password(gAccount, account.getPassword().c_str());
		}
		gaim_accounts_add(gAccount);
	}

	return gAccount;
}

bool GaimIMConnect::connectCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMConnect::_mutex);

	GaimIMConnect * imConnect = ((misc_t *)data)->instance;
	GaimAccount * gAccount = gaim_accounts_find(imConnect->getIMAccount().getLogin().c_str(),
		GaimIMPrcl::GetPrclId(imConnect->getIMAccount().getProtocol()));
	
	if (!gAccount) {
		if ((gAccount = (GaimAccount *)GaimIMConnect::CreateAccount(imConnect->getIMAccount()))) {
			GaimIMConnect::AddAccountParams(gAccount, imConnect->getIMAccount());
		} else {
			gaim_timeout_remove(((misc_t *)data)->cbk_id);
			free((misc_t *)data);

			return FALSE;
		}
	}

	std::string password = imConnect->getIMAccount().getPassword();
	if (!password.empty()) {
		gaim_account_set_password(gAccount, password.c_str());
	}

	// FIXME: must publish chosen status before connecting
	gaim_account_set_status(gAccount, "available", TRUE, NULL);
	gaim_account_set_enabled(gAccount, gaim_core_get_ui(), TRUE);

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free((misc_t *)data);

	return TRUE;
}

void GaimIMConnect::connect()
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));
	
	data->instance = this;

	Mutex::ScopedLock lock(GaimIMConnect::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMConnect::connectCbk, data);
}

bool GaimIMConnect::disconnectCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMConnect::_mutex);

	GaimIMConnect * imConnect = ((misc_t *)data)->instance;
	GaimAccount * gAccount = gaim_accounts_find(imConnect->getIMAccount().getLogin().c_str(),
		GaimIMPrcl::GetPrclId(imConnect->getIMAccount().getProtocol()));

	gaim_account_set_enabled(gAccount, gaim_core_get_ui(), FALSE);

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free((misc_t *)data);

	return TRUE;
}

void GaimIMConnect::disconnect(bool force)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;

	Mutex::ScopedLock lock(GaimIMConnect::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMConnect::disconnectCbk, data);
}
