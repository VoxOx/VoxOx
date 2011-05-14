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

#ifndef WEBCAMTEST_H
#define WEBCAMTEST_H

#include <webcam/WebcamDriver.h>

#include <pixertool/pixertool.h>

#include <thread/Mutex.h>

#include <QtGui/QtGui>

class WebcamTest : public QMainWindow {
	Q_OBJECT
public:

	WebcamTest();

	~WebcamTest();

	void frameCapturedEventHandler(IWebcamDriver * sender, piximage * image);

Q_SIGNALS:

	void frameCaptured(piximage * image);

private Q_SLOTS:

	void frameCapturedSlot(piximage * image);

private:

	void paintEvent(QPaintEvent *);

	IWebcamDriver * _driver;

	QImage * _image;

	piximage * _rgbImage;

	Mutex _mutex;
};

#endif	//WEBCAMTEST_H
