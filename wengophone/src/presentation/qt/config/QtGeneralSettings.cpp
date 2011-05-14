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
#include "QtGeneralSettings.h"

#include "ui_GeneralSettings.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/profile/SoftUpdatePrefs.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfileHandler.h>

#include <cutil/global.h>

#include <QtGui/QtGui>

QtGeneralSettings::QtGeneralSettings(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
		_cWengoPhone( cWengoPhone )
{

	_ui = new Ui::GeneralSettings();
	_ui->setupUi(this);

#ifdef OS_LINUX
	_ui->autoStartGroupBox->hide();
#endif

	readConfig();
}

QtGeneralSettings::~QtGeneralSettings() {
	delete _ui;
}

QString QtGeneralSettings::getName() const {
	return tr("General");
}

QString QtGeneralSettings::getTitle() const {
	return tr("General Settings");
}

QString QtGeneralSettings::getDescription() const {
	return tr("Manage your basic VoxOx settings");
}

QString QtGeneralSettings::getIconName() const {
	return "general";
}

void QtGeneralSettings::readConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	SoftUpdatePrefs& suPrefs = getSoftUpdatePrefs();	//VOXOX - JRT - 2009.12.10 

	_ui->startWengoPhoneCheckBox->setChecked(config.getGeneralAutoStart());
//	_ui->startShowRecommendedUpdatesCheckBox->setChecked(config.getSoftUpdateShowRecommended());//VOXOX -ASV- 2009.12.04
	_ui->startShowRecommendedUpdatesCheckBox->setChecked( suPrefs.autoCheck() );	//VOXOX - JRT - 2009.12.10 
	_ui->startFreeCallRadioButton->setChecked(config.getGeneralClickStartFreeCall());
	_ui->callCellPhoneCheckBox->setChecked(config.getGeneralClickCallCellPhone());
	_ui->startChatOnlyRadioButton->setChecked(config.getGeneralClickStartChat());

	int awayTimer = config.getGeneralAwayTimer();
	if (awayTimer != Config::NO_AWAY_TIMER) {
		_ui->showAwayCheckBox->setChecked(true);
		_ui->awaySpinBox->setValue(awayTimer);
	} else {
		_ui->showAwayCheckBox->setChecked(false);
	}
}

void QtGeneralSettings::saveConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	SoftUpdatePrefs& suPrefs = getSoftUpdatePrefs();		//VOXOX - JRT - 2009.12.10 

	config.set(Config::GENERAL_AUTOSTART_KEY, _ui->startWengoPhoneCheckBox->isChecked());
	config.set(Config::GENERAL_CLICK_START_FREECALL_KEY, _ui->startFreeCallRadioButton->isChecked());
	config.set(Config::GENERAL_CLICK_CALL_CELLPHONE_KEY, _ui->callCellPhoneCheckBox->isChecked());
	config.set(Config::GENERAL_CLICK_START_CHAT_KEY, _ui->startChatOnlyRadioButton->isChecked());

//	config.set(Config::SOFTUPDATE_SHOWRECOMMENDED, _ui->startShowRecommendedUpdatesCheckBox->isChecked()); //VOXOX -ASV- 2009.12.04
	suPrefs.setAutoCheck( _ui->startShowRecommendedUpdatesCheckBox->isChecked() );		//VOXOX - JRT - 2009.12.10 
	
	int awayTimer = _ui->awaySpinBox->value();
	if (_ui->showAwayCheckBox->isChecked()) {
		config.set(Config::GENERAL_AWAY_TIMER_KEY, awayTimer);
	} else {
		config.set(Config::GENERAL_AWAY_TIMER_KEY, Config::NO_AWAY_TIMER);
	}

	saveSoftUpdatePrefs();
}

//-----------------------------------------------------------------------------

SoftUpdatePrefs& QtGeneralSettings::getSoftUpdatePrefs()
{
	return _cWengoPhone.getUserSoftUpdatePrefs();
}

//-----------------------------------------------------------------------------

void QtGeneralSettings::saveSoftUpdatePrefs()
{
	return _cWengoPhone.saveUserSoftUpdatePrefs();
}

//-----------------------------------------------------------------------------
