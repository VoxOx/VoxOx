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
#include "SipAddress.h"

#include <util/Logger.h>

using namespace std;

SipAddress::SipAddress(const std::string & rawSipAddress) {
	_rawSipAddress = rawSipAddress;
	parseSipAddress(rawSipAddress);
}

SipAddress::~SipAddress() {
}

void SipAddress::parseSipAddress(const std::string & rawSipAddress) {
	static const string LOWER = "<";
	static const string UPPER = ">";
	static const string SIP = "sip:";
	static const string AT = "@";
	static const string BACKSLASH = "\"";
	static const string SPACE = " ";

	//phApi from = "jwagner_office" <sip:0170187873@192.168.70.20;user=phone>;tag=00-01430

	String tmp(rawSipAddress);
	_sipAddress = rawSipAddress;
	if (!tmp.empty()) {
		string::size_type posSipAddressBegin = tmp.find(LOWER);
		if (posSipAddressBegin != string::npos) {
			posSipAddressBegin += LOWER.length();
			string::size_type posSipAddressEnd = tmp.find(UPPER);
			if (posSipAddressEnd != string::npos) {
				_sipAddress = tmp.substr(posSipAddressBegin, posSipAddressEnd - posSipAddressBegin);
			}
		}
	}
	//_sipAddress = sip:0170187873@192.168.70.20;user=phone

	tmp = rawSipAddress;
	_userName = rawSipAddress;
	if (!tmp.empty()) {
		string::size_type posUserNameBegin = tmp.find(SIP);
		if (posUserNameBegin != string::npos) {
			posUserNameBegin += SIP.length();
			string::size_type posUserNameEnd = tmp.find(AT);
			if (posUserNameEnd != string::npos) {
				_userName = tmp.substr(posUserNameBegin, posUserNameEnd - posUserNameBegin);
			}
		}
	}
	//_userName = 0170187873

	tmp = rawSipAddress;
	_displayName = "";
	if (!tmp.empty()) {
		string::size_type posDisplayNameEnd = tmp.find(LOWER + SIP);
		if (posDisplayNameEnd != string::npos) {
			_displayName = tmp.substr(0, posDisplayNameEnd - 0);
			_displayName.remove(BACKSLASH);
			_displayName.remove(SPACE);
		}
	}
	//_displayName = jwagner_office
}

std::string SipAddress::toString() const {
	if (!getDisplayName().empty()) {
		return getDisplayName();
	}

	return getUserName();
}

SipAddress SipAddress::fromString(const std::string & str, const std::string & realm) {
	/*if (str.empty()) {
		return -1;
	}*/

	String sipUri(str);
	size_t length = strspn(sipUri.c_str(), " .,;:()[]{}-_/#+0123456789");
	if (length == sipUri.length()) {
		//sipUri is a real phone number not a SIP URI or a pseudo

		sipUri.remove(" ");
		sipUri.remove(".");
		sipUri.remove(",");
		sipUri.remove(";");
		sipUri.remove(":");
		sipUri.remove("(");
		sipUri.remove(")");
		sipUri.remove("[");
		sipUri.remove("]");
		sipUri.remove("{");
		sipUri.remove("}");
		sipUri.remove("-");
		sipUri.remove("_");
		sipUri.remove("/");
		sipUri.remove("#");

		/* //Replaces + by 00
		sipUri.replace("+", "00");*/
		//VOXOX CHANGE by Rolando 01-15-09, setting prefix to calls outside of USA
		//sipUri.replace("+", "011");//VOXOX CHANGE by Rolando - 2009.09.17 - commented to able calls to USA with prefix +1

		if (sipUri.beginsWith("+") && !sipUri.beginsWith("+1")) {//VOXOX CHANGE by Rolando - 2009.09.17  - fix to complete task "CHANGE: Pass +1 for USA calls"
			sipUri.replace("+", "011");//VOXOX CHANGE by Rolando - 2009.09.17 - fix to complete task "CHANGE: Pass +1 for USA calls"
		}
	}

	if (!sipUri.beginsWith("sip:")) {
		sipUri = "sip:" + sipUri;
	}

	if (!sipUri.contains("@")) {
		//Not a SIP URI
		sipUri += "@" + realm;
	}

	return SipAddress(sipUri);
}
