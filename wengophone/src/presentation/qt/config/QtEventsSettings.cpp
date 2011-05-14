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
#include "QtEventsSettings.h"

#include "ui_EventsSettings.h"
//#include "ui_EventsSettingsItem.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <sound/AudioDeviceManager.h>
#include <sound/Sound.h>

#include <util/File.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/DesktopService.h>
#include <qtutil/SafeConnect.h>

#include <QtGui/QFileDialog>
#include <QtGui/QWidget>

//QtEventsSettingsItem::QtEventsSettingsItem(const std::string& configKey,
//	const std::string& soundFile, const QString& caption)
//	: QWidget()
//	, _configKey(configKey)
//	, _soundFile(QString::fromStdString(soundFile))
//	, _caption(caption) {
//	_ui = new Ui::EventsSettingsItem;
//}
//
//QtEventsSettingsItem::~QtEventsSettingsItem() {
//	OWSAFE_DELETE(_ui);
//}

//void QtEventsSettingsItem::init(QWidget* parent) {
//	setParent(parent);
//	_ui->setupUi(this);
//
//	// add an icon to browseButton
//	QPixmap pixBrowse = DesktopService::getInstance()->desktopIconPixmap(DesktopService::FolderIcon, 16);
//	_ui->browseButton->setIcon(pixBrowse);
//	////
//	
//	SAFE_CONNECT(_ui->browseButton, SIGNAL(clicked()), SLOT(browseSounds()));
//	SAFE_CONNECT(_ui->playButton, SIGNAL(clicked()), SLOT(playSound()));
//	SAFE_CONNECT(_ui->soundLineEdit, SIGNAL(textChanged(const QString&)), SLOT(updatePlayButton()));
//	SAFE_CONNECT(_ui->checkBox, SIGNAL(toggled(bool)), SLOT(updatePlayButton()));
//
//	_ui->checkBox->setText(_caption);
//	_ui->checkBox->setChecked(!_soundFile.isEmpty());
//	_ui->frame->setEnabled(_ui->checkBox->isChecked());
//	_ui->soundLineEdit->setText(_soundFile);
//}
//
//void QtEventsSettingsItem::browseSounds() {
//	Config & config = ConfigManager::getInstance().getCurrentConfig();
//
//	QString file = QFileDialog::getOpenFileName(this,
//				tr("Choose a sound file"),
//				QString::fromStdString(config.getResourcesDir()) + "sounds",
//				tr("Sounds") + " (*.wav)");
//
//	std::string tmp = file.toStdString();
//	tmp = File::convertPathSeparators(tmp);
//	file = QString::fromStdString(tmp);
//
//	if (!file.isEmpty()) {
//		_ui->soundLineEdit->setText(file);
//	}
//}
//
//void QtEventsSettingsItem::playSound() {
//	std::string soundFile = _ui->soundLineEdit->text().toStdString();
//	Sound::play(soundFile, AudioDeviceManager::getInstance().getDefaultOutputDevice());
//}
//
//void QtEventsSettingsItem::saveConfig() {
//	Config & config = ConfigManager::getInstance().getCurrentConfig();
//	
//	std::string name;
//	if (_ui->checkBox->isChecked()) {
//		name = _ui->soundLineEdit->text().toStdString();
//	}
//	config.setResource(_configKey, name);
//}
//
//void QtEventsSettingsItem::updatePlayButton() {
//	_ui->playButton->setEnabled(!_ui->soundLineEdit->text().isEmpty());
//}

QtEventsSettings::QtEventsSettings(QWidget * parent)
	: QWidget(parent) {

	_ui = new Ui::EventsSettings();
	_ui->setupUi(this);

	/*QPixmap pixBrowse = DesktopService::getInstance()->desktopIconPixmap(DesktopService::FolderIcon, 16);
	_ui->browseButtonIncomingCall->setIcon(pixBrowse);
	_ui->browseButtonCallClose->setIcon(pixBrowse);
	_ui->browseButtonIncomingChat->setIcon(pixBrowse);*/

	SAFE_CONNECT(_ui->browseButtonIncomingCall, SIGNAL(clicked()), SLOT(browseIncomingCallSound()));
	SAFE_CONNECT(_ui->browseButtonCallClose, SIGNAL(clicked()), SLOT(browseCallCloseSound()));
	SAFE_CONNECT(_ui->browseButtonIncomingChat, SIGNAL(clicked()), SLOT(browseIncomingChatSound()));

	SAFE_CONNECT(_ui->playButtonIncomingCall, SIGNAL(clicked()), SLOT(playIncomingCallSound()));
	SAFE_CONNECT(_ui->playButtonCallClose, SIGNAL(clicked()), SLOT(playCallCloseSound()));
	SAFE_CONNECT(_ui->playButtonIncomingChat, SIGNAL(clicked()), SLOT(playIncomingChatSound()));


	readConfig();

	//SAFE_CONNECT(_ui->soundLineEdit, SIGNAL(textChanged(const QString&)), SLOT(updatePlayButton()));


	/*QVBoxLayout* layout = new QVBoxLayout(_ui->contentFrame);
	layout->setMargin(0);
	layout->setSpacing(0);*/

	/*Config & config = ConfigManager::getInstance().getCurrentConfig();
	_list
		<< new QtEventsSettingsItem(
			Config::AUDIO_INCOMINGCALL_FILE_KEY,
			config.getAudioIncomingCallFile(),
			tr("Incoming call (ringtone)"))

		<< new QtEventsSettingsItem(
			Config::AUDIO_CALLCLOSED_FILE_KEY,
			config.getAudioCallClosedFile(),
			tr("Call closed (hang up tone)"))

		<< new QtEventsSettingsItem(
			Config::AUDIO_INCOMINGCHAT_FILE_KEY,
			config.getAudioIncomingChatFile(),
			tr("Incoming chat"))

		<< new QtEventsSettingsItem(
			Config::AUDIO_IMACCOUNTCONNECTED_FILE_KEY,
			config.getAudioIMAccountConnectedFile(),
			tr("IM account connected"))

		<< new QtEventsSettingsItem(
			Config::AUDIO_IMACCOUNTDISCONNECTED_FILE_KEY,
			config.getAudioIMAccountDisconnectedFile(),
			tr("IM account disconnected"))

		<< new QtEventsSettingsItem(
			Config::AUDIO_CONTACTONLINE_FILE_KEY,
			config.getAudioContactOnlineFile(),
			tr("Contact online"))
		;

	Q_FOREACH(QtEventsSettingsItem* item, _list) {
		item->init(_ui->contentFrame);
		layout->addWidget(item);
	}*/
}



QString QtEventsSettings::browseSounds() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QString file = QFileDialog::getOpenFileName(this,
				tr("Choose a sound file"),
				QString::fromStdString(config.getResourcesDir()) + "sounds",
				tr("Sounds") + " (*.wav)");

	std::string tmp = file.toStdString();
	tmp = File::convertPathSeparators(tmp);
	file = QString::fromStdString(tmp);

	return file;
}

QString QtEventsSettings::browseRingtoneSounds() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QString file = QFileDialog::getOpenFileName(this,
				tr("Choose a sound file"),
				QString::fromStdString(config.getResourcesDir()) + "sounds/ringtones",
				tr("Sounds") + " (*.wav)");

	std::string tmp = file.toStdString();
	tmp = File::convertPathSeparators(tmp);
	file = QString::fromStdString(tmp);

	return file;
}

QString QtEventsSettings::browseAlertSounds() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QString file = QFileDialog::getOpenFileName(this,
				tr("Choose a sound file"),
				QString::fromStdString(config.getResourcesDir()) + "sounds/alerts",
				tr("Sounds") + " (*.wav)");

	std::string tmp = file.toStdString();
	tmp = File::convertPathSeparators(tmp);
	file = QString::fromStdString(tmp);

	return file;
}

void QtEventsSettings::browseIncomingCallSound() {
	QString path = browseRingtoneSounds();
	QFileInfo info = QFileInfo(path);
	if(info.isFile()){
		_incomingCallSoundPath = path;
		_ui->chkIncomingCallFileName->setText(info.fileName());
	}
}

void QtEventsSettings::browseIncomingChatSound() {
	QString path = browseAlertSounds();
	QFileInfo info = QFileInfo(path);
	if(info.isFile()){
		_incomingChatSoundPath = path;
		_ui->chkIncomingChatFileName->setText(info.fileName());
	}
}

void QtEventsSettings::browseCallCloseSound() {
	QString path = browseSounds();
	QFileInfo info = QFileInfo(path);
	if(info.isFile()){
		_callClosedSoundPath = path;
		_ui->chkCallClosedFileName->setText(info.fileName());
	}
}

void QtEventsSettings::playSound(QString & path) {
	std::string soundFile = path.toStdString();
	Sound::play(soundFile, AudioDeviceManager::getInstance().getDefaultOutputDevice());
}

void QtEventsSettings::playIncomingCallSound() {
	playSound(_incomingCallSoundPath);
}

void QtEventsSettings::playIncomingChatSound() {
	playSound(_incomingChatSoundPath);
}

void QtEventsSettings::playCallCloseSound() {
	playSound(_callClosedSoundPath);
}


QtEventsSettings::~QtEventsSettings() {
	OWSAFE_DELETE(_ui);
}

void QtEventsSettings::readConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	//VOXOX - CJC - 2009.05.25 Get path
	_incomingCallSoundPath = QString::fromStdString(config.getAudioIncomingCallFile());
	_callClosedSoundPath = QString::fromStdString(config.getAudioCallClosedFile());
	_incomingChatSoundPath = QString::fromStdString(config.getAudioIncomingChatFile());

	//VOXOX - CJC - 2009.05.25 Set file names
	QFileInfo incomingCallFile = QFileInfo(_incomingCallSoundPath);
	if(incomingCallFile.isFile()){
		_ui->chkIncomingCallFileName->setText(incomingCallFile.fileName());
		_ui->chkIncomingCallFileName->setChecked(config.getNotificationPlaySoundOnIncomingCall());
	}
	QFileInfo closeCallFile = QFileInfo(_callClosedSoundPath);
	if(closeCallFile.isFile()){
		_ui->chkCallClosedFileName->setText(closeCallFile.fileName());
		_ui->chkCallClosedFileName->setChecked(config.getNotificationPlaySoundOnCallClosed());
	}
	QFileInfo incomingChatFile = QFileInfo(_incomingChatSoundPath);
	if(incomingChatFile.isFile()){
		_ui->chkIncomingChatFileName->setText(incomingChatFile.fileName());
		_ui->chkIncomingChatFileName->setChecked(config.getNotificationPlaySoundOnIncomingChat());
	}
	//VOXOX - CJC - 2009.05.25 Set toasters
	_ui->chkIncomingCallPopup->setChecked(config.getNotificationShowToasterOnIncomingCall());
	_ui->chkIncomingChatPopup->setChecked(config.getNotificationShowToasterOnIncomingChat());

}

QString QtEventsSettings::getName() const {
	return tr("Events");
}

QString QtEventsSettings::getTitle() const {
	return tr("Events Settings");
}

QString QtEventsSettings::getDescription() const {
	return tr("Events Settings");
}

QString QtEventsSettings::getIconName() const {
	return "events";
}

void QtEventsSettings::saveConfig() {
	

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	//VOXOX - CJC - 2009.05.25 Toasters
	config.set(Config::NOTIFICATION_SHOW_TOASTER_ON_INCOMING_CALL_KEY, _ui->chkIncomingCallPopup->isChecked());
	config.set(Config::NOTIFICATION_SHOW_TOASTER_ON_INCOMING_CHAT_KEY, _ui->chkIncomingChatPopup->isChecked());
	//VOXOX - CJC - 2009.05.25 Sounds
	config.set(Config::NOTIFICATION_PLAY_SOUND_ON_INCOMING_CALL_KEY, _ui->chkIncomingCallFileName->isChecked());
	config.set(Config::NOTIFICATION_PLAY_SOUND_ON_INCOMING_CHAT_KEY, _ui->chkIncomingChatFileName->isChecked());
	config.set(Config::NOTIFICATION_PLAY_SOUND_ON_CALL_CLOSED, _ui->chkCallClosedFileName->isChecked());
	//VOXOX - CJC - 2009.05.25 Sound Paths
	config.set(Config::AUDIO_INCOMINGCALL_FILE_KEY, _incomingCallSoundPath.toStdString());
	config.set(Config::AUDIO_INCOMINGCHAT_FILE_KEY, _incomingChatSoundPath.toStdString());
	config.set(Config::AUDIO_CALLCLOSED_FILE_KEY, _callClosedSoundPath.toStdString());

}
