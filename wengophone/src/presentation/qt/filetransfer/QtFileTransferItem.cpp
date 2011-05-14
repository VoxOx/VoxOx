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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtFileTransferItem.h"
#include "QtFileTransferNotifyDialog.h"
#include "ui_FileTransferItem.h"

#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/WebBrowser.h>

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <QtGui/QtGui>

QtFileTransferItem::QtFileTransferItem(QWidget * parent, Type type)
	: QWidget(parent),
	_type(type) {

	_removeClicked = false;

	// init main widget
	_ui = new Ui::FileTransferItem();
	_ui->setupUi(this);
	_ui->progressBar->setMaximum(100);
	setProgress(0);
	
	// configure initial state
	if (_type == Download) {
		setState(tr("Downloading..."));
	} else {
		setState(tr("Waiting for peer..."));
	}
	updateButtonsInProgress();
	////

	// SIGNAL/SLOT for thread safe
	SAFE_CONNECT(this, SIGNAL(progressChangeEvent(int)),
		SLOT(setProgress(int)));
	SAFE_CONNECT(this, SIGNAL(stateChangeEvent(const QString &)),
		SLOT(setState(const QString &)));
	SAFE_CONNECT(this, SIGNAL(updateStateEvent(int)),
		SLOT(updateState(int)));
	////
}

QtFileTransferItem::~QtFileTransferItem() {
	OWSAFE_DELETE(_ui);
}

void QtFileTransferItem::setFilename(const QString & filename) {
	_ui->filenameLabel->setText("<b>" + filename + "</b>");
}

void QtFileTransferItem::setState(const QString & state) {
	_ui->statusLabel->setText(state);
}

void QtFileTransferItem::setContact(const QString & contact) {
	_contact = contact;
	if (_type == Download) {
		_ui->contactLabel->setText("  <i>" + tr("From: ") + contact + "</i>");
	} else {
		_ui->contactLabel->setText("  <i>" + tr("To: ") + contact + "</i>");
	}
}

void QtFileTransferItem::setFileSize(int size) {
}

void QtFileTransferItem::setProgress(int progress) {
	_ui->progressBar->setValue(progress);
	//HACK: IFileSessionEventFileTransferFinished is not always sent
	if (progress != 100) {
		stateChangeEventDownUp();
	} else {
		updateButtonsFinished();
	}
}

void QtFileTransferItem::updateButtonsFinished() {
	disconnectButtons();
	SAFE_CONNECT(_ui->cancelOpenButton, SIGNAL(clicked()), SLOT(open()));
	SAFE_CONNECT(_ui->removePauseResumeButton, SIGNAL(clicked()), SLOT(remove()));
	_ui->cancelOpenButton->setText(tr("Open"));
	_ui->removePauseResumeButton->setText(tr("Remove"));
	_ui->removePauseResumeButton->setEnabled(true);
	_ui->cancelOpenButton->setEnabled(true);
	//FIXME: remove the status bar from its layout
	//_ui->progressBar->hide();
}

void QtFileTransferItem::updateButtonsPaused() {
	disconnectButtons();
	SAFE_CONNECT(_ui->cancelOpenButton, SIGNAL(clicked()), SLOT(stop()));
	SAFE_CONNECT(_ui->removePauseResumeButton, SIGNAL(clicked()), SLOT(resume()));
	_ui->cancelOpenButton->setText(tr("Cancel"));
	_ui->removePauseResumeButton->setText(tr("Resume"));
	_ui->removePauseResumeButton->setEnabled(true);
	_ui->cancelOpenButton->setEnabled(false);
}

void QtFileTransferItem::updateButtonsPausedByPeer() {
	disconnectButtons();
	SAFE_CONNECT(_ui->cancelOpenButton, SIGNAL(clicked()), SLOT(stop()));
	SAFE_CONNECT(_ui->removePauseResumeButton, SIGNAL(clicked()), SLOT(pause()));
	_ui->cancelOpenButton->setText(tr("Cancel"));
	_ui->removePauseResumeButton->setText(tr("Pause"));
	_ui->removePauseResumeButton->setEnabled(false);
	_ui->cancelOpenButton->setEnabled(false);
}

void QtFileTransferItem::updateButtonsInProgress() {
	disconnectButtons();
	SAFE_CONNECT(_ui->cancelOpenButton, SIGNAL(clicked()), SLOT(stop()));
	SAFE_CONNECT(_ui->removePauseResumeButton, SIGNAL(clicked()), SLOT(pause()));
	_ui->cancelOpenButton->setText(tr("Cancel"));
	_ui->removePauseResumeButton->setText(tr("Pause"));
	_ui->removePauseResumeButton->setEnabled(true);
	_ui->cancelOpenButton->setEnabled(true);
}

void QtFileTransferItem::disconnectButtons() {
	_ui->cancelOpenButton->disconnect(SIGNAL(clicked()));
	_ui->removePauseResumeButton->disconnect(SIGNAL(clicked()));
}

void QtFileTransferItem::remove() {
	_removeClicked = true;
	removeClicked();
}

void QtFileTransferItem::open() {
	LOG_DEBUG("open from file transfer manager: " + _filename.toStdString());
#ifdef OS_WINDOWS
	//TODO: other systems
	WebBrowser::openUrl(_filename.toStdString());
#endif
}

void QtFileTransferItem::updateState(int e) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString companyWebSiteUrl = QString::fromStdString(config.getCompanyWebSiteUrl());

	IFileSession::IFileSessionEvent event = (IFileSession::IFileSessionEvent) e;
	QtFileTransferNotifyDialog  * qtFileTransferNotifyDialog;

	switch(event) {
	case IFileSession::IFileSessionEventInviteToTransfer:
		stateChangeEvent(tr("Starting"));
		updateButtonsInProgress();
		break;
	case IFileSession::IFileSessionEventWaitingForAnswer:
		stateChangeEvent(tr("Waiting for anwser..."));
		updateButtonsInProgress();
		break;
	case IFileSession::IFileSessionEventFileTransferFinished:
		stateChangeEvent(tr("Done"));
		updateButtonsFinished();
		break;
	case IFileSession::IFileSessionEventFileTransferFailed:
		stateChangeEvent(tr("Failed"));
		updateButtonsFinished();
		qtFileTransferNotifyDialog = new QtFileTransferNotifyDialog(this);
		qtFileTransferNotifyDialog->setTitle(tr("An error occured during the file transfer"));
		qtFileTransferNotifyDialog->setMessage(tr("This may be caused by :") + "<br>" +
			"<ul>" +
			"<li>" + tr("Your @product@ is not up to date. Please download the latest version on") +
			" " + companyWebSiteUrl + "<br>" +
			"<li>" + tr("The @company@ network may be temporarily unavailable. Please try later.") +
			"</ul>");
		qtFileTransferNotifyDialog->exec();
		OWSAFE_DELETE(qtFileTransferNotifyDialog);
		break;
	case IFileSession::IFileSessionEventFileTransferPaused:
		stateChangeEvent(tr("Paused"));
		updateButtonsPaused();
		break;
	case IFileSession::IFileSessionEventFileTransferPausedByPeer:
		stateChangeEvent(tr("Paused by peer"));
		updateButtonsPausedByPeer();
		break;
	case IFileSession::IFileSessionEventFileTransferResumed:
		stateChangeEventDownUp();
		updateButtonsInProgress();
		break;
	case IFileSession::IFileSessionEventFileTransferResumedByPeer:
		stateChangeEventDownUp();
		updateButtonsInProgress();
		break;
	case IFileSession::IFileSessionEventFileTransferCancelled:
		stateChangeEvent(tr("Cancelled"));
		updateButtonsFinished();
		break;
	case IFileSession::IFileSessionEventFileTransferCancelledByPeer:
		stateChangeEvent(tr("Cancelled by peer"));
		updateButtonsFinished();
		break;
	case IFileSession::IFileSessionEventFileTransferBegan:
		stateChangeEventDownUp();
		updateButtonsInProgress();
		break;
	default:
		LOG_FATAL("unknonw IFileSessionEvent: " + String::fromNumber(event));
	}
}

void QtFileTransferItem::stateChangeEventDownUp() {

	if (_type == Download) {
		stateChangeEvent(tr("Downloading..."));
	} else {
		stateChangeEvent(tr("Uploading..."));
	}
}
