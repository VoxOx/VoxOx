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
* Request translation data
* @author Chris Jimenez C 
* @date 2009.06.27
*/


#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "WsTranslationLanguages.h"


#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>

#include <tinyxml.h>

#include <util/Logger.h>

WsTranslationLanguages::WsTranslationLanguages(WengoAccount * wengoAccount , const std::string& securityKey )
	: WengoWebService(wengoAccount){

	_securityKey = securityKey;		
	_userKey = wengoAccount->getIdentity();
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	//	_did = wengoAccount->getIdentity();
	//Setup SMS web service
	//setHostname(config.getApiHostname());
	setHostname(config.getTranslationApiHostname());
	setServicePath(config.getVoxOxTranslationWSPathKey());
	setGet(true);
	setWengoAuthentication(false);

//#if 1
//	setHttps(true);	//VOXOX - JRT - 2009.10.05 
//	setPort(443);
//#else
	setHttps(false);
	setPort(80);
//#endif
}

int WsTranslationLanguages::getLanguages(){

	
	String key = "key="+_securityKey;
	String method = "&method=getLanguages";

	String params		= key + method;
	//params = String::enc
	
	setParameters(params);

	call(this);

	return 0;
	
}

void WsTranslationLanguages::answerReceived(const std::string & answer, int requestId) {

	

	if (answer.empty()) {
		return;
	}


	_languagesXML = answer;

}
