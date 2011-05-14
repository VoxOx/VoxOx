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

#ifndef QTIMCONTACTITEM_H
#define QTIMCONTACTITEM_H

#include <QtGui/QTreeWidgetItem>

class IMContact;

/**
 * Handles an item of the QTreeWidget.
 *
 * @see QtIMContactManager
 * @see QtIMAccountItem
 * @author Tanguy Krotoff
 */
class QtIMContactItem : public QTreeWidgetItem {
public:

	QtIMContactItem(QTreeWidget * parent, const QStringList & strings);

	void setIMContact(IMContact * imContact) {
		_imContact = imContact;
	}

	IMContact * getIMContact() const {
		return _imContact;
	}

private:

	IMContact * _imContact;
};

#endif	//QTIMCONTACTITEM_H
