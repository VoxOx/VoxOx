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

#include "VideoWindow.h"

#include <webcam/WebcamDriver.h>

#include <qtutil/SafeConnect.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

VideoWindow::VideoWindow()
	: QWidget() {

	WebcamDriver::getInstance()->frameCapturedEvent +=
		boost::bind(&VideoWindow::frameCapturedEventHandler, this, _1, _2);

	setMaximumSize(320, 240);
	setMinimumSize(320, 240);

	_image = NULL;
	_rgbImage = pix_alloc(PIX_OSI_RGB32, 320, 240);
}

VideoWindow::~VideoWindow() {
	pix_free(_rgbImage);
	OWSAFE_DELETE(_image);
}

void VideoWindow::frameCapturedEventHandler(IWebcamDriver * sender, piximage * image) {
	Mutex::ScopedLock lock(_mutex);

	pix_convert(PIX_NO_FLAG, _rgbImage, image);

	OWSAFE_DELETE(_image);
	_image = new QImage(QSize(width(), height()), QImage::Format_ARGB32);

	memcpy(_image->bits(), _rgbImage->data, _rgbImage->width * _rgbImage->height * 4);

	update();
}

void VideoWindow::mouseReleaseEvent(QMouseEvent * event) {
	VideoWindow * newWindow = new VideoWindow();
	newWindow->show();

	QWidget::mouseReleaseEvent(event);
}

void VideoWindow::paintEvent(QPaintEvent *) {
	Mutex::ScopedLock lock(_mutex);

	QPainter painter(this);
	if (_image) {
		painter.drawImage(0, 0, *_image, 0, 0, _image->width(), _image->height());
	}
}
