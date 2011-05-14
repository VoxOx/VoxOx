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

#ifndef WEBCAMDRIVER_H
#define WEBCAMDRIVER_H

#include <webcam/IWebcamDriver.h>

#include <pixertool/pixertool.h>

#include <cutil/global.h>

#include <thread/RecursiveMutex.h>

#if defined(OS_WIN32)
	class DirectXWebcamDriver;
#elif defined(OS_MACOSX)
	class QuicktimeWebcamDriver;
#elif defined(OS_LINUX)
    class V4LWebcamDriver;
	class V4L2WebcamDriver;
#endif

class WebcamDriverFactory;

/**
 * TODO: implements a setFlags method for WEBCAM_FORCE_PALETTE_CONVERSION
 *
 * @see IWebcamDriver
 * @author Philippe Bernery
 * @author Mathieu Stute
 */
class WebcamDriver : public IWebcamDriver {
public:

	/**
	 * Initialize the api.
	 * Under Windows any thread accessing this api must call
	 * this function in order to use it.
	 */
	WEBCAM_API static void apiInitialize();

	/**
	 * Uninitialize the api.
	 * Under Windows any thread that has called apiInitialize 
	 * must call this function.
	 */
	WEBCAM_API static void apiUninitialize();

	/**
	 * Sets current factory
	 *
	 * @param factory factory to use
	 */
	WEBCAM_API static void setFactory(WebcamDriverFactory * factory);

	/**
	 * Gets instance of WebcamDriver.
	 *
	 * Implements singleton pattern. This method create a WebcamDriver with
	 * WEBCAM_FORCE_IMAGE_FORMAT flag.
	 */
	WEBCAM_API static WebcamDriver * getInstance();

	WEBCAM_API void setFlags(int flags);

	WEBCAM_API void unsetFlags(int flags);

	WEBCAM_API bool isFlagSet(int flag);

	WEBCAM_API StringList getDeviceList();

	WEBCAM_API std::string getDefaultDevice();

	WEBCAM_API WebcamErrorCode setDevice(const std::string & deviceName);

	WEBCAM_API bool isOpen() const;

	WEBCAM_API void startCapture();

	WEBCAM_API void pauseCapture();

	/**
	 * Stops webcam capture.
	 *
	 * There must be same amount of call to stopCapture than to
	 * startCapture to uninitialize correctly the driver.
	 */
	WEBCAM_API void stopCapture();

	WEBCAM_API WebcamErrorCode setPalette(pixosi palette);

	WEBCAM_API pixosi getPalette() const;

	WEBCAM_API WebcamErrorCode setFPS(unsigned fps);

	WEBCAM_API unsigned getFPS() const;

	WEBCAM_API WebcamErrorCode setResolution(unsigned width, unsigned height);

	WEBCAM_API unsigned getWidth() const;

	WEBCAM_API unsigned getHeight() const;

	WEBCAM_API void setBrightness(int brightness);

	WEBCAM_API int getBrightness() const;

	WEBCAM_API void setContrast(int contrast);

	WEBCAM_API int getContrast() const;

	WEBCAM_API void flipHorizontally(bool flip);

	/** Do not use: this is an internal method. */
	void cleanup();

private:

	static WebcamDriver * instance;

	WebcamDriver(int flags);

	virtual ~WebcamDriver();

#if defined(OS_WINDOWS)
	friend class DirectXWebcamDriver;
#elif defined(OS_MACOSX)
	friend class QuicktimeWebcamDriver;
#elif defined(OS_LINUX)
    friend class V4LWebcamDriver;
	friend class V4L2WebcamDriver;
#endif

	/**
	 * Called by platform driver when a frame is available.
	 *
	 * The frame will then be sent if the time since last frame correponds
	 * to desired fps. The fps can be managed by the software if the webcam
	 * does not support the requested fps
	 * @param image captured image
	 */
	void frameBufferAvailable(piximage * image);

	/**
	 * @see WEBCAM_FORCE_IMAGE_FORMAT
	 */
	bool isFormatForced() const;

	/**
	 * Initializes _convImage.
	 */
	void initializeConvImage();

	/**
	 * Forces capture to stop.
	 */
	void forceStopCapture();

	/** Pointer to factory to use. */
	static WebcamDriverFactory * _factory;

	/** Pointer to paltform dependent webcam driver. */
	IWebcamDriver * _webcamPrivate;

	/** Desired FPS. */
	unsigned _forcedFPS;

	/** Time since last frame. */
	float _fpsTimerLast;

	/**
	 * True if FPS must be forced.
	 */
	bool _forceFPS;

	/** Desired palette. */
	pixosi _desiredPalette;

	/** Desired width. */
	unsigned _desiredWidth;

	/** Desired height */
	unsigned _desiredHeight;

	/** Flags. */
	int _flags;

	/** Contains the converted captured frame. */
	piximage * _convImage;

	/** Conversion flags. */
	int _convFlags;

	/** Call to startCapture counter. */
	unsigned _startCounter;

	/** True if the WebcamDriver is running and used. */
	bool _isRunning;

	/** Mutex for thread safety. */
	mutable RecursiveMutex _mutex;
};

#endif	//WEBCAMDRIVER_H
