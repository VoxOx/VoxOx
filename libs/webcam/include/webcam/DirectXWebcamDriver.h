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

#ifndef OWDIRECTXWEBCAMDRIVER_H
#define OWDIRECTXWEBCAMDRIVER_H

#include <webcam/WebcamDriver.h>

//Otherwise CoInitializeEx() is not recognized
//see http://support.microsoft.com/newsgroups/default.aspx?dg=microsoft.public.vc.atl&tid=85edd07d-040c-47a6-8e82-ddedfa77ce70&p=1
#define _WIN32_DCOM
//#define _WIN32_WINNT 0x0400

#include <tchar.h>
#include <dshow.h>
#include <atlbase.h>

//VOXOX - JRT - 2009.08.28 - Fix for problem with Windows DirectX SDK
#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
//END fix.
#include <qedit.h>

#include <comutil.h>

#include <iostream>

/**
 * DirectX implementation of webcam driver
 *
 * @author Mathieu Stute
 * @author David Ferlier
 * @author Jerome Wagner
 * @author Philippe Bernery
 */
class DirectXWebcamDriver : public IWebcamDriver, public ISampleGrabberCB {
public:

	DirectXWebcamDriver(WebcamDriver * driver, int flags);

	virtual ~DirectXWebcamDriver();

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


	STDMETHODIMP_(ULONG) AddRef() { return 2; }
	STDMETHODIMP_(ULONG) Release() { return 1; }

	// fake out any COM QI'ing
	STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);

	// callback to access the buffer - the original buffer is passed
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample * pSample);

	// callback to access the buffer - a copy is passed
	STDMETHODIMP BufferCB(double dblSampleTime, BYTE * pBuffer, long lBufferSize);

private:

	/** Pointer to main driver */
	WebcamDriver * _webcamDriver;

	CComPtr< IMediaControl > _pControl;
	CComPtr< IGraphBuilder > _pGraph;
	CComPtr< ICaptureGraphBuilder2 > _pBuild;
	CComPtr< IBaseFilter > _pCap;
	IBaseFilter * _pGrabberF;
	ISampleGrabber * _pGrabber;
	IBaseFilter * _pNull;
	IAMStreamConfig * _iam;

	/** Captured image width */
	unsigned _cachedWidth;

	/** Captured image height */
	unsigned _cachedHeight;

	/** Frame per seconds */
	unsigned _cachedFPS;

	/** Desired image palette */
	pixosi _cachedPalette;

	/** Piximage sended to callbacks */
	piximage _capturedImage;

	/** True if webcam is opened */
	bool _isOpen;

	/** Read device capabilities */
	void readCaps();

	/** Set device capabilites */
	WebcamErrorCode setCaps(pixosi palette, unsigned fps, unsigned resolutionWidth, unsigned resolutionHeight);
};

#endif	//OWDIRECTXWEBCAMDRIVER_H
