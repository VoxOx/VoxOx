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

#ifndef WEBCAMVIDEOFRAME_H
#define WEBCAMVIDEOFRAME_H

#include <util/Interface.h>

/**
 * Video frame received from a webcam.
 *
 * @author Tanguy Krotoff
 */
class WebcamVideoFrame : Interface {
public:

	WebcamVideoFrame() {
		_width = 0;
		_height = 0;
		_frame = NULL;
	}

	virtual ~WebcamVideoFrame() {
	}

	void setFrame(unsigned char * frame) {
		_frame = frame;
	}

	unsigned char * getFrame() const {
		return _frame;
	}

	void setWidth(int width) {
		_width = width;
	}

	int getWidth() const {
		return _width;
	}

	void setHeight(int height) {
		_height = height;
	}

	int getHeight() const {
		return _height;
	}

protected:

	int _width;

	int _height;

	unsigned char * _frame;
};

#endif	//WEBCAMVIDEOFRAME_H
