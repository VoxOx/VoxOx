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
#include "WsTranslation.h"


#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>

#include <tinyxml.h>

#include <util/Logger.h>

WsTranslation::WsTranslation(WengoAccount * wengoAccount)
	: WengoWebService(wengoAccount){

	_wsKey = "";
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

int WsTranslation::translate(const std::string & id,const std::string & textToTranslate,const std::string & sourceLocate,const std::string & destLocate){

	String modTextToTranslate = textToTranslate;
	modTextToTranslate = modTextToTranslate.trim();
	modTextToTranslate = String::encodeUrl(modTextToTranslate);
	
		String key = "key="+_wsKey;
		String method = "&method=synchronousTranslate";
		String lockKey = "&lockKey="+_userKey;
		String wsId = "&id="+id;
		String text = "&text="+modTextToTranslate;
		String sourceLocale = "&source_locale="+sourceLocate;
		String destLocale = "&dest_locale="+destLocate;
		String format = "&format=xml";
		String params = key+method+lockKey+wsId+text+sourceLocale+destLocale+format;
		//params = String::encodeUrl(params);
		setParameters(params);

		call(this);

		return 0;
	
}

void WsTranslation::answerReceived(const std::string & answer, int requestId) {

		
	std::string id = "";
	std::string translatedText = "";
	std::string originalText = "";
	std::string status = "";
	bool success;
	

	if (answer.empty()) {
		success = false; //VOXOX - CJC - 2010.01.20 Add notification responce when for some reason webservice return empty string
		LOG_DEBUG("Translation webservice responce is empty");
	}


	TiXmlDocument doc;
	doc.Parse(answer.c_str());

	TiXmlHandle docHandle(&doc);

	TiXmlHandle root = docHandle.FirstChild("Translator").FirstChild("synchronousTranslate");
	TiXmlText * idXML = root.FirstChild("id").FirstChild().Text();
	if (idXML) {
		id = idXML->Value();
	}

	TiXmlText * originalXML = root.FirstChild("original").FirstChild().Text();
	if (originalXML) {
		originalText = originalXML->Value();
	}

	TiXmlText * translatedXML = root.FirstChild("text").FirstChild().Text();
	if (translatedXML) {
		translatedText = translatedXML->Value();
	}

	TiXmlText * statustXml = root.FirstChild("status").FirstChild().Text();
	if (statustXml) {
		status = statustXml->Value();
	}
	
	if(status=="success"){
			success = true;
	}else{
			success = false;
	}

	translationDoneEvent(*this,id,originalText,translatedText,success);

}
