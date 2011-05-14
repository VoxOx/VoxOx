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

#include <settings/StringListXMLSerializer.h>

#include <util/StringList.h>
#include <util/String.h>//VOXOX CHANGE by Rolando - 2009.08.03 

#include <tinyxml.h>

using namespace std;

StringListXMLSerializer::StringListXMLSerializer(StringList & stringList)
	: _stringList(stringList) {
}

std::string StringListXMLSerializer::serialize() {
	string result;

	result += "<stringlist>\n";

	for (unsigned i = 0 ; i < _stringList.size() ; ++i) {
		String tmpValue = _stringList[i];//VOXOX CHANGE by Rolando - 2009.08.03
		tmpValue.encodeToXMLSpecialCharacters();//VOXOX CHANGE by Rolando - 2009.08.04 
		result += ("<elt>" + tmpValue + "</elt>\n");//VOXOX CHANGE by Rolando - 2009.08.03 
	}

	result += "</stringlist>\n";

	return result;
}

bool StringListXMLSerializer::unserialize(const std::string & data) {
	TiXmlDocument doc;
	doc.Parse(data.c_str());

	TiXmlHandle docHandle(& doc);
	TiXmlNode * stringlist = docHandle.FirstChild("stringlist").Node();

	TiXmlNode * lastChild = NULL;
	while ((lastChild = stringlist->IterateChildren("elt", lastChild))) {
		_stringList += lastChild->FirstChild()->Value();
	}

	return true;
}
