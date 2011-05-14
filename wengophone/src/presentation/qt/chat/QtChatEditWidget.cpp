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
#include "QtChatEditWidget.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>

#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragLeaveEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>

#include <util/String.h>
#include <util/Logger.h>

QtChatEditWidget::QtChatEditWidget(QWidget *parent)
: QTextEdit(parent) {
	setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	setAcceptDrops(true);
	setLineWrapMode(QTextEdit::WidgetWidth);
}

void QtChatEditWidget::dragEnterEvent(QDragEnterEvent *event) {
	if(event->mimeData()->hasFormat("text/uri-list")) {
		event->acceptProposedAction();
	}
}

void QtChatEditWidget::dragMoveEvent(QDragMoveEvent *event) {
	event->acceptProposedAction();
}

void QtChatEditWidget::dragLeaveEvent(QDragLeaveEvent * event) {
	event->accept();
}
//VOXOX - CJC - 2009.05.20 VoxOx change, do not do this
void QtChatEditWidget::dropEvent(QDropEvent *event) {
 //   const QMimeData *mimeData = event->mimeData();
	//if(mimeData->hasUrls()) {
	//	QList<QUrl> urls = mimeData->urls();
	//	for(QList<QUrl>::iterator i(urls.begin()); i != urls.end(); i++){
	//		QString fileName;
	//		qint64 fileSize;
	//		QString fileType;
	//		LOG_DEBUG((*i).toLocalFile().toStdString());
	//		QFile fileToSend((*i).toLocalFile());
	//		QFileInfo fileInfo(fileToSend);
	//		if(fileInfo.exists() && fileInfo.isReadable()) {
	//			fileName = fileInfo.absoluteFilePath();
	//			fileSize = fileInfo.size();
	//			fileType = fileInfo.completeSuffix();
	//			LOG_DEBUG(
	//				"filename: " + fileName.toStdString() +
	//				"size: " + String::fromLongLong((long long)fileSize) +
	//				"type: " + fileType.toStdString()
	//			);
	//			// TODO add the contact name (wengo_id)
	//			// TODO make a call to send it all to a file transfer plugin (via a controller)
	//			fileDragged(fileName);
	//		}
	//	}
	//}
 //   event->acceptProposedAction();
}
