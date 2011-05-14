/**
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

#ifndef COUNTRIES_H
#define COUNTRIES_H

/**
 * A Country is the association of its ISO 3166-1 code
 * + its english name
 * + country calling code
 *
 * @see http://kropla.com/dialcode.htm
 */
struct Country {

	/** English country name. */
	std::string englishName;

	/** ISO 3166-1 code. */
	std::string iso3166Code;

	/**
	 * The country code is the national prefix to be used
	 * when dialing to that particular country from another country.
	 * Ex: +33 for France
	 */
	int nationalPrefix;
};

/** Number of countries available. */
static const int COUNTRIES_SIZE = 250;

/**
 * Country list from ISO 3166-1.
 *
 * This list states the country names (official short names in English) in
 * alphabetical order as given in ISO 3166-1 and the corresponding ISO
 * 3166-1-alpha-2 code elements. The list is updated whenever a change to the
 * official code list in ISO 3166-1 is effected by the ISO 3166/MA. It lists 240
 * official short names and code elements. One line of text contains one entry.
 *
 * @see http://www.iso.org/iso/en/prods-services/iso3166ma/02iso-3166-code-lists/index.html
 * @author Tanguy Krotoff
 */
static const Country COUNTRIES[COUNTRIES_SIZE] = {
	{ "Afghanistan", "af", 93 },
	{ "Albania", "al", 355 },
	{ "Algeria", "dz", 213 },
	{ "American Samoa", "as", 00 },
	{ "Andorra", "ad", 376 },
	{ "Angola", "ao", 244 },
	{ "Anguilla", "ai", 00 },
	{ "Antarctica", "aq", 672 },
	{ "Antigua and Barbuda", "ag", 00 },
	{ "Argentina", "ar", 00 },
	{ "Armenia", "am", 00 },
	{ "Aruba", "aw", 00 },
	{ "Australia", "au", 00 },
	{ "Austria", "at", 00 },
	{ "Azerbaijan", "az", 00 },
	{ "Bahamas", "bs", 00 },
	{ "Bahrain", "bh", 00 },
	{ "Bangladesh", "bd", 00 },
	{ "Barbados", "bb", 00 },
	{ "Belarus", "by", 00 },
	{ "Belgium", "be", 00 },
	{ "Belize", "bz", 00 },
	{ "Benin", "bj", 00 },
	{ "Bermuda", "bm", 00 },
	{ "Bhutan", "bt", 00 },
	{ "Bolivia", "bo", 00 },
	{ "Bosnia and Herzegovina", "ba", 00 },
	{ "Botswana", "bw", 00 },
	{ "Bouvet Island", "bv", 00 },
	{ "Brazil", "br", 00 },
	{ "Brunei", "bn", 00 },
	{ "Bulgaria", "bg", 00 },
	{ "Burkina Faso", "bf", 00 },
	{ "Burundi", "bi", 00 },
	{ "Cambodia", "kh", 00 },
	{ "Cameroon", "cm", 00 },
	{ "Canada", "ca", 00 },
	{ "Cape Verde", "cv", 00 },
	{ "Cayman Islands", "ky", 00 },
	{ "Central African Republic", "cf", 00 },
	{ "Chad", "td", 00 },
	{ "Chile", "cl", 00 },
	{ "China", "cn", 00 },
	{ "Christmas Island", "cx", 00 },
	{ "Cocos (keeling) islands", "cc", 00 },
	{ "Colombia", "co", 00 },
	{ "Comoros", "km", 00 },
	{ "Congo", "cg", 00 },
	{ "Congo, The Democratic Republic of the", "cd", 00 },
	{ "Cook Islands", "ck", 00 },
	{ "Costa Rica", "cr", 00 },
	{ "Cote d'Ivoire", "ci", 00 },
	{ "Croatia", "hr", 00 },
	{ "Cuba", "cu", 00 },
	{ "Cyprus", "cy", 00 },
	{ "Czech Republic", "cz", 00 },
	{ "Denmark", "dk", 00 },
	{ "Djibouti", "dj", 00 },
	{ "Dominica", "dm", 00 },
	{ "Dominican Republic", "do", 00 },
	{ "Ecuador", "ec", 00 },
	{ "Egypt", "eg", 00 },
	{ "El Salvador", "sv", 00 },
	{ "Equatorial Guinea", "gq", 00 },
	{ "Eritrea", "er", 00 },
	{ "Estonia", "ee", 00 },
	{ "Ethiopia", "et", 00 },
	{ "Falkland Islands (Malvinas)", "fk", 00 },
	{ "Faroe Islands", "fo", 00 },
	{ "Fiji", "fj", 00 },
	{ "Finland", "fi", 00 },
	{ "France", "fr", 33 },
	{ "French Guiana", "gf", 00 },
	{ "French Polynesia", "pf", 00 },
	{ "French Southern Territories", "tf", 00 },
	{ "Gabon", "ga", 00 },
	{ "Gambia", "gm", 00 },
	{ "Georgia", "ge", 00 },
	{ "Germany", "de", 00 },
	{ "Ghana", "gh", 00 },
	{ "Gibraltar", "gi", 00 },
	{ "Greece", "gr", 00 },
	{ "Greenland", "gl", 00 },
	{ "Grenada", "gd", 00 },
	{ "Guadeloupe", "gp", 00 },
	{ "Guam", "gu", 00 },
	{ "Guatemala", "gt", 00 },
	{ "Guernsey", " gg", 00 },
	{ "Guinea", "gn", 00 },
	{ "Guinea-Bissau", "gw", 00 },
	{ "Guyana", "gy", 00 },
	{ "Haiti", "ht", 00 },
	{ "Heard Island and McDonald Islands", "hm", 00 },
	{ "Holy See (Vatican City State)", "va", 00 },
	{ "Honduras", "hn", 00 },
	{ "Hong Kong", "hk", 00 },
	{ "Hungary", "hu", 00 },
	{ "Iceland", "is", 00 },
	{ "India", "in", 00 },
	{ "Indonesia", "id", 00 },
	{ "Iran", "ir", 00 },
	{ "Iraq", "iq", 00 },
	{ "Ireland", "ie", 00 },
	{ "Isle of Man", "im", 00 },
	{ "Israel", "il", 00 },
	{ "Italy", "it", 00 },
	{ "Jamaica", "jm", 00 },
	{ "Japan", "jp", 00 },
	{ "Jersey", "je", 00 },
	{ "jordan", "jo", 00 },
	{ "Kazakhstan", "kz", 00 },
	{ "Kenya", "ke", 00 },
	{ "Kiribati", "ki", 00 },
	{ "Korea, Democratic people's Republic of", "kp", 00 },
	{ "Korea, Republic of", "kr", 00 },
	{ "Kuwait", "kw", 00 },
	{ "Kyrgyzstan", "kg", 00 },
	{ "Lao people's Democratic Republic", "la", 00 },
	{ "Latvia", "lv", 00 },
	{ "Lebanon", "lb", 00 },
	{ "Lesotho", "ls", 00 },
	{ "Liberia", "lr", 00 },
	{ "Libya", "ly", 00 },
	{ "Liechtenstein", "li", 00 },
	{ "Lithuania", "lt", 00 },
	{ "Luxembourg", "lu", 00 },
	{ "Macao", "mo", 00 },
	{ "Macedonia", "mk", 00 },
	{ "Madagascar", "mg", 00 },
	{ "Malawi", "mw", 00 },
	{ "Malaysia", "my", 00 },
	{ "Maldives", "mv", 00 },
	{ "Mali", "ml", 00 },
	{ "Malta", "mt", 00 },
	{ "Marshall Islands", "mh", 00 },
	{ "Martinique", "mq", 00 },
	{ "Mauritania", "mr", 00 },
	{ "Mauritius", "mu", 00 },
	{ "Mayotte", "yt", 00 },
	{ "Mexico", "mx", 00 },
	{ "Micronesia, Federated States of", "fm", 00 },
	{ "Moldova", "md", 00 },
	{ "Monaco", "mc", 00 },
	{ "Mongolia", "mn", 00 },
	{ "Montserrat", "ms", 00 },
	{ "Morocco", "ma", 00 },
	{ "Mozambique", "mz", 00 },
	{ "Myanmar", "mm", 00 },
	{ "Namibia", "na", 00 },
	{ "Nauru", "nr", 00 },
	{ "Nepal", "np", 00 },
	{ "Netherlands", "nl", 00 },
	{ "Netherlands Antilles", "an", 00 },
	{ "New Caledonia", "nc", 00 },
	{ "New Zealand", "nz", 00 },
	{ "Nicaragua", "ni", 00 },
	{ "Niger", "ne", 00 },
	{ "Nigeria", "ng", 00 },
	{ "Niue", "nu", 00 },
	{ "Norfolk Island", "nf", 00 },
	{ "Northern Mariana Islands", "mp", 00 },
	{ "Norway", "no", 00 },
	{ "Oman", "om", 00 },
	{ "Pakistan", "pk", 00 },
	{ "Palau", "pw", 00 },
	{ "Palestinian Territory", "ps", 00 },
	{ "Panama", "pa", 00 },
	{ "Papua New Guinea", "pg", 00 },
	{ "Paraguay", "py", 00 },
	{ "Peru", "pe", 00 },
	{ "Philippines", "ph", 00 },
	{ "Pitcairn", "pn", 00 },
	{ "Poland", "pl", 00 },
	{ "Portugal", "pt", 00 },
	{ "Puerto rico", "pr", 00 },
	{ "Qatar", "qa", 00 },
	{ "Reunion", "re", 00 },
	{ "Romania", "ro", 00 },
	{ "Russia", "ru", 00 },
	{ "Rwanda", "rw", 00 },
	{ "Saint Helena", "sh", 00 },
	{ "Saint Kitts and Nevis", "kn", 00 },
	{ "Saint Lucia", "lc", 00 },
	{ "Saint Pierre and Miquelon", "pm", 00 },
	{ "Saint Vincent and the Grenadines", "vc", 00 },
	{ "Samoa", "ws", 00 },
	{ "San marino", "sm", 00 },
	{ "Sao Tome and Principe", "st", 00 },
	{ "Saudi arabia", "sa", 00 },
	{ "Senegal", "sn", 00 },
	{ "Serbia and Montenegro", "cs", 00 },
	{ "Seychelles", "sc", 00 },
	{ "Sierra leone", "sl", 00 },
	{ "Singapore", "sg", 00 },
	{ "Slovakia", "sk", 00 },
	{ "Slovenia", "si", 00 },
	{ "Solomon islands", "sb", 00 },
	{ "Somalia", "so", 00 },
	{ "South Africa", "za", 00 },
	{ "South Georgia and the South Sandwich Islands", "gs", 00 },
	{ "Spain", "es", 00 },
	{ "Sri Lanka", "lk", 00 },
	{ "Sudan", "sd", 00 },
	{ "Suriname", "sr", 00 },
	{ "Svalbard and Jan Mayen", "sj", 00 },
	{ "Swaziland", "sz", 00 },
	{ "Sweden", "se", 00 },
	{ "Switzerland", "ch", 00 },
	{ "Syria", "sy", 00 },
	{ "Taiwan", "tw", 00 },
	{ "Tajikistan", "tj", 00 },
	{ "Tanzania", "tz", 00 },
	{ "Thailand", "th", 00 },
	{ "Timor-Leste", "tl", 00 },
	{ "Togo", "tg", 00 },
	{ "Tokelau", "tk", 00 },
	{ "Tonga", "to", 00 },
	{ "Trinidad and Tobago", "tt", 00 },
	{ "Tunisia", "tn", 00 },
	{ "Turkey", "tr", 00 },
	{ "Turkmenistan", "tm", 00 },
	{ "Turks and Caicos Islands", "tc", 00 },
	{ "Tuvalu", "tv", 00 },
	{ "Uganda", "ug", 00 },
	{ "Ukraine", "ua", 00 },
	{ "United Arab Emirates", "ae", 00 },
	{ "United Kingdom", "gb", 00 },
	{ "United States", "us", 00 },
	{ "Uruguay", "uy", 00 },
	{ "Uzbekistan", "uz", 00 },
	{ "Vanuatu", "vu", 00 },
	{ "Venezuela", "ve", 00 },
	{ "Vietnam", "vn", 00 },
	{ "Virgin Islands, British", "vg", 00 },
	{ "Virgin Islands, U.S.", "vi", 00 },
	{ "Wallis and Futuna", "wf", 00 },
	{ "Western Sahara", "eh", 00 },
	{ "Yemen", "ye", 00 },
	{ "Zambia", "zm", 00 },
	{ "Zimbabwe", "zw", 00 }
};

#endif	//COUNTRIES_H
