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

//VOXOX CHANGE REMOVE EVERYTHING FROM STATUS BAR WE DONT NEED THIS ANYMORE
#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtStatusBar.h"

#include "ui_StatusBarWidget.h"

#include <QtGui/QStatusBar>
#include <QtGui/QMenu>

#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

QtStatusBar::QtStatusBar(QStatusBar * statusBar)
	: QWidget(statusBar)
{
	init();

	_ui = new Ui::StatusBarWidget();
	_ui->setupUi(this);
	_statusBar = statusBar;
	_statusBar->addWidget(this,1);

	_ui->btnAdd->setImages   (":/pics/statusbar/btn_add.png",     ":/pics/statusbar/btn_add_selected.png"      );
	_ui->btnConfig->setImages(":/pics/statusbar/btn_settings.png",":/pics/statusbar/btn_settings_selected.png" );
}

QtStatusBar::~QtStatusBar() {
	OWSAFE_DELETE(_ui);

	OWSAFE_DELETE(_addMenu);	
	OWSAFE_DELETE(_settingsMenu);
}

void QtStatusBar::init() {
	_ui			  = NULL;
	_statusBar	  = NULL;
	_addMenu	  = NULL;	//VOXOX - JRT - 2009.06.04 - Fix crash in dtor
	_settingsMenu = NULL;	//VOXOX - JRT - 2009.06.04 - Fix crash om dtor
}

void QtStatusBar::setAddMenu(QMenu * addMenu,QString tooltip){
	 
	OWSAFE_DELETE(_addMenu);		//VOXOX - JRT - 2009.07.08 

	_addMenu =  addMenu;
	_ui->btnAdd->setMenu(_addMenu);
	_ui->btnAdd->setToolTip(tooltip);
}

void QtStatusBar::setSettingsMenu(QMenu * settingsMenu,QString tooltip){

	//OWSAFE_DELETE(_settingsMenu);	//VOXOX - JRT - 2009.07.08 //VOXOX - SEMR - 2009.07.10 THIS DELETE THE MENU WHEN TAB IS CHANGE
	//ALL MENUS ARE CREATED, they change according to the tab selected

	_settingsMenu = settingsMenu;
	_ui->btnConfig->setMenu(_settingsMenu);
	_ui->btnConfig->setToolTip(tooltip);
}

void QtStatusBar::languageChanged() {
	_ui->retranslateUi(this);
	init();
}
