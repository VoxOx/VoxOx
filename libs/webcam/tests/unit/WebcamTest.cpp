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

#include "NullWebcamDriverTest.h"

#if defined(OS_WINDOWS)
	#include "DirectXWebcamDriverTest.h"
#elif defined(OS_LINUX)
	#include "V4LWebcamDriverTest.h"
#elif defined(OS_MACOSX)
	#include "QuicktimeWebcamDriverTest.h"
#endif

#include <global.h>

test_suite * init_unit_test_suite(int argc, char * argv[]) {
	test_suite * test = BOOST_TEST_SUITE("Webcam TestSuite");

	test->add(new NullWebcamDriverTestSuite());

#if defined(OS_WINDOWS)
	test->add(new DirectXWebcamDriverTestSuite());
#elif defined(OS_LINUX)
	test->add(new V4LWebcamDriverTestSuite());
#elif defined(OS_MACOSX)
	test->add(new QuicktimeWebcamDriverTestSuite());
#endif
	return test;
}

