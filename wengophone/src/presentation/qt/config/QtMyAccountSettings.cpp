/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2007  Wengo
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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.05
#include "QtMyAccountSettings.h"

#include "ui_MyAccountSettings.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/profile/CUserProfile.h>
#include <model/profile/UserProfile.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/WidgetUtils.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>



QtMyAccountSettings::QtMyAccountSettings(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone){

	_ui = new Ui::MyAccountSettings();
	_ui->setupUi(this);


	readConfig();
}

QtMyAccountSettings::~QtMyAccountSettings() {
	OWSAFE_DELETE(_ui);
}



QString QtMyAccountSettings::getName() const {
	return tr("My Account");
}

QString QtMyAccountSettings::getTitle() const {
	return tr("My Account");
}

QString QtMyAccountSettings::getDescription() const {
	return tr("My Account");
}

QString QtMyAccountSettings::getIconName() const {
	return "myaccount";
}

void QtMyAccountSettings::saveConfig() {
	//Config & config = ConfigManager::getInstance().getCurrentConfig();

	//// Toolbar
	///*EnumToolBarMode::ToolBarMode mode;
	//if (_ui->hiddenRadioButton->isChecked()) {
	//	mode = EnumToolBarMode::ToolBarModeHidden;
	//} else if (_ui->iconsOnlyRadioButton->isChecked()) {
	//	mode = EnumToolBarMode::ToolBarModeIconsOnly;
	//} else {
	//	mode = EnumToolBarMode::ToolBarModeTextUnderIcons;
	//}
	//std::string text = EnumToolBarMode::toString(mode);
	//config.set(Config::GENERAL_TOOLBARMODE_KEY, text);*/

	//QString selectedItemText = _ui->skinComboBox->currentText().toLower();

	//config.set(Config::CONTACT_LIST_STYLE_KEY, selectedItemText.toStdString());

	//// Theme
	//QListWidgetItem* item = _ui->themeListWidget->currentItem();
	//if (item) {
	//	int row = _ui->themeListWidget->row(item);
	//	QString theme = _themeList[row];
	//	config.set(Config::APPEARANCE_CHATTHEME_KEY, theme.toStdString());
	//} else {
	//	LOG_WARN("No current chat theme");
	//}
}

void QtMyAccountSettings::readConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString accountId = QString::fromStdString(_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getVoxOxAccount()->getDisplayAccountId());
	QString md5 = QString::fromStdString(_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getMd5());
	QString url = QString::fromStdString(config.getMyAccountUrl());

	QString realUrl = QString("%1?username=%2&userkey=%3").arg(url).arg(accountId).arg(md5);

	_ui->webView->load(QUrl(realUrl));

	
}
