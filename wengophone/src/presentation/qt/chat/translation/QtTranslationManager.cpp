/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* CLASS DESCRIPTION 
* @author Chris Jimenez C 
* @date 2010.01.16
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtTranslationManager.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <QtGui/QtGui>
#include <QtXml/QtXml>

QtTranslationManager * QtTranslationManager::_instance = NULL;

QtTranslationManager::QtTranslationManager(const QString & xml) {

	loadTranslationLanguage(xml);


}

QtTranslationManager * QtTranslationManager::getInstance(const QString & xml) {
	if (!_instance) {
		_instance = new QtTranslationManager(xml);
	}
	return _instance;
}

void QtTranslationManager::loadTranslationLanguage(const QString & xml) {

	if(xml==""){//VOXOX - CJC - 2010.01.28 Load the file locally. 
	
		Config & config = ConfigManager::getInstance().getCurrentConfig();
		QString resource = QString::fromStdString(config.getResourcesDir());
		resource = resource.replace("\\","/");
		loadFromFile(resource + QString("chat/translation/translations.xml"));

	}else{
		loadFromXML(xml);
	}
}

QtTranslationLanguage QtTranslationManager::getTranslationLanguage(const QString & text) {
	QtTranslationLanguage language;

	for (QtTranslationLanguageList::iterator it = _translationLanguageList.begin(); it != _translationLanguageList.end(); it++) {
			if ((*it).getTranslationName().toUpper() == text.toUpper()) {
				return (*it);
			}
		
	}
	return language;
}

QtTranslationManager::QtTranslationLanguageList QtTranslationManager::getQtTranslationLanguageList() {
	
	return _translationLanguageList;
}

void QtTranslationManager::loadFromFile(const QString & filename) {
	_translationLanguageList.clear();
	QFile file(filename);
	QString ErrorMsg;
	int ErrorLine = 0;
	int ErrorCol = 0;
	QDomDocument doc("getLanguages");

	if (!file.open(QIODevice::ReadOnly)) {
		return;
	}

	if (!doc.setContent(&file, &ErrorMsg, &ErrorLine, &ErrorCol)) {
		file.close();
		return;
	}
	file.close();

	QDomElement tmpElement;
	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	QString tagName;

	while (!n.isNull()) {
		tmpElement = n.toElement();
		if (!tmpElement.isNull()) {
			tagName = tmpElement.tagName();
			if (tagName == "getLanguages") {
				readTranslation(n);
			}
		}
		n = n.nextSibling();
	}
}


void QtTranslationManager::loadFromXML(const QString & xml) {
	_translationLanguageList.clear();

	QString ErrorMsg;
	int ErrorLine = 0;
	int ErrorCol = 0;
	QDomDocument doc("getLanguages");


	if (!doc.setContent(xml, &ErrorMsg, &ErrorLine, &ErrorCol)) {
		return;
	}

	QDomElement tmpElement;
	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	QString tagName;

	while (!n.isNull()) {
		tmpElement = n.toElement();
		if (!tmpElement.isNull()) {
			tagName = tmpElement.tagName();
			if (tagName == "getLanguages") {
				readTranslation(n);
			}
		}
		n = n.nextSibling();
	}
}


void QtTranslationManager::readTranslation(const QDomNode & node) {
	QDomNode n = node.firstChild();
	QDomElement element;
	QString attributeName;
	while (!n.isNull()) {
		if (!n.toElement().isNull()) {
			if (n.toElement().tagName() == "language") {
				element = node.toElement();
				if (!element.isNull()) {
					readData(n);
				}
			}
		}
		n = n.nextSibling();
	}
}

void QtTranslationManager::readData(const QDomNode & node) {
	
	QDomNode n1 = node.firstChild();
	QStringList textList;
	QtTranslationLanguage language;
	while (!n1.isNull()) {
		QDomElement e1 = n1.toElement();
		if (!e1.isNull()) {
			if (e1.tagName() == "name") {
				language.setTranslationName(e1.text());
			}
			if (e1.tagName() == "abbv") {
				language.setTranslationAbbreviation(e1.text());
			}
			if (e1.tagName() == "sms") {
				if(e1.text()!=""){
					QString sms = e1.text();
					if(sms == "1"){
						language.setSupportSMS(true);
					}else{
						language.setSupportSMS(false);
					}
				}
			}
		}
		n1 = n1.nextSibling();
	}
	_translationLanguageList << language;
}


