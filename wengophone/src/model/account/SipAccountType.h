/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef SIPACCOUNTTYPE_H
#define SIPACCOUNTTYPE_H

#include "SipAccount.h"

class SipAccountType : public SipAccount {
public:

	virtual ~SipAccountType() { }

	virtual void init() = 0;

	virtual const std::string & getIdentity() const { return _identity; }

	virtual const std::string & getUsername() const { return _username; }

	virtual const std::string & getRealm() const { return _realm; }

	virtual const std::string & getDisplayName() const { return _displayName; }

	virtual const std::string & getRegisterServerHostname() const { return _registerServerHostname; }

	virtual unsigned getRegisterServerPort() const { return _registerServerPort; }

	virtual const std::string & getProxyServerHostname() const { return _proxyServerHostname; }

	virtual unsigned getProxyServerPort() const { return _proxyServerPort; }

	virtual const std::string & getPassword() const { return _password; }

	virtual const std::string & getTunnelServerHostname() const { return _tunnelServerHostname; }

	virtual unsigned getTunnelServerPort() const { return _tunnelServerPort; }

protected:
	std::string _identity;
	std::string _username;
	std::string _realm;
	std::string _displayName;
	std::string _registerServerHostname;
	unsigned _registerServerPort;
	std::string _proxyServerHostname;
	unsigned _proxyServerPort;
	std::string _password;
	std::string _tunnelServerHostname;
	unsigned _tunnelServerPort;
};

#endif	//SIPACCOUNTTYPE_H
