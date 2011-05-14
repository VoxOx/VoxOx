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

#ifndef OWIMACCOUNTPARAMETER_H
#define OWIMACCOUNTPARAMETER_H

#include "EnumIMProtocol.h"
#include <settings/CascadingSettings.h>

#include <util/String.h>

/**
 * Contains specific parameters needed by some protocols for connection.
 *
 * @author Philippe Bernery
 */
class IMAccountParameters : public CascadingSettings {
public:

	IMAccountParameters();

	IMAccountParameters(const IMAccountParameters & imAccountParameters);

	IMAccountParameters & operator=(const IMAccountParameters & imAccountParameters);

	virtual ~IMAccountParameters();

	/**
	 * @name Common parameters
	 * @{
	 */
	static const std::string MAIL_NOTIFICATION_KEY;
	bool isMailNotified() const;

	static const std::string REMEMBER_PASSWORD_KEY;
	bool isPasswordRemembered() const;

	static const std::string USE_HTTP_PROXY_KEY;
	bool isHttpProxyUsed() const;

	static const std::string HTTP_PROXY_SERVER_KEY;
	std::string getHttpProxyServer() const;

	static const std::string HTTP_PROXY_PORT_KEY;
	int getHttpProxyPort() const;

	static const std::string HTTP_PROXY_LOGIN_KEY;
	std::string getHttpProxyLogin() const;

	static const std::string HTTP_PROXY_PASSWORD_KEY;
	std::string getHttpProxyPassword() const;
	/** @} */

	/**
	 * @name Yahoo specific parameters
	 * @{
	 */
	static const std::string YAHOO_IS_JAPAN_KEY;
	bool isYahooJapan() const;

	static const std::string YAHOO_SERVER_KEY;
	std::string getYahooServer() const;

	static const std::string YAHOO_JAPAN_SERVER_KEY;
	std::string getYahooJapanServer() const;

	static const std::string YAHOO_PORT_KEY;
	int getYahooServerPort() const;

	static const std::string YAHOO_XFER_HOST_KEY;
	std::string getYahooXferHost() const;

	static const std::string YAHOO_JAPAN_XFER_HOST_KEY;
	std::string getYahooJapanXferHost() const;

	static const std::string YAHOO_XFER_PORT_KEY;
	int getYahooXferPort() const;

	static const std::string YAHOO_ROOM_LIST_LOCALE_KEY;
	std::string getYahooRoomListLocale() const;
	/** @} */

	/**
	 * @name MSN specific parameters
	 * @{
	 */
	static const std::string MSN_SERVER_KEY;
	std::string getMSNServer() const;

	static const std::string MSN_PORT_KEY;
	int getMSNServerPort() const;

	static const std::string MSN_HTTP_SERVER_KEY;
	std::string getMSNHttpServer() const;

	static const std::string MSN_USE_HTTP_KEY;
	bool isMSNHttpUsed() const;

	/** @} */

	/**
	 * @name MySpaceIM specific parameters
	 * @{
	 */

	static const std::string MYSPACE_SERVER_KEY;
	std::string getMySpaceServer() const;

	static const std::string MYSPACE_PORT_KEY;
	int getMySpaceServerPort() const;



	/** @} */

	/**
	 * @name Oscar (AIM/ICQ) specific parameters
	 * @{
	 */
	static const std::string OSCAR_SERVER_KEY;
	std::string getOscarServer() const;

	static const std::string OSCAR_PORT_KEY;
	int getOscarServerPort() const;

	static const std::string OSCAR_ENCODING_KEY;
	std::string getOscarEncoding() const;
	/** @} */

	/**
	 * @name Jabber specific parameters
	 * @{
	 */
	static const std::string JABBER_SERVER_KEY;
	std::string getJabberServer() const;

	static const std::string JABBER_PORT_KEY;
	int getJabberServerPort() const;

	static const std::string JABBER_RESOURCE_KEY;
	std::string getJabberResource() const;

	static const std::string JABBER_USE_TLS_KEY;
	bool isJabberTLSUsed() const;

	static const std::string JABBER_REQUIRE_TLS_KEY;
	bool isJabberTLSRequired() const;

	static const std::string JABBER_USE_OLD_SSL_KEY;
	bool isJabberOldSSLUsed() const;

	static const std::string JABBER_AUTH_PLAIN_IN_CLEAR_KEY;
	bool isJabberAuthPlainInClearUsed() const;

	static const std::string JABBER_CONNECTION_SERVER_KEY;
	std::string getJabberConnectionServer() const;
	/** @} */

	bool setByProtocol( EnumIMProtocol::IMProtocol protocol, bool isGtalk, const std::string& server );	//VOXOX - JRT - 2009.05.27 

	//VOXOX - JRT - 2009.05.27 
	static const int GOOGLETALK_PORT;
	static const std::string GOOGLETALK_SERVER;

private:
	virtual void copy(const IMAccountParameters & imAccountParameters);
};

#endif	//OWIMACCOUNTPARAMETER_H
