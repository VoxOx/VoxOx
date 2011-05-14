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

#ifndef FILETEST_H
#define FILETEST_H

#include <cutil/global.h>
#include <util/File.h>

#ifdef OS_POSIX
#include <sys/param.h>
#include <unistd.h>
#endif

class FileTest {
public:

	void testCreateTemporaryFile() {
		File tempFile = File::createTemporaryFile();
		BOOST_CHECK(tempFile.remove());

	#ifdef OS_POSIX
		char buffer[MAXPATHLEN];
		getcwd(buffer, sizeof(buffer));
		std::string pwd = buffer;
		setenv("TMPDIR", pwd.c_str(), 1);
		
		tempFile = File::createTemporaryFile();
		std::string fullPath = tempFile.getFullPath();
		BOOST_CHECK(tempFile.remove());

		BOOST_CHECK(fullPath.compare(0, pwd.size(), pwd) == 0);
	#endif
	}
};

class FileTestSuite : public test_suite {
public:
	FileTestSuite()
	: test_suite("FileTestSuite")
	{
		boost::shared_ptr<FileTest> instance(new FileTest());

		test_case * testCreateTemporaryFile = BOOST_CLASS_TEST_CASE(&FileTest::testCreateTemporaryFile, instance);

		add(testCreateTemporaryFile);
	}
};


#endif	//FILETEST_H
