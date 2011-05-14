/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
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

#ifndef OWQTFLAGSMANAGER_H
#define OWQTFLAGSMANAGER_H

#include "QtFlag.h"
#include <QtCore/QObject>
#include <QtCore/QHash>

class QDomNode;
class QString;

/**
 *
 * @ingroup presentation
 * @author Mathieu Stute
 */
class QtFlagsManager: public QObject {
Q_OBJECT
public:

	typedef QList<QtFlag> QtFlagList;

	static QtFlagsManager * getInstance();

	QtFlagsManager();

	QtFlag getFlagByCountryName(const QString & countryName, const QString & language);//added by Rolando 04-11-08
	
	QtFlag getFlagByCountryCode(const QString & countryCode, const QString & language);//added by Rolando 04-11-08
	
	QtFlag getFlagByFlagNameFile(const QString & flagName, const QString & language);//added by Rolando 04-11-08

	void loadFromFile(const QString & filename);

	QString textCountryName2Flag(const QString & text, const QString & language);

	QString flags2TextCountryName(const QString & text, const QString & language);
	
	QString textCountryCode2Flag(const QString & text, const QString & language);

	QString flags2TextCountryCode(const QString & text, const QString & language);

	QtFlagList getQtFlagList(const QString & language);

	QString getAreaCodeInText(QString text, const QString & language);

	int getLanguageCount();
	
	int getLargestCountryNameSize();//added by Rolando 04-11-08
	QString getLargestCountryNameText();//added by Rolando 04-11-08
	int getFlagCount(const QString & language);
		
	void setFlagNameFileDefault(QString countryName, QString language);//added by Rolando 04-11-08

	int getPixelSizeText(QString text, QFont font);//added by Rolando 04-11-08
	
private:

	void readLanguage(const QDomNode & node);

	void readFlag(const QDomNode & node, const QString & language);

	QString encode(const QString & text) const;

	typedef QHash<QString, QtFlagList> LanguageList;

	LanguageList _languageList;

	static QtFlagsManager * _instance;
	int _largestCountryNameSize;//added by Rolando 04-11-08
	QString _largestCountryNameText;//added by Rolando 04-11-08
};

#endif	//OWQTFLAGSMANAGER_H
