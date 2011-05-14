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

/**
 * Test program for Webcam.
 *
 * It tests multiple registration to the frameCapturedEvent on the WebcamDriver.
 *
 * @author Philippe Bernery
 */

#include "VideoWindow.h"

#include <webcam/WebcamDriver.h>

#include <QtGui/QtGui>

/**
 * Initializes and starts the WebcamDriver.
 *
 * It will use the default webcam installed on the system.
 */
static void startWebcamCapture() {
	WebcamDriver * driver = WebcamDriver::getInstance();

	std::string device = driver->getDefaultDevice();
	driver->setDevice(device);

	driver->setPalette(PIX_OSI_YUV420P);
	driver->setResolution(320, 240);

	if (driver->isOpen()) {
		driver->startCapture();
	} else {
		QMessageBox::critical(0, "Webcam multi-visu test", "No webcam found");
		exit(1);
	}
}

int main(int argc, char * argv[]) {
	QApplication app(argc, argv);

	startWebcamCapture();

	//Creating first window
	VideoWindow window;
	window.show();

	return app.exec();
}
