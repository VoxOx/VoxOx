/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* Send PA data
* @author Chris Jimenez C 
* @date 2009.06.27
*/


#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "WsPAGet.h"



#include <model/history/History.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>

#include <tinyxml.h>

#include <util/Logger.h>

WsPAGet::WsPAGet(WengoAccount * wengoAccount, UserProfile & userProfile)
	: WengoWebService(wengoAccount),
	_userProfile(userProfile) {

	_did = "";
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	_did = wengoAccount->getIdentity();
	//Setup SMS web service
	setHostname(config.getApiHostname());
	setGet(true);
	setServicePath(config.getVoxOxPAWSPath());
	setWengoAuthentication(false);

#if 1
	setHttps(true);	//VOXOX - JRT - 2009.10.05 
	setPort(443);
#else
	setHttps(false);
	setPort(80);
#endif
}

int WsPAGet::sendInfo() {

	if(_did!=""){//VOXOX - CJC - 2009.07.29 
		String key = "";

		String did = _did;
		String method = "getSettings" ;
		String params = "key="+key+"&method="+method+"&did="+did+"&contact_id=&contact_type=";
		//params = String::encodeUrl(params);
		setParameters(params);

		//Call the web service
		int requestId = call(this);

		////History: create a History Memento for this outgoing SMS
		//HistoryMemento * memento = new HistoryMemento(
		//	HistoryMemento::OutgoingSmsNok, phoneNumber, requestId, message);
		//_userProfile.getHistory().addMemento(memento);

		return requestId;
	}else{
		LOG_WARN("DID IS NULL");
	}
	return 0;
}

void WsPAGet::answerReceived(const std::string & answer, int requestId) {

		
	std::string genderType = "";

	std::string nameText = "";

	std::string nameGender = "";

	PAData * data = new PAData();


	if (answer.empty()) {
		return;
	}


	TiXmlDocument doc;
	doc.Parse(answer.c_str());

	TiXmlHandle docHandle(&doc);

	TiXmlHandle root = docHandle.FirstChild("Um").FirstChild("getSettings");
	TiXmlText * genderTypeXml = root.FirstChild("pa_voice_type").FirstChild().Text();
	if (genderTypeXml) {
		genderType = genderTypeXml->Value();
	}

	TiXmlText * nameTextXml = root.FirstChild("name_text").FirstChild().Text();
	if (nameTextXml) {
		nameText = nameTextXml->Value();
	}
	
	TiXmlText * nameGenderXml = root.FirstChild("name_gender").FirstChild().Text();
	if (nameGenderXml) {
		nameGender = nameGenderXml->Value();
	}

	data->setFullName(nameText);
	data->setGender(nameGender);
	data->setVoiceGender(genderType);

	paDataEvent(data);



}
