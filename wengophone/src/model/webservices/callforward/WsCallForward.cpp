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
#include "WsCallForward.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <util/Logger.h>

#include <tinyxml.h>

WsCallForward::WsCallForward(WengoAccount * wengoAccount) : WengoWebService(wengoAccount) {

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	//setup call forward service
	setHostname(config.getWengoServerHostname());
	setGet(true);
	setHttps(true);
	setServicePath(config.getWengoCirpackPath());
	setPort(443);
	setWengoAuthentication(true);
}

void WsCallForward::disableCallForward() {

	std::string query = "action=callforward";
	query += "&activate=0";

	setParameters(query);
	call(this);
}

void WsCallForward::forwardToVoiceMail() {

	std::string query = "action=callforward";
	query += "&activate=1";
	query += "&num1=voicemail";

	setParameters(query);
	call(this);
}

void WsCallForward::forwardToNumber(const std::string & number1, const std::string & number2, const std::string & number3) {

	std::string query = "action=callforward";
	query += "&activate=1";

	query += "&num1=" + number1;
	query += "&num2=" + number2;
	query += "&num3=" + number3;

	setParameters(query);
	call(this);
}

void WsCallForward::answerReceived(const std::string & answer, int requestId) {

	TiXmlDocument doc;
	doc.Parse(answer.c_str());

	TiXmlHandle docHandle(& doc);

	TiXmlElement * response = docHandle.FirstChild("response").Element();
	if (!response) {
		wsCallForwardEvent(*this, requestId, WsCallForwardStatusUrlError);
		return;
	}

	TiXmlElement * elt = response->FirstChildElement("code");
	if (elt && (elt->FirstChild())) {

		std::string r = elt->FirstChild()->ToText()->Value();

		//TODO: change when the ws doc will be updated
		if (r == "OK") {
			wsCallForwardEvent(*this, requestId, WsCallForwardStatusOk);
		} else if (r == "401") {
			wsCallForwardEvent(*this, requestId, WsCallForwardStatusAuthenticationError);
		} else if (r == "404") {
			wsCallForwardEvent(*this, requestId, WsCallForwardStatusUrlError);
		} else {
		}
	}
}
