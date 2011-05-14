/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#include "SkinDemo.h"

#include <WidgetFactory.h>
#include <Object.h>

#include <QPushButton>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>

#include <iostream>
using namespace std;

SkinDemo::SkinDemo() : QMainWindow() {

	_aboutWindow = NULL;

	setObjectName("mainWindow");

	QWidget * mainWindow = WidgetFactory::create("MainWindow.ui", this);

	setCentralWidget(mainWindow);
	setMenuBar(createMenuBar());
	setStatusBar(createStatusBar());
	addToolBar(createToolBar());

	QPushButton * searchButton = mainWindow->findChild<QPushButton *>("searchButton");
	connect(searchButton, SIGNAL(clicked()), this, SLOT(about()));

	QPushButton * updateButton = mainWindow->findChild<QPushButton *>("updateButton");
	connect(updateButton, SIGNAL(clicked()), this, SLOT(update()));

	QPushButton * exitButton = mainWindow->findChild<QPushButton *>("exitButton");
	connect(exitButton, SIGNAL(clicked()), this, SLOT(quit()));
}

void SkinDemo::about() {
	_aboutWindow = WidgetFactory::create("AboutWindow.ui", NULL);

	_aboutWindow->show();
}

void SkinDemo::update() {
	if (!_aboutWindow) {
		return;
	}

	QGroupBox * groupBox = _aboutWindow->findChild<QGroupBox *>("groupBox");

	groupBox->setEnabled(!groupBox->isEnabled());
}

void SkinDemo::quit() {
	::exit(EXIT_SUCCESS);
}

QMenuBar * SkinDemo::createMenuBar() {
	QMenuBar * menuBar = new QMenuBar();

	QAction * menuBarAction = menuBar->addAction("menuBarAction");
	menuBarAction->setObjectName("menuBarAction");

	QMenu * menuBarMenu = menuBar->addMenu("menuBarMenu");
	menuBarMenu->setObjectName("menuBarMenu");

	return menuBar;
}

QStatusBar * SkinDemo::createStatusBar() {
	QStatusBar * statusBar = new QStatusBar();

	QLabel * statusBarLabel = new QLabel("statusBarLabel");
	statusBarLabel->setObjectName("statusBarLabel");
	statusBar->addWidget(statusBarLabel);

	QPushButton * statusBarButton = new QPushButton("statusBarButton");
	statusBarButton->setObjectName("statusBarButton");
	statusBar->addPermanentWidget(statusBarButton);

	return statusBar;
}

QToolBar * SkinDemo::createToolBar() {
	QToolBar * toolBar = new QToolBar("toolBar");

	QAction * toolBarAction = toolBar->addAction("toolBarAction");
	toolBarAction->setObjectName("toolBarAction");

	QPushButton * toolBarButton = new QPushButton("toolBarButton");
	toolBarButton->setObjectName("toolBarButton");
	toolBar->addWidget(toolBarButton);

	return toolBar;
}


void addWidgetToLayout(QWidget * widget, QLayout * layout) {
	widget->setParent(NULL);
	layout->addWidget(widget);
}
