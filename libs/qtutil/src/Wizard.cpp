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

#include <qtutil/Wizard.h>

#include <qtutil/SafeConnect.h>

#include "ui_Wizard.h"

#include <util/Logger.h>

#include <QtGui/QtGui>

Wizard::Wizard(QWidget * parent) {
	_wizardDialog = new QDialog(parent);

	_ui = new Ui::Wizard();
	_ui->setupUi(_wizardDialog);

	SAFE_CONNECT(_ui->backButton, SIGNAL(clicked()), SLOT(backButtonClicked()));
	SAFE_CONNECT(_ui->nextButton, SIGNAL(clicked()), SLOT(nextButtonClicked()));
	SAFE_CONNECT(_ui->finishButton, SIGNAL(clicked()), SLOT(finishedButtonClicked()));
}

Wizard::~Wizard() {
	delete _ui;
}

void Wizard::show() {
	_ui->stackedWidget->setCurrentIndex(1);
	_history.append(_ui->stackedWidget->widget(1));
	switchPage(NULL);
	_wizardDialog->exec();
}

void Wizard::addPage(QWidget * page) {
	_ui->stackedWidget->addWidget(page);
}

void Wizard::finishedButtonClicked() {
	finished();
}

void Wizard::backButtonClicked() {
	_ui->nextButton->setEnabled(true);
	_ui->finishButton->setEnabled(true);

	QWidget * oldPage = _history.takeLast();
	switchPage(oldPage);
	delete oldPage;
}

void Wizard::nextButtonClicked() {
	_ui->nextButton->setEnabled(true);
	_ui->finishButton->setEnabled(_history.size() == _ui->stackedWidget->count() - 1);

	QWidget * oldPage = _history.last();
	_history.append(_ui->stackedWidget->widget(_history.size()));
	switchPage(oldPage);
}

void Wizard::switchPage(QWidget * oldPage) {
	QWidget * newPage = _history.last();
	_ui->stackedWidget->setCurrentWidget(newPage);
	newPage->show();
	newPage->setFocus();

	_ui->backButton->setEnabled(_history.size() != 1);
	if (_history.size() == _ui->stackedWidget->count()) {
		_ui->nextButton->setEnabled(false);
		_ui->finishButton->setDefault(true);
	} else {
		_ui->nextButton->setDefault(true);
		_ui->finishButton->setEnabled(false);
	}

	_wizardDialog->setWindowTitle(tr("Simple Wizard - Step %1 of %2")
		.arg(_history.size())
		.arg(_ui->stackedWidget->count()));
}
