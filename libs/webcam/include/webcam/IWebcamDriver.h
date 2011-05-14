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

#ifndef OWIWEBCAMDRIVER_H
#define OWIWEBCAMDRIVER_H

#include <webcam/webcam.h>

#include <pixertool/pixertool.h>

#include <util/Event.h>
#include <util/NonCopyable.h>
#include <util/StringList.h>
#include <util/Trackable.h>

#include <string>

/**
 * Drive a webcam device.
 *
 * @author Philippe Bernery
 */
class IWebcamDriver : NonCopyable, public Trackable {
public:

	/**
	 * Gets webcam device list.
	 */
	virtual StringList getDeviceList() = 0;

	/**
	 * Gets default webcam device.
	 *
	 * @return the name of the device; WEBCAM_NULL (empty string) if no webcam or error
	 */
	virtual std::string getDefaultDevice() = 0;

	/**
	 * A frame has been captured.
	 *
	 * @param sender sender of the event
	 * @param image captured image
	 */
	Event<void (IWebcamDriver * sender, piximage * image)> frameCapturedEvent;

	/**
	 * @param flags set the webcam driver flags
	 */
	IWebcamDriver(int flags) { }

	virtual ~IWebcamDriver() { }

	/**
	 * Initializes/cleanups private member variables.
	 */
	virtual void cleanup() = 0;

	/**
	 * Sets flags.
	 *
	 * @param flags OR_bitwise flags to set
	 * @see webcam.h for documentation about valid flags
	 */
	virtual void setFlags(int flags) = 0;

	/**
	 * Unsets flags.
	 *
	 * @param flags OR-bitwise flags to unset
	 */
	virtual void unsetFlags(int flags) = 0;

	/**
	 * Is given flag set?.
	 *
	 * @param flag flag to check
	 * @return true if set
	 */
	virtual bool isFlagSet(int flag) = 0;

	/**
	 * Sets webcam device to use.
	 *
	 * @param deviceName name of the device to use, if name == "" then use the default device
	 * @return WEBCAM_OK if no error; WEBCAM_NOK otherwise
	 */
	virtual WebcamErrorCode setDevice(const std::string & deviceName) = 0;

	/**
	 * @return true webcam if is opened
	 */
	virtual bool isOpen() const = 0;

	/**
	 * Starts capture of video frames.
	 */
	virtual void startCapture() = 0;

	/**
	 * Pauses capture of video frames.
	 */
	virtual void pauseCapture() = 0;

	/**
	 * Stops capture of video frames.
	 */
	virtual void stopCapture() = 0;

	/**
	 * Sets palette.
	 *
	 * @param palette palette to use
	 */
	virtual WebcamErrorCode setPalette(pixosi palette) = 0;

	/**
	 * Gets palette.
	 *
	 * @return current palette
	 */
	virtual pixosi getPalette() const = 0;

	/**
	 * Sets capture rate.
	 *
	 * @param fps desired frame per seconds
	 */
	virtual WebcamErrorCode setFPS(unsigned fps) = 0;

	/**
	 * Gets capture rate.
	 *
	 * @return current frame per seconds
	 */
	virtual unsigned getFPS() const = 0;

	/**
	 * Sets capture resolution.
	 *
	 * @param width desired width
	 * @param height desired height
	 */
	virtual WebcamErrorCode setResolution(unsigned width, unsigned height) = 0;

	/**
	 * Gets captured frame width.
	 *
	 * @return captured frame width
	 */
	virtual unsigned getWidth() const = 0;

	/**
	 * Gets captured frame height.
	 *
	 * @return captured frame height
	 */
	virtual unsigned getHeight() const = 0;

	/**
	 * Sets capture brightness.
	 *
	 * @param brightness brightness value
	 */
	virtual void setBrightness(int brightness) = 0;

	/**
	 * Gets capture brightness.
	 *
	 * @param brightness brightness value
	 */
	virtual int getBrightness() const = 0;

	/**
	 * Sets capture contrast.
	 *
	 * @param contrast contrast value
	 */
	virtual void setContrast(int contrast) = 0;

	/**
	 * Gets capture contrast.
	 *
	 * @param contrast contrast value
	 */
	virtual int getContrast() const = 0;

	/**
	 * Flips captured frame horizontally.
	 *
	 * @param flip if true the picture will be flipped horizontally
	 */
	virtual void flipHorizontally(bool flip) = 0;
};

#endif	//OWIWEBCAMDRIVER_H
