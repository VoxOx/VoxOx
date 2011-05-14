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

#ifndef OWQTCONTACTTREEKEYFILTER_H
#define OWQTCONTACTTREEKEYFILTER_H

#include <QtCore/QObject>

class QKeyEvent;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * Qt Key filter for Contact list
 *
 * @author Mr K
 * @author Mathieu Stute
 */
class QtContactTreeKeyFilter : public QObject {
	Q_OBJECT
public:

	QtContactTreeKeyFilter(QObject * parent, QTreeWidget * target);

Q_SIGNALS:

	void openItem(QTreeWidgetItem * item);

	void closeItem(QTreeWidgetItem * item);

	void enterPressed(QTreeWidgetItem * item);

	void cPressed(QTreeWidgetItem * item);

	void wPressed(QTreeWidgetItem * item);

	void sPressed(QTreeWidgetItem * item);

	void deleteItem(QTreeWidgetItem * item);

private:

	bool eventFilter(QObject * obj, QEvent * event);

	void keyPress(QKeyEvent * event);

	QTreeWidget * _tree;
};

#endif	//OWQTCONTACTTREEKEYFILTER_H
