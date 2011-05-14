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

#ifndef PIXERTOOLTEST_H
#define PIXERTOOLTEST_H

#include <global.h>

#ifdef OS_MACOSX
#include <SDL_image/SDL_image.h>
#else
#include <SDL_image.h>
#endif //!OS_MACOSX

#include <pixertool.h>

#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;

class PixerToolTest {
public:
	PixerToolTest() {
		formats.push_back(PIX_OSI_YUV422P);
		formats.push_back(PIX_OSI_YUV444P);
		formats.push_back(PIX_OSI_YUV422);
		formats.push_back(PIX_OSI_YUV411);
		formats.push_back(PIX_OSI_YUV424P);
		formats.push_back(PIX_OSI_YUV41P);
		formats.push_back(PIX_OSI_YUY2);
		formats.push_back(PIX_OSI_YUYV);
		formats.push_back(PIX_OSI_YVYU);
		formats.push_back(PIX_OSI_UYVY);
		formats.push_back(PIX_OSI_YV12);
		formats.push_back(PIX_OSI_UNSUPPORTED);
		formats.push_back(PIX_OSI_YUV420P);
		formats.push_back(PIX_OSI_I420);
		formats.push_back(PIX_OSI_RGB555);
		formats.push_back(PIX_OSI_RGB565);
		formats.push_back(PIX_OSI_RGB1);
		formats.push_back(PIX_OSI_RGB4);
		formats.push_back(PIX_OSI_RGB8);
		formats.push_back(PIX_OSI_RGB32);
		formats.push_back(PIX_OSI_RGB24);
		formats.push_back(PIX_OSI_NV12);
		
		flags.push_back(PIX_NO_FLAG);
		flags.push_back(PIX_FLIP_HORIZONTALLY);
	}
                               
	void testIdConversion() {
	}
	
	void testAllocFree() {
		for (vector<pixosi>::const_iterator curFormat = formats.begin() ; curFormat != formats.end() ; curFormat++) {
			if (pix_ffmpeg_from_pix_osi(*curFormat) != PIX_OSI_UNSUPPORTED) {
				piximage *image = pix_alloc(*curFormat, 320, 240);
				BOOST_CHECK_MESSAGE(image != NULL, "Error allocating 320x240 " << pixosiToString(*curFormat) << " picture");
				
				pix_free(image);
			}
		}
	}

	/**
	 * Check data conversion.
	 * 
	 * Check data conversion from a RGB24 picture to every available format
	 * with every available flags
	 */
	void testDataConversion() {
		SDL_Surface *image;
		int result;
		pixosi source_format = PIX_OSI_RGB24;
		piximage source;
		
		image = IMG_Load("test.bmp");
		BOOST_REQUIRE_MESSAGE(image != NULL, "Error while loading test picture: " << IMG_GetError());

		SDL_LockSurface(image);

		source.palette = source_format;
		source.width = image->w;
		source.height = image->h;
		source.data = (uint8_t *)image->pixels;

		for (vector<pixosi>::const_iterator curFormat = formats.begin() ; curFormat != formats.end() ; curFormat++) {
			if (pix_ffmpeg_from_pix_osi(*curFormat) != PIX_OSI_UNSUPPORTED) {		
				for (vector<pixflag>::const_iterator curFlag = flags.begin() ; curFlag != flags.end() ; curFlag++) {
					piximage *result = pix_convert(*curFlag,
								*curFormat,
								&source);
					BOOST_CHECK_MESSAGE(result != NULL, "Formats not supported");

					piximage *result2 = pix_convert(*curFlag,
								source.palette,
								result);
					BOOST_CHECK_MESSAGE(result2 != NULL, "Formats not supported");

					cout << "Checking conversion from " << pixosiToString(source.palette)
						<< " to " << pixosiToString(*curFormat) 
						<< " with " << ((*curFlag == PIX_FLIP_HORIZONTALLY) ? "horizontal flip" : "no flag") 
						<< endl;

					BOOST_CHECK(memcmp(source.data, result2->data, pix_size(source_format, image->w, image->h)) == 0);
					
					pix_free(result2);
					pix_free(result);
				}
			}
		}

		SDL_UnlockSurface(image);
		SDL_FreeSurface(image);
	}

private:
	vector<pixosi> formats;
	vector<int> flags;
	
	string pixosiToString(pixosi format) {
		string result;
		
		switch (format) {
		case PIX_OSI_YUV420P:
			result = "yuv420p";
			break;
		case PIX_OSI_I420:
			result = "i420";
			break;
		case PIX_OSI_YUV422:
			result = "yuv422";
			break;
		case PIX_OSI_YUV411:
			result = "yuv411";
			break;
		case PIX_OSI_YUV422P:
			result = "yuv422p";
			break;
		case PIX_OSI_YUV444P:
			result = "yuv444p";
			break;
		case PIX_OSI_YUV424P:
			result = "yuv424p";
			break;
		case PIX_OSI_YUV41P:
			result = "yuv41p";
			break;
		case PIX_OSI_YUY2:
			result = "yuy2";
			break;
		case PIX_OSI_YUYV:
			result = "yuyv";
			break;
		case PIX_OSI_YVYU:
			result = "yvyu";
			break;
		case PIX_OSI_UYVY:
			result = "uyvy";
			break;
		case PIX_OSI_YV12:
			result = "yv12";
			break;
		case PIX_OSI_RGB555:
			result = "rgb555";
			break;
		case PIX_OSI_RGB565:
			result = "rgb565";
			break;
		case PIX_OSI_RGB1:
			result = "rgb1";
			break;
		case PIX_OSI_RGB4:
			result = "rgb4";
			break;
		case PIX_OSI_RGB8:
			result = "rgb8";
			break;
		case PIX_OSI_RGB32:
			result = "rgb32";
			break;
		case PIX_OSI_RGB24:
			result = "rgb24";
			break;
		case PIX_OSI_NV12:
			result = "nv12";
			break;
		case PIX_OSI_UNSUPPORTED:
		default:
			result = "does not exist";
		}
		
		return result;
	}
};

class PixerToolTestSuite : public test_suite {
public:

	PixerToolTestSuite() : test_suite("PixerToolTestSuite") {
		boost::shared_ptr<PixerToolTest> instance(new PixerToolTest());

		test_case * testIdConversion = BOOST_CLASS_TEST_CASE(&PixerToolTest::testIdConversion, instance);
		test_case * testAllocFree = BOOST_CLASS_TEST_CASE(&PixerToolTest::testAllocFree, instance);
		test_case * testDataConversion = BOOST_CLASS_TEST_CASE(&PixerToolTest::testDataConversion, instance);

		add(testIdConversion);
		add(testAllocFree);
		add(testDataConversion);
	}
};

#endif //PIXERTOOLTEST_H

