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

#include <qtutil/ClickableLabel.h>

#include <QtGui/QtGui>

ClickableLabel::ClickableLabel(QWidget * parent)
	: QLabel(parent) {
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent * event) {
	//Gets the widget rect
	const QRect rect = this->rect();

	//Gets the mouse position relative to this widget
	QPoint mousePosition = event->pos();

	if ((mousePosition.x() >= rect.x()) && (mousePosition.x() <= rect.width())) {
		if ((mousePosition.y() >= rect.y()) && (mousePosition.y() <= rect.height())) {
			if (event->button() == Qt::LeftButton) {
				clicked();
			} else if (event->button() == Qt::RightButton) {
				rightClicked();
			}
		}
	}
}
