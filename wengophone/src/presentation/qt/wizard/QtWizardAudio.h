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
* @date 2009.06.01
*/


#ifndef QTWIZARDAUDIO_H
#define QTWIZARDAUDIO_H

#include "QtIWizard.h"

#include <QtGui/QWidget>

class CWengoPhone;
namespace Ui { class WizardAudio; }

class QtWizardAudio : public QWidget, public QtIWizard {
	Q_OBJECT
public:

	QtWizardAudio(CWengoPhone & cWengoPhone, QWidget * parent);

	virtual ~QtWizardAudio();

	virtual QString getName() const;

	virtual QString getTitle() const;

	virtual QString getDescription() const;
	virtual void saveConfig();

	virtual int getStepNumber() const;

	virtual QString getPixmapPath() const{return "";}

	virtual QWidget *getWidget() const {
		return (QWidget*)this;
	}

private Q_SLOTS:

	void testOutputDevice();

	void testRingingDevice();


private:

	virtual void readConfig();
	void fillAudioCodecList();

	Ui::WizardAudio * _ui;

	CWengoPhone & _cWengoPhone;
};

#endif	//QtWizardAudio_H
