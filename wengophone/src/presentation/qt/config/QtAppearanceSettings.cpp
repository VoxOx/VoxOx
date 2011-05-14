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
#include "QtAppearanceSettings.h"

#include "ui_AppearanceSettings.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/config/EnumToolBarMode.h>
#include <control/CWengoPhone.h>
#include <presentation/qt/QtWengoPhone.h>

#include <presentation/qt/contactlist/QtContactListStyle.h>
#include <presentation/qt/dialpad/QtDialpad.h>
#include <presentation/qt/QtWengoPhone.h>

#include <presentation/qt/chat/emoticons/QtEmoticonsManager.h>
#include <presentation/qt/chat/emoticons/QtEmoticon.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/WidgetUtils.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtCore/QTime>
#include <QtCore/QDate>
#include <presentation/qt/chat/QtChatTheme.h>
static const int THEMELIST_MINIMUM_WIDTH = 150;

QtAppearanceSettings::QtAppearanceSettings(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent),
	_cWengoPhone(cWengoPhone){

	_ui = new Ui::AppearanceSettings();
	_ui->setupUi(this);

	SAFE_CONNECT(_ui->cmbChatStyles, SIGNAL(activated(int)),
		SLOT(updateChatStylePreview()));

	SAFE_CONNECT(_ui->cmbChatStyleVariant, SIGNAL(activated (int)),
		SLOT(updateChatStyleVariant()));

	SAFE_CONNECT(_ui->cmbEmoticonList, SIGNAL(activated(int)),
		SLOT(updateEmoticonsPreview()));

	SAFE_CONNECT(_ui->skinComboBox, SIGNAL(activated(const QString &)),
		SLOT(updateContactListStyle(const QString &)));

	SAFE_CONNECT(_ui->changeKeypadBackground, SIGNAL(clicked()),
		SLOT(changeKeypadBackground()));

	_ui->groupBox->setVisible(false);

	readConfig();
	//updatePreview();
}

QtAppearanceSettings::~QtAppearanceSettings() {
	OWSAFE_DELETE(_ui);
}

void QtAppearanceSettings::updatePreview() {
	_ui->themePreviewBrowser->clear();

	QString theme = _ui->cmbChatStyles->currentText();
	QString variant = _ui->cmbChatStyleVariant->currentText();
	_ui->themePreviewBrowser->setTheme(theme,variant);
	_ui->themePreviewBrowser->init("other","Other","me",QTime(12, 34),QDate::currentDate());
	_ui->themePreviewBrowser->insertMessage("self", tr("Me"), tr("VoxOx"),tr("Hello :D"), QTime(12, 34),QDate::currentDate());
	_ui->themePreviewBrowser->insertMessage("self", tr("Me"), tr("VoxOx"), tr("How are you?"), QTime(12, 34),QDate::currentDate());
	_ui->themePreviewBrowser->insertMessage("other", tr("Other"), tr("VoxOx"), tr("I am fine"), QTime(12, 40),QDate::currentDate());
	_ui->themePreviewBrowser->insertMessage("other", tr("Other"), tr("VoxOx"), tr("Have to go"), QTime(12, 50),QDate::currentDate());
	_ui->themePreviewBrowser->insertMessage("other", tr("Other"), tr("VoxOx"), tr("Bye ;)"), QTime(12, 51),QDate::currentDate());
	_ui->themePreviewBrowser->insertStatusMessage("Other has left", QTime(12, 51));
}

void QtAppearanceSettings::updateChatStyleVariant() {
	
	QString variant = _ui->cmbChatStyleVariant->currentText();
	_ui->themePreviewBrowser->setMainStyle(variant);
	
}



void QtAppearanceSettings::updateContactListStyle(const QString & itemText) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QtWengoPhone *qtWengoPhone = dynamic_cast<QtWengoPhone *>(_cWengoPhone.getPresentation());

	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();

	QString selectedItemText = itemText.toLower();

	contactListStyle->loadStyleConfig(QString::fromStdString(config.getResourcesDir()) + contactListStyle->getContactStyleFolder() + selectedItemText);

	qtWengoPhone->updateStyle();

	config.set(Config::CONTACT_LIST_STYLE_KEY, selectedItemText.toStdString());
}

//VOXOX - SEMR - 2009.05.13 MEMORY ISSUE: Config/Settings Window
void QtAppearanceSettings::updateEmoticonsPreview(){
	updateEmoticons();
	updatePreview();

}

//VOXOX - SEMR - 2009.05.13 MEMORY ISSUE: Config/Settings Window
void QtAppearanceSettings::updateChatStylePreview(){
	updateChatStyle();
	updatePreview();

}
void QtAppearanceSettings::changeKeypadBackground(){//VOXOX - SEMR - 2009.07.31 change keypad 
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QString tmpString;
	QString filters = "Images ";

	//QString startDir = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);

	QString startDir = QString::fromStdString(config.getResourcesDir()) + "keypad/";

	QList<QByteArray> qByteArrayList = QImageReader::supportedImageFormats();

	for (int j = 0; j < qByteArrayList.size(); ++j) {
		tmpString = "*." + qByteArrayList.at(j);
		filters += tmpString + " ";
	}

	filters = filters.trimmed();

	QString filePath = QFileDialog::getOpenFileName(this, tr("Select your avatar"),startDir, filters);

	if(filePath == ""){
		return; 
	}

	config.set(Config::KEYPAD_BACKGROUND_PATH_KEY, filePath.toStdString());
	_ui->keypadBackgrounLabel->setText(QFileInfo(filePath).fileName());
	QtWengoPhone * qtWengoPhone = dynamic_cast<QtWengoPhone *>(_cWengoPhone.getPresentation());
	QtDialpad * keypad = qtWengoPhone->getQtDialpad();
	keypad->setKeypadStyleSheet(filePath);

}
void QtAppearanceSettings::updateEmoticons() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QString emoticonDir = _ui->cmbEmoticonList->currentText();

	QtEmoticonsManager * emoticonsManager = QtEmoticonsManager::getInstance();

	emoticonsManager->loadEmoticons(emoticonDir);

	QString richTextEmoticonPreview="";
	int count = 0;
	QtEmoticonsManager * qtEmoticonsManager = QtEmoticonsManager::getInstance();
	QtEmoticonsManager::QtEmoticonList emoticonList = qtEmoticonsManager->getQtEmoticonList();
	QtEmoticonsManager::QtEmoticonList::iterator it;
	for (it = emoticonList.begin(); it != emoticonList.end(); it++) {
		QtEmoticon & emoticon = (*it);
		richTextEmoticonPreview+=QString("<img src=\"%1\" />").arg(emoticon.getPath());
		count++;
		//We only one 20 emoticons to preview
		if(count>15){
			break;
		}
	}

	_ui->lblEmoticonPreview->setText(richTextEmoticonPreview);

}


QIcon QtAppearanceSettings::getEmoticonPreview(const QString & emoticonDir) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QtEmoticonsManager * emoticonsManager = QtEmoticonsManager::getInstance();

	emoticonsManager->loadEmoticons(emoticonDir);

	
	
	QtEmoticonsManager * qtEmoticonsManager = QtEmoticonsManager::getInstance();
	QtEmoticonsManager::QtEmoticonList emoticonList = qtEmoticonsManager->getQtEmoticonList();
	QtEmoticonsManager::QtEmoticonList::iterator it;
	for (it = emoticonList.begin(); it != emoticonList.end(); it++) {
		QtEmoticon & emoticon = (*it);
		return QIcon(emoticon.getPath());
		
	}
	return QIcon("");

}




QString QtAppearanceSettings::getName() const {
	return tr("Appearance");
}

void QtAppearanceSettings::updateChatStyle(){
	// Variant
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	_ui->cmbChatStyleVariant->clear();
	// Variant
	QString currentVariantTheme = QString::fromUtf8(config.getChatThemeVariant().c_str());
	_themeVariantList = _ui->themePreviewBrowser->getSelectedThemeVariantList(_ui->cmbChatStyles->currentText());
	Q_FOREACH(QString themeVariant, _themeVariantList) {

		QString name = themeVariant;
		// Prettify the name a bit
		name[0] = name[0].toUpper();
	
		_ui->cmbChatStyleVariant->addItem(name);
		if (name == currentVariantTheme) {	
			_ui->cmbChatStyleVariant->setCurrentIndex(_ui->cmbChatStyleVariant->findText(name,Qt::MatchExactly));
		}
	}
}


QString QtAppearanceSettings::getTitle() const {
	return tr("Appearance Settings");
}

QString QtAppearanceSettings::getDescription() const {
	return tr("Appearance Settings");
}

QString QtAppearanceSettings::getIconName() const {
	return "appearance";
}

void QtAppearanceSettings::saveConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	// Toolbar
	/*EnumToolBarMode::ToolBarMode mode;
	if (_ui->hiddenRadioButton->isChecked()) {
		mode = EnumToolBarMode::ToolBarModeHidden;
	} else if (_ui->iconsOnlyRadioButton->isChecked()) {
		mode = EnumToolBarMode::ToolBarModeIconsOnly;
	} else {
		mode = EnumToolBarMode::ToolBarModeTextUnderIcons;
	}
	std::string text = EnumToolBarMode::toString(mode);
	config.set(Config::GENERAL_TOOLBARMODE_KEY, text);*/

	/*QString selectedItemText = _ui->skinComboBox->currentText();

	config.set(Config::CONTACT_LIST_STYLE_KEY, selectedItemText.toStdString());*/

	QString theme = _ui->cmbChatStyles->currentText();
	
	config.set(Config::APPEARANCE_CHATTHEME_KEY, theme.toStdString());

	QString variant = _ui->cmbChatStyleVariant->currentText();
	
	config.set(Config::APPEARANCE_CHATTHEME_VARIANT_KEY, variant.toStdString());

	QString emoticon = _ui->cmbEmoticonList->currentText();
	
	config.set(Config::EMOTICON_CHAT_KEY, emoticon.toStdString());

}

void QtAppearanceSettings::readConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	//VOXOX - SEMR - 2009.07.31 keypad background change

	std::string keypadBackgroundPath = config.getKeypadBackgroundPath();
	if(QFileInfo(QString::fromStdString(keypadBackgroundPath)).exists()){
		_ui->keypadBackgrounLabel->setText(QFileInfo(QString::fromStdString(keypadBackgroundPath)).fileName()) ;
	}

	std::string text = config.getToolBarMode();
	EnumToolBarMode::ToolBarMode toolBarMode = EnumToolBarMode::toToolBarMode(text);
	QtContactListStyle * contactListStyle = QtContactListStyle::getInstance();

	// Toolbar
	/*if (toolBarMode == EnumToolBarMode::ToolBarModeHidden) {
		_ui->hiddenRadioButton->setChecked(true);
	} else if (toolBarMode == EnumToolBarMode::ToolBarModeIconsOnly) {
		_ui->iconsOnlyRadioButton->setChecked(true);
	} else {
		_ui->textUnderIconsRadioButton->setChecked(true);
	}*/


	_contactListStyleList = contactListStyle->getStyleList();
	QString currentContactListStyleTheme = QString::fromUtf8(config.getContactListStyle().c_str());
	Q_FOREACH(QString styleDir, _contactListStyleList) {
		

		QString name = styleDir;
		// Prettify the name a bit
		/*name[0] = name[0].toUpper();*/
		

		_ui->skinComboBox->addItem(name);
		if (name == currentContactListStyleTheme) {
			_ui->skinComboBox->setCurrentIndex(_ui->skinComboBox->findText(name,Qt::MatchExactly));
		}
	}
		


	// Theme
	QString currentTheme = QString::fromUtf8(config.getChatTheme().c_str());
	_themeList = _ui->themePreviewBrowser->getThemeList();
	Q_FOREACH(QString themeDir, _themeList) {

		QString name = themeDir;
		// Prettify the name a bit
		name[0] = name[0].toUpper();

		_ui->cmbChatStyles->addItem(name);
		if (name == currentTheme) {
			_ui->cmbChatStyles->setCurrentIndex(_ui->cmbChatStyles->findText(name,Qt::MatchExactly));
		}
	}

	// Emoticon
	QtEmoticonsManager * qtEmoticonsManager = QtEmoticonsManager::getInstance();
	QString currentEmoticon = QString::fromUtf8(config.getEmoticonPack().c_str());
	_emoticonList = qtEmoticonsManager->getEmoticonPackList();
	Q_FOREACH(QString emoticonDir, _emoticonList) {

		QString name = emoticonDir;
		// Prettify the name a bit
		name[0] = name[0].toUpper();

		_ui->cmbEmoticonList->addItem(getEmoticonPreview(emoticonDir),name);
		if (name == currentEmoticon) {
			_ui->cmbEmoticonList->setCurrentIndex(_ui->cmbEmoticonList->findText(name,Qt::MatchExactly));
		}
	}
	
	updateEmoticons();

	updateChatStyle();

	updatePreview();
	
}
