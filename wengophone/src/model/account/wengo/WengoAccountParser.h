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

#ifndef OWWENGOACCOUNTPARSER_H
#define OWWENGOACCOUNTPARSER_H

#include "WengoAccount.h"

#include <string>

class StringList;

/**
 * Parses the XML datas from the Single Sign On authentication on the Wengo platform (SSO).
 *
 * This a separated class rather than to be inside the class WengoAccount,
 * this allows more modularity and encapsulates the use of TinyXML.
 *
 * XML scheme
 *
 * http://ws.wengo.fr/softphone-sso/sso.php?login=LOGIN&password=PASSWORD
 *
 * Replace LOGIN and PASSWORD by the login/password provided by Wengo.
 *
 * If the login/password are wrong it will return:
 * <pre>
 * <?xml version="1.0" ?>
 * <sso vapi="1">
 *     <status code="401">Unauthorized</status>
 * </sso>
 * </pre>
 *
 * If the login/password are correct it will return:
 * <pre>
 * <?xml version="1.0" encoding="UTF-8" ?>
 * <sso mib="1">
 * 	<notice><![CDATA[DEPRECATED on 12/01/2006]]></notice>
 * 	<status code="200"><![CDATA[OK]]></status>
 * 	<d k="sip.auth.userid" v="toto"/>
 * 	<d k="sip.auth.password" v="secret"/>
 * 	<d k="sip.auth.realm" v="192.168.1.1"/>
 * 	<d k="sip.address.name" v="toto"/>
 * 	<d k="sip.address.displayname" v="pseudo"/>
 * 	<d k="sip.address.server.host" v="192.168.1.1"/>
 * 	<d k="sip.address.server.port" v="5060"/>
 * 	<d k="sip.outbound" v="1"/>
 * 	<d k="sip.outbound.proxy.host" v="192.168.1.1"/>
 * 	<d k="sip.outbound.proxy.port" v="5060"/>
 * 	<d k="netlib.stun.host" v="192.168.1.1"/>
 * 	<d k="netlib.stun.port" v="5060"/>
 *	<d k="netlib.tunnel.http">
 *		<l v="80.118.99.31"/>
 *		<l v="80.118.99.32"/>
 *	</d>
 *	<d k="netlib.tunnel.https">
 *		<l v="80.118.99.51"/>
 *		<l v="80.118.99.52"/>
 *		<l v="80.118.99.53"/>
 *		<l v="80.118.99.54"/>
 *	</d>
 * </sso>
 * </pre>
 *
 * @ingroup model
 * @author Mathieu Stute
 * @author Tanguy Krotoff
 */
class WengoAccountParser {
public:

	WengoAccountParser(WengoAccount & account, const std::string & data);

	/**
	 * Check if password is correct
	 *
	 * @return true if login ok
	 */
	bool isLoginPasswordOk() const {
		return _loginPasswordOk;
	}

	std::string getStatusCode(){
		return _statusCode;
	}

	//VOXOX CHANGE by Rolando - 2009.09.10 
	std::string getLoginErrorMessage(){
		return _loginErrorMessage;
	}
private:

	/**
	 * Choose a random http tunnel
	 *
	 * @param httpTunnels a list of http tunnel ip
	 * @return a http tunnel ip
	 */
	std::string chooseHttpTunnel(const StringList & httpTunnels);

	std::string createEncyptionKey( const std::string& userID, const std::string& did );	//VOXOX - JRT - 2009.09.28 

	bool _loginPasswordOk;
	std::string _statusCode;//VOXOX CHANGE by Rolando - 2009.09.10 
	std::string _loginErrorMessage;//VOXOX CHANGE by Rolando - 2009.09.10 
};

#endif	//OWWENGOACCOUNTPARSER_H
