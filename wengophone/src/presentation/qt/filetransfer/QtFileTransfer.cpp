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
#include "QtFileTransfer.h"
#include "QtFileTransferAcceptDialog.h"
#include "QtFileTransferNotifyDialog.h"
#include "QtFileTransferWidget.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <coipmanager/CoIpManager.h>
#include <control/contactlist/CContactList.h>

#include <imwrapper/IMContactSet.h>
#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <QtGui/QtGui>

#include <limits.h>

QtFileTransfer::QtFileTransfer(QObject * parent, CoIpManager * coIpManager)
	: QObject(parent), _coIpManager(coIpManager) {

	_qtFileTransferWidget = new QtFileTransferWidget(NULL);

	SAFE_CONNECT(this, SIGNAL(newReceiveFileSessionCreatedEventHandlerSignal(ReceiveFileSession *)),
		SLOT(newReceiveFileSessionCreatedEventHandlerSlot(ReceiveFileSession *)));
	_coIpManager->getFileSessionManager().newReceiveFileSessionCreatedEvent +=
		boost::bind(&QtFileTransfer::newReceiveFileSessionCreatedEventHandler, this, _1, _2);

	SAFE_CONNECT(this, SIGNAL(needUpgradeEventHandlerSignal()),
		SLOT(needUpgradeEventHandlerSlot()));
	_coIpManager->getFileSessionManager().needUpgradeEvent +=
		boost::bind(&QtFileTransfer::needUpgradeEventHandler, this, _1);

	SAFE_CONNECT(this, SIGNAL(peerNeedsUpgradeEventHandlerSignal(const QString &)),
		SLOT(peerNeedsUpgradeEventHandlerSlot(const QString &)));
	_coIpManager->getFileSessionManager().peerNeedsUpgradeEvent +=
		boost::bind(&QtFileTransfer::peerNeedsUpgradeEventHandler, this, _1, _2);
}

QtFileTransfer::~QtFileTransfer() {
	_qtFileTransferWidget->hide();
	OWSAFE_DELETE(_qtFileTransferWidget);
}

void QtFileTransfer::newReceiveFileSessionCreatedEventHandler(
	FileSessionManager & sender, ReceiveFileSession fileSession) {

	ReceiveFileSession * newFileSession = new ReceiveFileSession(fileSession);
	newReceiveFileSessionCreatedEventHandlerSignal(newFileSession);
}

void QtFileTransfer::newReceiveFileSessionCreatedEventHandlerSlot(ReceiveFileSession * fileSession) {

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString downloadFolder;
	QString filename =  QString::fromUtf8(fileSession->getFileName().c_str());
	QString contact = QString::fromStdString(fileSession->getIMContact().getDisplayContactId());

	LOG_DEBUG("incoming file: " + fileSession->getFileName() +
		"from: " + fileSession->getIMContact().getDisplayContactId());

	_qtFileTransferWidget->showAndRaise();
	QtFileTransferAcceptDialog qtFileTransferAcceptDialog(_qtFileTransferWidget);
	qtFileTransferAcceptDialog.setFileName(filename);
	qtFileTransferAcceptDialog.setContactName(contact);

	// the user accept the file transfer
	if (qtFileTransferAcceptDialog.exec() == QDialog::Accepted) {

		QDir dir(QString::fromUtf8(config.getFileTransferDownloadFolder().c_str()));
		// if no download folder set then choose one
		// or if the choosen folder does not exists anymore.
		if ((config.getFileTransferDownloadFolder().empty()) || (!dir.exists())) {

			downloadFolder = QFileDialog::getExistingDirectory(
				_qtFileTransferWidget,
				tr("Choose a directory"),
				QString::null,
				QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
			);

			if (!downloadFolder.isEmpty()) {
				config.set(Config::FILETRANSFER_DOWNLOAD_FOLDER_KEY, std::string(downloadFolder.toUtf8().constData()));
			} else {
				//TODO: warn the user is has set no download folder.
				fileSession->stop();
				OWSAFE_DELETE(fileSession);
				return;
			}
		} else {
			downloadFolder = QString::fromUtf8(config.getFileTransferDownloadFolder().c_str());
		}

		// here we're sure to have a download folder,
		// but we must check if the file already exists.
		QFile file(downloadFolder + QString(QDir::separator()) + filename);
		if (file.exists()) {

			if (QMessageBox::question(_qtFileTransferWidget, tr("Overwrite File?"),
					tr("A file called %1 already exists."
					"Do you want to overwrite it?").arg(filename),
					tr("&Yes"), tr("&No"), QString(), 0, 1)) {

				fileSession->stop();
				OWSAFE_DELETE(fileSession);
				return;
			}
		}

		_qtFileTransferWidget->setDownloadFolder(downloadFolder);
		_qtFileTransferWidget->addReceiveItem(fileSession);
		fileSession->setFilePath(std::string(downloadFolder.toUtf8().constData()));
		fileSession->start();

	// the user refuse the file transfer.
	} else {
		fileSession->stop();
		OWSAFE_DELETE(fileSession);
		return;
	}
}

void QtFileTransfer::createSendFileSession(IMContactSet imContactSet, const QString & filename, CContactList & cContactList) {

	QFileInfo fileinfo(QString(filename.toUtf8().constData()));
	File file(std::string(filename.toUtf8().constData()), File::EncodingUTF8);

	// check the fileinfo
	if (fileinfo.isDir()) {
		//~ if (QMessageBox::question(_qtFileTransferWidget,
			//~ tr("%1 is a directory.").arg(fileinfo.fileName()),
			//~ tr("Do you want to send all files included in %1 ?").arg(fileinfo.filePath()),
			//~ QMessageBox::No, QMessageBox::Yes, 0)
			//~ == QMessageBox::Yes) {

			//~ //gets all files in directory and sends them
			//~ QDir rep(fileinfo.filePath());
			//~ QFileInfoList tobesent = rep.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs,QDir::Unsorted);
			//~ for (int i = 0; i < tobesent.size(); ++i) {
				//~ createSendFileSession(imContactSet, tobesent.at(i).filePath(), cContactList);
			//~ }
			//~ ////
		//~ }
		QMessageBox::warning(_qtFileTransferWidget,
			tr("%1 is a directory.").arg(fileinfo.fileName()),
			tr("You can not send a directory."),
			QMessageBox::Ok, 0, 0
		);
	}
#if defined(OS_WINDOWS)
	else if (fileinfo.suffix() == QString::fromStdString("lnk")) {
		QMessageBox::warning(_qtFileTransferWidget,
			tr("%1 is a shortcut.").arg(fileinfo.fileName()),
			tr("You can not send a shortcut."),
			QMessageBox::Ok, 0, 0
		);
	}
#endif
	else if (file.getSize() > INT_MAX) {
		QMessageBox::warning(_qtFileTransferWidget, tr("File size error"),
			tr("%1 exceeds the maximum authorized size.").arg(fileinfo.filePath()),
			QMessageBox::Ok, 0, 0
		);
	} else if (file.getSize() <= 0) {
		QMessageBox::warning(_qtFileTransferWidget, tr("File size error"),
			tr("%1 has size 0.").arg(fileinfo.filePath()),
			QMessageBox::Ok, 0, 0
		);
	} else {
		File file(std::string(filename.toUtf8().constData()), File::EncodingUTF8);
		SendFileSession * fileSession = _coIpManager->getFileSessionManager().createSendFileSession();
		fileSession->addFile(file);

		for (IMContactSet::const_iterator it = imContactSet.begin(); it != imContactSet.end(); ++it) {

			std::string contactId = cContactList.findContactThatOwns(*it);
			fileSession->addContact(contactId);
			_qtFileTransferWidget->addSendItem(fileSession, filename.toStdString(), (*it).getDisplayContactId());
		}

		// HACK : Unique file transfer hack
		_coIpManager->getFileSessionManager().queueSession(fileSession);
	}
}

void QtFileTransfer::needUpgradeEventHandler(FileSessionManager & sender) {
	needUpgradeEventHandlerSignal();
}

void QtFileTransfer::peerNeedsUpgradeEventHandler(FileSessionManager & sender, const std::string contactID) {
	peerNeedsUpgradeEventHandlerSignal(QString::fromStdString(contactID));
}

void QtFileTransfer::needUpgradeEventHandlerSlot() {

	QtFileTransferNotifyDialog qtFileTransferNotifyDialog(_qtFileTransferWidget);
	qtFileTransferNotifyDialog.setTitle(tr("Please upgrade<br> your @product@"));
	qtFileTransferNotifyDialog.setMessage(tr("The file cannot be received:"
		"you must upgrade your @product@ in order to receive it."));

	qtFileTransferNotifyDialog.exec();
}

void QtFileTransfer::peerNeedsUpgradeEventHandlerSlot(const QString & contactID) {

	QtFileTransferNotifyDialog qtFileTransferNotifyDialog(_qtFileTransferWidget);
	qtFileTransferNotifyDialog.setTitle(tr("Tell your contact<br> to upgrade<br> his @product@"));
	QString message = contactID + " " + QString(tr("is trying to send you a file "
		"but his @product@ must be upgraded in order to receive it."
		"Tell him to download the latest version."));
	qtFileTransferNotifyDialog.setMessage(message);

	qtFileTransferNotifyDialog.exec();
}

const QString QtFileTransfer::getChosenFile() const {

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString startDir = QString::fromStdString(config.getLastUploadedFileFolder());

	QString filename = QFileDialog::getOpenFileName(
		_qtFileTransferWidget,
		"Choose a file",
		startDir,
		tr("All Files (*.* *)")
	);
	return filename;
}
