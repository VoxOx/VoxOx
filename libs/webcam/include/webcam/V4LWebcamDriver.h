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

#ifndef OWV4LWEBCAMDRIVER_H
#define OWV4LWEBCAMDRIVER_H

#include <webcam/WebcamDriver.h>

#include <thread/Thread.h>

#include <linux/videodev.h>

#include <map>
#include <string>

class V4L2WebcamDriver;

/**
 * V4L implementation of webcam driver.
 *
 * @author David Ferlier
 * @author Philippe Bernery
 */
class V4LWebcamDriver : public IWebcamDriver, public Thread {
public:

	V4LWebcamDriver(WebcamDriver * driver, int flags);

	virtual ~V4LWebcamDriver();

	void cleanup();

	void setFlags(int flags) {}

	void unsetFlags(int flags) {}

	bool isFlagSet(int flag) { return false;}

	StringList getDeviceList();

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
	friend class V4L2WebcamDriver;

	void run();
	void terminate();

	WebcamDriver * _webcamDriver;

	int _fhandle;

  	struct video_capability _vCaps;
	struct video_window _vWin;
	struct video_picture _vPic;
	struct video_clip _vClips;

	bool _isOpen;
	bool _terminate;

	void readCaps();

	unsigned _fps;

	/** device / name associative array */
	typedef std::map < std::string , std::string > DevNameArray;
	DevNameArray getDevices();

	/** getDevices on 2.6 kernel based system */
	DevNameArray getDevices2_6();
};

#endif	//OWV4LWEBCAMDRIVER_H
