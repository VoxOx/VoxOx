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

#ifndef OWQTVIDEOXV_H
#define OWQTVIDEOXV_H

#include "QtVideo.h"
#include "XVWindow.h"

class QFrame;
class XVWindow;

class QtVideoXV : public QtVideo {
	Q_OBJECT
public:

	QtVideoXV(QWidget * parent, int remoteVideoFrameWidth, int remoteVideoFrameHeight, int localVideoFrameWidth, int localVideoFrameHeight);

	~QtVideoXV();

	void showImage(piximage * remoteVideoFrame, piximage * localVideoFrame);

	QSize getFrameSize() const;

	bool isInitialized() const {
		return (_remoteWindow != NULL);
	}

	bool isFullScreen() const;

	void unFullScreen();

	void fullScreen();

private Q_SLOTS:

	void flipWebcamButtonClicked();

	void fullScreenButtonClicked();

private:

	XVWindow * _localWindow;

	Display* _localDisplay;

	XVWindow * _remoteWindow;

	Display* _remoteDisplay;
};

#endif	//OWQTVIDEOXV_H
