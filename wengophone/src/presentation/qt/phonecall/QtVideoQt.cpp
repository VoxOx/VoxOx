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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtVideoQt.h"

#include "ui_VideoWindow.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <webcam/WebcamDriver.h>

#include <qtutil/PaintEventFilter.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <cutil/global.h>

#include <QtGui/QtGui>

#if defined OS_WINDOWS && defined CC_MSVC
	#include <windows.h>
#endif

static const int MAX_WIDTH = 640;
static const int MAX_HEIGHT = 480;
static double IGNORE_RATIO_DELTA = 0.2;


static void setFullScreenDesktopResolution(int width, int height) {
#if defined OS_WINDOWS && defined CC_MSVC
	//TODO put inside LibUtil

	int nModeExist;
	int nModeSwitch;
	DEVMODE devMode;
	memset(&devMode, 0, sizeof(DEVMODE));

	//Tries to switch to desktop resolution 640x480
	//Uses less processor time if the desktop resolution is lower
	devMode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, 0, &devMode);
	for (int i = 1; ;i++) {
		nModeExist = EnumDisplaySettings(NULL, i, &devMode);
		if (nModeExist == 0) {
					//End of modes, bail out.
			break;
		}
		if ((devMode.dmBitsPerPel == 32) && (devMode.dmPelsWidth == width) && (devMode.dmPelsHeight == height)) {
			nModeSwitch = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
			break;
		}
	}
#endif
}


static void restoreDesktopResolution() {
#if defined OS_WINDOWS && defined CC_MSVC
	//Restores previous resolution
	ChangeDisplaySettings(NULL, 0);
#endif
}


static QSize desktopResolution() {
	return QApplication::desktop()->screenGeometry().size();
}


void QtVideoQt::convertPixImageToQImage(piximage* pixImage, const QSize& size, QImage* image) {
	Q_ASSERT(image);
	// No need to realloc the image buffer if it's already of the right size
	if (image->size() != size || image->format() != QImage::Format_RGB32) {
		*image = QImage(size, QImage::Format_RGB32);
	}

	// This is a bit ugly. We do not create the piximage with pix_alloc but
	// initialize the piximage fields ourself. This make it possible to point
	// the data field to the QImage buffer, saving an alloc and a memcpy.
	piximage resizedPixImage;
	resizedPixImage.palette = PIX_OSI_RGB32;
	resizedPixImage.width = image->width();
	resizedPixImage.height = image->height();
	resizedPixImage.data = image->bits();

	pix_convert(PIX_NO_FLAG, &resizedPixImage, pixImage);
}

QtVideoQt::QtVideoQt(QWidget * parent)
	: QtVideo(parent)
	, _fullScreen(false)
	, _detached(false)
	{
	_videoWindow->setMaximumSize(MAX_WIDTH, MAX_HEIGHT);

	_ui->frame->installEventFilter(this);

	// Set a black background
	QPalette palette = _ui->frame->palette();
	palette.setColor(_ui->frame->backgroundRole(), Qt::black);
	
	// Set QPalette::Window too, because it will be used when the video window
	// will be detached, see documentation of QWidget::setAutoFillBackground()
	palette.setColor(QPalette::Window, Qt::black);

	_ui->frame->setAutoFillBackground(true);
	_ui->frame->setPalette(palette);
	////

	PaintEventFilter * paintFilter = new PaintEventFilter(this, SLOT(paintEvent()));
	_ui->frame->installEventFilter(paintFilter);

	connect(_ui->flipButton, SIGNAL(clicked()), SLOT(flipWebcamButtonClicked()));
	connect(_ui->fullScreenButton, SIGNAL(clicked()), SLOT(fullScreenButtonClicked()));
	connect(_ui->attachDetachButton, SIGNAL(clicked()), SLOT(toggleAttachDetach()));

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	_showLocalVideo = config.getVideoEnable();
}

QtVideoQt::~QtVideoQt() {
	if (_detached) {
		// If _ui->frame is detached, it won't be automatically deleted by Qt
		OWSAFE_DELETE(_ui->frame);
	}
}

QSize QtVideoQt::getFrameSize() const {
	return _ui->frame->frameRect().size();
}

QSize QtVideoQt::compensateForFullScreenRatio(QSize imageSize) {
	if (_normalDesktopResolution == _fullScreenDesktopResolution) {
		// Don't do anything if resolution is the same, since we already have
		// a big black border
		return imageSize;
	}

	double imageRatio = imageSize.width() / double(imageSize.height());
	double normalRatio = _normalDesktopResolution.width() / double(_normalDesktopResolution.height());
	double fullScreenRatio = _fullScreenDesktopResolution.width() / double(_fullScreenDesktopResolution.height());

	if (qAbs(imageRatio - normalRatio) < IGNORE_RATIO_DELTA) {
		// If the image and normal ratios are close enough, just stretch
		// the image, to avoid thin black stripes
		return _fullScreenDesktopResolution;
	} else {
		// Keep image ratio, but compensate the difference between normal and
		// fullscreen ratios.
		imageSize.setHeight(int(imageSize.height() * normalRatio / fullScreenRatio));
		imageSize.scale(_fullScreenDesktopResolution, Qt::KeepAspectRatio);
		return imageSize;
	}
}

void QtVideoQt::showImage(piximage * remoteVideoFrame, piximage * localVideoFrame) {
	QSize size(remoteVideoFrame->width, remoteVideoFrame->height);

	QSize maxSize = _ui->frame->size();
	if (maxSize.width() > MAX_WIDTH && maxSize.height() > MAX_HEIGHT) {
		maxSize = QSize(MAX_WIDTH, MAX_HEIGHT);
	}
	size.scale(maxSize, Qt::KeepAspectRatio);

	if (_fullScreen) {
		size = compensateForFullScreenRatio(size);
	}

	// Make sure dimensions are even, otherwise we get green lines
	size = QSize(size.width() & ~1, size.height() & ~1);

	convertPixImageToQImage(remoteVideoFrame, size, &_image);

	//If we want to embed the local webcam picture, we do it here
	if (_showLocalVideo) {
		const unsigned offset_x = 10;
		const unsigned offset_y = 10;
		const unsigned ratio = 5;
		const unsigned border_size = 1;
		const QBrush border_color = Qt::black;

		//Force the ratio of the remote frame on the webcam frame (ignoring the webcam's aspect ratio)
		unsigned width = (size.width() / ratio) & ~1;
		unsigned height = (size.height() / ratio) & ~1;
		unsigned posx = size.width() - width - offset_x;
		unsigned posy = size.height() - height - offset_y;

		QImage localImage;
		QSize localSize(width, height);
		convertPixImageToQImage(localVideoFrame, localSize, &localImage);

		QPainter painter;
		painter.begin(&_image);
		//Draw a 1-pixel border around the local embedded frame
		painter.fillRect(posx - border_size, posy - border_size, width + 2 * border_size,
			height + 2 * border_size, border_color);
		//Embed the image
		painter.drawImage(posx, posy, localImage);
		painter.end();
	}

	_ui->frame->update();
}

void QtVideoQt::paintEvent() {
	if (!_image.isNull()) {
		QPainter painter(_ui->frame);

		if (_fullScreen) {
			painter.fillRect(_ui->frame->rect(), Qt::black);
		}

		int xpos = (_ui->frame->width() - _image.width()) / 2;
		int ypos = (_ui->frame->height() - _image.height()) / 2;

		painter.drawImage(xpos, ypos, _image);
		
		if (!_fullScreen && !_detached) {
			// Depending on the frame size, the video may leave one row or
			// column of pixel empty. This looks bad under the Osd buttons.
			// To avoid this bad appearance, we draw a thin black border
			// around the video.
			painter.setPen(Qt::black);
			painter.drawRect(0, 0, _ui->frame->width() - 1, _ui->frame->height() - 1);
		}
	}
}

void QtVideoQt::fullScreenButtonClicked() {
	_ui->frame->setFocus();

	if (!_fullScreen) {
		this->fullScreen();
	} else {
		this->unFullScreen();
	}
}

void QtVideoQt::flipWebcamButtonClicked() {
	toggleFlipVideoImageSignal();
}

void QtVideoQt::fullScreen() {
	_ui->fullScreenButton->setIcon(QIcon(":/pics/video_unfullscreen.png"));

	QLayout * layout = _videoWindow->layout();
	layout->removeWidget(_ui->frame);

	_ui->frame->setParent(NULL);

	_normalDesktopResolution = desktopResolution();
	setFullScreenDesktopResolution(MAX_WIDTH, MAX_HEIGHT);
	_fullScreenDesktopResolution = desktopResolution();

	_fullScreen = true;
	_ui->attachDetachButton->hide();
	_ui->frame->showFullScreen();
}

void QtVideoQt::unFullScreen() {
	_ui->fullScreenButton->setIcon(QIcon(":/pics/video_fullscreen.png"));

	// Call showNormal instead of show because otherwise, next time you detach
	// the frame, it won't have any window decoration (at least on Mac OSX).
	//
	// We also need to do this before reparenting the frame otherwise it won't
	// work.
	_ui->frame->showNormal();
	_fullScreen = false;
	_ui->attachDetachButton->show();

	restoreDesktopResolution();

	if (!_detached) {
		_ui->frame->setParent(_videoWindow);
		QLayout * layout = _videoWindow->layout();
		layout->addWidget(_ui->frame);
	}
}

void QtVideoQt::toggleAttachDetach() {
	QString iconName;
	if (_detached) {
		// Attach
		_ui->frame->setParent(_videoWindow);
		_videoWindow->layout()->addWidget(_ui->frame);
		iconName = ":/pics/video_detach.png";
	} else {
		// Detach
		_videoWindow->layout()->removeWidget(_ui->frame);
		_ui->frame->setParent(0);

		QSize size = _image.size();
		size.scale(MAX_WIDTH, MAX_HEIGHT, Qt::KeepAspectRatio);
		_ui->frame->resize(size);

		QRect desktopRect = QApplication::desktop()->availableGeometry(_videoWindow->window());
		int posX = desktopRect.x() + (desktopRect.width() - size.width()) / 2;
		int posY = desktopRect.y() + (desktopRect.height() - size.height()) / 2;
		_ui->frame->move(posX, posY);

		_ui->frame->show();
		iconName = ":/pics/video_attach.png";
		_ui->frame->window()->setWindowTitle(tr("@product@"));
	}
	_detached = ! _detached;
	_ui->attachDetachButton->setIcon(QIcon(iconName));
}


bool QtVideoQt::eventFilter(QObject * object, QEvent * event) {
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_Escape) {
			unFullScreen();
			return true;
		} else if ((keyEvent->key() == Qt::Key_F) && (keyEvent->modifiers() == Qt::ControlModifier)) {
			toggleFullScreen();
			return true;
		}

	} else if (event->type() == QEvent::MouseButtonDblClick) {
		toggleFullScreen();
		return true;
	} else if (event->type() == QEvent::Close) {
		if (_detached) {
			// If the user closes the detached video window, we want the video
			// to get attached back
			toggleAttachDetach();
			event->ignore();
			return true;
		} else if (_fullScreen) {
			// Same for fullscreen, if the user presses Alt+F4
			unFullScreen();
			event->ignore();
			return true;
		}
	}

	return false;
}
