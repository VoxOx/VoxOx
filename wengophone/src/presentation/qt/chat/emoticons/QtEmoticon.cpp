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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtEmoticon.h"

#include <util/Logger.h>

#include <QtGui/QtGui>

QtEmoticon::QtEmoticon() {
}

QtEmoticon::QtEmoticon(const QtEmoticon & source) {
	_path = source._path;
	_text = QStringList(source._text);
	_pixmap = source._pixmap;
	_regExp = source._regExp;
	_buttonPixmap = source._buttonPixmap;
}

QtEmoticon::~QtEmoticon() {
}

QString QtEmoticon::getHtml() const {
	return QString("<img src=\"%1\" />").arg(_path);
}

QString QtEmoticon::getHtmlRegExp() const {
	return QString("<img src=\"%1\" />").arg(_path);
}

bool QtEmoticon::isNull() const {
	return _text.isEmpty();
}

void QtEmoticon::setRegExp(const QString & regExp) {
	_regExp = regExp;
}

QtEmoticon & QtEmoticon::operator=(const QtEmoticon & source) {
	_path = source._path;
	_text = source._text;
	_regExp = source._regExp;
	_pixmap = source._pixmap;
	_buttonPixmap = source._buttonPixmap;
	return *this;
}
