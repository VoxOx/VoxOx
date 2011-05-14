/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#include "SoundWidget.h"

#include <sound/Sound.h>
#include <sound/AudioDeviceManager.h>
#include <sound/VolumeControl.h>

#include <ui_SoundWidget.h>

SoundWidget::SoundWidget() : QWidget(0) {
	_ui = new Ui::SoundWidget();
	_ui->setupUi(this);

	connect(_ui->fileSelectorButton, SIGNAL(clicked()), SLOT(selectFile()));
	connect(_ui->playButton, SIGNAL(clicked()), SLOT(playSlot()));
	connect(_ui->stopButton, SIGNAL(clicked()), SLOT(stopSlot()));
	connect(_ui->staticPlayButton, SIGNAL(clicked()), SLOT(staticPlay()));
	connect(_ui->readSettingsButton, SIGNAL(clicked()), SLOT(readSettings()));

	connect(_ui->inputCheckBox, SIGNAL(toggled(bool)), SLOT(setMute(bool)));
	connect(_ui->outputCheckBox, SIGNAL(toggled(bool)), SLOT(setMute(bool)));

	connect(_ui->inputSlider, SIGNAL(valueChanged(int)), SLOT(setVolume(int)));
	connect(_ui->outputSlider, SIGNAL(valueChanged(int)), SLOT(setVolume(int)));

	readSettings();
}

SoundWidget::~SoundWidget() {
	delete _ui;
}


void SoundWidget::selectFile() {
	QString name = QFileDialog::getOpenFileName(this, "Choose a sound file", QString(), "Wav files (*.wav);;All files (*.*)");
	if (name.isEmpty()) {
		return;
	}

	_fileName = name;
	_ui->fileSelectorButton->setText(_fileName);

	_sound.reset( new Sound(_fileName.toStdString()) );
}


void SoundWidget::playSlot() {
	if (!_sound.get()) {
		return;
	}
	_sound->setLoops(1);
	_sound->play();
}


void SoundWidget::stopSlot() {
	if (_sound.get()) {
		_sound->stop();
	}
}


void SoundWidget::staticPlay() {
	Sound::play(_fileName.toStdString());
}


void SoundWidget::setMute(bool value) {
	AudioDevice device;
	if (sender() == _ui->inputCheckBox) {
		device = AudioDeviceManager::getInstance().getDefaultInputDevice();
	} else {
		device = AudioDeviceManager::getInstance().getDefaultOutputDevice();
	}
	VolumeControl volumeControl(device);
	volumeControl.setMute(value);
}


void SoundWidget::setVolume(int value) {
	AudioDevice device;
	if (sender() == _ui->inputSlider) {
		device = AudioDeviceManager::getInstance().getDefaultInputDevice();
	} else {
		device = AudioDeviceManager::getInstance().getDefaultOutputDevice();
	}
	VolumeControl volumeControl(device);
	volumeControl.setLevel(value);
}


void SoundWidget::readSettings() {
	AudioDevice device;

	device = AudioDeviceManager::getInstance().getDefaultInputDevice();
	{
		VolumeControl volumeControl(device);

		_ui->inputCheckBox->setChecked(volumeControl.isMuted());
		_ui->inputSlider->setValue(volumeControl.getLevel());
	}

	device = AudioDeviceManager::getInstance().getDefaultOutputDevice();
	{
		VolumeControl volumeControl(device);

		_ui->outputCheckBox->setChecked(volumeControl.isMuted());
		_ui->outputSlider->setValue(volumeControl.getLevel());
	}
}
