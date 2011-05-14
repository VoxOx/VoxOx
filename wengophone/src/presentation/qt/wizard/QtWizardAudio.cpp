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
* CLASS DESCRIPTION 
* @author Chris Jimenez C 
* @date 2009.06.01
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtWizardAudio.h"

#include "ui_WizardAudio.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>
#include <model/phoneline/IPhoneLine.h>
#include <model/wenbox/EnumWenboxStatus.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <presentation/qt/QtWengoPhone.h>

#include <sound/AudioDeviceManager.h>
#include <sound/Sound.h>
#include <qtutil/StringListConvert.h>
#include <qtutil/SafeConnect.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

static AudioDevice getAudioDeviceFromComboBox(QComboBox* comboBox, const std::list<AudioDevice> deviceList) {
	std::string concatString = comboBox->itemData(comboBox->currentIndex()).toString().toStdString();
	for (std::list<AudioDevice>::const_iterator it = deviceList.begin();
		it != deviceList.end();
		++it) {
		if (it->getData().toString() == concatString) {
			return *it;
		}
	}
	return AudioDevice();
}

QtWizardAudio::QtWizardAudio(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone) {

	_ui = new Ui::WizardAudio();
	_ui->setupUi(this);


	SAFE_CONNECT(_ui->testOutputDeviceButton, SIGNAL(pressed()), SLOT(testOutputDevice()));
	SAFE_CONNECT(_ui->testRingingDeviceButton, SIGNAL(pressed()), SLOT(testRingingDevice()));
	
	readConfig();
}

QtWizardAudio::~QtWizardAudio() {
	OWSAFE_DELETE(_ui);
}

QString QtWizardAudio::getName() const {
	return tr("Audio");
}

QString QtWizardAudio::getTitle() const {
	return tr("Audio");
}

QString QtWizardAudio::getDescription() const {
	return tr("Configure your audio devices.");
}


int QtWizardAudio::getStepNumber() const {
	return 3;
}


void QtWizardAudio::saveConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	AudioDevice device = getAudioDeviceFromComboBox(_ui->inputDeviceComboBox, AudioDeviceManager::getInstance().getInputDeviceList());
	config.set(Config::AUDIO_INPUT_DEVICEID_KEY, device.getData());

	device = getAudioDeviceFromComboBox(_ui->outputDeviceComboBox, AudioDeviceManager::getInstance().getOutputDeviceList());
	config.set(Config::AUDIO_OUTPUT_DEVICEID_KEY, device.getData());

	device = getAudioDeviceFromComboBox(_ui->ringingDeviceComboBox, AudioDeviceManager::getInstance().getOutputDeviceList());
	config.set(Config::AUDIO_RINGER_DEVICEID_KEY, device.getData());

}

void QtWizardAudio::readConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	//inputDeviceList
	_ui->inputDeviceComboBox->clear();

#ifdef OS_LINUX
	bool savedSettingsFound = false;
	AudioDevice tmpDev(config.getAudioInputDeviceId());
#endif
	std::list<AudioDevice> inputDeviceList = AudioDeviceManager::getInstance().getInputDeviceList();
	for (std::list<AudioDevice>::const_iterator it = inputDeviceList.begin();
		it != inputDeviceList.end();
		++it) {
		_ui->inputDeviceComboBox->addItem(
			QString::fromUtf8((*it).getName().c_str()),
			QString::fromStdString((*it).getData().toString())
		);
#ifdef OS_LINUX
		if ((*it).getData() == tmpDev.getData()) {
			savedSettingsFound = true;
		}
#endif
	}

#ifdef OS_LINUX
	if (!savedSettingsFound) {
		_ui->outputDeviceComboBox->addItem(
			QString::fromUtf8(tmpDev.getName().c_str()),
			QString::fromStdString(tmpDev.getData().toString())
		);
	}
#endif
	QString currentInputDeviceId = 
		QString::fromUtf8(config.getAudioInputDeviceId().toString().c_str());
	_ui->inputDeviceComboBox->setCurrentIndex(
		_ui->inputDeviceComboBox->findData(currentInputDeviceId)
	);
	////

	//outputDeviceList
	_ui->outputDeviceComboBox->clear();

#ifdef OS_LINUX
	savedSettingsFound = false;
	tmpDev = AudioDevice(config.getAudioOutputDeviceId());
#endif
	std::list<AudioDevice> outputDeviceList = AudioDeviceManager::getInstance().getOutputDeviceList();	
	for (std::list<AudioDevice>::const_iterator it = outputDeviceList.begin();
		it != outputDeviceList.end();
		++it) {
		_ui->outputDeviceComboBox->addItem(
			QString::fromUtf8((*it).getName().c_str()),
			QString::fromStdString((*it).getData().toString())
		);
#ifdef OS_LINUX
		if ((*it).getData() == tmpDev.getData()) {
			savedSettingsFound = true;
		}
#endif
	}

#ifdef OS_LINUX
	if (!savedSettingsFound) {
		_ui->outputDeviceComboBox->addItem(
			QString::fromUtf8(tmpDev.getName().c_str()),
			QString::fromStdString(tmpDev.getData().toString())
		);
	}
#endif
	QString currentOutputDeviceId = 
		QString::fromUtf8(config.getAudioOutputDeviceId().toString().c_str());
	_ui->outputDeviceComboBox->setCurrentIndex(
		_ui->outputDeviceComboBox->findData(currentOutputDeviceId)
	);
	////

	//ringingDeviceList = outputDeviceList
	_ui->ringingDeviceComboBox->clear();
#ifdef OS_LINUX
	savedSettingsFound = false;
	tmpDev = AudioDevice(config.getAudioOutputDeviceId());
#endif
	for (std::list<AudioDevice>::const_iterator it = outputDeviceList.begin();
		it != outputDeviceList.end();
		++it) {
		_ui->ringingDeviceComboBox->addItem(
			QString::fromUtf8((*it).getName().c_str()),
			QString::fromStdString((*it).getData().toString())
		);
#ifdef OS_LINUX
		if ((*it).getData() == tmpDev.getData()) {
			savedSettingsFound = true;
		}
#endif
	}

#ifdef OS_LINUX
	if (!savedSettingsFound) {
		_ui->ringingDeviceComboBox->addItem(
			QString::fromUtf8(tmpDev.getName().c_str()),
			QString::fromStdString(tmpDev.getData().toString())
		);
	}
#endif
	QString currentRingerDeviceId = 
		QString::fromUtf8(config.getAudioRingerDeviceId().toString().c_str());
	_ui->ringingDeviceComboBox->setCurrentIndex(
		_ui->ringingDeviceComboBox->findData(currentRingerDeviceId)
	);
	////
	

}


void QtWizardAudio::testOutputDevice() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	Sound* testSound = new Sound(config.getAudioCallClosedFile());
	AudioDevice device = getAudioDeviceFromComboBox(_ui->outputDeviceComboBox, AudioDeviceManager::getInstance().getOutputDeviceList());	
	testSound->setWaveOutDevice(device);
	//Play the sound 4 times
	testSound->setLoops(4);
	testSound->play();
}

void QtWizardAudio::testRingingDevice() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	Sound* testSound = new Sound(config.getAudioCallClosedFile());
	AudioDevice device = getAudioDeviceFromComboBox(_ui->ringingDeviceComboBox, AudioDeviceManager::getInstance().getOutputDeviceList());
	testSound->setWaveOutDevice(device);
	//Play the sound 4 times
	testSound->setLoops(4);
	testSound->play();
}


