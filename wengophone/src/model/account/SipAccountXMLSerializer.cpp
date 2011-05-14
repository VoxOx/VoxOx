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
#include "SipAccountXMLSerializer.h"

#include <model/account/wengo/WengoAccount.h>

#include <util/Base64.h>
#include <util/Logger.h>
#include <util/String.h>

#include <tinyxml.h>

SipAccountXMLSerializer::SipAccountXMLSerializer(SipAccount & sipAccount) 
	: _sipAccount(sipAccount) {
}

std::string SipAccountXMLSerializer::serialize() {
	std::string result;

	result += "<sipaccount>\n";
	result += "<visiblename>" + _sipAccount.getVisibleName() + "</visiblename>\n";
	result += "<identity>" + _sipAccount.getIdentity() + "</identity>\n";
	result += "<username>" + _sipAccount.getUsername() + "</username>\n";
	result += "<displayname>" + _sipAccount.getDisplayName() + "</displayname>\n";
	result += "<password>" + Base64::encode(_sipAccount.getPassword()) + "</password>\n";
	result += "<realm>" + _sipAccount.getRealm() + "</realm>\n";
	result += "<registerserver>" + _sipAccount.getRegisterServerHostname() + "</registerserver>\n";
	result += "<registerport>" + String::fromNumber(_sipAccount.getRegisterServerPort()) + "</registerport>\n";
	result += "<sipproxyserver>" + _sipAccount.getSIPProxyServerHostname() + "</sipproxyserver>\n";
	result += "<sipproxyport>" + String::fromNumber(_sipAccount.getSIPProxyServerPort()) + "</sipproxyport>\n";
	result += "<voicemail>" + _sipAccount.getVoicemailNumber() + "</voicemail>\n";
	result += "<pim>" + String::fromBoolean(_sipAccount.isPIMEnabled()) + "</pim>\n";
	result += "</sipaccount>\n";
	
	return result;
}

bool SipAccountXMLSerializer::unserialize(const std::string & data) {
	
	TiXmlDocument doc;
	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle sipaccount = docHandle.FirstChild("sipaccount");

	// Retrieving visible name
	TiXmlNode * visiblename = sipaccount.FirstChild("visiblename").FirstChild().Node();
	if (visiblename) {
		_sipAccount.setVisibleName(visiblename->Value());
	} else {
		LOG_ERROR("Invalid WengoAccount: no visiblename set");
		return false;
	}
	////

	// Retrieving identity
	TiXmlNode * identity = sipaccount.FirstChild("identity").FirstChild().Node();
	if (identity) {
		_sipAccount.setIdentity(identity->Value());
	} else {
		LOG_ERROR("Invalid WengoAccount: no identity set");
		return false;
	}
	////

	// Retrieving username
	TiXmlNode * username = sipaccount.FirstChild("username").FirstChild().Node();
	if (username) {
		_sipAccount.setUsername(username->Value());
	}
	////

	// Retrieving displayname
	TiXmlNode * displayname = sipaccount.FirstChild("displayname").FirstChild().Node();
	if (displayname) {
		_sipAccount.setDisplayName(displayname->Value());
	}
	////

	// Retrieving password
	TiXmlNode * password = sipaccount.FirstChild("password").FirstChild().Node();
	if (password) {
		_sipAccount.setPassword(Base64::decode(password->Value()));
		//_sipAccount.setPassword(password->Value());
	}
	////

	// Retrieving realm
	TiXmlNode * realm = sipaccount.FirstChild("realm").FirstChild().Node();
	if (realm) {
		_sipAccount.setRealm(realm->Value());
	}
	////

	// Retrieving registerserver
	TiXmlNode * registerserver = sipaccount.FirstChild("registerserver").FirstChild().Node();
	if (registerserver) {
		_sipAccount.setRegisterServerHostname(registerserver->Value());
	}
	////

	// Retrieving registerport
	TiXmlNode * registerport = sipaccount.FirstChild("registerport").FirstChild().Node();
	if (registerport) {
		String sSipRegisterServerPort = registerport->Value();
		_sipAccount.setRegisterServerPort(sSipRegisterServerPort.toInteger());
	}
	////

	// Retrieving sipproxyserver
	TiXmlNode * sipproxyserver = sipaccount.FirstChild("sipproxyserver").FirstChild().Node();
	if (sipproxyserver) {
		_sipAccount.setSIPProxyServerHostname(sipproxyserver->Value());
	}
	////

	// Retrieving sipproxyport
	TiXmlNode * sipproxyport = sipaccount.FirstChild("sipproxyport").FirstChild().Node();
	if (sipproxyport) {
		String sSipProxyServerPort = sipproxyport->Value();
		_sipAccount.setSIPProxyServerPort(sSipProxyServerPort.toInteger());
	}
	////

	// Retrieving voicemail number
	TiXmlNode * voicemail = sipaccount.FirstChild("voicemail").FirstChild().Node();
	if (voicemail) {
		_sipAccount.setVoicemailNumber(voicemail->Value());
	}
	////

	// Retrieving pim
	TiXmlNode * pim = sipaccount.FirstChild("pim").FirstChild().Node();
	if (pim) {
		String validStr = pim->Value();
		_sipAccount.enablePIM(validStr.toBoolean());
	}
	////

	return true;
}
