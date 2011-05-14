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
#include "StreetAddressXMLSerializer.h"

#include "StreetAddress.h"

#include <tinyxml.h>

using namespace std;

StreetAddressXMLSerializer::StreetAddressXMLSerializer(StreetAddress & streetAddress)
: _streetAddress(streetAddress) {

}

std::string StreetAddressXMLSerializer::serialize() {
	string result;

	result += "<address type=\"home\">\n";

	if (!_streetAddress._street1.empty()) {
		result += ("<street><![CDATA[" + _streetAddress._street1 + "]]></street>\n");
	}

	if (!_streetAddress._city.empty()) {
		result += ("<locality><![CDATA[" + _streetAddress._city + "]]></locality>\n");
	}

	if (!_streetAddress._stateProvince.empty()) {
		result += ("<region><![CDATA[" + _streetAddress._stateProvince + "]]></region>\n");
	}

	if (!_streetAddress._postalCode.empty()) {
		result += ("<postcode><![CDATA[" + _streetAddress._postalCode + "]]></postcode>\n");
	}

	if (!_streetAddress._country.empty()) {
		result += ("<country><![CDATA[" + _streetAddress._country + "]]></country>\n");
	}

	result += "</address>\n";

	return result;
}

bool StreetAddressXMLSerializer::unserialize(const std::string & data) {
	TiXmlDocument doc;
	doc.Parse(data.c_str());

	TiXmlHandle docHandle(& doc);

	TiXmlNode * street = docHandle.FirstChild("address").FirstChild("street").FirstChild().Node();
	if (street) {
		_streetAddress._street1 = street->Value();
	}

	TiXmlNode * locality = docHandle.FirstChild("address").FirstChild("locality").FirstChild().Node();
	if (locality) {
		_streetAddress._city = locality->Value();
	}

	TiXmlNode * region = docHandle.FirstChild("address").FirstChild("region").FirstChild().Node();
	if (region) {
		_streetAddress._stateProvince = region->Value();
	}

	TiXmlNode * postcode = docHandle.FirstChild("address").FirstChild("postcode").FirstChild().Node();
	if (postcode) {
		_streetAddress._postalCode = postcode->Value();
	}

	TiXmlNode * country = docHandle.FirstChild("address").FirstChild("country").FirstChild().Node();
	if (country) {
		_streetAddress._country = country->Value();
	}

	return true;
}
