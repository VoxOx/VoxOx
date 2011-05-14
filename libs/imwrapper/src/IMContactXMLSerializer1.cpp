/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include <imwrapper/IMContactXMLSerializer1.h>

#include <imwrapper/IMContact.h>
#include <imwrapper/IMAccountList.h>

#include <util/Logger.h>
#include <util/Base64.h>

#include <tinyxml.h>

using namespace std;

IMContactXMLSerializer1::IMContactXMLSerializer1(IMContact & imContact, IMAccountList & imAccountList)
: _imContact(imContact), _imAccountList(imAccountList) {
}

std::string IMContactXMLSerializer1::serialize() {
	string result;

	result += ("<im protocol=\"" 
		+ EnumIMProtocol::toString(_imContact.getProtocol())
		+ "\">\n");

	result += ("<id>" + _imContact.getContactId() + "</id>\n");

	if (!_imContact.getAlias().empty()) {
		result += ("<alias><![CDATA[" + _imContact.getAlias() + "]]></alias>\n");
	}
	//VOXOX CHANGE CJC ADD SUPPORT FOR STATUS MESSAGE
	if (!_imContact.getStatusMessage().empty()) {
		result += ("<statusMessage><![CDATA[" + _imContact.getStatusMessage() + "]]></statusMessage>\n");
	}

	///VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
	result += ("<blocked>" + String::fromBoolean(_imContact.isBlocked()) + "</blocked>\n");

	if (_imContact._imAccount) {
		result += ("<account>" + _imContact._imAccount->getLogin() + "</account>\n");	
	}

	if (!_imContact._icon.getData().empty()) {
		result += ("<photo><![CDATA[" + Base64::encode(_imContact._icon.getData()) + "]]></photo>");
	}

	result += "</im>\n";

	return result;
}

bool IMContactXMLSerializer1::unserialize(const std::string & data) {
	TiXmlDocument doc;

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle im = docHandle.FirstChild("im");

	// Retrieving associated account
	EnumIMProtocol::IMProtocol protocol;

	TiXmlElement * lastChildElt = im.Element();
	if (lastChildElt) {
		protocol = EnumIMProtocol::toIMProtocol(lastChildElt->Attribute("protocol"));
	} else {
		return false;
	}

	TiXmlText * login = im.FirstChild("account").FirstChild().Text();
	if (login) {		
		const IMAccount *imAccount = NULL;
		std::string loginValue = std::string(login->Value());

		//VOXOX - JRT - 2009.04.09 
		//for (IMAccountList::const_iterator it = _imAccountList.begin();
		//	it != _imAccountList.end();
		//	++it) {
		//	if ((it->getLogin() == loginValue) &&		
		//		(it->getProtocol() == protocol) ) {
		//		imAccount = &(*it);
		//	}
		//}
		imAccount = _imAccountList.findByLoginInfo( loginValue, protocol );
		//End VOXOX

		if (imAccount) {
			_imContact.setIMAccount(imAccount);
		} else {
			LOG_ERROR("this IMAccount does not exist in IMAccountList: " + std::string(login->Value()));
			return false;
		}
		////
	} else {
		_imContact._imAccount = NULL;
		_imContact._protocol = protocol;
	}
	////

	//Retrieving contactId
	TiXmlText * contactId = im.FirstChild("id").FirstChild().Text();
	if (contactId) {
	
		// wengo or sip account should have a domain
		// (unfortunately not saved before 2.1rc2...
		String completeLogin(contactId->Value());
		if (!completeLogin.contains("@")) 
		{
			if (protocol == EnumIMProtocol::IMProtocolWengo) {
				completeLogin += "@voip.wengo.fr";
			} 
			//else if (protocol == EnumIMProtocol::IMProtocolSIP) {
			//	// TO DO ?
			//}
		}
		////
	
		_imContact._contactId = completeLogin;
	}
	////

	// Retrieving alias
	TiXmlText * alias = im.FirstChild("alias").FirstChild().Text();
	if (alias) {
		_imContact._alias = alias->Value();
	}
	////
	
	//VOXOX CHANGE CJC ADD SUPPORT FOR STATUS MESSAGE
	// Retrieving statusMessage
	TiXmlText * statusMessage = im.FirstChild("statusMessage").FirstChild().Text();
	if (statusMessage) {
		_imContact._statusMessage = statusMessage->Value();
	}

	///VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009
	TiXmlText * blocked = im.FirstChild("blocked").FirstChild().Text();
	if (blocked) {
		String strBlocked = blocked->Value();
		_imContact._blocked = strBlocked.toBoolean();
	}

	// Retrieving icon
	TiXmlText * photo = im.FirstChild("photo").FirstChild().Text();
	if (photo) {
		OWPicture picture = OWPicture::pictureFromData(Base64::decode(photo->Value()));
		_imContact.setIcon(picture);
	}
	////

	return true;
}
