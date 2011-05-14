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

#ifndef QTVIDEOSETTINGS_H
#define QTVIDEOSETTINGS_H

#include "QtISettings.h"

#include <pixertool/pixertool.h>

#include <thread/Mutex.h>
#include <util/Trackable.h>

#include <QtGui/QPixmap>
#include <QtGui/QWidget>

class IWebcamDriver;

class QImage;
class QHideEvent;
class QString;
class CWengoPhone;
namespace Ui { class VideoSettings; }

/**
 * Video configuration panel.
 *
 * Inherits from QWidget so we can overwrite QWidget::hideEvent() and
 * QWidget::showEvent() that are protected methods from QWidget.
 *
 * @author Tanguy Krotoff
 */
class QtVideoSettings : public QWidget, public QtISettings, public Trackable {
	Q_OBJECT
public:

	QtVideoSettings(CWengoPhone & cWengoPhone, QWidget * parent);

	virtual ~QtVideoSettings();

	virtual QString getName() const;

	virtual QString getTitle() const;

	virtual QString getDescription() const;
	virtual void saveConfig();

	virtual QString getIconName() const;

	virtual QWidget *getWidget() const {
		return (QWidget*)this;
	}

private Q_SLOTS:

	void webcamPreview();

	void enableVideoChecked(int);

	void newWebcamImageCaptured();

	void startWebcamPreview(const QString & deviceName);

	void makeTestCallClicked();

Q_SIGNALS:

	void newWebcamImage();

private:

	virtual void readConfig();

	void stopWebcamPreview();

	void frameCapturedEventHandler(IWebcamDriver * sender, piximage * image);

	void hideEvent(QHideEvent * event);

	Ui::VideoSettings * _ui;

	IWebcamDriver * _webcamDriver;

	/** Contains the converted picture from the Webcam. */
	QImage _rgbImage;

	QPixmap _lastWebcamPixmap;

	Mutex _mutex;

	CWengoPhone & _cWengoPhone;

	bool _previewStarted;
};

#endif	//QTVIDEOSETTINGS_H
