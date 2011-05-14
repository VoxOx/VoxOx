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
#include "QtContactTreeKeyFilter.h"

#include <QtGui/QtGui>

QtContactTreeKeyFilter::QtContactTreeKeyFilter(QObject * parent, QTreeWidget * target) {
	_tree = target;
	target->installEventFilter(this);
}

bool QtContactTreeKeyFilter::eventFilter(QObject * obj, QEvent * event) {

	switch (event->type()) {
	case QEvent::KeyPress:
		keyPress((QKeyEvent *) event);
		return false;

	default:
		return QObject::eventFilter(obj, event);
	}
}

void QtContactTreeKeyFilter::keyPress(QKeyEvent * event) {
	QTreeWidgetItem * item = _tree->currentItem();
	switch (event->key()) {
	case Qt::Key_Left:
		closeItem(item);
		break;
	case Qt::Key_Right:
		openItem(item);
		break;
	case Qt::Key_Enter:
	case Qt::Key_Return:
		enterPressed(item);
		break;
	case Qt::Key_Delete:
		deleteItem(item);
		break;
	case Qt::Key_C:
		cPressed(item);
		break;
	case Qt::Key_W:
		wPressed(item);
		break;
	case Qt::Key_S:
		sPressed(item);
		break;

	default:
		break;
	}
}
