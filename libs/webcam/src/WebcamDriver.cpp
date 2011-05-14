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

#include <webcam/WebcamDriver.h>

#include <webcam/DefaultWebcamDriverFactory.h>

#include <util/Logger.h>
#include <util/String.h>
#include <util/SafeDelete.h>

WebcamDriverFactory * WebcamDriver::_factory = NULL;

#include <iostream>
using namespace std;

WebcamDriver * WebcamDriver::instance = NULL;

void WebcamDriver::setFactory(WebcamDriverFactory * factory) {
	_factory = factory;
}

void WebcamDriver::apiInitialize() {
#if defined(CC_MSVC)
	CoInitialize(NULL);
#endif
}

void WebcamDriver::apiUninitialize() {
#if defined(CC_MSVC)
	CoUninitialize();
#endif
}

WebcamDriver * WebcamDriver::getInstance() {
	if (!instance) {
		instance = new WebcamDriver(WEBCAM_FORCE_IMAGE_FORMAT);
	}

	return instance;
}

WebcamDriver::WebcamDriver(int flags)
	: IWebcamDriver(flags),
	_desiredPalette(PIX_OSI_YUV420P),
	_desiredWidth(320),
	_desiredHeight(240) {

	if (!_factory) {
		_factory = new DefaultWebcamDriverFactory();
	}

	_webcamPrivate = _factory->create(this, flags);

	_convImage = NULL;

	cleanup();

	_forceFPS = false;
	_forcedFPS = 15;
	_fpsTimerLast = 0;

	_flags = flags;

	_convFlags = PIX_NO_FLAG;

	_startCounter = 0;
}

WebcamDriver::~WebcamDriver() {
	forceStopCapture();
	OWSAFE_DELETE(_webcamPrivate);

	if (_convImage) {
		pix_free(_convImage);
	}
}

void WebcamDriver::cleanup() {
	LOG_DEBUG("Cleaning up the Meta webcam driver");
	_webcamPrivate->cleanup();

	initializeConvImage();
}

void WebcamDriver::setFlags(int flags) {
	_flags |= flags;
}

void WebcamDriver::unsetFlags(int flags) {
	_flags &= ~flags;
}

bool WebcamDriver::isFlagSet(int flag) {
	//FIXME warning under Visual C++ 7.1
	return (_flags & flag);
}

StringList WebcamDriver::getDeviceList() {
	RecursiveMutex::ScopedLock lock(_mutex);

	return _webcamPrivate->getDeviceList();
}

std::string WebcamDriver::getDefaultDevice() {
	RecursiveMutex::ScopedLock lock(_mutex);

	return _webcamPrivate->getDefaultDevice();
}

WebcamErrorCode WebcamDriver::setDevice(const std::string & deviceName) {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (_startCounter == 0) {
		cleanup();

		std::string actualDeviceName = deviceName;
		if (actualDeviceName.empty()) {
			actualDeviceName = getDefaultDevice();
		}

		LOG_DEBUG("desired device=" + deviceName + ", actual device=" + actualDeviceName);
		return _webcamPrivate->setDevice(actualDeviceName);
	} else {
		LOG_WARN("WebcamDriver is running. Can't set a device.");
		//FIXME: should return WEBCAM_NOK but does not because:
		// If 2 objects registers to the frameCapturedEvent and call
		// startCapture, the second one will not work, even if the deviceName is the same.
		return WEBCAM_OK;
	}
}

bool WebcamDriver::isOpen() const {
	RecursiveMutex::ScopedLock lock(_mutex);

	return _webcamPrivate->isOpen();
}

void WebcamDriver::startCapture() {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (_startCounter == 0) {
		LOG_DEBUG("starting capture");
		_webcamPrivate->startCapture();
	} else {
		LOG_INFO("capture is already started");
	}

	_startCounter++;
}

void WebcamDriver::pauseCapture() {
	RecursiveMutex::ScopedLock lock(_mutex);

	LOG_DEBUG("pausing capture");
	_webcamPrivate->pauseCapture();
}

void WebcamDriver::stopCapture() {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (_startCounter > 0) {
		_startCounter--;
	}

	if (_startCounter == 0) {
		LOG_DEBUG("stopping capture");
		_webcamPrivate->stopCapture();
		cleanup();
	}
}

WebcamErrorCode WebcamDriver::setPalette(pixosi palette) {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (_startCounter == 0) {
		if (_webcamPrivate->setPalette(palette) == WEBCAM_NOK) {
			LOG_DEBUG("this webcam does not support palette #" + String::fromNumber(palette));
			if (isFormatForced()) {
				LOG_DEBUG("palette conversion will be forced");
				_desiredPalette = palette;
				initializeConvImage();
				return WEBCAM_OK;
			} else {
				return WEBCAM_NOK;
			}
		} else {
			LOG_DEBUG("this webcam supports palette #" + String::fromNumber(palette));
			_desiredPalette = palette;
			return WEBCAM_OK;
		}
	} else {
		LOG_INFO("WebcamDriver is running, can't set palette");
		return WEBCAM_NOK;
	}
}

pixosi WebcamDriver::getPalette() const {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (isFormatForced()) {
		return _desiredPalette;
	} else {
		return _webcamPrivate->getPalette();
	}
}

WebcamErrorCode WebcamDriver::setFPS(unsigned fps) {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (_startCounter == 0) {
		if (_webcamPrivate->setFPS(fps) == WEBCAM_NOK) {
			LOG_DEBUG("this webcam does not support the desired fps(" + String::fromNumber(fps) + "), will force it");
			_forceFPS = true;
		} else {
			LOG_DEBUG("webcam FPS changed to=" + String::fromNumber(fps));
			_forceFPS = false;
		}

		_forcedFPS = fps;

		return WEBCAM_OK;
	} else {
		LOG_INFO("WebcamDriver is running, can't set FPS");
		return WEBCAM_NOK;
	}
}

unsigned WebcamDriver::getFPS() const {
	RecursiveMutex::ScopedLock lock(_mutex);

	return _forcedFPS;
}

WebcamErrorCode WebcamDriver::setResolution(unsigned width, unsigned height) {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (_startCounter == 0) {
		LOG_DEBUG("try to change resolution: (width, height)=" + String::fromNumber(width) + "," + String::fromNumber(height));
		if (_webcamPrivate->setResolution(width, height) == WEBCAM_NOK) {
			if (isFormatForced()) {
				_desiredWidth = width;
				_desiredHeight = height;
				initializeConvImage();
				return WEBCAM_OK;
			} else {
				return WEBCAM_NOK;
			}
		} else {
			_desiredWidth = width;
			_desiredHeight = height;
			return WEBCAM_OK;
		}
	} else {
		LOG_INFO("WebcamDriver is running, can't set resolution");
		return WEBCAM_NOK;
	}
}

unsigned WebcamDriver::getWidth() const {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (isFormatForced()) {
		return _desiredWidth;
	} else {
		return _webcamPrivate->getWidth();
	}
}

unsigned WebcamDriver::getHeight() const {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (isFormatForced()) {
		return _desiredHeight;
	} else {
		return _webcamPrivate->getHeight();
	}
}

void WebcamDriver::setBrightness(int brightness) {
	RecursiveMutex::ScopedLock lock(_mutex);

	_webcamPrivate->setBrightness(brightness);
}

int WebcamDriver::getBrightness() const {
	RecursiveMutex::ScopedLock lock(_mutex);

	return _webcamPrivate->getBrightness();
}

void WebcamDriver::setContrast(int contrast) {
	RecursiveMutex::ScopedLock lock(_mutex);

	_webcamPrivate->setContrast(contrast);
}

int WebcamDriver::getContrast() const {
	RecursiveMutex::ScopedLock lock(_mutex);

	return _webcamPrivate->getContrast();
}

void WebcamDriver::flipHorizontally(bool flip) {
	RecursiveMutex::ScopedLock lock(_mutex);

	if (flip) {
		LOG_DEBUG("enable horizontal flip");
		_convFlags |= PIX_FLIP_HORIZONTALLY;
	} else {
		LOG_DEBUG("disable horizontal flip");
		_convFlags &= ~PIX_FLIP_HORIZONTALLY;
	}
}

void WebcamDriver::frameBufferAvailable(piximage * image) {
	float now;
	float fpsTimerInter = 1000 / (float)_forcedFPS;

#ifdef CC_MSVC
	now = timeGetTime();
#else
	now = clock() / ((float)CLOCKS_PER_SEC / 1000.0);
#endif
	if (_forceFPS) {
		if ((now - _fpsTimerLast) < fpsTimerInter) {
			return;
		}
		_fpsTimerLast = now;
	}

	if ((isFormatForced() && ((_desiredPalette != image->palette)
			|| (_desiredWidth != image->width) || (_desiredHeight != image->height)))
		|| (_convFlags != PIX_NO_FLAG)) {
		/* Disable this log because it's called several times per second
		LOG_DEBUG("conversion needed: from palette #" + String::fromNumber(image->palette)
			+ " to palette #" + String::fromNumber(_desiredPalette)
			+ " and from (" + String::fromNumber(image->width) + "x" + String::fromNumber(image->height)
			+ ") to (" + String::fromNumber(_desiredWidth) + "x" + String::fromNumber(_desiredHeight) + ")");
		*/

		pix_convert(_convFlags, _convImage, image);

		frameCapturedEvent(this, _convImage);
	} else {
		//LOG_DEBUG("no conversion needed");
		frameCapturedEvent(this, image);
	}
}

bool WebcamDriver::isFormatForced() const {
	return _flags & WEBCAM_FORCE_IMAGE_FORMAT;
}

void WebcamDriver::initializeConvImage() {
	if (_convImage) {
		pix_free(_convImage);
	}

	_convImage = pix_alloc(_desiredPalette, _desiredWidth, _desiredHeight);
}

void WebcamDriver::forceStopCapture() {
	unsigned startCounter = _startCounter;

	for (unsigned i = 0; i < startCounter; i++) {
		stopCapture();
	}
}
