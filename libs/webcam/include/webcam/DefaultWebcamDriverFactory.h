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

#ifndef OWDEFAULTWEBCAMDRIVERFACTORY_H
#define OWDEFAULTWEBCAMDRIVERFACTORY_H

#include <webcam/WebcamDriverFactory.h>

#include <cutil/global.h>

#if defined(CC_MSVC)
	#include <webcam/DirectXWebcamDriver.h>
#elif defined(OS_LINUX)
	#include <webcam/V4L2WebcamDriver.h>
#elif defined(OS_MACOSX)
	#include <webcam/QuicktimeWebcamDriver.h>
#else
	#include <webcam/NullWebcamDriver.h>
#endif

/**
 * Default webcam driver factory.
 *
 * The default webcam driver depends on the build platform:
 *	- MacOS X: Quicktime
 *	- Linux: Video for Linux (V4L)
 *	- Windows: DirectX
 *
 * @author Philippe Bernery
 */
class DefaultWebcamDriverFactory : public WebcamDriverFactory {
public:
	DefaultWebcamDriverFactory() {};
	virtual ~DefaultWebcamDriverFactory() {};

	IWebcamDriver * create(WebcamDriver * driver, int flags) const {
#if defined(CC_MSVC)
		return new DirectXWebcamDriver(driver, flags);
#elif defined(OS_LINUX)
		return new V4L2WebcamDriver(driver, flags);
#elif defined(OS_MACOSX)
		return new QuicktimeWebcamDriver(driver, flags);
#else
		return new NullWebcamDriver(driver, flags);
#endif
	}
};

#endif	//OWDEFAULTWEBCAMDRIVERFACTORY_H
