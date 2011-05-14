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

#include "webcam_test.h"

#include <util/SafeDelete.h>

#include <cstdio>
#include <iostream>
#include <fstream>
using namespace std;

const unsigned DEST_W = 320;
const unsigned DEST_H = 240;

void frame_captured(webcam * sender, piximage * image, void * userdata) {
	WebcamTest * wctest = (WebcamTest *) userdata;

	wctest->frameCaptured(image);

	cout << "frame_captured" << endl;
}

void WebcamTest::frameCaptured(piximage * image) {
	memcpy(_image->bits(), image->data, image->width * image->height * 4);

	update();

	cout << "I am called!!" << endl;
}

void WebcamTest::paintEvent(QPaintEvent *) {
	QPainter painter(this);

	if (_image) {
		painter.drawImage(0, 0, *_image, 0, 0, _image->width(), _image->height());
	}
}

WebcamTest::WebcamTest()
	: _image(NULL) {

	string device = webcam_get_default_device(_webcam);

	_webcam = webcam_get_instance();

	webcam_add_callback(_webcam, frame_captured, this);
	if (webcam_set_device(_webcam, device.c_str()) == WEBCAM_NOK) {
		return;
	}
	webcam_set_palette(_webcam, PIX_OSI_RGB32);
	webcam_set_resolution(_webcam, DEST_W, DEST_H);

	string title = "Webcam test: using " + device;

	setWindowTitle(title.c_str());
	cout << "Using width: " << webcam_get_width(_webcam)
		<< ", height: " << webcam_get_height(_webcam)
		<< " FPS: " << webcam_get_fps(_webcam) << endl;
	setFixedSize(webcam_get_width(_webcam), webcam_get_height(_webcam));
	show();

	if (webcam_is_open(_webcam)) {
		_image = new QImage(QSize(webcam_get_width(_webcam), webcam_get_height(_webcam)), QImage::Format_ARGB32);
		if (!_image) {
			QMessageBox::critical(0, "Webcam test", "error while creating _image");
		}
		cout << "**Starting capture..." << endl;
		webcam_start_capture(_webcam);
	} else {
		QMessageBox::critical(0, "Webcam test", "No webcam found");
	}
}

WebcamTest::~WebcamTest() {
	OWSAFE_DELETE(_image);
	webcam_release(_webcam);
}

int main(int argc, char **argv) {
	QApplication app(argc, argv);

	new WebcamTest();

	return app.exec();
}
