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
#include <qtutil/ExpandButton.h>

#include <QtGui/QStyleOption>
#include <QtGui/QStylePainter>

ExpandButton::ExpandButton(QWidget* parent)
: QToolButton(parent)
, _expandDirection(ED_Down) {
	setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	
	// Set icon, so that QToolButton::sizeHint().width() is correct
	QPixmap pix(":/pics/expand_button/right.png");
	setIcon(pix);

	setAutoRaise(true);
	setCheckable(true);
}

QSize ExpandButton::sizeHint() const {
	QSize size = QToolButton::sizeHint();
	size.setHeight(fontMetrics().height());
	return size;
}

void ExpandButton::paintEvent(QPaintEvent* event) {
	QStylePainter painter(this);

	QStyleOption option;
	option.initFrom(this);

	QString pixmapName;
	if (isChecked()) {
		if (_expandDirection == ED_Down) {
			pixmapName = "down";
		} else {
			pixmapName = "up";
		}
	} else {
		pixmapName = "right";
	}

	if (option.state & QStyle::State_MouseOver) {
		pixmapName += "_hover";
	}
	QPixmap pixmap(QString(":/pics/expand_button/%1.png").arg(pixmapName));
	painter.drawPixmap(0, 0, pixmap);

	QRect textRect = rect();
	textRect.setLeft(pixmap.width());

	painter.drawItemText(textRect, Qt::TextShowMnemonic, option.palette, isEnabled(), text());
}


void ExpandButton::setExpandDirection(ExpandDirection direction) {
	_expandDirection = direction;
	update();
}


ExpandButton::ExpandDirection ExpandButton::expandDirection() const {
	return _expandDirection;
}
