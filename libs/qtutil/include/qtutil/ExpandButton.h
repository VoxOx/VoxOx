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
#ifndef EXPANDBUTTON_H
#define EXPANDBUTTON_H

#include <QtGui/QToolButton>


/**
 * A button which looks flat and shows an arrow. By default the arrow will
 * point to the right, but when the button is on, it will point up or down
 * (depending on the value of expandDirection()).
 *
 * To implement expandable frames, add a button like this above your frame,
 * connect the toggled(bool) signal of the button to the setVisible(bool) slot
 * of the frame.
 * By default the button is not on, so be sure to hide the frame.
 */
class ExpandButton : public QToolButton {
public:
	enum ExpandDirection { ED_Up, ED_Down };
	ExpandButton(QWidget* parent);

	/**
	 * Defines whether the arrow should point down (default) or up.
	 */
	void setExpandDirection(ExpandDirection);

	ExpandDirection expandDirection() const;

	virtual QSize sizeHint() const;

protected:
	virtual void paintEvent(QPaintEvent* event);

private:
	ExpandDirection _expandDirection;
};


#endif /* EXPANDBUTTON_H */
