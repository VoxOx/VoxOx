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

#include <webcam_test.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <iostream>
#include <fstream>
using namespace std;

const unsigned DEST_W = 176;
const unsigned DEST_H = 144;

void WebcamTest::frameCapturedEventHandler(IWebcamDriver * sender, piximage * image) {
	frameCaptured(image);
}

void WebcamTest::frameCapturedSlot(piximage * image) {
	Mutex::ScopedLock lock(_mutex);

	pix_free(_rgbImage);
	_rgbImage = pix_alloc(PIX_OSI_RGB32, width(), height());
	pix_convert(PIX_NO_FLAG, _rgbImage, image);

	OWSAFE_DELETE(_image);
	_image = new QImage(QSize(width(), height()), QImage::Format_ARGB32);

	memcpy(_image->bits(), _rgbImage->data, _rgbImage->width * _rgbImage->height * 4);
	update();
}

void WebcamTest::paintEvent(QPaintEvent *) {
	Mutex::ScopedLock lock(_mutex);

	QPainter painter(this);
	if (_image) {
		painter.drawImage(0, 0, *_image, 0, 0, _image->width(), _image->height());
	}
}

WebcamTest::WebcamTest()
	: _image(NULL) {

	_driver = WebcamDriver::getInstance();

	connect(this, SIGNAL(frameCaptured(piximage *)), SLOT(frameCapturedSlot(piximage*)));

	_rgbImage = pix_alloc(PIX_OSI_RGB32, DEST_W, DEST_H);

	LOG_INFO("Device list:");
	StringList deviceList = _driver->getDeviceList();
	for (register unsigned i = 0 ; i < deviceList.size() ; i++) {
		LOG_INFO("- " + deviceList[i]);
	}

	string device = _driver->getDefaultDevice();

	string title = "Webcam test: using " + device;
	LOG_INFO(title);

	_driver->frameCapturedEvent += boost::bind(&WebcamTest::frameCapturedEventHandler, this, _1, _2);

	_driver->setDevice(device);
	_driver->setPalette(PIX_OSI_YUV420P);
	_driver->setResolution(DEST_W, DEST_H);

	setWindowTitle(title.c_str());
	cout << "Using width: " << _driver->getWidth() << ", height: " << _driver->getHeight()
		<< " FPS: " << _driver->getFPS() << endl;
	show();

	if (_driver->isOpen()) {
		cout << "**Starting capture..." << endl;
		_driver->startCapture();
	} else {
		QMessageBox::critical(0, "Webcam test", "No webcam found");
	}
}

WebcamTest::~WebcamTest() {
	pix_free(_rgbImage);
	OWSAFE_DELETE(_image);
	OWSAFE_DELETE(_driver);
}

int main(int argc, char **argv) {
	QApplication app(argc, argv);

	new WebcamTest();

	return app.exec();
}
