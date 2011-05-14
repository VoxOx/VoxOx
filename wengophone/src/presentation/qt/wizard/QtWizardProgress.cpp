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
* Wizard Progress
* @author Chris Jimenez C 
* @date 2009.06.01
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtWizardProgress.h"

#include "ui_WizardProgress.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <cutil/global.h>

#include <QtGui/QtGui>

QtWizardProgress::QtWizardProgress(CWengoPhone & cWengoPhone, QWidget * parent)
	: QWidget(parent) {

	_ui = new Ui::WizardProgress();
	_ui->setupUi(this);
	
	_ui->lblChkImage1->setVisible(false);
	_ui->lblChkImage2->setVisible(false);
	_ui->lblChkImage3->setVisible(false);
	_ui->lblChkImage4->setVisible(false);
	_ui->lblChkImage5->setVisible(false);
	
	readConfig();
}

QtWizardProgress::~QtWizardProgress() {
	delete _ui;
}

QString QtWizardProgress::getName() const {
	return tr("WizardProgress");
}

QString QtWizardProgress::getTitle() const {
	return tr("VoxOx Setup Wizard             ");
}

QString QtWizardProgress::getDescription() const {
	return tr("The Wizard will guide you through 5 important steps that will help you get\n the most out of VoxOx. This process will only take a few minutes to complete.");//VOXOX -ASV- 07-10-2009: changed the text
}


int QtWizardProgress::getStepNumber() const {
	return 0;
}



void QtWizardProgress::readConfig() {
	_ui->lblChkImage1->setVisible(false);
	_ui->lblChkImage2->setVisible(false);
	_ui->lblChkImage3->setVisible(false);
	_ui->lblChkImage4->setVisible(false);
	_ui->lblChkImage5->setVisible(false);
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	int wizardStep = config.getWizardStepsComplete();
	switch(wizardStep){
		case 1:
			_ui->lblChkImage1->setVisible(true);
			break;
		case 2:
			_ui->lblChkImage1->setVisible(true);
			_ui->lblChkImage2->setVisible(true);
			break;
		case 3:
			_ui->lblChkImage1->setVisible(true);
			_ui->lblChkImage2->setVisible(true);
			_ui->lblChkImage3->setVisible(true);
			break;
		case 4:
			_ui->lblChkImage1->setVisible(true);
			_ui->lblChkImage2->setVisible(true);
			_ui->lblChkImage3->setVisible(true);
			_ui->lblChkImage4->setVisible(true);
			break;
		case 5:
			_ui->lblChkImage1->setVisible(true);
			_ui->lblChkImage2->setVisible(true);
			_ui->lblChkImage3->setVisible(true);
			_ui->lblChkImage4->setVisible(true);
			_ui->lblChkImage5->setVisible(true);
			break;
	}
}

void QtWizardProgress::saveConfig() {
}
