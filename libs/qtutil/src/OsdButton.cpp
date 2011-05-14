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
#include <qtutil/OsdButton.h>

#include <QtGui/QIcon>
#include <QtGui/QPainter>
#include <QtGui/QStyleOption>

OsdButton::OsdButton(QWidget* parent)
: QToolButton(parent) {}

QSize OsdButton::sizeHint() const {
	return iconSize();
}

void OsdButton::paintEvent(QPaintEvent*) {
	QPainter painter(this);

	QStyleOption option;
	option.initFrom(this);

	QPixmap pix = icon().pixmap(iconSize());
	QPainter pixPainter(&pix);
	if (isDown()) {
		pixPainter.fillRect(pix.rect(), QColor(0, 0, 0, 64));
		pixPainter.setPen(QPen(QColor(0, 0, 0, 192)));
		pixPainter.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
	} else if (option.state & QStyle::State_MouseOver) {
		pixPainter.fillRect(pix.rect(), QColor(255, 255, 255, 64));
		pixPainter.setPen(QPen(QColor(255, 255, 255, 192)));
		pixPainter.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
	}
	pixPainter.end();
	painter.drawPixmap(0, 0, pix);
}
