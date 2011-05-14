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
* WS TO Request translation data
* @author Chris Jimenez C 
* @date 2009.06.27
*/


#ifndef OWWsTranslationLanguagesLANGUAGES_H
#define OWWsTranslationLanguagesLANGUAGES_H

#include <model/webservices/WengoWebService.h>


class UserProfile;

class WsTranslationLanguages : public WengoWebService {
public:

	
	WsTranslationLanguages(WengoAccount * wengoAccount, const std::string& securityKey );

	virtual ~WsTranslationLanguages() {}

	int getLanguages();

	std::string getLanguagesXML(){return _languagesXML;}

	/*Event<void (WsTranslationLanguages & sender,const std::string id,const std::string originalText,
			const std::string & translatedText,bool success)> translationDoneEvent;*/


private:

	void answerReceived(const std::string & answer, int requestId);

    std::string  _wsKey;

    std::string  _securityKey;

	std::string  _userKey;

	std::string _languagesXML;

	
};

#endif //OWWSTRANSLATIONLANGUAGES
