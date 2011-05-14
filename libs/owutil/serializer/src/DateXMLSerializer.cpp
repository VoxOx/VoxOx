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

#include <serializer/DateXMLSerializer.h>

#include <util/Date.h>
#include <util/String.h>

#include <tinyxml.h>

using namespace std;

DateXMLSerializer::DateXMLSerializer(Date & date)
	: _date(date) {
}

string DateXMLSerializer::serialize() {
	string result;

	result += "<date>\n";

	result += ("<day>" + String::fromNumber(_date._day) + "</day>\n");
	result += ("<month>" + String::fromNumber(_date._month) + "</month>\n");
	result += ("<year>" + String::fromNumber(_date._year) + "</year>\n");

	result += "</date>\n";

	return result;
}

bool DateXMLSerializer::unserialize(const std::string & data) {
	TiXmlDocument doc;

	doc.Parse(data.c_str());

	TiXmlHandle docHandle(&doc);
	TiXmlHandle date = docHandle.FirstChild("date");

	//Retrieving day
	_date._day = String(date.FirstChild("day").FirstChild().Text()->Value()).toInteger();

	//Retrieving month
	_date._month = String(date.FirstChild("month").FirstChild().Text()->Value()).toInteger();

	//Retrieving year
	_date._year = String(date.FirstChild("year").FirstChild().Text()->Value()).toInteger();

	return true;
}
