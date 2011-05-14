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

#ifndef OWQTFILETRANSFERWIDGET_H
#define OWQTFILETRANSFERWIDGET_H

#include <QtGui/QDialog>
#include <QtCore/QString>

#include <string>

class ReceiveFileSession;
class SendFileSession;

namespace Ui { class FileTransferDialog; }

/**
 * Qt file transfer widget.
 *
 * @author Mathieu Stute
 */
class QtFileTransferWidget : public QDialog {
	Q_OBJECT
public:

	/**
	 * Default constructor.
	 * @param parent the parent widget
	 */
	QtFileTransferWidget(QWidget * parent);

	/**
	 * Destructor.
	 */
	~QtFileTransferWidget();

	/**
	 * Set (graphically and in Config) the download folder.
	 * @param folder the path to the download folder.
	 */
	void setDownloadFolder(const QString & folder);

	/**
	 * Add a download item in the widget.
	 * @param fileSession the fileSession
	 */
	void addReceiveItem(ReceiveFileSession * fileSession);

	/**
	 * Add an upload item in the widget
	 * @param fileSession the file session
	 * @param filename the filename
	 * @param contact the contact
	 */
	void addSendItem(SendFileSession * fileSession, const std::string & filename,
		const std::string & contact);

	/**
	 * Show and raise the widget.
	 */
	void showAndRaise();

private Q_SLOTS:

	/**
	 * Clean button has been clicked.
	 */
	void cleanButtonClicked();

	/**
	 * Path button has been clicked.
	 */
	void pathButtonClicked();

	/**
	 * A remove button has been clicked.
	 */
	void itemRemoveClicked();

private:

	void clean(bool cleanButton);

	/**
	 * Set the upload tab active.
	 */
	void showUploadTab();

	/**
	 * Set the download tab active.
	 */
	void showDownloadTab();

	/** a cache for the path to the download folder */
	QString _downloadFolder;

	Ui::FileTransferDialog * _ui;
};

#endif	//OWQTFILETRANSFERWIDGET_H
