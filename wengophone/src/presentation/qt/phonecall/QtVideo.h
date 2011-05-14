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

#ifndef OWQTVIDEO_H
#define OWQTVIDEO_H

#include <util/Interface.h>

#include <QtCore/QObject>

#include <pixertool/pixertool.h>

class QSize;
class QWidget;
namespace Ui { class VideoWindow; }

/**
 * Plugin interface for different video backends (Qt, XV, DirectX, OpenGL...)
 *
 * @author Mathieu Stute
 * @author Tanguy Krotoff
 */
class QtVideo : public QObject, Interface {
public:

	QtVideo(QWidget * parent);

	virtual ~QtVideo();

	virtual void showImage(piximage * remoteVideoFrame, piximage * localVideoFrame) = 0;

	QWidget * getWidget() const {
		return _videoWindow;
	}

	virtual QSize getFrameSize() const = 0;

	virtual bool isInitialized() const = 0;

	virtual bool isFullScreen() const = 0;

	virtual void unFullScreen() = 0;

	virtual void fullScreen() = 0;

	void toggleFullScreen();

protected:

	Ui::VideoWindow * _ui;

	QWidget * _videoWindow;
};

#endif	//OWQTVIDEO_H
