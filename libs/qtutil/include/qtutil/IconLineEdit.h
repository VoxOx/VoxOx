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
#ifndef ICONLINEEDIT_H
#define ICONLINEEDIT_H

#include <QtGui/QLineEdit>

class QAbstractButton;
class QIcon;

/**
 * A QLineEdit which can display icons on its left and/or right side.
 * Icons inherit from QAbstractButton*, so you can connect the clicked()
 * signal to them.
 *
 * @author Aurelien Gateau
 */
class IconLineEdit : public QLineEdit {
public:
	IconLineEdit(QWidget* parent = 0);

	/**
	 * Creates the left button, and returns it.
	 */
	QAbstractButton* createLeftButton(const QIcon& icon);

	/**
	 * Creates the right button, and returns it.
	 */
	QAbstractButton* createRightButton(const QIcon& icon);

protected:
	virtual void resizeEvent(QResizeEvent*);
	QAbstractButton* _leftButton;
	QAbstractButton* _rightButton;

private:
	QAbstractButton* createButton(const QIcon& icon);
	void updateCss();
	void moveButtons();
};


#endif /* ICONLINEEDIT_H */
