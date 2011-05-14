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

#ifndef OWQTVIDEOQT_H
#define OWQTVIDEOQT_H

#include "QtVideo.h"

#include <QtGui/QImage>

class QtVideoQtEventManager;

class QSize;
class QImage;

class QtVideoQt : public QtVideo {
	Q_OBJECT
public:

	QtVideoQt(QWidget * parent);

	~QtVideoQt();

	void showImage(piximage * remoteVideoFrame, piximage * localVideoFrame);

	QSize getFrameSize() const;

	bool isInitialized() const {
		return true;
	}

	bool isFullScreen() const {
		return _fullScreen;
	}

	void fullScreen();

	void unFullScreen();
	
	static void convertPixImageToQImage(piximage* pixImage, const QSize& size, QImage* image);

Q_SIGNALS:

	void toggleFlipVideoImageSignal();

protected:
	virtual bool eventFilter(QObject*, QEvent*);

private Q_SLOTS:
	void toggleAttachDetach();

	void paintEvent();

	void fullScreenButtonClicked();

	void flipWebcamButtonClicked();

private:
	/** Current frame to show inside the video window. */
	QImage _image;

	bool _fullScreen;

	bool _detached;

	bool _showLocalVideo;

	/** Store desktop resolution before switching to fullscreen */
	QSize _normalDesktopResolution;
	QSize _fullScreenDesktopResolution;

	/** 
	 * Recompute size to compensate for the ratio differences between normal
	 * and fullscreen resolutions
	 */
	QSize compensateForFullScreenRatio(QSize);
};


#endif	//OWQTVIDEOQT_H
