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

#include <imwrapper/IMAccountXMLSerializer1.h>

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountParametersXMLSerializer.h>

#include <util/Base64.h>

#include <tinyxml.h>

using namespace std;

IMAccountXMLSerializer1::IMAccountXMLSerializer1(IMAccount & imAccount)
: _imAccount(imAccount) {
}

void IMAccountXMLSerializer1::setIMAccountParametersData(const std::string & imAccountParametersData) {
	_imAccountParametersData = imAccountParametersData;
}

std::string IMAccountXMLSerializer1::serialize() {
	string result;
	EnumIMProtocol enumIMProtocol;

//	result += "<account protocol=\"" + enumIMProtocol.toString(_imAccount._protocol) + "\">\n";
	result += "<account protocol=\"" + enumIMProtocol.toString(_imAccount.getProtocol() ) + "\">\n";	//VOXOX - JRT - 2009.04.24 
	result += ("<login>" + _imAccount.getLogin() + "</login>\n");
	result += ("<password>" + Base64::encode(_imAccount.getPassword()) + "</password>\n");
	result += ("<statusMessage>" + _imAccount.getStatusMessage() + "</statusMessage>\n");//VOXOX CHANGE by Rolando - 2009.07.01 
//	result += ("<presence>" + EnumPresenceState::toString(_imAccount._presenceState) + "</presence>\n");
	result += ("<initialPresence>" + EnumPresenceState::toString(_imAccount.getInitialPresenceState()) + "</initialPresence>\n");
	IMAccountParametersXMLSerializer serializer(_imAccount.getIMAccountParameters());
	result += serializer.serialize();
	result += "</account>\n";

	return result;
}

bool IMAccountXMLSerializer1::unserialize(const std::string & data) {
	TiXmlDocument doc;
	EnumIMProtocol imProtocol;

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle account = docHandle.FirstChild("account");

	// Retrieving protocol
	TiXmlElement * lastChildElt = account.Element();
	if (lastChildElt) {
//		_imAccount._protocol = imProtocol.toIMProtocol(lastChildElt->Attribute("protocol"));
		_imAccount.setProtocol( imProtocol.toIMProtocol(lastChildElt->Attribute("protocol") ) );	//VOXOX - JRT - 2009.04.24
	} else {
		return false;
	}

	//Retrieving login
	TiXmlText * login = account.FirstChild("login").FirstChild().Text();
	if (login) {
		_imAccount.setLogin( login->Value() );
	} else {
		return false;
	}

	//Retrieving password
	TiXmlText * password = account.FirstChild("password").FirstChild().Text();
	if (password) {
		_imAccount.setPassword( Base64::decode(password->Value()) );
	}

	//Retrieving statusMessage
	TiXmlText * statusMessage = account.FirstChild("statusMessage").FirstChild().Text();//VOXOX CHANGE by Rolando - 2009.07.01 
	if (statusMessage) {//VOXOX CHANGE by Rolando - 2009.07.01 
		_imAccount.setStatusMessage( statusMessage->Value() );//VOXOX CHANGE by Rolando - 2009.07.01 
	}

	//Retireving Presence state
	//TiXmlText * presence = account.FirstChild("presence").FirstChild().Text();
	//if (presence) {
	//	_imAccount._presenceState = EnumPresenceState::fromString(presence->Value());
	//}
	//Retireving Presence state
	TiXmlText * presence = account.FirstChild("initialPresence").FirstChild().Text();
	if (presence) 
	{
		_imAccount.setInitialPresenceState( EnumPresenceState::fromString(presence->Value()) );
	}

	//Retrieving IMAccountParameters
	IMAccountParameters imAccountParameters;
	IMAccountParametersXMLSerializer serializer(imAccountParameters);
	serializer.unserializeSystemSettings(_imAccountParametersData);
	TiXmlNode * settingsNode = account.FirstChild("settings").Node();
	if (settingsNode) 
	{
		string nodeData;
		nodeData << *settingsNode;
		serializer.unserialize(_imAccountParametersData, nodeData);
	}

	_imAccount.setIMAccountParameters( imAccountParameters );

	return true;
}
