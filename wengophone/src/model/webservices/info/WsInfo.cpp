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
#include "WsInfo.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <util/Logger.h>

#include <tinyxml.h>

#include <sstream>

const char * getValueFromKey(TiXmlElement * element, const std::string & key);

const std::string WsInfo::WENGOSCOUNT_TAG = "contract.counter.wengos";
const std::string WsInfo::SMSCOUNT_TAG = "contract.counter.sms";
const std::string WsInfo::ACTIVEMAIL_TAG = "ucf.email.isactive";
const std::string WsInfo::UNREADVOICEMAILCOUNT_TAG = "tph.mbox.unseencount";
const std::string WsInfo::ACTIVEVOICEMAIL_TAG = "tph.mbox.isactive";
const std::string WsInfo::CALLFORWARD_TAG = "tph.callforward";
const std::string WsInfo::LANDLINENUMBER_TAG = "contract.option.pstnnum";

const std::string WsInfo::CALLFORWARD_MODE_TAG = "tph.callforward.mode";
const std::string WsInfo::CALLFORWARD_TO_LANDLINE_DEST1_TAG = "tph.callforward.destination1";
const std::string WsInfo::CALLFORWARD_TO_LANDLINE_DEST2_TAG = "tph.callforward.destination2";
const std::string WsInfo::CALLFORWARD_TO_LANDLINE_DEST3_TAG = "tph.callforward.destination3";

WsInfo::WsInfo(WengoAccount * wengoAccount)
	: WengoWebService(wengoAccount) {

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	_wengosCount = false;
	_smsCount = false;
	_activeMail = false;
	_unreadVoiceMail = false;
	_callForward = false;
	_landlineNumber = false;

	//setup info web service
	setHostname(config.getWengoServerHostname());
	//VOXOX CHANGE FIX PROBLEM WITH NOT RESPONDING WEBSERVICE
	//TODO: FIX THIS, WE NEED TO COMPILE OPENSSL TO BE ABLE TO SEND HTTPS TRUE, AND ALSO PASSWORD AND OTHERS SHOULD BE ENCRIPYED
	setGet(false);
	setServicePath(config.getWengoInfoPath());
	setWengoAuthentication(true);

#if 1
	setHttps(true);	//VOXOX - JRT - 2009.10.05 
	setPort(443);
#else
	setHttps(false);
	setPort(80);
#endif

	getCallForwardInfo(false);
}

void WsInfo::getWengosCount(bool wengosCount) {
	_wengosCount = wengosCount;
}

void WsInfo::getSmsCount(bool smsCount) {
	_smsCount = smsCount;
}

void WsInfo::getActiveMail(bool activeMail) {
	_activeMail = activeMail;
}

void WsInfo::getUnreadVoiceMail(bool unreadVoiceMail) {
	_unreadVoiceMail = unreadVoiceMail;
}

void WsInfo::getCallForwardInfo(bool callForward) {
	_callForward = callForward;
}

void WsInfo::getLandlineNumber(bool landlineNumber) {
	_landlineNumber = landlineNumber;
}

int WsInfo::execute() {

	//build the query
	std::string query = "query=";
	if (_wengosCount) {
		query += WENGOSCOUNT_TAG + "|";
	}
	if (_smsCount) {
		query += SMSCOUNT_TAG + "|";
	}
	if (_activeMail) {
		query += ACTIVEMAIL_TAG + "|";
	}
	if (_unreadVoiceMail) {
		query += UNREADVOICEMAILCOUNT_TAG + "|";
		query += ACTIVEVOICEMAIL_TAG + "|";
	}
	if (_landlineNumber) {
		query += LANDLINENUMBER_TAG + "|";
	}
	if (_callForward) {
		query += CALLFORWARD_TAG + "|";
		query += ACTIVEVOICEMAIL_TAG + "|";
	}

	//remove the last pipe if any
	if (query != "query=") {
		query = query.substr(0, query.length() - 1);
	}

	setParameters(query);

	return call(this);
}

void WsInfo::answerReceived(const std::string & answer, int id) {

	const char * value = NULL;
	std::string forwardMode;
	std::string voiceMailNumber;
	std::string dest1;
	std::string dest2;
	std::string dest3;
	bool forward2VoiceMail = false;

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	TiXmlDocument document;
	document.Parse(answer.c_str());

	TiXmlElement * root = document.FirstChildElement("output");
	if (root) {

		//iterate over "o" element
		TiXmlElement * element = root->FirstChildElement("o");
		while (element) {

			//find the key
			const char * key = element->Attribute("k");
			if (key) {

				//wengos count
				if (std::string(key) == WENGOSCOUNT_TAG) {
					value = getValueFromKey(element, WENGOSCOUNT_TAG);
					if (value) {
						std::string numValue = value;
						wsInfoWengosEvent(*this, id, WsInfoStatusOk, numValue);
					}

				//sms count
				} else if (key == SMSCOUNT_TAG) {
					int sms  = 0;
					value = getValueFromKey(element, SMSCOUNT_TAG);
					if (value) {
						std::stringstream ss( value );
						ss >> sms;
						wsInfoSmsCountEvent(*this, id, WsInfoStatusOk, sms);
					}

				//active mail
				} else if (key == ACTIVEMAIL_TAG) {
					int activeMail = 0;
					value = getValueFromKey(element, ACTIVEMAIL_TAG);
					if (value) {
						std::stringstream ss(value);
						ss >> activeMail;
						wsInfoActiveMailEvent(*this, id, WsInfoStatusOk, activeMail);
					}

				//unread voice mail
				} else if (key == UNREADVOICEMAILCOUNT_TAG) {
					int voiceMail = 0;
					value = getValueFromKey(element, UNREADVOICEMAILCOUNT_TAG);
					if (value) {
						std::stringstream ss(value);
						ss >> voiceMail;
						wsInfoVoiceMailEvent(*this, id, WsInfoStatusOk, voiceMail);
					}

				//active voice mail
				} else if (key == ACTIVEVOICEMAIL_TAG) {
					bool voiceMailActive = false;
					value = getValueFromKey(element, UNREADVOICEMAILCOUNT_TAG);
					if (value) {
						if (std::string(value) == "true") {
							voiceMailActive = true;
						}
						config.set(Config::VOICE_MAIL_ACTIVE_KEY, voiceMailActive);
					}
					wsInfoActiveVoiceMailEvent(*this, id, WsInfoStatusOk, voiceMailActive);

				//call forward
				} else if (key == CALLFORWARD_MODE_TAG) {
					value = getValueFromKey(element, CALLFORWARD_MODE_TAG);
					if (value) {
						forwardMode = std::string(value);
					}

				//call forward
				} else if (key == CALLFORWARD_TO_LANDLINE_DEST1_TAG) {
					value = getValueFromKey(element, CALLFORWARD_TO_LANDLINE_DEST1_TAG);
					if (value) {
						dest1 = std::string(value);
					}

				//call forward
				} else if (key == CALLFORWARD_TO_LANDLINE_DEST2_TAG) {
					value = getValueFromKey(element, CALLFORWARD_TO_LANDLINE_DEST2_TAG);
					if (value) {
						dest2 = std::string(value);
					}

				//call forward
				} else if (key == CALLFORWARD_TO_LANDLINE_DEST3_TAG) {
					value = getValueFromKey(element, CALLFORWARD_TO_LANDLINE_DEST3_TAG);
					if (value) {
						dest3 = std::string(value);
					}

				//landline number
				} else if (key == LANDLINENUMBER_TAG) {
					value = getValueFromKey(element, LANDLINENUMBER_TAG);
					if (value) {
						wsInfoLandlineNumberEvent(*this, id, WsInfoStatusOk, std::string(value));
					}

				}
			}
			element = element->NextSiblingElement("o");
		}
	}

	//emit call forward event
	if (_callForward) {

		//remove XXX coming from the ws
		if (dest1 == "XXX") {
			dest1 = "";
		}
		if (dest2 == "XXX") {
			dest2 = "";
		}
		if (dest3 == "XXX") {
			dest3 = "";
		}

		if (forwardMode == "enable") {

			if ((dest1 == "voicemail")) {
				config.set(Config::CALL_FORWARD_MODE_KEY, std::string("voicemail"));
				wsCallForwardInfoEvent(*this, id, WsInfoStatusOk, WsInfoCallForwardModeVoicemail, forward2VoiceMail, dest1, dest2, dest3);
			} else {
				config.set(Config::CALL_FORWARD_MODE_KEY, std::string("number"));
				wsCallForwardInfoEvent(*this, id, WsInfoStatusOk, WsInfoCallForwardModeNumber, forward2VoiceMail, dest1, dest2, dest3);
			}

		} else if (forwardMode == "disable") {
			config.set(Config::CALL_FORWARD_MODE_KEY, forwardMode);
			wsCallForwardInfoEvent(*this, id, WsInfoStatusOk, WsInfoCallForwardModeDisabled, forward2VoiceMail, dest1, dest2, dest3);
		} else if (forwardMode == "unauthorized") {
			config.set(Config::CALL_FORWARD_MODE_KEY, forwardMode);
			wsCallForwardInfoEvent(*this, id, WsInfoStatusOk, WsInfoCallForwardModeUnauthorized, forward2VoiceMail, dest1, dest2, dest3);
		}

		//write info to the Config
		config.set(Config::CALL_FORWARD_PHONENUMBER1_KEY, dest1);
		config.set(Config::CALL_FORWARD_PHONENUMBER2_KEY, dest2);
		config.set(Config::CALL_FORWARD_PHONENUMBER3_KEY, dest3);
	}
}

const char * getValueFromKey(TiXmlElement * element, const std::string & key) {

	//find value
	TiXmlElement * elt = element->FirstChildElement("v");
	if (elt) {
		const char * type = elt->Attribute("t");

		//check for error
		if ((type) && (std::string(type) != "e")) {

			//extract value text
			if (elt->FirstChild()) {
				TiXmlText  * text = elt->FirstChild()->ToText();
				if (text) {
					return text->Value();
				}
			}
		}
	}
	return NULL;
}
