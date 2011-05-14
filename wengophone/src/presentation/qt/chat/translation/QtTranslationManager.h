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
* Translation
* @author Chris Jimenez C 
* @date 2010.01.16
*/


#ifndef OWQTTRANSLATIONMANAGER_H
#define OWQTTRANSLATIONMANAGER_H

#include "QtTranslationLanguage.h"

#include <QtCore/QHash>

class QDomNode;
class QString;

/**
 *
 * @ingroup presentation
 * @author Mathieu Stute
 */
class QtTranslationManager {
public:

	typedef QList<QtTranslationLanguage> QtTranslationLanguageList;

	static QtTranslationManager * getInstance(const QString & xml = QString());

	QtTranslationManager(const QString & xml = QString());

	QtTranslationLanguage getTranslationLanguage(const QString & text);

	void loadTranslationLanguage(const QString & xml);

	void loadFromFile(const QString & filename);

	void loadFromXML(const QString & xml);

	QtTranslationLanguageList getQtTranslationLanguageList();

private:

	void readTranslation(const QDomNode & node);

	void readData(const QDomNode & node);

	QtTranslationLanguageList _translationLanguageList;

	static QtTranslationManager * _instance;
};

#endif	//OWQtTranslationManager_H
