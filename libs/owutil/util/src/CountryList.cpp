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

//#include "stdafx.h"		//VOXOX - JRT - 2009.04.03 - For precompiled headers.
#include <util/CountryList.h>

StringList CountryList::getCountryList() {
	StringList result;

	// List was taken from http://en.wikipedia.org/wiki/List_of_sovereign_states on 18th sept. 2006
	result += "Afghanistan";
	result += "Albania";
	result += "Algeria";
	result += "Andorra";
	result += "Angola";
	result += "Antigua and Barbuda";
	result += "Argentina";
	result += "Armenia";
	result += "Australia";
	result += "Austria";
	result += "Azerbaijan";
	result += "Bahamas, The";
	result += "Bahrain";
	result += "Bangladesh";
	result += "Barbados";
	result += "Belarus";
	result += "Belgium";
	result += "Belize";
	result += "Benin";
	result += "Bermuda";
	result += "Bhutan";
	result += "Bolivia";
	result += "Bosnia and Herzegovina";
	result += "Botswana";
	result += "Brazil";
	result += "Brunei";
	result += "Bulgaria";
	result += "Burkina Faso";
	result += "Burundi";
	result += "Cambodia";
	result += "Cameroon";
	result += "Canada";
	result += "Cape Verde";
	result += "Cayman Islands";
	result += "Central African Republic";
	result += "Chad";
	result += "Chile";
	result += "China, People's Republic of";
	result += "China, Republic of";	
	result += "Colombia";
	result += "Comoros";
	result += "Congo, Republic of";
	result += "Congo, Democratic Republic of";
	result += "Costa Rica";
	result += "Côte d'Ivoire";
	result += "Croatia";
	result += "Cuba";
	result += "Cyprus";
	result += "Czech Republic";
	result += "Denmark";
	result += "Djibouti";
	result += "Dominica";
	result += "Dominican Republic";
	result += "East Timor";
	result += "Ecuador";
	result += "Egypt";
	result += "El Salvador";
	result += "Equatorial Guinea";
	result += "Eritrea";
	result += "Estonia";
	result += "Ethiopia";
	result += "Fiji";
	result += "Finland";
	result += "France";
	result += "Gabon";
	result += "Gambia, The";
	result += "Georgia";
	result += "Germany";
	result += "Ghana";
	result += "Greece";
	result += "Grenada";
	result += "Guatemala";
	result += "Guinea";
	result += "Guinea-Bissau";
	result += "Guyana";
	result += "Haiti";
	result += "Honduras";
	result += "Hungary";
	result += "Iceland";
	result += "India";
	result += "Indonesia";
	result += "Iran";
	result += "Iraq";
	result += "Ireland";
	result += "Israel";
	result += "Italy";
	result += "Jamaica";
	result += "Japan";
	result += "Jordan";
	result += "Kazakhstan";
	result += "Kenya";
	result += "Kiribati";
	result += "Korea, North";
	result += "Korea, South";
	result += "Kuwait";
	result += "Kyrgyzstan";
	result += "Laos";
	result += "Latvia";
	result += "Lebanon";
	result += "Lesotho";
	result += "Liberia";
	result += "Libya";
	result += "Liechtenstein";
	result += "Lithuania";
	result += "Luxembourg";
	result += "Macao";
	result += "Macedonia";
	result += "Madagascar";
	result += "Malawi";
	result += "Malaysia";
	result += "Maldives";
	result += "Mali";
	result += "Malta";
	result += "Marshall Islands";
	result += "Mauritania";
	result += "Mauritius";
	result += "Mexico";
	result += "Micronesia";
	result += "Moldova";
	result += "Monaco";
	result += "Mongolia";
	result += "Montenegro";
	result += "Morocco";
	result += "Mozambique";
	result += "Myanmar";
	result += "Namibia";
	result += "Nauru";
	result += "Nepal";
	result += "Netherlands";
	result += "New Zealand";
	result += "Nicaragua";
	result += "Niger";
	result += "Nigeria";
	result += "Norway";
	result += "Oman";
	result += "Pakistan";
	result += "Palau";
	result += "Palestine";
	result += "Panama";
	result += "Papua New Guinea";
	result += "Paraguay";
	result += "Peru";
	result += "Philippines";
	result += "Poland";
	result += "Portugal";
	result += "Qatar";
	result += "Romania";
	result += "Russia";
	result += "Rwanda";
	result += "Saint Kitts and Nevis";
	result += "Saint Lucia";
	result += "Saint Vincent and the Grenadines";
	result += "Samoa";
	result += "San Marino";
	result += "São Tomé and Príncipe";
	result += "Saudi Arabia";
	result += "Senegal";
	result += "Serbia";
	result += "Seychelles";
	result += "Sierra Leone";
	result += "Singapore";
	result += "Slovakia";
	result += "Slovenia";
	result += "Solomon Islands";
	result += "Somalia";
	result += "Somaliland";
	result += "South Africa";
	result += "Spain";
	result += "Sri Lanka";
	result += "Sudan";
	result += "Suriname";
	result += "Swaziland";
	result += "Sweden";
	result += "Switzerland";
	result += "Syria";
	result += "Tajikistan";
	result += "Tanzania";
	result += "Thailand";
	result += "Togo";
	result += "Tonga";
	result += "Trinidad and Tobago";
	result += "Tunisia";
	result += "Turkey";
	result += "Turkmenistan";
	result += "Tuvalu";
	result += "Uganda";
	result += "Ukraine";
	result += "United Arab Emirates";
	result += "United Kingdom";
	result += "United States";
	result += "Uruguay";
	result += "Uzbekistan";
	result += "Vanuatu";
	result += "Vatican City";
	result += "Venezuela";
	result += "Vietnam";
	result += "Yemen";
	result += "Zambia";
	result += "Zimbabwe";

	return result;
}
