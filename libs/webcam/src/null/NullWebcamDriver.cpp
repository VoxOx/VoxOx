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

#include <webcam/NullWebcamDriver.h>

using namespace std;

NullWebcamDriver::NullWebcamDriver(WebcamDriver *driver, int flags)
	: IWebcamDriver(flags) {
}

NullWebcamDriver::~NullWebcamDriver() {
}

void NullWebcamDriver::cleanup() {
}

StringList NullWebcamDriver::getDeviceList() {
	static StringList deviceList;

	return deviceList;
}

string NullWebcamDriver::getDefaultDevice() {
	static string defaultDevice;

	return defaultDevice;
}

WebcamErrorCode NullWebcamDriver::setDevice(const std::string & deviceName) {
	return WEBCAM_OK;
}

bool NullWebcamDriver::isOpen() const {
	return false;
}

void NullWebcamDriver::startCapture() {
}

void NullWebcamDriver::pauseCapture() {
}

void NullWebcamDriver::stopCapture() {
}

WebcamErrorCode NullWebcamDriver::setPalette(pixosi palette) {
	return WEBCAM_OK;
}

pixosi NullWebcamDriver::getPalette() const {
	return PIX_OSI_UNSUPPORTED;
}

WebcamErrorCode NullWebcamDriver::setFPS(unsigned fps) {
	return WEBCAM_OK;
}

unsigned NullWebcamDriver::getFPS() const {
	return 0;
}

WebcamErrorCode NullWebcamDriver::setResolution(unsigned width, unsigned height) {
	return WEBCAM_OK;
}

unsigned NullWebcamDriver::getWidth() const {
	return 0;
}

unsigned NullWebcamDriver::getHeight() const {
	return 0;
}

void NullWebcamDriver::setBrightness(int brightness) {
}

int NullWebcamDriver::getBrightness() const {
	return 0;
}

void NullWebcamDriver::setContrast(int contrast) {
}

int NullWebcamDriver::getContrast() const {
	return 0;
}

void NullWebcamDriver::flipHorizontally(bool flip) {
}
