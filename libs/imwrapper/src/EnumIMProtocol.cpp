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

#include <imwrapper/EnumIMProtocol.h>

#include <util/String.h>
#include <util/Logger.h>

#include <map>

typedef std::map<EnumIMProtocol::IMProtocol, std::string> ProtocolMap;
static ProtocolMap _protocolMap;

static void init() {
	_protocolMap[EnumIMProtocol::IMProtocolUnknown]   = "Unknown";
	_protocolMap[EnumIMProtocol::IMProtocolAll]		  = "All";

	_protocolMap[EnumIMProtocol::IMProtocolSIPSIMPLE] = "SIP/SIMPLE";
	_protocolMap[EnumIMProtocol::IMProtocolSIP]		  = "SIP";

	_protocolMap[EnumIMProtocol::IMProtocolMSN]		  = "MSN";
	_protocolMap[EnumIMProtocol::IMProtocolMYSPACE]   = "MySpace";
	_protocolMap[EnumIMProtocol::IMProtocolFacebook]  = "Facebook";
	_protocolMap[EnumIMProtocol::IMProtocolTwitter]	  = "Twitter";
	_protocolMap[EnumIMProtocol::IMProtocolSkype]	  = "Skype";
	_protocolMap[EnumIMProtocol::IMProtocolYahoo]	  = "Yahoo";
	_protocolMap[EnumIMProtocol::IMProtocolAIM]		  = "AIM";
	_protocolMap[EnumIMProtocol::IMProtocolICQ]		  = "ICQ";
	_protocolMap[EnumIMProtocol::IMProtocolJabber]	  = "Jabber";
	_protocolMap[EnumIMProtocol::IMProtocolWengo]	  = "VoxOx";
}

std::string EnumIMProtocol::toString(IMProtocol protocol) {
	init();
	std::string tmp = _protocolMap[protocol];
	if (tmp.empty()) {
		LOG_FATAL("unknown IMProtocol=" + String::fromNumber(protocol));
	}
	return tmp;
}

EnumIMProtocol::IMProtocol EnumIMProtocol::toIMProtocol(const std::string & protocol) {
	init();
	for (ProtocolMap::const_iterator it = _protocolMap.begin();
		it != _protocolMap.end();
		++it) {

		if ((*it).second == protocol) {
			return (*it).first;
		}
	}

	LOG_FATAL("unknown protocol=" + protocol);
	return IMProtocolUnknown;
}

//static
bool EnumIMProtocol::isSip( IMProtocol protocol )
{
	bool result = false;

	switch( protocol )
	{
	case EnumIMProtocol::IMProtocolSIPSIMPLE:
	case EnumIMProtocol::IMProtocolSIP:
	case EnumIMProtocol::IMProtocolWengo:
		result = true;
		break;

	default:
		result = false;
	}

	return result;
}
