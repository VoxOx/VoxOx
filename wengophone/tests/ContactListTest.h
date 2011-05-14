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
#ifndef CONTACTLISTTEST_H
#define CONTACTLISTTEST_H
 
#include <model/contactlist/ContactList.h>
#include <model/contactlist/ContactGroup.h>
#include <model/contactlist/Contact.h>

#include <string>
using namespace std;

#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class ContactListTest {
public:

	ContactListTest() {
	}

	void testContactGroup() {
		const string groupName = "test group";
		ContactGroup group(groupName);
		
		BOOST_CHECK(group.toString() == groupName);
	}
	
	void testContactList() {
		const string groupName1 = "test group 1";
		const string groupName2 = "test group 2";
		ContactGroup group1(groupName1);
		ContactGroup group2(groupName2);
		ContactList list;
		
		list.addContactGroup(&group1);
		BOOST_CHECK(list.size() == 1);
		BOOST_CHECK(list[0] == group1);
				
		list.addContactGroup(&group2);
		BOOST_CHECK(list.size() == 2);
		BOOST_CHECK(list[0] == group1);
		BOOST_CHECK(list[1] == group2);
		
		BOOST_CHECK(*list[groupName1] == group1);
		BOOST_CHECK(*list[groupName2] == group2);
		
		list.removeContactGroup(&group1);
		BOOST_CHECK(list.size() == 1);
		BOOST_CHECK(list[0] == group2);
		
		list.removeContactGroup(&group1);
		BOOST_CHECK(list.size() == 1);
		
		list.removeContactGroup(&group2);
		BOOST_CHECK(list.size() == 0);
	}

private:
};


class ContactListTestSuite : public test_suite {
public:

	ContactListTestSuite() : test_suite("ContactListTestSuite") {
		boost::shared_ptr<ContactListTest> instance(new ContactListTest());

		test_case * testContactGroup = BOOST_CLASS_TEST_CASE(&ContactListTest::testContactGroup, instance);
		test_case * testContactList = BOOST_CLASS_TEST_CASE(&ContactListTest::testContactList, instance);

		add(testContactGroup);
		add(testContactList);
	}
};
 
#endif //CONTACTLISTTEST_H