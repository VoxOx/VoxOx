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

#ifndef OWQUICKTIMEWEBCAMDRIVER_H
#define OWQUICKTIMEWEBCAMDRIVER_H

#include <webcam/IWebcamDriver.h>

#include <thread/Timer.h>

#include <Quicktime/Quicktime.h>

class WebcamDriver;

/**
 * Quicktime implementation of webcam driver.
 *
 * @author Philippe Bernery
 */
class QuicktimeWebcamDriver : public IWebcamDriver {
public:
	QuicktimeWebcamDriver(WebcamDriver * driver, int flags);

	virtual ~QuicktimeWebcamDriver();

	void cleanup();

	void setFlags(int flags) {}
	void unsetFlags(int flags) {}
	bool isFlagSet(int flag) { return false;}

	/**
	 * @see IWebcamDriver::getDeviceList
	 * On MacOS X, device listing works only if no device are currently in used.
	 * To prevent error, a getDeviceList is done at webcam driver launch. Then
	 * if getDeviceList is called and if a webcam is in use, this list will be returned,
	 * otherwise the list is updated.
	 */
	StringList getDeviceList();

	/**
	 * @see IWebcamDriver::getDefaultDevice()
	 * @see QuicktimeWebcamDriver::getDeviceList
	 */
	std::string getDefaultDevice();

	WebcamErrorCode setDevice(const std::string & deviceName);

	bool isOpen() const;

	void startCapture();
	void pauseCapture();
	void stopCapture();

	WebcamErrorCode setPalette(pixosi palette);
	pixosi getPalette() const;

	WebcamErrorCode setFPS(unsigned fps);
	unsigned getFPS() const;

	WebcamErrorCode setResolution(unsigned width, unsigned height);
	unsigned getWidth() const;
	unsigned getHeight() const;

	void setBrightness(int brightness);
	int getBrightness() const;

	void setContrast(int contrast);
	int getContrast() const;

	void flipHorizontally(bool flip);

private:

	/** Out capture procedure */
	friend OSErr mySGDataProc(SGChannel c,
				Ptr p,
				long len,
				long * offset,
				long chRefCon,
				TimeValue time,
				short writeType,
				long refCon);

	/** The sequence grabber component. */
	SeqGrabComponent _seqGrab;

	/** The sequence grabber channel component. */
	SGChannel _SGChanVideo;

	/** OWPicture size. */
	Rect _boundsRect;

	/** GWorld used for decompression. */
	GWorldPtr _PGWorld;

	/** Decompression sequence. */
	ImageSequence _decomSeq;

	/** Base Webcam driver. */
	WebcamDriver *_webcamDriver;

	/** Used to idle the sequence grabber */
	Timer _timer;

	/** True if webcam is opened */
	bool _isOpen;

	/** Webcam palette */
	pixosi _palette;

	/** Device list. @see QuicktimWebcamDriver::getDeviceList */
	StringList _deviceList;

	/** Default device. @see QuicktimeWebcamDriver::getDefaultDevice */
	std::string _defaultDevice;

	/**
	 * Called by SGDataProc when a frame has been captured and decompressed.
	 */
	void frameBufferAvailable(piximage * image);

	/**
	 * Setup the decompressor
	 */
	void setupDecompressor();

	/**
	 * Idle the sequence grabber.
	 */
	void idleSeqGrab();

	/**
	 * Create the GWorld and find a working palette.
	 * @return true if GWorld created
	 */
	bool createGWorld();

	/**
	 * Convert a pascal string to a std:string.
	 */
	static std::string pascalToStdString(unsigned char * pStr);

	/**
	 * Convert a std::string to a pascal string.
	 */
	static unsigned char * stdToPascalString(const std::string & str);

	/**
	 * Generate a valid device name.
	 * @param device device name (e.g "Philips Vesta")
	 * @param inputIndex input index (e.g "0")
	 * @param input input name (e.g "Camera")
	 * @see getDefaultDevice
	 */
	static std::string genDeviceName(unsigned char * device,
		short inputIndex, unsigned char * input);

	/**
	 * Log information about a PixMap
	 */
	static void logPixMap(PixMap * pm);

	/**
	 * Initialize common components: Sequence grabber component, a video channel.
	 */
	void initializeCommonComponents();
};

#endif	//OWQUICKTIMEWEBCAMDRIVER_H
