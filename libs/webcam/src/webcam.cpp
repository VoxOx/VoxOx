/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include <webcam/webcam.h>

#include <webcam/WebcamDriver.h>

#include <thread/Mutex.h>

#include <stdlib.h>

using namespace std;

typedef struct _webcamcallbacklist {
	/** sender */
	webcam * wc;

	/** The callback */
	webcamcallback * cbk;

	/** User data associated with this callback */
	void * data;

	/** Link to previous and next callback. Linked list */
	_webcamcallbacklist * prev, * next;

} webcamcallbacklist;

struct opaquewebcam {
	WebcamDriver * driver;
};

//FIXME: there is only one list for all created webcam struct.
static webcamcallbacklist * cbks = NULL;
static Mutex mutex;

static void webcam_frame_captured_event_handler(IWebcamDriver * sender, piximage * image) {
	Mutex::ScopedLock lock(mutex);

	webcamcallbacklist * cur = cbks;

	while (cur) {
		cur->cbk(cur->wc, image, cur->data);
		cur = cur->next;
	}
}

WebcamDeviceList * webcam_get_device_list(webcam *) {
	IWebcamDriver * driver = WebcamDriver::getInstance();
	StringList deviceList = driver->getDeviceList();
	WebcamDeviceList * devlist = (WebcamDeviceList *) malloc(sizeof(WebcamDeviceList));

	devlist->count = deviceList.size();
	devlist->name = (const char **) malloc(devlist->count * sizeof(const char *));

	for (register unsigned i = 0; i < devlist->count; i++) {
		devlist->name[i] = (const char *) malloc(deviceList[i].size());
		sprintf((char *) devlist->name[i], deviceList[i].c_str(), deviceList[i].size());
	}

	return devlist;
}

void webcam_release_WebcamDeviceList(WebcamDeviceList * devlist) {
	if (devlist) {
		if (devlist->name) {
			for (register unsigned i = 0; i < devlist->count; i++) {
				if (devlist->name[i]) {
					free((void *) devlist->name[i]);
				}
			}
			free(devlist->name);
		}
		free(devlist);
	}
}

const char * webcam_get_default_device(webcam *) {
	IWebcamDriver * driver = WebcamDriver::getInstance();
	static string device;

	device = driver->getDefaultDevice();

	return device.c_str();
}

webcam * webcam_get_instance() {
	webcam * wc = (webcam *) malloc(sizeof(webcam));

	wc->driver = WebcamDriver::getInstance();
	wc->driver->frameCapturedEvent += &webcam_frame_captured_event_handler;

	return wc;
}

void webcam_set_flags(webcam * wc, int flags) {
	wc->driver->setFlags(flags);
}

void webcam_unset_flags(webcam * wc, int flags) {
	wc->driver->unsetFlags(flags);
}

int webcam_is_flag_set(webcam * wc, int flag) {
	return wc->driver->isFlagSet(flag);
}

void webcam_release(webcam * wc) {
	webcamcallbacklist *nxt, *cur;

	webcam_stop_capture(wc);
	if (wc) {
		free(wc);

		Mutex::ScopedLock lock(mutex);

		cur = cbks;
		while (cur) {
			nxt = cur->next;
			free(cur);
			cur = nxt;
		}
		cbks = NULL;
	}
}

WebcamErrorCode webcam_set_device(webcam * wc, const char * device_name) {
	return wc->driver->setDevice(device_name);
}

void webcam_add_callback(webcam * wc, webcamcallback * callback, void * userData) {
	Mutex::ScopedLock lock(mutex);
	webcamcallbacklist * cur;

	if (cbks == NULL) {
		cbks = (webcamcallbacklist *) malloc(sizeof(webcamcallbacklist));
		cbks->prev = NULL;
		cbks->next = NULL;
		cur = cbks;
	} else {
		//Find last
		cur = cbks;
		while (cur->next != NULL) {
			cur = cur->next;
		}

		cur->next = (webcamcallbacklist *) malloc(sizeof(webcamcallbacklist));
		cur->next->prev = cur;
		cur->next->next = NULL;
		cur = cur->next;
	}

	cur->wc = wc;
	cur->cbk = callback;
	cur->data = userData;
}

void webcam_remove_callback(webcam * wc, webcamcallback * callback) {
	Mutex::ScopedLock lock(mutex);
	webcamcallbacklist * cur = cbks;

	while ((cur != NULL) && (cur->cbk != callback)) {
		cur = cur->next;
	}

	if (cur != NULL) {
		if (cur->prev) {
			cur->prev->next = cur->next;
		} else {
			cbks = cur->next;
		}

		if (cur->next) {
			cur->next->prev = cur->prev;
		}
		free(cur);
	}
}

void webcam_start_capture(webcam * wc) {
	if (wc) {
		wc->driver->startCapture();
	}
}

void webcam_pause_capture(webcam * wc) {
	if (wc) {
		wc->driver->pauseCapture();
	}
}

void webcam_stop_capture(webcam * wc) {
	if (wc) {
		wc->driver->stopCapture();
	}
}

unsigned webcam_get_width(webcam * wc) {
	unsigned result = 0;

	if (wc) {
		result = wc->driver->getWidth();
	}

	return result;
}

unsigned webcam_get_height(webcam * wc) {
	unsigned result = 0;

	if (wc) {
		result = wc->driver->getHeight();
	}

	return result;
}

pixosi webcam_get_palette(webcam * wc) {
	pixosi result = PIX_OSI_UNSUPPORTED;

	if (wc) {
		result = wc->driver->getPalette();
	}

	return result;
}

WebcamErrorCode webcam_set_palette(webcam * wc, pixosi palette) {
	WebcamErrorCode result = WEBCAM_NOK;

	if (wc) {
		result = wc->driver->setPalette(palette);
	}

	return result;
}

int webcam_is_open(webcam * wc) {
	int result = 0;

	if (wc) {
		result = wc->driver->isOpen();
	}

	return result;
}

WebcamErrorCode webcam_set_fps(webcam * wc, unsigned fps) {
	WebcamErrorCode result = WEBCAM_NOK;

	if (wc) {
		result = wc->driver->setFPS(fps);
	}

	return result;
}

unsigned webcam_get_fps(webcam * wc) {
	unsigned result = 0;

	if (wc) {
		result = wc->driver->getFPS();
	}

	return result;
}

void webcam_set_resolution(webcam * wc, unsigned width, unsigned height) {
	if (wc) {
		wc->driver->setResolution(width, height);
	}
}

void webcam_flip_horizontally(webcam * wc, int flip) {
	if (wc) {
		wc->driver->flipHorizontally(flip);
	}
}

void webcam_set_brightness(webcam * wc, int brightness) {
	if (wc) {
		wc->driver->setBrightness(brightness);
	}
}

int webcam_get_brightness(webcam * wc) {
	int result = 0;

	if (wc) {
		result = wc->driver->getBrightness();
	}

	return result;
}

void webcam_set_contrast(webcam * wc, int contrast) {
	if (wc) {
		wc->driver->setContrast(contrast);
	}
}

int webcam_get_contrast(webcam * wc) {
	int result = 0;

	if (wc) {
		result = wc->driver->getContrast();
	}

	return result;
}

void webcam_api_initialize() {
	WebcamDriver::apiInitialize();
}

void webcam_api_uninitialize() {
	WebcamDriver::apiUninitialize();
}
