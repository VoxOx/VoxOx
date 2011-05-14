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

#include <settings/SettingsXMLSerializer.h>

#include <settings/StringListXMLSerializer.h>
#include <settings/Settings.h>

#include <thread/RecursiveMutex.h>
#include <util/String.h>

#include <tinyxml.h>

using namespace std;

SettingsXMLSerializer::SettingsXMLSerializer(Settings & settings)
	: _settings(settings) {
}

std::string SettingsXMLSerializer::serialize() {
	string result;

	result += "<settings>\n";

	for (Settings::Keys::const_iterator it = _settings._keyMap.begin(); it != _settings._keyMap.end(); ++it) {
		string key = (*it).first;
		boost::any keyValue = (*it).second;
		string value;

		if (Settings::isString(keyValue)) {
			/*value = "<string>" + boost::any_cast<std::string>(keyValue) + "</string>";*///VOXOX CHANGE by Rolando - 2009.08.03
			String tmpValue = boost::any_cast<std::string>(keyValue);//VOXOX CHANGE by Rolando - 2009.08.03
			tmpValue.encodeToXMLSpecialCharacters();//VOXOX CHANGE by Rolando - 2009.08.04 			
			value = "<string>" + tmpValue + "</string>";//VOXOX CHANGE by Rolando - 2009.08.03 
		} else if (Settings::isStringList(keyValue)) {
			StringList list = boost::any_cast<StringList>(keyValue);
			StringListXMLSerializer serializer(list);
			value = serializer.serialize();
		} else if (Settings::isBoolean(keyValue)) {
			value = "<bool>" + String::fromBoolean(boost::any_cast<bool>(keyValue)) + "</bool>";
		} else if (Settings::isInteger(keyValue)) {
			value = "<int>" + String::fromNumber(boost::any_cast<int>(keyValue)) + "</int>";
		}

		result += ("<" + key + ">" + value + "</" + key + ">\n");
	}

	result += "</settings>\n";

	return result;
}

bool SettingsXMLSerializer::unserialize(const std::string & data) {
	{
		RecursiveMutex::ScopedLock lock(_settings._mutex);
		_settings._keyMap.clear();
	}

	TiXmlBase::SetCondenseWhiteSpace(false);

	TiXmlDocument doc;
	doc.Parse(data.c_str());

	TiXmlHandle docHandle(& doc);
	TiXmlNode * settings = docHandle.FirstChild("settings").Node();

	if (settings == NULL)
		return false;

	TiXmlNode * lastChild = NULL;
	while ((lastChild = settings->IterateChildren(lastChild))) {
		string key = lastChild->Value();
		TiXmlNode * value = lastChild->FirstChild();
		if (value) {
			string valueNode;
			valueNode << *value;
			string valueType = value->Value();
			TiXmlNode * valueDataNode = value->FirstChild();
			string valueData;
			if (valueDataNode) {
				valueData = valueDataNode->Value();
			}

			if (valueType == "stringlist") {
				StringList list;
				StringListXMLSerializer serializer(list);
				serializer.unserialize(valueNode);
				_settings.set(key, list);
			} else if (valueType == "string") {
				_settings.set(key, valueData);
			} else if (valueType == "bool") {
				_settings.set(key, String(valueData).toBoolean());
			} else if (valueType == "int") {
				_settings.set(key, String(valueData).toInteger());
			}
		}
	}

	return true;
}
