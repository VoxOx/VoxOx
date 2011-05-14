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

#ifndef OWNULLWEBCAMDRIVER_H
#define OWNULLWEBCAMDRIVER_H

#include <webcam/WebcamDriver.h>

/**
 * Null webcam driver.
 *
 * @author Philippe Bernery
 */
class NullWebcamDriver : public IWebcamDriver {
public:

	NullWebcamDriver(WebcamDriver * driver, int flags);

	virtual ~NullWebcamDriver();

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
};

#endif	//OWNULLWEBCAMDRIVER_H
