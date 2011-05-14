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

#ifndef OWSTRINGLIST_H
#define OWSTRINGLIST_H

#include <util/owutildll.h>
#include <util/List.h>

#include <string>
#include <list>

/**
 * std::list<std::string> wrapper/helper.
 *
 * Inspired from the class QStringList from the Qt library.
 *
 * For converting a StringList object to a QStringList,
 * check libqtutil.
 *
 * @see QStringList
 * @see std::list
 * @author Tanguy Krotoff
 */
class OWUTIL_API StringList : public List<std::string> {
public:

	 static StringList null;

	 StringList();

	/**
	 * Converts a std::list<std::string> to a StringList using a constructor.
	 *
	 * @param strList list<string> to convert to StringList
	 */
	 StringList(const std::list<std::string> & strList);

	/**
	 * Converts this StringList to std::list<std::string>.
	 *
	 * @return the converted StringList to std::list<std::string>
	 */
	 operator std::list<std::string>();

	/**
	 * Permits to use StringList as an array.
	 *
	 * @see List::operator[]
	 * @param i index inside the array
	 * @return the std::string that corresponds to the index i inside the StringList
	 *         or String::null if index i is bigger than length()
	 */
	 std::string operator[](unsigned i) const;

	/**
	 * @see List::operator+=
	 */
	 void operator+=(const StringList & strList);

	/**
	 * @see List::operator+=
	 */
	 void operator+=(const std::string & str);

	/**
	 * Gets the number of occurrences of a string contained inside the StringList.
	 *
	 * Example:
	 * <pre>
	 * StringList strList;
	 * strList += "wengo";
	 * strList += "is";
	 * strList += "good";
	 * strList += "wengo";
	 * unsigned int i = strList.contains("wengo");	//i = 2
	 * unsigned int j = strList.contains("Wengo");	//j = 0
	 * unsigned int k = strList.contains("Wengo", false);	//k = 2
	 * </pre>
	 *
	 * @param str value to look up inside the StringList
	 * @param caseSensitive the search is case sensitive; otherwise the search is case insensitive
	 * @return number of occurences of the value str inside the StringList (0 if no occurence)
	 */
	 unsigned contains(const std::string & str, bool caseSensitive = true) const;

	/**
	 * Joins the string list into a single string given a separator.
	 *
	 * @param separator string delimiter
	 * @return the joined string with each element separated by the separator
	 */
	 std::string join(const std::string & separator) const;

	enum SortingOrder {
		Ascendant,
		Descendant
	};

	/**
	 * Sorts the string list alphabetically.
	 *
	 * @param order sorting order
	 */
	 void sort(SortingOrder order = Ascendant);

	/**
	 * Removes duplicated strings if any.
	 */
	 void removeDuplicatedStrings();

	/**
	 * Constructs a single string from every string
	 * contained in this StringList.
	 *
	 * @param separator the separator used to constructs the string
	 * @return the resulting string
	 */
	 std::string toString(const std::string & separator = " ") const;

private:

	class StringCompareDescendant {
	public:
		bool operator()(const std::string & s1, const std::string & s2) {
			return s2 < s1;
		}
	};
};

#endif	//OWSTRINGLIST_H
