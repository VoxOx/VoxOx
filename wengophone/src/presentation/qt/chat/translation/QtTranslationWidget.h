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


#ifndef OWQTTRANSLATIONWIDGET_H
#define OWQTTRANSLATIONWIDGET_H

#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <control/chat/CChatHandler.h>
#include "QtEnumTranslationMode.h"
class QtTranslationLanguage;
class QString;
class QStringList;

#include <QtGui/QtGui>

namespace Ui { class TranslationWidget; }


/**
 *
 * @ingroup presentation
 * @author Mr K.
 */
class QtTranslationWidget : public QDialog {
	Q_OBJECT
public:

	QtTranslationWidget(CChatHandler & cChatHandler, QWidget * parent);

	void initSettings(QtEnumTranslationMode::Mode mode, const QtTranslationLanguage & sourceLanguage, const QtTranslationLanguage & destLanguage);
	void showDialog();

	

public Q_SLOTS:

	void okClicked();
	void updateComboState();// VOXOX -ASV- 2010.02.01
	
Q_SIGNALS:

	void translationSettingsSignal(QtEnumTranslationMode::Mode mode,const QtTranslationLanguage & source,const QtTranslationLanguage & dest);

private:
	
	void initComboBoxes();
	QtTranslationLanguage getLanguage(const QString & text);
	QtEnumTranslationMode::Mode getMode(const QString & text);

	
	CChatHandler & _cChatHandler;

	Ui::TranslationWidget * _ui;

};

#endif	//OWQTQtTranslationWidget_H
