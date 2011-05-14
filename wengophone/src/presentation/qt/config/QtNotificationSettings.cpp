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
#include "QtNotificationSettings.h"

#include "ui_NotificationSettings.h"
//#include "ui_NotificationSettingsItem.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <sound/AudioDeviceManager.h>
#include <sound/Sound.h>

#include <util/File.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/profile/CUserProfile.h>
#include <model/profile/UserProfile.h>


#include <qtutil/DesktopService.h>
#include <qtutil/SafeConnect.h>

#include <QtGui/QFileDialog>
#include <QtGui/QWidget>

#include <QtWebKit/QWebFrame>


QtNotificationSettings::QtNotificationSettings(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone) {

	_ui = new Ui::NotificationSettings();
	_ui->setupUi(this);

	readConfig();
}

QtNotificationSettings::~QtNotificationSettings() {
	OWSAFE_DELETE(_ui);
}

void QtNotificationSettings::readConfig() {

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QString url  = QString::fromStdString(config.getConfigNotificationSettingsUrl());

	QString accountId = QString::fromStdString(_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getVoxOxAccount()->getDisplayAccountId());
	QString md5 = QString::fromStdString(_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getMd5());
	
	QString realUrl = QString("%1?username=%2&userkey=%3").arg(url).arg(accountId).arg(md5);

	_ui->webView->setUrl(realUrl);
}

QString QtNotificationSettings::getName() const {
	return tr("Notifications");
}

QString QtNotificationSettings::getTitle() const {
	return tr("Notifications Settings");
}

QString QtNotificationSettings::getDescription() const {
	return tr("Notifications Settings");
}

QString QtNotificationSettings::getIconName() const {
	return "notifications";
}

void QtNotificationSettings::saveConfig() {

	QString script = QString("save()");

	_ui->webView->page()->mainFrame()->evaluateJavaScript(script);
	
	//_ui->webView->page()->mainFrame()->evaluateJavaScript("save()");
}
