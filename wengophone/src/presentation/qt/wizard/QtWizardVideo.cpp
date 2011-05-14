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
* Wizard video
* @author Chris Jimenez C 
* @date 2009.06.01
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtWizardVideo.h"

#include "ui_WizardVideo.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>
#include <model/phoneline/IPhoneLine.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <presentation/qt/QtWengoPhone.h>
#include <presentation/qt/phonecall/QtVideoQt.h>

#include <webcam/WebcamDriver.h>
#include <sipwrapper/EnumVideoQuality.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/StringListConvert.h>
#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

static const int VIDEO_QUALITY_COLUMN = 0;

QtWizardVideo::QtWizardVideo(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(NULL),
	_cWengoPhone(cWengoPhone) {

	_ui = new Ui::WizardVideo();
	_ui->setupUi(this);

	_webcamDriver = WebcamDriver::getInstance();
	_previewStarted = false;

	SAFE_CONNECT_TYPE(this, SIGNAL(newWebcamImage()), SLOT(newWebcamImageCaptured()), Qt::QueuedConnection);

	SAFE_CONNECT(_ui->webcamPreviewButton, SIGNAL(clicked()), SLOT(webcamPreview()));
	SAFE_CONNECT(_ui->enableVideoChk, SIGNAL(stateChanged(int)), SLOT(enableVideoChecked(int)));
	
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	//No webcam driver
	if (_webcamDriver->getDeviceList().empty()) {
		config.set(Config::VIDEO_ENABLE_KEY, false);
		_ui->enableVideoChk->setChecked(false);
		_ui->enableVideoChk->setEnabled(false);
	}
	////
	readConfig();
}

QtWizardVideo::~QtWizardVideo() {
	OWSAFE_DELETE(_ui);
}

QString QtWizardVideo::getName() const {
	return tr("Video");
}

QString QtWizardVideo::getTitle() const {
	return tr("Video Phone");
}

QString QtWizardVideo::getDescription() const {
	return tr("Take control of your video calls.");
}

int QtWizardVideo::getStepNumber() const {
	return 4;
}


void QtWizardVideo::enableVideoChecked(int checked){
	if(checked == Qt::Checked){
		_ui->webcamDeviceComboBox->setEnabled(true);
		_ui->webcamPreviewButton->setEnabled(true);
	}else if(checked == Qt::Unchecked){
		_ui->webcamDeviceComboBox->setEnabled(false);
		_ui->webcamPreviewButton->setEnabled(false);
	}
	
}

void QtWizardVideo::saveConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	config.set(Config::VIDEO_ENABLE_KEY, _ui->enableVideoChk->isChecked());
	config.set(Config::VIDEO_WEBCAM_DEVICE_KEY, std::string(_ui->webcamDeviceComboBox->currentText().toUtf8().constData()));

}

void QtWizardVideo::readConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	_ui->enableVideoChk->setChecked(config.getVideoEnable());

	
	if(_ui->enableVideoChk->isChecked()){
		_ui->webcamDeviceComboBox->setEnabled(true);
		_ui->webcamPreviewButton->setEnabled(true);
	}else{
		_ui->webcamDeviceComboBox->setEnabled(false);
		_ui->webcamPreviewButton->setEnabled(false);
	}


	_ui->webcamDeviceComboBox->clear();
	_ui->webcamDeviceComboBox->addItems(StringListConvert::toQStringList(_webcamDriver->getDeviceList()));
	int findDevice = _ui->webcamDeviceComboBox->findText(QString::fromUtf8(config.getVideoWebcamDevice().c_str()));
	if(findDevice!=-1){
		_ui->webcamDeviceComboBox->setCurrentIndex(findDevice);
	}
	
}

void QtWizardVideo::frameCapturedEventHandler(IWebcamDriver * sender, piximage * image) {

	QSize qs(_ui->webcamPreviewLabel->width(), _ui->webcamPreviewLabel->height());

	QImage tmpRgbImage(qs,  QImage::Format_RGB32);
	QtVideoQt::convertPixImageToQImage(image, qs, &tmpRgbImage);

	QPixmap tmp = QPixmap::fromImage(tmpRgbImage);
	{
	    Mutex::ScopedLock lock(_mutex);

	    _lastWebcamPixmap =  tmp;
	}

	newWebcamImage();
}

void QtWizardVideo::newWebcamImageCaptured() {
	Mutex::ScopedLock lock(_mutex);
	
	if (_lastWebcamPixmap.isNull())
	  return;

	_ui->webcamPreviewLabel->setPixmap(_lastWebcamPixmap);
}

void QtWizardVideo::startWebcamPreview(const QString & deviceName) {
	if (!_previewStarted) {
		_webcamDriver->frameCapturedEvent += boost::bind(&QtWizardVideo::frameCapturedEventHandler, this, _1, _2);
		_webcamDriver->setDevice(deviceName.toStdString());
		_webcamDriver->setResolution(320, 240);
		_webcamDriver->setPalette(PIX_OSI_YUV420P);
		_webcamDriver->startCapture();

		_previewStarted = true;
	}
}

void QtWizardVideo::stopWebcamPreview() {
	if (_previewStarted) {
		_webcamDriver->frameCapturedEvent -= boost::bind(&QtWizardVideo::frameCapturedEventHandler, this, _1, _2);
		_webcamDriver->stopCapture();

		_previewStarted = false;
	}
}

void QtWizardVideo::webcamPreview() {
	startWebcamPreview(_ui->webcamDeviceComboBox->currentText());
}

void QtWizardVideo::hideEvent(QHideEvent * event) {
	stopWebcamPreview();
}

