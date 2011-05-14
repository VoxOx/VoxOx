/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#include "WengoAccountXMLSerializer.h"

#include <model/account/wengo/WengoAccount.h>

#include <util/Base64.h>
#include <util/Logger.h>
#include <util/String.h>

#include <tinyxml.h>

WengoAccountXMLSerializer::WengoAccountXMLSerializer(WengoAccount & wengoAccount)
	: _wengoAccount(wengoAccount) {
}

std::string WengoAccountXMLSerializer::serialize() {
	std::string result;

	result += "<wengoaccount>\n";

	result += "<login>" + _wengoAccount.getWengoLogin() + "</login>\n";

	//result += "<password>" + Base64::encode(_wengoAccount.getWengoPassword()) + "</password>\n";
	
	//VOXOX CHANGE
	//CJC
	// We dont need encription for now
	//TODO:Add encription
	result += "<password>" + _wengoAccount.getWengoPassword() + "</password>\n";

	result += "<rememberp>" + String::fromBoolean(_wengoAccount.isPasswordRemembered()) + "</rememberp>\n";

	result += "<rememberu>" + String::fromBoolean(_wengoAccount.isUsernameRemembered()) + "</rememberu>\n";

	result += "<loginautomatically>" + String::fromBoolean(_wengoAccount.isAutoLoginVoxOx()) + "</loginautomatically>\n";
	result += "<voicemail>" + _wengoAccount.getVoicemailNumber() + "</voicemail>\n";
	
	result += "</wengoaccount>\n";

	return result;
}

bool WengoAccountXMLSerializer::unserialize(const std::string & data) {
	TiXmlDocument doc;

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle wengoaccount = docHandle.FirstChild("wengoaccount");

	// Retrieving login
	TiXmlNode * login = wengoaccount.FirstChild("login").Node();
	if (login && login->FirstChild()) {
		_wengoAccount._wengoLogin = login->FirstChild()->Value();
	} else {
		LOG_ERROR("Invalid WengoAccount: no login set");
		return false;
	}
	////

	// Retrieving password
	TiXmlNode * password = wengoaccount.FirstChild("password").Node();
	if (password && password->FirstChild()) {
		//_wengoAccount._wengoPassword = Base64::decode(password->FirstChild()->Value());
		//VOXOX CHANGE
		//CJC
		// We dont need encription for now
		//TODO:Add encription
		_wengoAccount._wengoPassword = password->FirstChild()->Value();
	}
	////

	// Retrieving autologin
	TiXmlNode * rememberpassword = wengoaccount.FirstChild("rememberp").Node();
	if (rememberpassword && rememberpassword->FirstChild()) {
		String rememberpasswordStr = rememberpassword->FirstChild()->Value();
		_wengoAccount._rememberPassword = rememberpasswordStr.toBoolean();
	}
	////

	TiXmlNode * rememberusername = wengoaccount.FirstChild("rememberu").Node();
	if (rememberusername && rememberusername->FirstChild()) {
		String rememberusernameStr = rememberusername->FirstChild()->Value();
		_wengoAccount._rememberUsername = rememberusernameStr.toBoolean();
	}
	////

	TiXmlNode * loginautomatically = wengoaccount.FirstChild("loginautomatically").Node();
	if (loginautomatically && loginautomatically->FirstChild()) {
		String loginAutomaticallyStr = loginautomatically->FirstChild()->Value();
		_wengoAccount._autoLoginVoxOx = loginAutomaticallyStr.toBoolean();
	}

	// Retrieving voicemail number
	TiXmlNode * voicemail = wengoaccount.FirstChild("voicemail").FirstChild().Node();
	if (voicemail) {
		_wengoAccount.setVoicemailNumber(voicemail->Value());
	}
	////
	
	return true;
}
