/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef STRINGLISTTEST_H
#define STRINGLISTTEST_H

#include <util/StringList.h>

#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include <vector>
using namespace std;

class StringListTest {
public:

	void testOperatorAdd() {
		testOperatorIndex();
	}

	void testOperatorIndex() {
		StringList strList;
		strList += "wengo0";
		strList += "wengo1";
		strList += "wengo2";
		strList += "wengo3";
		BOOST_CHECK(strList[0] == "wengo0");
		BOOST_CHECK(strList[1] == "wengo1");
		BOOST_CHECK(strList[2] == "wengo2");
		BOOST_CHECK(strList[3] == "wengo3");
		BOOST_CHECK(strList[4] == String::null);
		BOOST_CHECK(strList[4].empty());
	}

	void testContains() {
		StringList strList;
		strList += "wengo";
		strList += "is";
		strList += "good";
		strList += "wengo";
		BOOST_CHECK(strList.contains("wengo") == 2);
		BOOST_CHECK(strList.contains("Wengo") == 0);
		BOOST_CHECK(strList.contains("Wengo", false) == 2);
	}
	
	void testSort() {
		vector<string> list;
		list.push_back("c");
		list.push_back("e");
		list.push_back("d");
		list.push_back("a");
		list.push_back("b");
		
		StringList strList;
		
		for (vector<string>::const_iterator i = list.begin() ; i != list.end() ; i++) {
			strList += *i;
		}
		
		// Ascending order
		sort(list.begin(), list.end());
		strList.sort();
		
		for (register unsigned i = 0 ; i < strList.size() ; i++) {
			BOOST_CHECK(list[i] == strList[i]);
		}
		
		//Descending order
		sort(list.begin(), list.end(), StringCompareDescendant());
		strList.sort(StringList::Descendant);

		for (register unsigned i = 0 ; i < strList.size() ; i++) {
			BOOST_CHECK(list[i] == strList[i]);
		}	
	}
	
private:
	class StringCompareDescendant {
	public:
		bool operator() (const string & s1, const string & s2) {
			return s2 < s1;
		}
	};
};


class StringListTestSuite : public test_suite {
public:

	StringListTestSuite() : test_suite("StringListTestSuite") {
		boost::shared_ptr<StringListTest> instance(new StringListTest());

		test_case * testOperatorIndex = BOOST_CLASS_TEST_CASE(&StringListTest::testOperatorIndex, instance);
		test_case * testOperatorAdd = BOOST_CLASS_TEST_CASE(&StringListTest::testOperatorAdd, instance);
		test_case * testContains = BOOST_CLASS_TEST_CASE(&StringListTest::testContains, instance);
		test_case * testSort = BOOST_CLASS_TEST_CASE(&StringListTest::testSort, instance);
		
		add(testOperatorIndex);
		add(testOperatorAdd);
		add(testContains);
		add(testSort);
	}
};

#endif	//STRINGLISTTEST_H
