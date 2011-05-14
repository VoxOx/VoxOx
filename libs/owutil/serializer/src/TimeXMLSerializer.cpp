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

#include <serializer/TimeXMLSerializer.h>

#include <util/Time.h>
#include <util/String.h>

#include <tinyxml.h>

using namespace std;

TimeXMLSerializer::TimeXMLSerializer(Time & time)
	: _time(time) {
}

string TimeXMLSerializer::serialize() {
	string result;

	result += "<time>\n";

	result += ("<hour>" + String::fromNumber(_time._hour) + "</hour>\n");
	result += ("<minute>" + String::fromNumber(_time._minute) + "</minute>\n");
	result += ("<second>" + String::fromNumber(_time._second) + "</second>\n");

	result += "</time>\n";

	return result;
}

bool TimeXMLSerializer::unserialize(const std::string & data) {
	TiXmlDocument doc;

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle date = docHandle.FirstChild("time");

	//Retrieving hour
	_time._hour = String(date.FirstChild("hour").FirstChild().Text()->Value()).toInteger();

	//Retrieving minute
	_time._minute = String(date.FirstChild("minute").FirstChild().Text()->Value()).toInteger();

	//Retrieving second
	_time._second = String(date.FirstChild("second").FirstChild().Text()->Value()).toInteger();

	return true;
}
