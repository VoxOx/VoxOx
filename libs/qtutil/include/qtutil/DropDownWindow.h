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
#ifndef DROPDOWNWINDOW_H
#define DROPDOWNWINDOW_H

#include <QtGui/QFrame>

class QAbstractButton;

class DropDownWindow : public QFrame {
	Q_OBJECT
public:
	DropDownWindow(QAbstractButton* button);

	QAbstractButton* button() const;

protected:
	virtual void showEvent(QShowEvent*);
	virtual void hideEvent(QHideEvent*);

private Q_SLOTS:
	void slotButtonToggled(bool);

private:
	QAbstractButton* _button;
};

#endif /* DROPDOWNWINDOW_H */
