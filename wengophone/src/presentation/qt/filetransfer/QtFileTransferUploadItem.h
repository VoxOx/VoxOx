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

#ifndef OWQTFILETRANSFERUPLOADITEM_H
#define OWQTFILETRANSFERUPLOADITEM_H

#include "QtFileTransferItem.h"

#include <filesessionmanager/IFileSession.h>

class SendFileSession;
class File;
class IMContact;
class CoIpModule;

/**
 * A QtFileTransferItem dedicate to upload.
 * @author Mathieu Stute
 */
class QtFileTransferUploadItem : public QtFileTransferItem {
	Q_OBJECT
public:

	/**
	 * Constructor.
	 */
	QtFileTransferUploadItem(QWidget * parent, SendFileSession * fileSession, 
		const QString & filename, const std::string & contact);

	/**
	 * @see QtFileTransferItem::isRunning().
	 */
	bool isRunning() const {
		return (_sendFileSession != NULL);
	}

private Q_SLOTS:

	/**
	 * @see ReceiveFileSession::pause().
	 */
	void pause();

	/**
	 * @see ReceiveFileSession::resume().
	 */
	void resume();

	/**
	 * @see ReceiveFileSession::stop().
	 */
	void stop();

private:

	/**
	 * @see SendFileSession::fileTransferProgressionEvent.
	 */
	void fileTransferProgressionEventHandler(SendFileSession & sender, IMContact imContact,
		File sentFile, int percentage);

	/**
	 * @see SendFileSession::fileTransferEvent.
	 */
	void fileTransferEventHandler(SendFileSession & sender, IFileSession::IFileSessionEvent event,
		IMContact imContact, File sentFile);

	/**
	 * The module has finished its job, it can be released.
	 * @see CoIpModule::moduleFinishedEvent
	 */
	void moduleFinishedEventHandler(CoIpModule & sender);

	/**
	 * Direct link to SendFileSession.
	 */
	SendFileSession * _sendFileSession;
};

#endif	//OWQTFILETRANSFERUPLOADITEM_H
