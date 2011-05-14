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

#include <imwrapper/IMAccountListXMLSerializer1.h>

#include <imwrapper/IMAccountList.h>
#include <imwrapper/IMAccountXMLSerializer1.h>
#include <imwrapper/IMAccount.h>

#include <tinyxml.h>

using namespace std;

IMAccountListXMLSerializer1::IMAccountListXMLSerializer1(IMAccountList & imAccountList)
: _imAccountList(imAccountList) {
}

std::string IMAccountListXMLSerializer1::serialize() {
	string result;

	result += "<imaccounts>\n";

	for (IMAccountList::const_iterator it = _imAccountList.begin() ; it != _imAccountList.end() ; it++) {
//		IMAccountXMLSerializer1 serializer((IMAccount &)*it);	//VOXOX - JRT - 2009.04.10 
		IMAccountXMLSerializer1 serializer( const_cast<IMAccount&>(it->second) );
		result += serializer.serialize();
	}

	result += "</imaccounts>\n";

	return result;	
}

bool IMAccountListXMLSerializer1::unserialize(const std::string & data) {
	TiXmlDocument doc;
	doc.Parse(data.c_str());
	
	TiXmlHandle docHandle(& doc);
	TiXmlNode * imaccounts = docHandle.FirstChild("imaccounts").Node();
	
	if (imaccounts) {
		TiXmlNode * lastChild = NULL;
		while ((lastChild = imaccounts->IterateChildren("account", lastChild))) {
			string accountData;
			accountData << *lastChild;
			
			IMAccount account;
			IMAccountXMLSerializer1 serializer(account);
			serializer.unserialize(accountData);
//			_imAccountList.push_back(account);	//VOXOX - JRT - 2009.04.09 
			_imAccountList.Add( account );
		}

		return true;
	}

	return false;
}
