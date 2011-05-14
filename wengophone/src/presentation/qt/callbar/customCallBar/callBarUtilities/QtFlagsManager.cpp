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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.05
#include "QtFlagsManager.h"

#include <util/Logger.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <QtGui/QtGui>
#include <QtXml/QtXml>

const int FLAGS_WIDTH = 16;
const int FLAGS_HEIGHT = 11;

QtFlagsManager * QtFlagsManager::_instance = NULL;

QtFlagsManager::QtFlagsManager() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	_largestCountryNameSize = 0;
	loadFromFile(QString(":/flags/flagsdef.xml"));
}

QtFlagsManager * QtFlagsManager::getInstance() {
	if (!_instance) {
		_instance = new QtFlagsManager();
	}
	return _instance;
}

int QtFlagsManager::getLanguageCount() {
	return _languageList.size();
}

int QtFlagsManager::getFlagCount(const QString & language) {
	if (_languageList.find(language) == _languageList.end()) {
		return 0;
	}
	return _languageList[language].size();
}


QtFlag QtFlagsManager::getFlagByCountryName(const QString & countryName, const QString & language) {
	QtFlag flag(FLAGS_WIDTH,FLAGS_HEIGHT);

	if (_languageList.find(language) == _languageList.end()) {
		return flag;
	}
	QtFlagList flagList = _languageList[language];

	for (QtFlagList::iterator it = flagList.begin(); it != flagList.end(); it++) {
		QStringList stringList = (*it).getCountryName();

		for (QStringList::iterator it2 = stringList.begin(); it2 != stringList.end(); it2++) {
			if ((*it2).toUpper() == countryName.toUpper()) {
				return (*it);
			}
		}
	}
	return flag;
}


//WARNING: not use this function because there are countries like USA and Canada that use the same code area, cannot return the correct flag 
QtFlag QtFlagsManager::getFlagByCountryCode(const QString & countryCode, const QString & language) {
	QtFlag flag(FLAGS_WIDTH,FLAGS_HEIGHT);

	if (_languageList.find(language) == _languageList.end()) {
		return flag;
	}
	QtFlagList flagList = _languageList[language];

	for (QtFlagList::iterator it = flagList.begin(); it != flagList.end(); it++) {
		QString stringCode = (*it).getCountryCode();		
		if (stringCode == countryCode) {
			return (*it);
		}
		
	}
	return flag;
}


QtFlag QtFlagsManager::getFlagByFlagNameFile(const QString & flagName, const QString & language){
	QtFlag flag(FLAGS_WIDTH,FLAGS_HEIGHT);

	if (_languageList.find(language) == _languageList.end()) {
		return flag;
	}
	QtFlagList flagList = _languageList[language];

	for (QtFlagList::iterator it = flagList.begin(); it != flagList.end(); it++) {
		QString stringFlagNameFile = (*it).getFlagNameFile();		
		if (stringFlagNameFile == flagName) {
			return (*it);
		}
		
	}
	return flag;
}

QtFlagsManager::QtFlagList QtFlagsManager::getQtFlagList(const QString & language) {
	QtFlagList flagList;
	if (_languageList.find(language) == _languageList.end()) {
		return flagList;
	}
	flagList = _languageList[language];
	return flagList;
}

void QtFlagsManager::loadFromFile(const QString & filename) {
	QFile file(filename);
	QString ErrorMsg;
	int ErrorLine = 0;
	int ErrorCol = 0;
	QDomDocument doc("voxoxFlags");

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
			if (tagName == "language") {
								
				readLanguage(n);
			}
		}
		n = n.nextSibling();
	}
}

void QtFlagsManager::readLanguage(const QDomNode & node) {
	QDomNode n = node.firstChild();
	QDomElement element;
	QString attributeName;
	while (!n.isNull()) {
		if (!n.toElement().isNull()) {
			if (n.toElement().tagName() == "flag") {
						
				element = node.toElement();
				if (!element.isNull()) {
					attributeName = element.attribute("name");					
					
					readFlag(n, attributeName);
					readFlag(n, "all");
				}
			}
		}
		n = n.nextSibling();
	}
}

void QtFlagsManager::readFlag(const QDomNode & node, const QString & language) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QDomNode n1 = node.firstChild();
	QStringList textListCountryName;
	QString textCountryCode;
	QtFlag flag(FLAGS_WIDTH,FLAGS_HEIGHT);
	while (!n1.isNull()) {
		QDomElement e1 = n1.toElement();
		if (!e1.isNull()) {
			if (e1.tagName() == "countryname") {
				int textSize = e1.text().size();
				if(textSize > _largestCountryNameSize){
					_largestCountryNameSize = textSize;
					_largestCountryNameText = e1.text();
					_largestCountryNameText = _largestCountryNameText.trimmed();
				}
				QString countryName = e1.text();
				countryName = countryName.trimmed();
				textListCountryName << countryName;

			}
			if (e1.tagName() == "countrycode") {
				
			
				textCountryCode = e1.text();
				textCountryCode = textCountryCode.trimmed();
			}
			if (e1.tagName() == "flagname") {				
				QString flagName = e1.text();
				flagName = flagName.trimmed();
				QString flagPath = QString(":/flags/") + flagName;
				flag.setPath(flagPath);				
				
				flag.setFlagName(flagName);
				
				QPixmap flagPix = QPixmap(flagPath);
				flag.setPixmap(flagPix);
				flag.setButtonPixmap(flagPix);
			}
		}
		n1 = n1.nextSibling();
	}
	flag.setCountryName(textListCountryName);
	flag.setCountryCode(textCountryCode);
	_languageList[language] << flag;
}

QString QtFlagsManager::textCountryName2Flag(const QString & text, const QString & language) {
	QString ret = text;

	if (_languageList.find(language) == _languageList.end()) {
		return text;
	}

	QtFlagList flagList = _languageList[language];
	for (QtFlagList::iterator it = flagList.begin(); it != flagList.end(); it++) {
		QStringList stringList = (*it).getCountryName();
		for (QStringList::iterator it2 = stringList.begin(); it2 != stringList.end(); it2++) {
			ret.replace((*it2), (*it).getHtml(), Qt::CaseInsensitive);
		}
	}
	return ret;
}

QString QtFlagsManager::textCountryCode2Flag(const QString & text, const QString & language) {
	QString ret = text;

	if (_languageList.find(language) == _languageList.end()) {
		return text;
	}

	QtFlagList flagList = _languageList[language];
	for (QtFlagList::iterator it = flagList.begin(); it != flagList.end(); it++) {
		QString stringCode = (*it).getCountryCode();		
		ret.replace(stringCode, (*it).getHtml(), Qt::CaseInsensitive);		
	}
	return ret;
}

QString QtFlagsManager::flags2TextCountryName(const QString & text, const QString & language) {
	QString ret = text;

	if (_languageList.find(language) == _languageList.end()) {
		return text;
	}
	QtFlagList flagList = _languageList[language];

	for (QtFlagList::iterator it = flagList.begin(); it != flagList.end(); it++) {
		QStringList stringList = (*it).getCountryName();
		for (QStringList::iterator it2 = stringList.begin(); it2 != stringList.end(); it2++) {
			ret.replace((*it).getHtml(), encode((*it2)), Qt::CaseInsensitive);
		}
	}
	return ret;
}

QString QtFlagsManager::flags2TextCountryCode(const QString & text, const QString & language) {
	QString ret = text;

	if (_languageList.find(language) == _languageList.end()) {
		return text;
	}
	QtFlagList flagList = _languageList[language];

	for (QtFlagList::iterator it = flagList.begin(); it != flagList.end(); it++) {
		QString stringCode = (*it).getCountryCode();
		ret.replace((*it).getHtml(), encode(stringCode), Qt::CaseInsensitive);
		
	}
	return ret;
}

QString QtFlagsManager::encode(const QString & text) const {
	QByteArray ba;

	for (int i = 0; i < text.size(); i++) {
		switch(text[i].toAscii()) {
			case '<':
				ba += "&lt;";
				break;
			case '>':
				ba += "&gt;";
				break;
			case '&':
				ba += "&amp;";
				break;
			case '\'':
				ba += "&apos;";
				break;
			case '"':
				ba += "&apos;";
				break;
			default:
				ba += text[i];
		}
	}
	return QString(ba);
}

/* 
 * setFlagNameFileDefault: sets in config as default the flagNameFile according parameter countryName received
 * parameters:  QString countryName, QString language
 * returns void
*/
void QtFlagsManager::setFlagNameFileDefault(QString countryName, QString language){

	Config & config = ConfigManager::getInstance().getCurrentConfig();	
	QtFlag qtFlag  = getFlagByCountryName(countryName, language);
	QString flagName = qtFlag.getFlagNameFile();

	QString defaultFlagNameFile = QString::fromStdString(config.getCurrentFlag());

	if(defaultFlagNameFile != flagName){/*if the flagNameFile stored in config is not equal to the 
		                                obtained according parameter countryName*/

		if(!countryName.isEmpty() && countryName.toLower() != QString("none")){			
			config.set(Config::CURRENT_FLAG_KEY, flagName.toStdString());
			config.set(Config::CURRENT_COUNTRY_DEFAULT_KEY, qtFlag.getDefaultCountryName().toStdString());
		}
		else{			
			QString countryNameDefault = QString::fromStdString(config.getCurrentCountryDefault());
			qtFlag  = getFlagByCountryName(countryNameDefault, language);
			defaultFlagNameFile = qtFlag.getFlagNameFile();
			config.set(Config::CURRENT_FLAG_KEY, defaultFlagNameFile.toStdString());			
		}
	}	
	
}


int QtFlagsManager::getLargestCountryNameSize(){
	return _largestCountryNameSize;
}

QString QtFlagsManager::getLargestCountryNameText(){
	return _largestCountryNameText;
}

int QtFlagsManager::getPixelSizeText(QString text, QFont font){
	return QFontMetrics(font).width(text);

}


/* 
 * getAreaCodeInText: sets in config as default the flagNameFile according parameter countryName received
 * parameters:  QString countryName, QString language
 * returns QString, areaCode contained in text or QString::null if is not contained an area code in text
*/
QString QtFlagsManager::getAreaCodeInText(QString text, const QString & language){
	int maxLength = 0;
	int matchedLength = 0;
	QString areaCode = QString("");

	if( text.indexOf(QRegExp("^\\+[0-9]+")) != -1){//if it is a phone number not a contact name
		QtFlagsManager::QtFlagList flagList = getQtFlagList(language);
		QtFlagsManager::QtFlagList::iterator it;
		for (it = flagList.begin(); it != flagList.end(); it++) {
			QString countryCode = it->getCountryCode();//gets the country code
			if( countryCode.startsWith ( QChar('+')) ){
								
				QRegExp codeAreaRegExp1(QString("^\\") + countryCode );//because char "+" is a reserved word in regular expressions we have to add "\\" before it
				if(codeAreaRegExp1.indexIn(text) != -1){
					matchedLength = codeAreaRegExp1.matchedLength();

					/*because we need to check all the area codes in QtFlagsManager, 
					could happen we get a largest matched text according parameter text every cycle in for, 
					if that happens we need to get that areaCode*/
					if(matchedLength > maxLength ){
						maxLength = matchedLength; 
						areaCode = codeAreaRegExp1.cap();
					}
					
				}
				
			}

			
		}
	
	}
	else{
		if( text.indexOf(QRegExp("^011[0-9]+")) != -1){//if it is a phone number not a contact name
			QtFlagsManager::QtFlagList flagList = getQtFlagList(language);
			QtFlagsManager::QtFlagList::iterator it;
			for (it = flagList.begin(); it != flagList.end(); it++) {
				QString countryCode = it->getCountryCode();//gets the country code
				if( countryCode.startsWith ( QChar('+')) ){
					text.replace(0,3,QString("+"));				
					QRegExp codeAreaRegExp1(QString("^\\") + countryCode );//because char "+" is a reserved word in regular expressions and countryCode contains "+" we have to add "\\" before it
					if(codeAreaRegExp1.indexIn(text) != -1){
						matchedLength = codeAreaRegExp1.matchedLength();
						/*because we need to check all the area codes in QtFlagsManager, 
						could happen we get a largest matched text according parameter text every cycle in for, 
						if that happens we need to get that areaCode*/
						if(matchedLength > maxLength ){
							maxLength = matchedLength; 
							areaCode = codeAreaRegExp1.cap();
						}
						
					}
					
				}

				
			}
		}
		else{
			//VOXOX CHANGE by Rolando - 2009.06.28 
			if( text.indexOf(QRegExp("^1[0-9]+")) != -1){//if it is a phone number from usa
				areaCode = "1";
			}
		}
	
	}


	if(maxLength > 0){
		return areaCode;
	}
	else{
		return QString::null;
	}

}
