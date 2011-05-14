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
#include "QtTextSettings.h"

#include "ui_TextSettings.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/config/EnumToolBarMode.h>
#include <control/CWengoPhone.h>

#include <control/profile/CUserProfileHandler.h>
#include <control/profile/CUserProfile.h>
#include <model/profile/UserProfile.h>

#include <presentation/qt/QtWengoPhone.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/WidgetUtils.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtCore/QTime>
#include <QtCore/QDate>

#include <QtWebKit/QWebFrame>

QtTextSettings::QtTextSettings(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone){

	_ui = new Ui::TextSettings();
	_ui->setupUi(this);

	//_jsAction = "";

	SAFE_CONNECT(_ui->webView, SIGNAL(loadFinished(bool)), SLOT(documentCompleteSlot(bool)));


	readConfig();
}

QtTextSettings::~QtTextSettings() {
	OWSAFE_DELETE(_ui);
}



QString QtTextSettings::getName() const {
	return tr("Texting");
}

QString QtTextSettings::getTitle() const {
	return tr("Texting");
}

QString QtTextSettings::getDescription() const {
	return tr("");
}

QString QtTextSettings::getIconName() const {
	return "text";
}

void QtTextSettings::saveConfig() {

	QString script = QString("save()");

	_ui->webView->page()->mainFrame()->evaluateJavaScript(script);
		
}

void QtTextSettings::documentCompleteSlot(bool ok) {

	
	
	//Provide login
	/*QString javascript = QString("login('%1','%2')").arg(accountId).arg(md5);
	_ui->webView->page()->mainFrame()->evaluateJavaScript(javascript);*/

	//Perform any provided JavaScript
	
	//if ( !_jsAction.isEmpty() )
	//{
		//_ui->webView->page()->mainFrame()->evaluateJavaScript( _jsAction );
	//}
}

void QtTextSettings::readConfig() {

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QString accountId = QString::fromStdString(_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getVoxOxAccount()->getDisplayAccountId());
	QString md5 = QString::fromStdString(_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getMd5());

	QString url = QString("%1?username=%2&userkey=%3").arg(QString::fromStdString(config.getConfigTextSettingsUrl())).arg(accountId).arg(md5);
	_ui->webView->setUrl(url);

	//std::string text = config.getToolBarMode();
	//EnumToolBarMode::ToolBarMode toolBarMode = EnumToolBarMode::toToolBarMode(text);
	//QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();

	//// Toolbar
	///*if (toolBarMode == EnumToolBarMode::ToolBarModeHidden) {
	//	_ui->hiddenRadioButton->setChecked(true);
	//} else if (toolBarMode == EnumToolBarMode::ToolBarModeIconsOnly) {
	//	_ui->iconsOnlyRadioButton->setChecked(true);
	//} else {
	//	_ui->textUnderIconsRadioButton->setChecked(true);
	//}*/

	//QString currentContactListStyleTheme = QString::fromUtf8(config.getContactListStyle().c_str());
	//_contactListStyleList = contactListStyle->getStyleList();
	//Q_FOREACH(QString styleDir, _contactListStyleList) {
	//	

	//	QString name = styleDir;
	//	// Prettify the name a bit
	//	name[0] = name[0].toUpper();
	//	

	//	_ui->skinComboBox->addItem(name);
	//	if (styleDir == currentContactListStyleTheme) {
	//		_ui->skinComboBox->setCurrentIndex(_ui->skinComboBox->findText(name,Qt::MatchExactly));
	//	}
	//}


	//// Theme
	//QString currentTheme = QString::fromUtf8(config.getChatTheme().c_str());
	//_themeList = QtChatTheme::getThemeList();
	//Q_FOREACH(QString themeDir, _themeList) {
	//	QListWidgetItem* item = new QListWidgetItem(_ui->themeListWidget);

	//	QString name = themeDir;
	//	// Prettify the name a bit
	//	name[0] = name[0].toUpper();
	//	item->setText(name);

	//	_ui->themeListWidget->addItem(item);
	//	if (themeDir == currentTheme) {
	//		_ui->themeListWidget->setCurrentItem(item);
	//	}
	//}

	//int width = WidgetUtils::computeListViewMinimumWidth(_ui->themeListWidget);
	//if (width < THEMELIST_MINIMUM_WIDTH) {
	//	width = THEMELIST_MINIMUM_WIDTH;
	//}
	//_ui->themeListWidget->setFixedWidth(width);
}
