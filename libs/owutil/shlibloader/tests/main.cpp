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

#include <SharedLibLoader.h>

#include <global.h>

#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class SharedLibLoaderTest {
public:

	SharedLibLoaderTest() {
		_dlltest = "dlltest";
		_loader = new SharedLibLoader(_dlltest);
	}

	void testLoad() {
		BOOST_CHECK(_loader->load());
	}

	void testUnload() {
		testLoad();
		BOOST_CHECK(_loader->unload());
	}

	void testResolve() {
		testLoad();

		function_noparam noparam = (function_noparam) _loader->resolve("function_noparam");
		BOOST_CHECK(noparam);
		noparam();

		function_param_int param_int = (function_param_int) _loader->resolve("function_param_int");
		BOOST_CHECK(param_int);
		param_int(10);

		function_return_int return_int = (function_return_int) _loader->resolve("function_return_int");
		BOOST_CHECK(return_int);
		BOOST_CHECK(return_int());

		function_return_pointer return_pointer = (function_return_pointer) _loader->resolve("function_return_pointer");
		BOOST_CHECK(return_pointer);
		BOOST_CHECK(*return_pointer());
	}

	void testStaticResolve() {
		function_noparam noparam = (function_noparam) SharedLibLoader::resolve(_dlltest, "function_noparam");
		BOOST_CHECK(noparam);
		noparam();

		function_param_int param_int = (function_param_int) SharedLibLoader::resolve(_dlltest, "function_param_int");
		BOOST_CHECK(param_int);
		param_int(10);

		function_return_int return_int = (function_return_int) SharedLibLoader::resolve(_dlltest, "function_return_int");
		BOOST_CHECK(return_int);
		BOOST_CHECK(return_int());

		function_return_pointer return_pointer = (function_return_pointer) SharedLibLoader::resolve(_dlltest, "function_return_pointer");
		BOOST_CHECK(return_pointer);
		BOOST_CHECK(*return_pointer());
	}

private:

	typedef void (*function_noparam)();
	typedef void (*function_param_int)(int);
	typedef int (*function_return_int)();
	typedef int * (*function_return_pointer)();

	SharedLibLoader * _loader;

	std::string _dlltest;
};

class SharedLibLoaderTestSuite : public test_suite {
public:

	SharedLibLoaderTestSuite() : test_suite("SharedLibLoaderTestSuite") {
		boost::shared_ptr<SharedLibLoaderTest> instance(new SharedLibLoaderTest());

		test_case * testLoad = BOOST_CLASS_TEST_CASE(&SharedLibLoaderTest::testLoad, instance);
		test_case * testUnload = BOOST_CLASS_TEST_CASE(&SharedLibLoaderTest::testUnload, instance);
		test_case * testResolve = BOOST_CLASS_TEST_CASE(&SharedLibLoaderTest::testResolve, instance);
		test_case * testStaticResolve = BOOST_CLASS_TEST_CASE(&SharedLibLoaderTest::testStaticResolve, instance);

		add(testLoad);
		add(testUnload);
		add(testResolve);
		add(testStaticResolve);
	}
};

test_suite * init_unit_test_suite(int argc, char * argv[]) {
	test_suite * test = BOOST_TEST_SUITE("SharedLibLoader TestSuite");

	test->add(new SharedLibLoaderTestSuite());

	return test;
}
