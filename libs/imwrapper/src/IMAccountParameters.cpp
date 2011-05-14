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

#include <imwrapper/IMAccountParameters.h>

#include <util/Logger.h>

using namespace std;

const string IMAccountParameters::MAIL_NOTIFICATION_KEY		= "common.mail_notification";
const string IMAccountParameters::REMEMBER_PASSWORD_KEY		= "common.remember_password";
const string IMAccountParameters::USE_HTTP_PROXY_KEY		= "common.use_http_proxy";
const string IMAccountParameters::HTTP_PROXY_SERVER_KEY		= "common.http_proxy_server";
const string IMAccountParameters::HTTP_PROXY_PORT_KEY		= "common.http_proxy_port";
const string IMAccountParameters::HTTP_PROXY_LOGIN_KEY		= "common.http_proxy_login";
const string IMAccountParameters::HTTP_PROXY_PASSWORD_KEY	= "common.http_proxy_password";

const string IMAccountParameters::YAHOO_IS_JAPAN_KEY		= "yahoo.yahoojp";
const string IMAccountParameters::YAHOO_SERVER_KEY			= "yahoo.server";
const string IMAccountParameters::YAHOO_JAPAN_SERVER_KEY	= "yahoo.serverjp";
const string IMAccountParameters::YAHOO_PORT_KEY			= "yahoo.port";
const string IMAccountParameters::YAHOO_XFER_HOST_KEY		= "yahoo.xfer_host";
const string IMAccountParameters::YAHOO_JAPAN_XFER_HOST_KEY = "yahoo.xferjp_host";
const string IMAccountParameters::YAHOO_XFER_PORT_KEY		= "yahoo.xfer_port";
const string IMAccountParameters::YAHOO_ROOM_LIST_LOCALE_KEY = "yahoo.room_list_locale";

const string IMAccountParameters::MSN_SERVER_KEY			= "msn.server";
const string IMAccountParameters::MSN_PORT_KEY				= "msn.port";
const string IMAccountParameters::MSN_HTTP_SERVER_KEY		= "msn.http_method_server";
const string IMAccountParameters::MSN_USE_HTTP_KEY			= "msn.http_method";

const string IMAccountParameters::MYSPACE_SERVER_KEY		= "myspace.server";
const string IMAccountParameters::MYSPACE_PORT_KEY			= "myspace.port";

const string IMAccountParameters::OSCAR_SERVER_KEY			= "oscar.server";
const string IMAccountParameters::OSCAR_PORT_KEY			= "oscar.port";
const string IMAccountParameters::OSCAR_ENCODING_KEY		= "oscar.encoding";

const string IMAccountParameters::JABBER_SERVER_KEY					= "jabber.server";
const string IMAccountParameters::JABBER_PORT_KEY					= "jabber.port";
const string IMAccountParameters::JABBER_RESOURCE_KEY				= "jabber.resource";
const string IMAccountParameters::JABBER_USE_TLS_KEY				= "jabber.use_tls";
const string IMAccountParameters::JABBER_REQUIRE_TLS_KEY			= "jabber.require_tls";
const string IMAccountParameters::JABBER_USE_OLD_SSL_KEY			= "jabber.old_ssl";
const string IMAccountParameters::JABBER_AUTH_PLAIN_IN_CLEAR_KEY	= "jabber.auth_plain_in_clear";
const string IMAccountParameters::JABBER_CONNECTION_SERVER_KEY		= "jabber.connect_server";

//VOXOX - JRT - 2009.05.27 
const int	  IMAccountParameters::GOOGLETALK_PORT   = 80;
const string  IMAccountParameters::GOOGLETALK_SERVER = "talk.google.com";

IMAccountParameters::IMAccountParameters() : CascadingSettings() {
}

IMAccountParameters::IMAccountParameters(const IMAccountParameters & imAccountParameters)
: CascadingSettings() {
	copy(imAccountParameters);
}

IMAccountParameters & IMAccountParameters::operator = (const IMAccountParameters & imAccountParameters) {
	copy(imAccountParameters);
	return *this;
}

void IMAccountParameters::copy(const IMAccountParameters & imAccountParameters) {
	CascadingSettings::copy(imAccountParameters);
	//_keyDefaultValueMap = imAccountParameters._keyDefaultValueMap;
}

IMAccountParameters::~IMAccountParameters() {
}

bool IMAccountParameters::isMailNotified() const {
	return getBooleanKeyValue(MAIL_NOTIFICATION_KEY);
}

bool IMAccountParameters::isPasswordRemembered() const {
	return getBooleanKeyValue(REMEMBER_PASSWORD_KEY);
}

bool IMAccountParameters::isHttpProxyUsed() const {
	return getBooleanKeyValue(USE_HTTP_PROXY_KEY);
}

string IMAccountParameters::getHttpProxyServer() const {
	return getStringKeyValue(HTTP_PROXY_SERVER_KEY);
}

int IMAccountParameters::getHttpProxyPort() const {
	return getIntegerKeyValue(HTTP_PROXY_PORT_KEY);
}

string IMAccountParameters::getHttpProxyLogin() const {
	return getStringKeyValue(HTTP_PROXY_LOGIN_KEY);
}

string IMAccountParameters::getHttpProxyPassword() const {
	return getStringKeyValue(HTTP_PROXY_PASSWORD_KEY);
}

bool IMAccountParameters::isYahooJapan() const {
	return getBooleanKeyValue(YAHOO_IS_JAPAN_KEY);
}

string IMAccountParameters::getYahooServer() const {
	return getStringKeyValue(YAHOO_SERVER_KEY);
}

string IMAccountParameters::getYahooJapanServer() const {
	return getStringKeyValue(YAHOO_JAPAN_SERVER_KEY);
}

int IMAccountParameters::getYahooServerPort() const {
	return getIntegerKeyValue(YAHOO_PORT_KEY);
}

string IMAccountParameters::getYahooXferHost() const {
	return getStringKeyValue(YAHOO_XFER_HOST_KEY);
}

string IMAccountParameters::getYahooJapanXferHost() const {
	return getStringKeyValue(YAHOO_JAPAN_XFER_HOST_KEY);
}

int IMAccountParameters::getYahooXferPort() const {
	return getIntegerKeyValue(YAHOO_XFER_PORT_KEY);
}

string IMAccountParameters::getYahooRoomListLocale() const {
	return getStringKeyValue(YAHOO_ROOM_LIST_LOCALE_KEY);
}

string IMAccountParameters::getMSNServer() const {
	return getStringKeyValue(MSN_SERVER_KEY);
}

int IMAccountParameters::getMSNServerPort() const {
	return getIntegerKeyValue(MSN_PORT_KEY);
}

string IMAccountParameters::getMSNHttpServer() const {
	return getStringKeyValue(MSN_HTTP_SERVER_KEY);
}

bool IMAccountParameters::isMSNHttpUsed() const {
	return getBooleanKeyValue(MSN_USE_HTTP_KEY);
}

string IMAccountParameters::getMySpaceServer() const {
	return getStringKeyValue(MYSPACE_SERVER_KEY);
}

int IMAccountParameters::getMySpaceServerPort() const {
	return getIntegerKeyValue(MYSPACE_PORT_KEY);
}


string IMAccountParameters::getOscarServer() const {
	return getStringKeyValue(OSCAR_SERVER_KEY);
}

int IMAccountParameters::getOscarServerPort() const {
	return getIntegerKeyValue(OSCAR_PORT_KEY);
}

string IMAccountParameters::getOscarEncoding() const {
	return getStringKeyValue(OSCAR_ENCODING_KEY);
}

string IMAccountParameters::getJabberServer() const {
	return getStringKeyValue(JABBER_SERVER_KEY);
}

int IMAccountParameters::getJabberServerPort() const {
	return getIntegerKeyValue(JABBER_PORT_KEY);
}

string IMAccountParameters::getJabberResource() const {
	return getStringKeyValue(JABBER_RESOURCE_KEY);
}

bool IMAccountParameters::isJabberTLSUsed() const {
	return getBooleanKeyValue(JABBER_USE_TLS_KEY);
}

bool IMAccountParameters::isJabberTLSRequired() const {
	return getBooleanKeyValue(JABBER_REQUIRE_TLS_KEY);
}

bool IMAccountParameters::isJabberOldSSLUsed() const {
	return getBooleanKeyValue(JABBER_USE_OLD_SSL_KEY);
}

bool IMAccountParameters::isJabberAuthPlainInClearUsed() const {
	return getBooleanKeyValue(JABBER_AUTH_PLAIN_IN_CLEAR_KEY);
}

string IMAccountParameters::getJabberConnectionServer() const {
	return getStringKeyValue(JABBER_CONNECTION_SERVER_KEY);
}

//-----------------------------------------------------------------------------

bool IMAccountParameters::setByProtocol( EnumIMProtocol::IMProtocol protocol, bool isGtalk, const std::string& server )	//VOXOX - JRT - 2009.05.27 
{
	bool bSet = false;

	switch(protocol)
	{
	case EnumIMProtocol::IMProtocolMSN:
		set(IMAccountParameters::MSN_USE_HTTP_KEY, false);
		bSet = true;
		break;

	case EnumIMProtocol::IMProtocolYahoo:
		set(IMAccountParameters::YAHOO_IS_JAPAN_KEY, false);
		set(IMAccountParameters::YAHOO_PORT_KEY, 23);
		bSet = true;
		break;

	case EnumIMProtocol::IMProtocolAIM:
		set(IMAccountParameters::OSCAR_PORT_KEY, 443);
		bSet = true;
		break;

	case EnumIMProtocol::IMProtocolICQ:
		set(IMAccountParameters::OSCAR_PORT_KEY, 443);
		bSet = true;
		break;

	case EnumIMProtocol::IMProtocolJabber:
		if(isGtalk)
		{
			set(IMAccountParameters::JABBER_USE_TLS_KEY,			true);
			set(IMAccountParameters::JABBER_CONNECTION_SERVER_KEY,	GOOGLETALK_SERVER);
			set(IMAccountParameters::JABBER_PORT_KEY,				GOOGLETALK_PORT);

			bSet = true;
		}
		else
		{
//			assert( !server.empty() );
			set(IMAccountParameters::JABBER_CONNECTION_SERVER_KEY, server);
			bSet = true;
		}
		break;

	case EnumIMProtocol::IMProtocolFacebook:
	case EnumIMProtocol::IMProtocolMYSPACE:
	case EnumIMProtocol::IMProtocolTwitter:
	case EnumIMProtocol::IMProtocolSkype:
		break;

	default:
		assert(false);		//New protocol?
	}

	return bSet;
}

//-----------------------------------------------------------------------------

