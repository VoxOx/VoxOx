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

#ifndef OWQTFILETRANSFER_H
#define OWQTFILETRANSFER_H

#include <QtCore/QObject>

#include <filesessionmanager/ReceiveFileSession.h>
#include <filesessionmanager/SendFileSession.h>

#include <util/Trackable.h>
#include <util/List.h>

class CoIpManager;
class FileSessionManager;
class QtFileTransferWidget;
class IMContactSet;
class CContactList;

/**
 * Qt file transfer object.
 *
 * @author Mathieu Stute
 * @author Philippe Bernery
 */
class QtFileTransfer : public QObject, public Trackable {
	Q_OBJECT
public:

	/**
	 * Constructor.
	 */
	QtFileTransfer(QObject * parent, CoIpManager * coIpManager);

	/**
	 * Destructor.
	 */
	~QtFileTransfer();

	/**
	 * Creates a SendFileSession.
	 * @param imContactSet the set of contacts.
	 * @param filename the file to upload.
	 * @param cContactList a reference to the CContactList.
	 */
	void createSendFileSession(IMContactSet imContactSet, const QString & filename, CContactList & cContactList);

	/**
	 * Return a pointer to the File Transfer Widget.
	 * @return a pointer to the QtFileTransferWidget.
	 */
	QtFileTransferWidget * getFileTransferWidget() { return _qtFileTransferWidget; }

	/**
	 * Open a file chooser and return the chosen one.
	 * @return the chosen file.
	 */
	const QString getChosenFile() const;

Q_SIGNALS:

	/**
	 * @brief Thread safe code
	 */
	void newReceiveFileSessionCreatedEventHandlerSignal(ReceiveFileSession * fileSession);

	/**
	 * @brief Thread safe code
	 */
	void needUpgradeEventHandlerSignal();

	/**
	 * @brief Thread safe code
	 */
	void peerNeedsUpgradeEventHandlerSignal(const QString & contactID);

private Q_SLOTS:

	/**
	 * @brief Thread safe code
	 */
	void newReceiveFileSessionCreatedEventHandlerSlot(ReceiveFileSession * fileSession);

	/**
	 * @brief Thread safe code
	 */
	void needUpgradeEventHandlerSlot();

	/**
	 * @brief Thread safe code
	 */
	void peerNeedsUpgradeEventHandlerSlot(const QString & contactID);

private:

	/**
	 * @see FileSessionManager::newReceiveFileSessionCreatedEvent
	 */
	void newReceiveFileSessionCreatedEventHandler(FileSessionManager & sender, ReceiveFileSession fileSession);

	/**
	 * @see FileSessionManager::needUpgradeEvent
	 */
	void needUpgradeEventHandler(FileSessionManager & sender);

	/**
	 * @see FileSessionManager::peerNeedsUpgradeEvent
	 */
	void peerNeedsUpgradeEventHandler(FileSessionManager & sender, const std::string contactID);

	/**
	 * Direct link to the transfer widget.
	 */
	QtFileTransferWidget * _qtFileTransferWidget;

	/**
	 * Direct link to CoIpManager.
	 */
	CoIpManager * _coIpManager;
};

#endif	//OWQTFILETRANSFER_H
