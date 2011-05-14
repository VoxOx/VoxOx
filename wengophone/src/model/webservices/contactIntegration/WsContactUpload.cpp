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

#include "WsContactUpload.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/contactlist/Contact.h>
#include <model/account/wengo/WengoAccount.h>
#include <WengoPhoneBuildId.h>
#include <model/profile/UserProfile.h>

#include <cutil/global.h>
#include <util/Logger.h>

#include <tinyxml.h>

#include <sstream>

WsContactUpload::WsContactUpload(UserProfile & userProfile)
	: WengoWebService(userProfile.getWengoAccount()),
	_userProfile(userProfile) {

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	//Setup the web service
	setHostname(config.getWengoServerHostname());
	setGet(true);
	setServicePath(config.getVoxoxContactUploadPath());
	setWengoAuthentication(false);

#if 1
	setHttps(true);	//VOXOX - JRT - 2009.10.05 
	setPort(443);
#else
	setHttps(false);
	setPort(80);
#endif
}

void WsContactUpload::addContact(Contact & contact) {

	_contactList.push_back(&contact);

}


void WsContactUpload::sendInfo(const std::string & contactXml) {

	
	if(_wengoAccount){
		_xml="<request>";
		_xml+="<action>add</action>";
		_xml+="<user>"+_wengoAccount->getWengoLogin()+"</user>";
		_xml+="<password>"+_wengoAccount->getWengoPassword()+"</password>";
		//_xml+="<contact><uri>msurfer_@hotmail.com</uri><network>MSN.voxox.com</network></contact>";

	
		_xml+= contactXml;

		_xml+="</request>";	

		
		setParameters(std::string("data=") + _xml);

		//Calls the web service
		call(this);
	}
}

void WsContactUpload::answerReceived(const std::string & answer, int requestId) {
	if (answer.empty()) {
		return;
	}

	/*
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

	contactInfoEvent(*this, contactNumber);*/

}
