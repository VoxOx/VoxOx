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

#include <webcam/QuicktimeWebcamDriver.h>

#include <webcam/WebcamDriver.h>

#include <pixertool/quicktime-pixertool.h>

#include <util/Logger.h>

#include <iostream>
using namespace std;

QuicktimeWebcamDriver::QuicktimeWebcamDriver(WebcamDriver * driver, int flags)
	: IWebcamDriver(flags) {
	EnterMovies();

	_webcamDriver = driver;
	_timer.timeoutEvent += boost::bind(&QuicktimeWebcamDriver::idleSeqGrab, this);

	_seqGrab = NULL;
	_PGWorld = NULL;
	_SGChanVideo = NULL;
	_decomSeq = 0;
	_isOpen = false;

	getDeviceList();
	getDefaultDevice();
}

QuicktimeWebcamDriver::~QuicktimeWebcamDriver() {
}

void QuicktimeWebcamDriver::cleanup() {
	_timer.stop();

	if (_decomSeq) {
		CDSequenceEnd(_decomSeq);
	}

	if (_SGChanVideo) {
		SGDisposeChannel(_seqGrab, _SGChanVideo);
	}

	if (_seqGrab) {
		CloseComponent(_seqGrab);
	}

	if (_PGWorld) {
		DisposeGWorld(_PGWorld);
	}

	_boundsRect.top = 0;
	_boundsRect.left = 0;
	_boundsRect.right = 320;
	_boundsRect.bottom = 240;

	_seqGrab = NULL;
	_PGWorld = NULL;
	_SGChanVideo = NULL;
	_decomSeq = 0;
	_isOpen = false;
	_palette = PIX_OSI_UNSUPPORTED;
}

void QuicktimeWebcamDriver::initializeCommonComponents() {
	OSErr err = noErr;

	LOG_DEBUG("Opening component");
	_seqGrab = OpenDefaultComponent(SeqGrabComponentType, 0);
	if (_seqGrab == NULL) {
		LOG_ERROR("can't get default sequence grabber component");
		return;
	}

	LOG_DEBUG("Initializing component");
	err = SGInitialize(_seqGrab);
	if (err != noErr) {
		LOG_ERROR("can't initialize sequence grabber component");
		return;
	}

	LOG_DEBUG("SetDataRef");
	err = SGSetDataRef(_seqGrab, 0, 0, seqGrabDontMakeMovie);
	if (err != noErr) {
		LOG_ERROR("can't set the destination data reference");
		return;
	}

	LOG_DEBUG("Creating new channel");
	err = SGNewChannel(_seqGrab, VideoMediaType, &_SGChanVideo);
	if (err != noErr) {
		LOG_ERROR("can't create a video channel");
		return;
	}
}

StringList QuicktimeWebcamDriver::getDeviceList() {
	SGDeviceList sgDeviceList;
	OSErr err = noErr;
	StringList deviceList;

	if (!isOpen()) {
		cleanup();
		initializeCommonComponents();

		err = SGGetChannelDeviceList(_SGChanVideo, sgDeviceListIncludeInputs, &sgDeviceList);
		if (err != noErr) {
			LOG_ERROR("can't get device list");
			return deviceList;
		}

		for (register short i = 0 ; i < (*sgDeviceList)->count ; i++) {
			if (!((*sgDeviceList)->entry[i].flags & sgDeviceNameFlagDeviceUnavailable)) {
				SGDeviceInputList inputs = (*sgDeviceList)->entry[i].inputs;

				for (register short j = 0 ; j < (*inputs)->count ; j++) {
					if (!((*inputs)->entry[j].flags & sgDeviceInputNameFlagInputUnavailable)) {
						deviceList += genDeviceName((*sgDeviceList)->entry[i].name,
							(*inputs)->selectedIndex,
							(*inputs)->entry[j].name);
					}
				}
			}
		}

		SGDisposeDeviceList(_seqGrab, sgDeviceList);
		cleanup();

		_deviceList = deviceList;
	}

	return _deviceList;
}

string QuicktimeWebcamDriver::getDefaultDevice() {
	SGDeviceList sgDeviceList;
	OSErr err = noErr;

	if (!isOpen()) {
		cleanup();
		initializeCommonComponents();

		err = SGGetChannelDeviceList(_SGChanVideo, sgDeviceListIncludeInputs, &sgDeviceList);
		if (err != noErr) {
			LOG_ERROR("can't get device list");
			return WEBCAM_NULL;
		}

		if ((*sgDeviceList)->count) {
			short i = (*sgDeviceList)->selectedIndex;

			SGDeviceInputList inputs = (*sgDeviceList)->entry[i].inputs;

			if ((*inputs)->count) {
				short j = (*inputs)->selectedIndex;

				string dev = genDeviceName((*sgDeviceList)->entry[i].name,
					(*inputs)->selectedIndex,
					(*inputs)->entry[j].name);

				_defaultDevice = dev;
			}
		}

		SGDisposeDeviceList(_seqGrab, sgDeviceList);
		cleanup();
	}

	return _defaultDevice;
}

WebcamErrorCode QuicktimeWebcamDriver::setDevice(const std::string & deviceName) {
	OSErr err = noErr;
	unsigned colonPos = deviceName.find(":", 0);
	string device = deviceName.substr(0, colonPos);
	short input = atoi(deviceName.substr(colonPos + 1).c_str());

	LOG_DEBUG("opening device: " + deviceName);

	initializeCommonComponents();

	err = SGSetChannelDevice(_SGChanVideo, stdToPascalString(device));
	if (err != noErr) {
		LOG_ERROR("can't set channel device");
		return WEBCAM_NOK;
	}

	err = SGSetChannelDeviceInput(_SGChanVideo, input);
	if (err != noErr) {
		LOG_ERROR("can't set channel device input");
		return WEBCAM_NOK;
	}

	LOG_DEBUG("createGWorld");
	if (!createGWorld()) {
		return WEBCAM_NOK;
	}

	LOG_DEBUG("SGSetDataProc");
	err = SGSetDataProc(_seqGrab, NewSGDataUPP(mySGDataProc), (long) this);
	if (err != noErr) {
		LOG_ERROR("can't set data proc");
		return WEBCAM_NOK;
	}

	LOG_DEBUG("SGSetChannelUsage");
	err = SGSetChannelUsage(_SGChanVideo, seqGrabRecord);
	if (err != noErr) {
		LOG_ERROR("can't set channel usage");
		return WEBCAM_NOK;
	}

	LOG_DEBUG("SGPrepare");
	err = SGPrepare(_seqGrab,  false, true);
	if (err != noErr) {
		LOG_ERROR("can't prepare sequence grabber component");
		return WEBCAM_NOK;
	}

	setupDecompressor();

	_isOpen = true;

	return WEBCAM_OK;
}

bool QuicktimeWebcamDriver::createGWorld() {
	OSErr err = noErr;

	pixosi pixTable[] = {
		PIX_OSI_BGR24,
		PIX_OSI_ARGB32,
		PIX_OSI_RGBA32
	};

	/* FIXME: These formats are not supported because of offset suppressor in mySGDataProc.
		PIX_OSI_RGB24,
		PIX_OSI_YUV420P,
		PIX_OSI_I420,
		PIX_OSI_YUV422,
		PIX_OSI_YUV411,
		PIX_OSI_YUV422P,
		PIX_OSI_YUV444P,
		PIX_OSI_YUV424P,
		PIX_OSI_YUV41P,
		PIX_OSI_YUY2,
		PIX_OSI_YUYV,
		PIX_OSI_YVYU,
		PIX_OSI_UYVY,
		PIX_OSI_YV12,
		PIX_OSI_NV12,
		PIX_OSI_RGB565,
		PIX_OSI_RGB555,
		PIX_OSI_ARGB32,
		PIX_OSI_RGB8,
		PIX_OSI_RGB4,
		PIX_OSI_RGB1
	};
	*/

	/*
	The webcam will always be initialized with the first recognized palette
	usually:
		MacOSX G5 PPC knows
				k32ARGBPixelFormat	(OSI 20)
		imac intel knows
				k32ARGBPixelFormat	(OSI 20)
				k24BGRPixelFormat	(OSI 22)
	*/
	for (register unsigned i = 0; i < sizeof(pixTable) / sizeof(pixosi); i++) {
		LOG_DEBUG("attempting to create a GWorld with palette #" + String::fromNumber(pixTable[i]));
		LOG_DEBUG("trying resolution: (top, left, right, bottom) = ("+String::fromNumber(_boundsRect.top)+","+String::fromNumber(_boundsRect.left)+","+String::fromNumber(_boundsRect.right)+","+String::fromNumber(_boundsRect.bottom)+")");
		err = QTNewGWorld(&_PGWorld, //Does'nt work with planar format
			pix_quicktime_from_pix_osi(pixTable[i]),
			&_boundsRect,
			0,
			NULL,
			0);
		if (err == noErr) {
			LOG_DEBUG("success with palette #" + String::fromNumber(pixTable[i]));
			_palette = pixTable[i];
			break;
		}
	}

	if (err != noErr) {
		LOG_ERROR("can't create GWorld");
		return false;
	}

	if (!LockPixels(GetPortPixMap(_PGWorld))) {
		LOG_ERROR("can't lock pixmap for decompression");
		return false;
	}

	err = SGSetGWorld(_seqGrab, _PGWorld, GetMainDevice());
	if (err != noErr) {
		LOG_ERROR("can't set GWorld");
		return false;
	}

	return true;
}

bool QuicktimeWebcamDriver::isOpen() const {
	return _isOpen;
}

void QuicktimeWebcamDriver::startCapture() {
	OSErr err = noErr;

	err = SGStartRecord(_seqGrab);
	if (err != noErr) {
		LOG_ERROR("can't start capture");
		return;
	}

	_timer.start(100, 100);
}

void QuicktimeWebcamDriver::pauseCapture() {
}

void QuicktimeWebcamDriver::stopCapture() {
	SGStop(_seqGrab);

	_timer.stop();
}

WebcamErrorCode QuicktimeWebcamDriver::setPalette(pixosi palette) {
	if (_PGWorld) {
		QTUpdateGWorld(&_PGWorld,
			pix_quicktime_from_pix_osi(palette),
			&_boundsRect,
			0,
			NULL,
			noNewDevice);

		PixMapHandle pixmap = GetGWorldPixMap(_PGWorld);
		if ((*pixmap)->pixelFormat != (unsigned) pix_quicktime_from_pix_osi(palette)) {
			LOG_WARN("can't set palette");
			return WEBCAM_NOK;
		}

		_palette = palette;

		if (_decomSeq)
			CDSequenceEnd(_decomSeq);

		setupDecompressor();

		return WEBCAM_OK;
	} else {
		LOG_ERROR("can't set palette when no device were specified before");
		return WEBCAM_NOK;
	}
}

pixosi QuicktimeWebcamDriver::getPalette() const {
	return _palette;
}

WebcamErrorCode QuicktimeWebcamDriver::setFPS(unsigned fps) {
	OSErr err = noErr;

	err = SGSetFrameRate(_SGChanVideo, fps);
	if (err != noErr) {
		return WEBCAM_NOK;
	}

	return WEBCAM_OK;
}

unsigned QuicktimeWebcamDriver::getFPS() const {
	Fixed framerate;
	OSErr err = noErr;

	err = SGGetFrameRate(
		_SGChanVideo,
		&framerate);
	if (err != noErr) {
		return 0;
	}

	return (unsigned)framerate;
}

WebcamErrorCode QuicktimeWebcamDriver::setResolution(unsigned width, unsigned height) {
	OSErr err = noErr;
	Rect newBounds;

	newBounds.top = 0;
	newBounds.left = 0;
	newBounds.right = width;
	newBounds.bottom = height;

	LOG_DEBUG("try to change resolution: (width, height) = " + String::fromNumber(width) + "," + String::fromNumber(height));
	err = SGSetChannelBounds(_SGChanVideo, &newBounds);
	if (err != noErr) {
		LOG_DEBUG("could not change resolution...");
		err = SGGetSrcVideoBounds(_SGChanVideo, &_boundsRect);
		if (err != noErr) {
			LOG_WARN("can't get source video bounds");
		}
		return WEBCAM_NOK;
	} else {
		_boundsRect.top = newBounds.top;
		_boundsRect.left = newBounds.left;
		_boundsRect.right = newBounds.right;
		_boundsRect.bottom = newBounds.bottom;
		LOG_DEBUG("new resolution: (top, left, right, bottom) = ("+String::fromNumber(newBounds.top)+","+String::fromNumber(newBounds.left)+","+String::fromNumber(newBounds.right)+","+String::fromNumber(newBounds.bottom)+")");
	}

	if (_decomSeq) {
		CDSequenceEnd(_decomSeq);
	}

	setupDecompressor();

	return WEBCAM_OK;
}

unsigned QuicktimeWebcamDriver::getWidth() const {
	return _boundsRect.right - _boundsRect.left;
}

unsigned QuicktimeWebcamDriver::getHeight() const {
	return _boundsRect.bottom - _boundsRect.top;
}

void QuicktimeWebcamDriver::setBrightness(int brightness) {
}

int QuicktimeWebcamDriver::getBrightness() const {
	return 0;
}

void QuicktimeWebcamDriver::setContrast(int contrast) {
}

int QuicktimeWebcamDriver::getContrast() const {
	return 0;
}

void QuicktimeWebcamDriver::flipHorizontally(bool flip) {
}

void QuicktimeWebcamDriver::frameBufferAvailable(piximage * image) {
	_webcamDriver->frameBufferAvailable(image);
}

void QuicktimeWebcamDriver::logPixMap(PixMap * pm) {
	LOG_DEBUG(string("\nPixMap:")
		+ "\n\tbaseAddr: " + String::fromNumber((unsigned)pm->baseAddr)
		+ "\n\tPixMap?: " + String::fromNumber((short)(pm->rowBytes & 32768 ? 1 : 0))
		+ "\n\tthe other bit: " + String::fromNumber((short)(pm->rowBytes & 65536 ? 1 : 0))
		+ "\n\trowBytes: " + String::fromNumber((short)(pm->rowBytes & (((unsigned short) 0xFFFF) >> 2)))
		+ "\n\tbounds: " + String::fromNumber(pm->bounds.top) + "-" + String::fromNumber(pm->bounds.bottom)
			+ "-" + String::fromNumber(pm->bounds.left) + "-" + String::fromNumber(pm->bounds.right)
		+ "\n\tpmVersion: " + String::fromNumber(pm->pmVersion)
		+ "\n\tpackType: " + String::fromNumber(pm->packType)
		+ "\n\tpackSize: " + String::fromNumber(pm->packSize)
		+ "\n\thRes: " + String::fromNumber(pm->hRes)
		+ "\n\tvRes: " + String::fromNumber(pm->vRes)
		+ "\n\tpixelType: " + String::fromNumber(pm->pixelType)
		+ "\n\tpixelSize: " + String::fromNumber(pm->pixelSize)
		+ "\n\tcmpCount: " + String::fromNumber(pm->cmpCount)
		+ "\n\tcmpSize: " + String::fromNumber(pm->cmpSize)
		+ "\n\tpixelFormat: " + pix_quicktime_to_string(pm->pixelFormat)
		+ "\n\tpmTable: " + String::fromNumber((unsigned)pm->pmTable)
		+ "\n\tpmExt: " + String::fromNumber((unsigned)pm->pmExt));
}

OSErr mySGDataProc(SGChannel c,
	Ptr p,
	long len,
	long * offset,
	long chRefCon,
	TimeValue time,
	short writeType,
	long refCon) {

	CodecFlags ignore;
	ComponentResult err = noErr;
	QuicktimeWebcamDriver * webcamDriver = (QuicktimeWebcamDriver *)refCon;
	piximage * image;

	if (webcamDriver->_PGWorld) {
		/* Do not flood logger
		LOG_DEBUG("decompressing");
		*/
		err = DecompressSequenceFrameS(webcamDriver->_decomSeq,
					p,
					len,
					0,
					&ignore,
					NULL);
		if (err != noErr) {
			LOG_ERROR("can't decompress sequence frame in GWorld");
			return err;
		}
	}

	// Shifting data. MacOS X introduces an offset on each picture line
	// FIXME: This shift does work only with RGB format
	image = pix_alloc(webcamDriver->_palette, webcamDriver->getWidth(), webcamDriver->getHeight());
	/* Do not flood logger
	LOG_DEBUG("allocated a trimmed image: palette: " + String::fromNumber(webcamDriver->_palette)
		+ ", width: " + String::fromNumber(webcamDriver->getWidth())
		+ ", height: " + String::fromNumber(webcamDriver->getHeight()));
	*/

	PixMap * pixmap = *GetGWorldPixMap(webcamDriver->_PGWorld);
	uint8_t * data;
	unsigned rowBytes = pixmap->rowBytes & (((unsigned short) 0xFFFF) >> 2); // The tow first bits are flags
	unsigned pixelSize = pixmap->pixelSize / 8; // Pixel size in bytes
	unsigned lineOffset = rowBytes - image->width * pixelSize;
	/* Do not flood logger
	LOG_DEBUG("rowBytes: " + String::fromNumber(rowBytes)
		+ ", pixelSize: " + String::fromNumber(pixelSize)
		+ ", lineOffset: " + String::fromNumber(lineOffset));

	QuicktimeWebcamDriver::logPixMap(pixmap);
	*/

	data = (uint8_t *) GetPixBaseAddr(GetGWorldPixMap(webcamDriver->_PGWorld));

	for (unsigned line = 0 ; line < image->height ; line++) {
		unsigned offset = line * (image->width * pixelSize + lineOffset);
		memcpy(image->data + ((line * image->width) * pixelSize), data + offset, (rowBytes - lineOffset));
	}

	webcamDriver->frameBufferAvailable(image);

	pix_free(image);

	return noErr;
}

void QuicktimeWebcamDriver::setupDecompressor() {
	ComponentResult	err = noErr;
	Rect sourceRect = { 0, 0, getWidth(), getHeight() };
	MatrixRecord scaleMatrix;
	ImageDescriptionHandle imageDesc = (ImageDescriptionHandle)NewHandle(sizeof(ImageDescription));

	err = SGGetChannelSampleDescription(_SGChanVideo, (Handle)imageDesc);

	if (err != noErr) {
		LOG_ERROR("can't get channel sample description:" + String::fromNumber(err));
		return;
	}

	sourceRect.right = (**imageDesc).width;
	sourceRect.bottom = (**imageDesc).height;
	RectMatrix(&scaleMatrix, &sourceRect, &_boundsRect);

	err = DecompressSequenceBegin(&_decomSeq,
				imageDesc,
				_PGWorld,
				NULL,
				NULL,
				&scaleMatrix,
				srcCopy,
				NULL,
				0,
				codecNormalQuality,
				bestSpeedCodec);
	if (err != noErr) {
		LOG_ERROR("can't begin decompress sequences");
		return;
	}

	DisposeHandle((Handle)imageDesc);
	imageDesc = NULL;
}

void QuicktimeWebcamDriver::idleSeqGrab() {
	OSErr err;

	err = SGIdle(_seqGrab);
	if (err != noErr) {
		SGStop(_seqGrab);
		SGStartRecord(_seqGrab);
	}
}

string QuicktimeWebcamDriver::pascalToStdString(Str255 pStr) {
	unsigned size = pStr[0];
	char buffer[255]; // A pascal string is not longer than 255 bytes

	memcpy(buffer, pStr + 1, size);
	buffer[size] = 0;

	return buffer;
}

unsigned char *QuicktimeWebcamDriver::stdToPascalString(const std::string & str) {
	if (str.size() <= 255) {
		static Str255 buffer;
		buffer[0] = str.size();

		memcpy(buffer + 1, str.c_str(), str.size());

		return buffer;
	} else {
		return NULL;
	}
}

string QuicktimeWebcamDriver::genDeviceName(unsigned char * device,
	short inputIndex, unsigned char * input) {

	char buffer[32];
	sprintf(buffer, "%d", inputIndex);

	string devName = pascalToStdString(device) +
		":" +
		buffer +
		":" +
		pascalToStdString(input);

	return devName;
}
