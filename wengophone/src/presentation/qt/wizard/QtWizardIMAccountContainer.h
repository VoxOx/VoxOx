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
* Container for the im account adds
* @author Chris Jimenez C 
* @date 2009.06.02
*/


#ifndef QTWIZARDWizardIMAccountContainer_H
#define QTWIZARDWizardIMAccountContainer_H

#include "QtIWizard.h"
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.05.29 

#include <imwrapper/IMAccount.h>

#include <QtCore/QObject>
#include <QtGui/QWidget>

class QtIMAccountPlugin;
class CWengoPhone;

namespace Ui { class WizardIMAccountContainer; }

class QtWizardIMAccountContainer : public QWidget, public QtIWizard  {
	Q_OBJECT
public:


	QtWizardIMAccountContainer(CWengoPhone & cWengoPhone,QtEnumIMProtocol::IMProtocol imProtocol, QWidget * parent);

	virtual ~QtWizardIMAccountContainer();

	virtual QString getName() const;

	virtual QString getTitle() const;

	virtual QString getDescription() const;

	virtual int getStepNumber() const;

	virtual QString getPixmapPath() const;

	virtual void saveConfig();

	virtual void readConfig();

	virtual QWidget *getWidget() const {
		return (QWidget*)this;
	}


private:

	void createIMProtocolWidget(QtEnumIMProtocol::IMProtocol imProtocol);

	Ui::WizardIMAccountContainer * _ui;

	IMAccount _imAccount;

	CWengoPhone & _cWengoPhone;

	QtIMAccountPlugin * _imAccountPlugin;

	QtEnumIMProtocol::IMProtocol _createdIMProtocol;
};

#endif	//QTWizardIMAccountContainer_H
