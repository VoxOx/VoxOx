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

#ifndef QTPHONESETTINGS_H
#define QTPHONESETTINGS_H

#include "QtISettings.h"

#include <QtGui/QWidget>

class CWengoPhone;

namespace Ui { class PhoneSettings; }

/**
 * Appearance configuration panel.
 *
 * @author Aurelien Gateau
 */
class QtPhoneSettings : public QWidget, public QtISettings {
	Q_OBJECT
public:

	QtPhoneSettings(CWengoPhone & cWengoPhone, QWidget * parent);

	virtual ~QtPhoneSettings();

	virtual QString getName() const;

	virtual QString getTitle() const;

	virtual QString getDescription() const;
	virtual void saveConfig();

	virtual QString getIconName() const;

	virtual QWidget *getWidget() const {
		return (QWidget*)this;
	}

	void setJsAction( QString jsAction )			{ _jsAction = jsAction;	}	//VOXOX - JRT - 2009.07.02 

private Q_SLOTS:
	
	void documentCompleteSlot(bool);

	void linkSlot () ;//VOXOX - CJC - 2009.07.08 

private:

	virtual void readConfig();

	Ui::PhoneSettings * _ui;

	CWengoPhone & _cWengoPhone;

	QString		_jsAction;
};

#endif	//QtPhoneSettings_H
