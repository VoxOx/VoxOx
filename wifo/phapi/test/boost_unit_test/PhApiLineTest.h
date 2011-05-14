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

#ifndef PHAPILINETEST_H
#define PHAPILINETEST_H

#include <phapi.h>
#include <phapi-old.h>

#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/**
 * TODO
 * - replace phcfg direct accesses by getters
 * - unify NULL parameter passing handling
 */

class PhApiLineTest {
public:

};

class PhApiLineTestSuite : public test_suite {
public:

	PhApiLineTestSuite() : test_suite("PhApiLineTestSuite") {
		boost::shared_ptr<PhApiLineTest> instance(new PhApiLineTest());

	}
};

#endif /* PHAPILINETEST_H */