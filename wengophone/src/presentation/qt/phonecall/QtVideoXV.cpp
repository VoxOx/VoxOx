/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
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

#include "QtVideoXV.h"

#include "ui_VideoWindow.h"

#include <webcam/WebcamDriver.h>

#include <util/Logger.h>

#include <cutil/global.h>

QtVideoXV::QtVideoXV(QWidget * parent,
	int remoteVideoFrameWidth, int remoteVideoFrameHeight,
	int localVideoFrameWidth, int localVideoFrameHeight)
	: QtVideo(parent) {

	//flipButton
	connect(_ui->flipButton, SIGNAL(clicked()), SLOT(flipWebcamButtonClicked()));

	//fullScreenButton
	connect(_ui->fullScreenButton, SIGNAL(clicked()), SLOT(fullScreenButtonClicked()));

	_remoteWindow = new XVWindow();
	_remoteDisplay=XOpenDisplay(NULL);
	if (_remoteWindow->init(_remoteDisplay, DefaultRootWindow(_remoteDisplay),
		     0, 0, remoteVideoFrameWidth * 2, remoteVideoFrameHeight * 2, remoteVideoFrameWidth, remoteVideoFrameHeight)) {
			_localWindow = new XVWindow();
			LOG_DEBUG("remote window initialization: success");

    			_localDisplay=XOpenDisplay(NULL);
			if (_localWindow->init(_localDisplay, _remoteWindow->getWindow(), 0, 0, localVideoFrameWidth, localVideoFrameHeight, localVideoFrameWidth, localVideoFrameHeight)) {

				_remoteWindow->registerSlave(_localWindow);
				_localWindow->registerMaster(_remoteWindow);
				LOG_DEBUG("local window initialization: success");

			} else {
				delete _localWindow;
				XCloseDisplay(_localDisplay);
				_localWindow = NULL;

				//TODO in this case to have the local video image
				//we must compute the final image

				LOG_DEBUG("local window initialization: failed");
			}

	} else {
		delete _remoteWindow;
		XCloseDisplay(_remoteDisplay);
		_remoteWindow = NULL;
		_localWindow = NULL;
		LOG_DEBUG("remote window initialization: failed");
	}
}

QtVideoXV::~QtVideoXV() {
	if (_remoteWindow) {
		_remoteWindow->registerSlave(NULL);
		if (_localWindow) {
			_localWindow->registerMaster(NULL);
			delete _localWindow;
			XCloseDisplay(_localDisplay);
		}
		delete _remoteWindow;
		XCloseDisplay(_remoteDisplay);
	}
}

bool QtVideoXV::isFullScreen() const {
	return _remoteWindow->isFullScreen();
}

QSize QtVideoXV::getFrameSize() const {
	return _ui->frame->frameRect().size();
}

void QtVideoXV::showImage(piximage * remoteVideoFrame, piximage * localVideoFrame) {
	if (_remoteWindow) {
		_remoteWindow->putFrame(remoteVideoFrame->data, remoteVideoFrame->width, remoteVideoFrame->height);
		if (_localWindow) {
			_localWindow->putFrame(localVideoFrame->data, localVideoFrame->width, localVideoFrame->height);
		}
	}
}

void QtVideoXV::flipWebcamButtonClicked() {
	static bool flip = true;

	IWebcamDriver * driver = WebcamDriver::getInstance();
	driver->flipHorizontally(flip);

	flip = !flip;
}

void QtVideoXV::fullScreenButtonClicked() {
	_remoteWindow->toggleFullscreen();
}

void QtVideoXV::unFullScreen() {
	if (isFullScreen()) {
		_remoteWindow->toggleFullscreen();
	}
}

void QtVideoXV::fullScreen() {
	if (!isFullScreen()) {
		_remoteWindow->toggleFullscreen();
	}
}
