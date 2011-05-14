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
#include "QtEmoticonsManager.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

#include <QtGui/QtGui>
#include <QtXml/QtXml>

QtEmoticonsManager * QtEmoticonsManager::_instance = NULL;

QtEmoticonsManager::QtEmoticonsManager() {

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	loadEmoticons(QString::fromStdString(config.getEmoticonPack()));

}

QtEmoticonsManager * QtEmoticonsManager::getInstance() {
	if (!_instance) {
		_instance = new QtEmoticonsManager();
	}
	return _instance;
}

void QtEmoticonsManager::loadEmoticons(QString emoticonDir) {
	
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString resource = QString::fromStdString(config.getResourcesDir());
	resource = resource.replace("\\","/");
	loadFromFile(resource + QString("chat/emoticons/"+emoticonDir+"/Emoticons.xml"),emoticonDir);
}


int QtEmoticonsManager::getEmoticonCount() {
	
	return _emoticonList.size();
}

QtEmoticon QtEmoticonsManager::getEmoticon(const QString & text) {
	QtEmoticon emoticon;

	for (QtEmoticonList::iterator it = _emoticonList.begin(); it != _emoticonList.end(); it++) {
		QStringList stringList = (*it).getText();
		for (QStringList::iterator it2 = stringList.begin(); it2 != stringList.end(); it2++) {
			if ((*it2).toUpper() == text.toUpper()) {
				return (*it);
			}
		}
	}
	return emoticon;
}

QtEmoticonsManager::QtEmoticonList QtEmoticonsManager::getQtEmoticonList() {
	
	return _emoticonList;
}

void QtEmoticonsManager::loadFromFile(const QString & filename, QString & emoticonDir) {
	_emoticonList.clear();
	QFile file(filename);
	QString ErrorMsg;
	int ErrorLine = 0;
	int ErrorCol = 0;
	QDomDocument doc("emoticons");

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
			if (tagName == "emoticons") {
				readEmoticons(n,emoticonDir);
			}
		}
		n = n.nextSibling();
	}
}

void QtEmoticonsManager::readEmoticons(const QDomNode & node,QString & emoticonDir) {
	QDomNode n = node.firstChild();
	QDomElement element;
	QString attributeName;
	while (!n.isNull()) {
		if (!n.toElement().isNull()) {
			if (n.toElement().tagName() == "emoticon") {
				element = node.toElement();
				if (!element.isNull()) {
					readIcon(n,emoticonDir);
				}
			}
		}
		n = n.nextSibling();
	}
}

void QtEmoticonsManager::readIcon(const QDomNode & node,QString & emoticonDir) {
	
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString resource = QString::fromStdString(config.getResourcesDir());
	resource = resource.replace("\\","/");
	QDomNode n1 = node.firstChild();
	QStringList textList;
	QtEmoticon emoticon;
	while (!n1.isNull()) {
		QDomElement e1 = n1.toElement();
		if (!e1.isNull()) {
			if (e1.tagName() == "text") {
				textList << e1.text();
			}
			if (e1.tagName() == "name") {
				emoticon.setName(e1.text());
			}
			if (e1.tagName() == "object") {
				QString emoticonPath = resource +QString("chat/emoticons/"+emoticonDir)+"/"+ e1.text();
				emoticon.setPath(emoticonPath);
				QPixmap emoticonPix = QPixmap(emoticonPath);
				emoticon.setPixmap(emoticonPix);
				emoticon.setButtonPixmap(emoticonPix);
			}
		}
		n1 = n1.nextSibling();
	}
	emoticon.setText(textList);
	_emoticonList << emoticon;
}

QString QtEmoticonsManager::text2Emoticon(const QString & text) {

	QString result;

	// Match full tags (<bla>) or text
	QRegExp rx("([^><]+|<[^>]+>)");
	int pos = 0;
	while (true) {
		pos = rx.indexIn(text, pos);
		if (pos == -1) {
			break;
		}

		QString token = rx.cap(0);

		QRegExp * regExp = new QRegExp("<a[\\s]+[^>]*href[\\s]?=.*[^<]+</a>", Qt::CaseInsensitive);
	
		if(regExp->indexIn(text) == -1){
		if (token[0] != '<') {
			// Not a tag, replace smileys in it
			for (QtEmoticonList::iterator it = _emoticonList.begin(); it != _emoticonList.end(); it++) {
				QStringList stringList = it->getText();
				for (QStringList::iterator it2 = stringList.begin(); it2 != stringList.end(); it2++) {
					token.replace(*it2, it->getHtml(), Qt::CaseInsensitive);
					}
				}
			}
		}

		result += token;
		pos += rx.matchedLength();
	}

	return result;
}

QString QtEmoticonsManager::emoticons2Text(const QString & text) {
	QString ret = text;

	for (QtEmoticonList::iterator it = _emoticonList.begin(); it != _emoticonList.end(); it++) {
		QStringList stringList = (*it).getText();
		for (QStringList::iterator it2 = stringList.begin(); it2 != stringList.end(); it2++) {
			ret.replace((*it).getHtml(), encode((*it2)), Qt::CaseInsensitive);
		}
	}
	return ret;
}

QStringList QtEmoticonsManager::getEmoticonPackList() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString resource = QString::fromStdString(config.getResourcesDir());
	resource = resource.replace("\\","/");
	QDir dir(resource + QString::fromStdString("chat/emoticons/"));

	QStringList list;
	Q_FOREACH(QFileInfo info, dir.entryInfoList()) {
		if (info.isDir()) {
			QString name = info.fileName();
			if (name[0] != '.') {
				list << name;
			}
		}
	}

	return list;
}

QString QtEmoticonsManager::encode(const QString & text) const {
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
