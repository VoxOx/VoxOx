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
#include "WsContactNumber.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <cutil/global.h>
#include <util/Logger.h>

#include <tinyxml.h>

#include <sstream>

WsContactNumber::WsContactNumber() {

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	//Setup the web service
	setHostname(config.getWengoServerHostname());
	setGet(true);
	setServicePath(config.getVoxoxContactNumberPath());
	setWengoAuthentication(false);

#if 1
	setHttps(true);	//VOXOX - JRT - 2009.10.05 
	setPort(443);
#else
	setHttps(false);
	setPort(80);
#endif
}

void WsContactNumber::setContactId(const std::string pContactId) {
	_contactId = pContactId;
}


void WsContactNumber::execute() {
	
	setParameters(std::string("jid=") + _contactId);

	//Calls the web service
	call(this);
}

void WsContactNumber::answerReceived(const std::string & answer, int requestId) {
	if (answer.empty()) {
		return;
	}

	
	std::string contactNumber;


	TiXmlDocument doc;
	doc.Parse(answer.c_str());

	TiXmlHandle docHandle(&doc);

	TiXmlHandle root = docHandle.FirstChild("contact").FirstChild("portainfo");
	TiXmlText * text = root.FirstChild("number").FirstChild().Text();
	if (text) {
		contactNumber = text->Value();
	}else{
		contactNumber = "noNumber";
	}

	contactNumberEvent(*this, contactNumber, _contactId);

}
