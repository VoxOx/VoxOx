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
#include "WsPA.h"

#include <model/history/History.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>
#include <model/webservices/info/WsInfo.h>

#include <util/Logger.h>

WsPA::WsPA(WengoAccount * wengoAccount, UserProfile & userProfile)
	: WengoWebService(wengoAccount),
	_userProfile(userProfile) {

	_did = "";
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	_did = wengoAccount->getIdentity();//VOXOX - CJC - 2009.07.29 
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

int WsPA::sendInfo(const std::string & name, const std::string & gender, int paGender ) {
	if(_did!=""){//VOXOX - CJC - 2009.07.29 
		//Encode the message
		String encodedName = String::encodeUrl(name);
		encodedName.replace("%20", "+", false);
		String encodedGender = String::encodeUrl(gender);
		String key = "";
		String did = _did;
		String method = "quicksaveSettings" ;
		String params = "key="+key+"&method="+method+"&did="+did+"&pa_voice_type="+String::fromNumber(paGender)+"&name_text="+encodedName+"&name_gender="+gender;
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

void WsPA::answerReceived(const std::string & answer, int requestId) {
	String tmp(answer);
	////TODO: replace this ugly "parsing" with a real XML parsing
	//static const std::string STATUS_UNAUTHORIZED = "401";
	//static const std::string STATUS_OK = "200";

	//String tmp(answer);

	//if (!tmp.empty()) {
	//	if (tmp.contains(STATUS_OK) && !tmp.contains(STATUS_UNAUTHORIZED)) {

	//		//SMS sent
	//		smsStatusEvent(*this, requestId, EnumSmsState::SmsStateOk);

	//		//History: retrieve the HistoryMemento & update its state to Ok
	//		_userProfile.getHistory().updateSMSState(requestId, HistoryMemento::OutgoingSmsOk);

	//		WsInfo * wsInfo = _userProfile.getWsInfo();
	//		if (wsInfo) {
	//			wsInfo->getWengosCount(true);
	//			wsInfo->execute();
	//		}
	//		return;
	//	}
	//}

	////SMS unsent
	//smsStatusEvent(*this, requestId, EnumSmsState::SmsStateError);

	////History: retrieve the HistoryMemento & update its state to Nok
	//_userProfile.getHistory().updateSMSState(requestId, HistoryMemento::OutgoingSmsNok);
}
