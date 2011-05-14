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

#ifndef OWV4L2WEBCAMDRIVER_H
#define OWV4L2WEBCAMDRIVER_H

#include <webcam/WebcamDriver.h>

#include <thread/Thread.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

#include <map>
#include <string>

#include <webcam/V4LWebcamDriver.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

typedef enum {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
    IO_METHOD_UNKNOWN,
} IoMethod;

typedef struct _IOBuffer {
    size_t   length; 
    piximage * img;
} IOBuffer;
        
#ifdef __cplusplus
}
#endif /*__cplusplus*/

class V4LWebcamDriver;

/**
 * V4L2 implementation of webcam driver.
 *
 * @author David Ferlier
 * @author Philippe Bernery
 */
class V4L2WebcamDriver : public IWebcamDriver, public Thread {
public:

	V4L2WebcamDriver(WebcamDriver * driver, int flags);

	virtual ~V4L2WebcamDriver();

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

	WebcamErrorCode setResolution(unsigned int width, unsigned int height);

	unsigned getWidth() const;

	unsigned getHeight() const;

	void setBrightness(int brightness);

	int getBrightness() const;

	void setContrast(int contrast);

	int getContrast() const;

	void flipHorizontally(bool flip);

private:

	void run();
	void terminate();

	V4LWebcamDriver _oldApiDriver; /* for handling comaptible only with old API devices */

	WebcamDriver * _webcamDriver;

	int _fhandle;

	struct v4l2_capability _vCaps;
	struct v4l2_format _vPixFormat;

	unsigned int _minWidth;
	unsigned int _minHeight;

	unsigned int _maxWidth;
	unsigned int _maxHeight;

	bool _oldApiDevice;

	IOBuffer* buffers;
	unsigned int n_buffers;
	unsigned int _bufferSize;

	IoMethod ioMethod;

	bool _isOpen;
	bool _terminate;

	unsigned _fps;

	/** device / name associative array */
	typedef std::map < std::string , std::string > DevNameArray;
	DevNameArray getDevices();

	void readCaps();

	/** getDevices on 2.6 kernel based system */
	DevNameArray getDevices2_6();

	unsigned int reqDeviceBuffers(enum v4l2_memory mem, unsigned int num);
	void freeDeviceBuffers(enum v4l2_memory mem);

	void initRead();
	void initMmap();
	void initUserp();
	
	void initDevice();
	void checkDevice();

	void uninitDevice();

	WebcamErrorCode readFrame();
};

#endif //OWV4L2WEBCAMDRIVER_H

