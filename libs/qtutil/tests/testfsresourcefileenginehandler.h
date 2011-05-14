/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#ifndef TESTFSRESOURCEFILEENGINEHANDLER_H
#define TESTFSRESOURCEFILEENGINEHANDLER_H

#include <QtCore/QDir>

#include <qtutil/FSResourceFileEngineHandler.h>

#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;

void testWithEndSlash() {
	QString path = QDir::currentPath();
	if (!path.endsWith("/")) {
		path = path.append("/");
	}
	FSResourceFileEngineHandler handler(path);
	{
		QFile file(":/test.qm");
		BOOST_CHECK(file.open(QIODevice::ReadOnly));
	}
	{
		QFile file(":test.qm");
		BOOST_CHECK(file.open(QIODevice::ReadOnly));
	}
}

void testWithoutEndSlash() {
	QString path = QDir::currentPath();
	if (path.endsWith("/")) {
		path = path.left(path.length() - 1);
	}
	FSResourceFileEngineHandler handler(path);
	{
		QFile file(":/test.qm");
		BOOST_CHECK(file.open(QIODevice::ReadOnly));
	}
	{
		QFile file(":test.qm");
		BOOST_CHECK(file.open(QIODevice::ReadOnly));
	}
}

class FSResourceFileEngineHandlerTestSuite : public test_suite {
public:
	FSResourceFileEngineHandlerTestSuite() : test_suite("FSResourceFileEngineHandlerTestSuite") {
		add(BOOST_TEST_CASE(&testWithEndSlash));
		add(BOOST_TEST_CASE(&testWithoutEndSlash));
	}
};

#endif /* TESTFSRESOURCEFILEENGINEHANDLER_H */
