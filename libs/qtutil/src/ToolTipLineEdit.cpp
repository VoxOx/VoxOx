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

#include <qtutil/ToolTipLineEdit.h>

#include <qtutil/LanguageChangeEventFilter.h>

#include <QtGui/QtGui>

ToolTipLineEdit::ToolTipLineEdit(QWidget * parent)
	: QLineEdit(parent) {

	LANGUAGE_CHANGE(this);
	init();
}

void ToolTipLineEdit::init() {
	_cleared = false;
	_toolTipTextDone = false;
	_originalPalette = palette();

	//Grey color
	_greyPalette.setColor(QPalette::Text, Qt::gray);
}

void ToolTipLineEdit::setText(const QString & text) {
	if (!_toolTipTextDone) {
		_toolTipTextDone = true;
		_toolTip = text;
		setToolTipText();
	} else {
		clearLineEdit();
		QLineEdit::setText(text);
	}
}

QString ToolTipLineEdit::text() const {
	if (QLineEdit::text() == _toolTip) {
		return QString::null;
	} else {
		return QLineEdit::text();
	}
}

void ToolTipLineEdit::setToolTipText() {
	//Text color is grey
	setPalette(_greyPalette);

	QLineEdit::setText(_toolTip);
	_cleared = false;
}

void ToolTipLineEdit::clearLineEdit() {
	//Text color back to original color
	setPalette(_originalPalette);

	clear();
	_cleared = true;
}

void ToolTipLineEdit::mousePressEvent(QMouseEvent * event) {
	if (!_cleared) {
		clearLineEdit();
	}
	QLineEdit::mousePressEvent(event);
}

void ToolTipLineEdit::keyPressEvent(QKeyEvent * event) {
	if (!_cleared) {
		clearLineEdit();
	}
	QLineEdit::keyPressEvent(event);
}

//VOXOX CHANGE for VoxOx by Rolando 01-09-09, in VoxOx version we use leaveEvent(QEvent * event) instead the current declaration
void ToolTipLineEdit::focusOutEvent(QFocusEvent * event) {
	if (text().isEmpty()) {
		setToolTipText();
	}
//VOXOX CHANGE for VoxOx by Rolando 01-09-09, in VoxOx version we use QLineEdit::leaveEvent(event); instead focusOutEvent
	QLineEdit::focusOutEvent(event);
}

void ToolTipLineEdit::languageChanged() {
	init();
}
