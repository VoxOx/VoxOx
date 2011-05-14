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
#include "QtPrivacySettings.h"

#include "ui_PrivacySettings.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <QtGui/QtGui>

QtPrivacySettings::QtPrivacySettings(QWidget * parent)
	: QWidget(parent) {

	_ui = new Ui::PrivacySettings();
	_ui->setupUi(this);

	readConfig();
}

QtPrivacySettings::~QtPrivacySettings() {
	delete _ui;
}

QString QtPrivacySettings::getName() const {
	return tr("Privacy");
}

QString QtPrivacySettings::getTitle() const {
	return tr("Privacy Settings");
}

QString QtPrivacySettings::getDescription() const {
	return tr("Privacy Settings");
}

QString QtPrivacySettings::getIconName() const {
	return "privacy";
}

void QtPrivacySettings::readConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	_ui->allowCallsFromAnyoneRadioButton->setChecked(config.getPrivacyAllowCallFromAnyone());
	_ui->allowCallsOnlyFromContactListRadioButton->setChecked(config.getPrivacyAllowCallOnlyFromContactList());
	_ui->allowChatsFromAnyoneRadioButton->setChecked(config.getPrivacyAllowChatsFromAnyone());
	_ui->allowChatsFromOnlyContactListRadioButton->setChecked(config.getPrivacyAllowChatOnlyFromContactList());
	_ui->alwaysSignAsInvisible->setChecked(config.getPrivacySignAsInvisible());
}

void QtPrivacySettings::saveConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	config.set(Config::PRIVACY_ALLOW_CALL_FROM_ANYONE_KEY, _ui->allowCallsFromAnyoneRadioButton->isChecked());
	config.set(Config::PRIVACY_ALLOW_CALL_ONLY_FROM_CONTACT_LIST_KEY, _ui->allowCallsOnlyFromContactListRadioButton->isChecked());
	config.set(Config::PRIVACY_ALLOW_CHAT_FROM_ANYONE_KEY, _ui->allowChatsFromAnyoneRadioButton->isChecked());
	config.set(Config::PRIVACY_ALLOW_CHAT_ONLY_FROM_CONTACT_LIST_KEY, _ui->allowChatsFromOnlyContactListRadioButton->isChecked());
	config.set(Config::PRIVACY_SIGN_AS_INVISIBLE_KEY, _ui->alwaysSignAsInvisible->isChecked());
}
