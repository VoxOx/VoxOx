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

#ifndef OWQTCHATEDITWIDGET_H
#define OWQTCHATEDITWIDGET_H

#include <QtGui/QTextEdit>

class QDragEnterEvent;
class QDropEvent;
class QDragMoveEvent;
class QDragLeaveEvent;

/**
 * Subclass  a QTextEdit with drag and drop support for file transfer.
 *
 * @author Nicolas Couturier
 */
class QtChatEditWidget : public QTextEdit {
	Q_OBJECT
public:

	QtChatEditWidget(QWidget *parent);

Q_SIGNALS:

	//void fileDragged(const QString & filename);

protected:

	void dragEnterEvent(QDragEnterEvent * event);

	void dragMoveEvent(QDragMoveEvent * event);

	void dropEvent(QDropEvent * event);

	void dragLeaveEvent(QDragLeaveEvent * event);
};

#endif	//OWQTCHATEDITWIDGET_H
