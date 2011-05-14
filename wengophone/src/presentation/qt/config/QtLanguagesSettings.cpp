/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007 Wengo
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
#include "QtLanguagesSettings.h"

#include "ui_LanguagesSettings.h"

#include <presentation/qt/QtLanguage.h>

#include <model/config/LanguageList.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/webservices/url/WsUrl.h>

#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>

QtLanguagesSettings::QtLanguagesSettings(QWidget * parent)
	: QWidget(parent) {

	_ui = new Ui::LanguagesSettings();
	_ui->setupUi(this);

	SAFE_CONNECT(_ui->launchpadLink,
		SIGNAL(linkActivated(const QString &)),
		SLOT(launchpadLinkClicked(const QString &))
	);

	_ui->listWidget->addItems(QtLanguage::getAvailableLanguages());

	readConfig();
}

QtLanguagesSettings::~QtLanguagesSettings() {
	OWSAFE_DELETE(_ui);
}

QString QtLanguagesSettings::getName() const {
	return tr("Language");
}

QString QtLanguagesSettings::getTitle() const {
	return tr("Language Settings");
}

QString QtLanguagesSettings::getDescription() const {
	return tr("Language Settings");
}

QString QtLanguagesSettings::getIconName() const {
	return "languages";
}

void QtLanguagesSettings::saveConfig() {
	QListWidgetItem * item = _ui->listWidget->currentItem();

	std::string language;
	if (item) {
		language = item->text().toUtf8().constData();
	}

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	config.set(Config::LANGUAGE_KEY, LanguageList::getISO639Code(language));
}

void QtLanguagesSettings::readConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string language = LanguageList::getLanguageName(config.getLanguage());

	QList<QListWidgetItem *> list = _ui->listWidget->findItems(QString::fromUtf8(language.c_str()), Qt::MatchExactly);
	if (!list.isEmpty()) {
		_ui->listWidget->setCurrentItem(list[0]);
	}
}

void QtLanguagesSettings::launchpadLinkClicked(const QString & /*link*/) {
	WsUrl::showLauchpadPage();
}
