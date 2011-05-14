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
#include <qtutil/IconLineEdit.h>

#include <QtGui/QAbstractButton>
#include <QtGui/QIcon>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QStylePainter>


class LineEditButton : public QAbstractButton {
public:
	LineEditButton(QWidget* parent)
	: QAbstractButton(parent) {
	}

	QSize sizeHint() const {
		return iconSize();
	}

protected:
	void paintEvent(QPaintEvent* event) {
		QStylePainter painter(this);
		QStyleOptionButton option;
		option.initFrom(this);
		option.icon = icon();
		option.iconSize = iconSize();
		painter.drawControl(QStyle::CE_PushButtonLabel, option);
	}
};


IconLineEdit::IconLineEdit(QWidget* parent)
: QLineEdit(parent) {
	_leftButton = 0;
	_rightButton = 0;
	updateCss();
}

QAbstractButton* IconLineEdit::createLeftButton(const QIcon& icon) {
	_leftButton = createButton(icon);
	moveButtons();
	updateCss();
	return _leftButton;
}

QAbstractButton* IconLineEdit::createRightButton(const QIcon& icon) {
	_rightButton = createButton(icon);
	moveButtons();
	updateCss();
	return _rightButton;
}

QAbstractButton* IconLineEdit::createButton(const QIcon& icon) {
	LineEditButton* button = new LineEditButton(this);
	button->setIcon(icon);
	button->setCursor(Qt::ArrowCursor);
	button->setFocusPolicy(Qt::NoFocus);
	return button;
}

void IconLineEdit::updateCss() {
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	int leftPadding = 0;
	if (_leftButton) {
		leftPadding = _leftButton->sizeHint().width() + frameWidth;
	}

	int rightPadding = 0;
	if (_rightButton) {
		rightPadding = _rightButton->sizeHint().width() + frameWidth;
	}

	QString css = QString("QLineEdit { padding-left: %1px; padding-right: %2px }")
		.arg(leftPadding).arg(rightPadding);
	setStyleSheet(css);
}

void IconLineEdit::moveButtons() {
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	if (_leftButton) {
		QSize buttonSize = _leftButton->sizeHint();
		int posY = (height() - buttonSize.height()) / 2;
		_leftButton->move(frameWidth * 2, posY);
	}

	if (_rightButton) {
		QSize buttonSize = _rightButton->sizeHint();
		int posX = rect().right() - buttonSize.width() - (frameWidth * 2);
		int posY = (height() - buttonSize.height()) / 2;
		_rightButton->move(posX, posY);
	}
}

void IconLineEdit::resizeEvent(QResizeEvent* event) {
	QLineEdit::resizeEvent(event);
	moveButtons();
}
