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
#include <util/StringList.h>

#include <util/String.h>
#include <util/Logger.h>

using namespace std;

StringList StringList::null;

StringList::StringList(const std::list<std::string> & strList) {
	for (std::list<string>::const_iterator it = strList.begin();
		it != strList.end(); ++it) {

		push_back(*it);
	}
}

StringList::StringList() {
}

StringList::operator std::list<std::string>() {
	std::list<std::string> strList;
	for (unsigned i = 0; i < size(); i++) {
		strList.push_back((*this)[i]);
	}
	return strList;
}

std::string StringList::operator[](unsigned i) const {
	if (i >= size()) {
		return String::null;
	}

	return List<std::string>::operator[](i);
}

void StringList::operator+=(const std::string & str) {
	List<std::string>::operator+=(str);
}

void StringList::operator+=(const StringList & strList) {
	for (unsigned i = 0; i < strList.size(); i++) {
		(*this) += strList[i];
	}
}

unsigned StringList::contains(const std::string & str, bool caseSensitive) const {
	unsigned result = 0;
	for (unsigned i = 0; i < size(); i++) {
		String tmp1 = str;
		String tmp2 = (*this)[i];
		if (!caseSensitive) {
			tmp1 = tmp1.toLowerCase();
			tmp2 = tmp2.toLowerCase();
		}

		if (tmp1 == tmp2) {
			++result;
		}
	}

	return result;
}

void StringList::sort(SortingOrder order) {
	switch(order) {
	case Ascendant:
		std::sort(begin(), end());
		break;

	case Descendant:
		std::sort(begin(), end(), StringCompareDescendant());
		break;

	default:
		LOG_FATAL("unknown sorting order=" + String::fromNumber(order));
	}
}

std::string StringList::join(const std::string & separator) const {
	std::string joinedString;
	for (unsigned i = 0; i < size(); i++) {
		//Last token
		if (i == (size() - 1)) {
			joinedString += (*this)[i];
		}
		else {
			joinedString += (*this)[i] + separator;
		}
	}
	return joinedString;
}

void StringList::removeDuplicatedStrings() {
	for (unsigned i = 0; i < size(); i++) {
		std::string tmp = (*this)[i];
		for (unsigned j = 0; j < size(); j++) {
			if (tmp == (*this)[j] && i != j) {
				remove(tmp);

				//Restarts the entire loop
				i = 0;
				break;
			}
		}
	}
}

std::string StringList::toString(const std::string & separator) const {
	std::string result;

	for (const_iterator it = begin(); it != end(); ++it) {
		if (it != begin()) {
			result += separator;
		}

		result += *it;
	}

	return result;
}
