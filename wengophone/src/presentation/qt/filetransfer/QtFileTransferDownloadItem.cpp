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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtFileTransferDownloadItem.h"

#include <filesessionmanager/ReceiveFileSession.h>
#include <imwrapper/IMContact.h>

#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

QtFileTransferDownloadItem::QtFileTransferDownloadItem(QWidget * parent, ReceiveFileSession * fileSession, const QString & path)
	: QtFileTransferItem(parent, QtFileTransferItem::Download), _receiveFileSession(fileSession) {

	_filename = path + QString(QDir::separator()) + QString::fromUtf8(fileSession->getFileName().c_str());

	setFilename(QString::fromUtf8(fileSession->getFileName().c_str()));
	setContact(QString::fromStdString(_receiveFileSession->getIMContact().getDisplayContactId()));

	// bind to fileSession events
	_receiveFileSession->fileTransferProgressionEvent +=
		boost::bind(&QtFileTransferDownloadItem::fileTransferProgressionEventHandler, this, _1, _2, _3, _4);
	_receiveFileSession->fileTransferEvent +=
		boost::bind(&QtFileTransferDownloadItem::fileTransferEventHandler, this, _1, _2, _3, _4);
	_receiveFileSession->moduleFinishedEvent +=
		boost::bind(&QtFileTransferDownloadItem::moduleFinishedEventHandler, this, _1);
	////

	//TODO: check if the fileSession has been cancelled, if yes call updateState
	if (_receiveFileSession->getLastEvent() == IFileSession::IFileSessionEventFileTransferCancelledByPeer) {
		updateStateEvent((int)IFileSession::IFileSessionEventFileTransferCancelledByPeer);
	}
}

void QtFileTransferDownloadItem::pause() {
	_receiveFileSession->pause();
}

void QtFileTransferDownloadItem::resume() {
	_receiveFileSession->resume();
}

void QtFileTransferDownloadItem::stop() {
	_receiveFileSession->stop();
}

void QtFileTransferDownloadItem::fileTransferProgressionEventHandler(
	ReceiveFileSession & sender, IMContact imContact, File sentFile, int percentage) {

	LOG_DEBUG("progress: " + String::fromNumber(percentage));
	progressChangeEvent(percentage);
}

void QtFileTransferDownloadItem::fileTransferEventHandler(ReceiveFileSession & sender,
	IFileSession::IFileSessionEvent event, IMContact imContact, File sentFile) {

	LOG_DEBUG("filetransfer event: " + String::fromNumber(event));

	updateStateEvent((int)event);
}

void QtFileTransferDownloadItem::moduleFinishedEventHandler(CoIpModule & sender) {
	LOG_DEBUG("module receiveFileSession has finished, delete it");

	// prevent multiple calls
	if (!_receiveFileSession) {
		return;
	}
	////

	// unbind to fileSession events
	_receiveFileSession->fileTransferProgressionEvent -=
		boost::bind(&QtFileTransferDownloadItem::fileTransferProgressionEventHandler, this, _1, _2, _3, _4);
	_receiveFileSession->fileTransferEvent -=
		boost::bind(&QtFileTransferDownloadItem::fileTransferEventHandler, this, _1, _2, _3, _4);
	_receiveFileSession->moduleFinishedEvent -=
		boost::bind(&QtFileTransferDownloadItem::moduleFinishedEventHandler, this, _1);
	////

	OWSAFE_DELETE(_receiveFileSession);
}
