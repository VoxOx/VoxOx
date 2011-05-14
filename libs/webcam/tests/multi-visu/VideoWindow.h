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

#ifndef OWVIDEOWINDOW_H
#define OWVIDEOWINDOW_H

#include <pixertool/pixertool.h>
#include <thread/Mutex.h>
#include <util/Trackable.h>

#include <QtGui/QWidget>

class IWebcamDriver;
class QImage;

/**
 * A simple Video window.
 *
 * It displays frames coming from the WebcamDriver.
 * Clicking on the picture will create a new VideoWindow displaying the same frame.
 *
 * @author Philippe Bernery
 */
class VideoWindow : public QWidget, public Trackable {
public:

	VideoWindow();

	~VideoWindow();

private:

	void frameCapturedEventHandler(IWebcamDriver * sender, piximage * image);

	virtual void mouseReleaseEvent(QMouseEvent * event);

	virtual void paintEvent(QPaintEvent *);

	QImage * _image;

	piximage * _rgbImage;

	Mutex _mutex;
};

#endif	//OWVIDEOWINDOW_H
