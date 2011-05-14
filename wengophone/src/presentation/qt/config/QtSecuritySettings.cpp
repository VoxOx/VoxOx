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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtSecuritySettings.h"

#include "ui_SecuritySettings.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileHandler.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfileHandler.h>

#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/StringList.h>

#include <QtGui/QtGui>

QtSecuritySettings::QtSecuritySettings(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone) {

	_ui = new Ui::SecuritySettings();
	_ui->setupUi(this);
	readConfig();
}

QtSecuritySettings::~QtSecuritySettings() {
	OWSAFE_DELETE(_ui);
}

QString QtSecuritySettings::getName() const {
	return tr("Security");
}

QString QtSecuritySettings::getTitle() const {
	return tr("Security Settings") + "  -  " + tr("BETA");
}

QString QtSecuritySettings::getDescription() const {
	return tr("Security Settings") + "  -  " + tr("BETA");
}

QString QtSecuritySettings::getIconName() const {
	return "privacy";
}

void QtSecuritySettings::readConfig() {
	
}

void QtSecuritySettings::saveConfig() {
	
}
