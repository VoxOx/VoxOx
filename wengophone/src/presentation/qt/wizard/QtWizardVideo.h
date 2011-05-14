/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* @Wizard Video
* @author Chris Jimenez C 
* @date 2009.06.01
*/


#ifndef QTWIZARDVIDEO_H
#define QTWIZARDVIDEO_H

#include "QtIWizard.h"

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
namespace Ui { class WizardVideo; }

class QtWizardVideo : public QWidget, public QtIWizard, public Trackable {
	Q_OBJECT
public:

	QtWizardVideo(CWengoPhone & cWengoPhone, QWidget * parent);

	virtual ~QtWizardVideo();

	virtual QString getName() const;

	virtual QString getTitle() const;

	virtual QString getDescription() const;

	virtual void saveConfig();

	virtual int getStepNumber() const;

	virtual QString getPixmapPath() const{return "";}

	virtual QWidget *getWidget() const {
		return (QWidget*)this;
	}

private Q_SLOTS:

	void webcamPreview();

	void enableVideoChecked(int);

	void newWebcamImageCaptured();

	void startWebcamPreview(const QString & deviceName);


Q_SIGNALS:

	void newWebcamImage();

private:

	virtual void readConfig();

	void stopWebcamPreview();

	void frameCapturedEventHandler(IWebcamDriver * sender, piximage * image);

	void hideEvent(QHideEvent * event);

	Ui::WizardVideo * _ui;

	IWebcamDriver * _webcamDriver;

	/** Contains the converted picture from the Webcam. */
	QImage _rgbImage;

	QPixmap _lastWebcamPixmap;

	Mutex _mutex;

	CWengoPhone & _cWengoPhone;

	bool _previewStarted;
};

#endif	//QTWizardVideo_H
