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
#include "QtLanguage.h"

#include <model/config/LanguageList.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <util/File.h>
#include <util/Logger.h>

#include <settings/Settings.h>
#include <settings/SettingsXMLSerializer.h>

#include <qtutil/KeywordTranslator.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>

static const char * LANGUAGE_FILE_EXTENSION = "qm";
static const char * LANG_DIR = "lang";

static const char * BRANDING_FILENAME = "config/branding.xml";

static KeywordTranslator::KeywordHash readKeywordHash(const std::string& fileName) {
	FileReader reader(fileName);
	KeywordTranslator::KeywordHash hash;
	if (!reader.open()) {
		LOG_ERROR("Couldn't open file " + fileName);
		return hash;
	}

	std::string data = reader.read();

	Settings settings;
	SettingsXMLSerializer serializer(settings);
	serializer.unserialize(data);

	StringList list = settings.getAllKeys();
	StringList::const_iterator
		it = list.begin(),
		end = list.end();
	for (; it!=end; ++it) {
		std::string keyword = *it;
		std::string value = settings.getStringKeyValue(keyword);
		hash[QString::fromStdString(keyword)] = QString::fromUtf8(value.c_str());
	}

	return hash;
}

QtLanguage::QtLanguage(QObject * parent)
	: QObjectThreadSafe(parent) {

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	config.valueChangedEvent += boost::bind(&QtLanguage::configChangedEventHandler, this, _1);

	_qtTranslator = new QTranslator(QCoreApplication::instance());

	//VOXOX CHANGE for VoxOx by Rolando 12-31-08: deleted to eliminate reading xml 
	//KeywordTranslator::KeywordHash hash = readKeywordHash(config.getResourcesDir() + "/" + BRANDING_FILENAME);

	_appTranslator = new KeywordTranslator(QCoreApplication::instance());

	//VOXOX CHANGE for VoxOx by Rolando 12-31-08, added to avoid reading xml
	KeywordTranslator::KeywordHash hash;

	//VOXOX CHANGE for VoxOx by Rolando 12-31-08, added to avoid reading xml
	hash[QString("company")] = QString("VoxOx");

	//VOXOX CHANGE for VoxOx by Rolando 12-31-08, added to avoid reading xml 
	hash[QString("product")] = QString("VoxOx");		

	_appTranslator->setKeywordHash(hash);
	_appTranslator->setFallbackTranslator(_qtTranslator);
	QCoreApplication::installTranslator(_appTranslator);

	loadLanguageFromConfig();
}

QtLanguage::~QtLanguage() {
}

void QtLanguage::configChangedEventHandler(const std::string & key) {
	typedef PostEvent1<void (const std::string &), std::string> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtLanguage::configChangedEventHandlerThreadSafe, this, _1), key);
	postEvent(event);
}

void QtLanguage::configChangedEventHandlerThreadSafe(const std::string & key) {
	if (key == Config::LANGUAGE_KEY) {
		loadLanguageFromConfig();
	}
}

QString QtLanguage::getLocaleFileName() {
	QString localeName;
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if (config.getLanguage() == Config::LANGUAGE_AUTODETECT_KEYVALUE) {
		localeName = QLocale::system().name();
		if (localeName == "C") {
			// No locale defined, just get out
			return QString::null;
		}
	} else {
		localeName = QString::fromStdString(config.getLanguage());
	}
	return localeName;
}

QString QtLanguage::getQtLocaleFileName() {
	QString localeName = QtLanguage::getLocaleFileName();
	if (!localeName.isEmpty()) {
		return "qt_" + localeName;
	} else {
		return QString::null;
	}
}

QString QtLanguage::getWengoPhoneLocaleFileName() {
	QString localeName = QtLanguage::getLocaleFileName();
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString langDir = QString::fromStdString(config.getResourcesDir()) + LANG_DIR;
	if (!localeName.isEmpty()) {
		return langDir + QDir::separator() + "qtwengophone_" + localeName;
	} else {
		return QString::null;
	}
}

void QtLanguage::loadLanguageFromConfig() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString qtTranslationName = getQtLocaleFileName();

	QString langDir = QString::fromStdString(config.getResourcesDir()) + LANG_DIR;
	bool qtTranslationLoaded = _qtTranslator->load(qtTranslationName, langDir);

#ifdef QT_TRANSLATION_DIR
	if (!qtTranslationLoaded) {
		qtTranslationLoaded = _qtTranslator->load(qtTranslationName, QT_TRANSLATION_DIR);
	}
#endif
	if (!qtTranslationLoaded) {
		LOG_INFO("no Qt translation available for locale '" + getLocaleFileName().toStdString() + "'");
	}

	LOG_DEBUG(getWengoPhoneLocaleFileName().toStdString());
	if (!_appTranslator->load(getWengoPhoneLocaleFileName())) {
		LOG_INFO("no application translation available for locale '" + getLocaleFileName().toStdString() + "'");
	}
}

static QString getIso639CodeFromFileName(const QString & fileName) {
	// qtwengophone_en.qm -> en
	QRegExp rx("qtwengophone_([^.]+)");
	rx.indexIn(fileName);
	QStringList list = rx.capturedTexts();
	if (list.size() != 2) {
		return QString();
    }
    QString code = list[1];
    code.replace("_", "-");
    return code.toLower();
}

QStringList QtLanguage::getAvailableLanguages() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QStringList languageList;
	QDir dir(QString::fromStdString(config.getResourcesDir()) + LANG_DIR);

	Q_FOREACH(QFileInfo fileInfo, dir.entryInfoList()) {

		if (fileInfo.isFile() &&
			fileInfo.size() &&
			(fileInfo.completeSuffix().toStdString() == LANGUAGE_FILE_EXTENSION)) {

			QString iso639Code = getIso639CodeFromFileName(fileInfo.fileName());
			if (iso639Code.isEmpty()) {
				// This can happen if the translation dir contains other files
				// (like Qt translations)
				continue;
			}
			std::string language = LanguageList::getLanguageName(iso639Code.toStdString());
			languageList += QString::fromUtf8(language.c_str());
		}
	}

	return languageList;
}
