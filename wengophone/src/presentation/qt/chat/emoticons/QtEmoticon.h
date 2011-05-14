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

#ifndef OWQTEMOTICON_H
#define OWQTEMOTICON_H

#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtGui/QPixmap>

/**
 *
 * @ingroup presentation
 * @author Mr K.
 */
class QtEmoticon {
public:

	QtEmoticon();

	QtEmoticon(const QtEmoticon & source);

	~QtEmoticon();

	void setName(const QString & name) { _name = name; }

	void setText(const QStringList & text) { _text = text; }

	void setPath(const QString & path) { _path = path; }

	void setPixmap(const QPixmap & pixmap) { _pixmap = pixmap; }

	void setButtonPixmap(const QPixmap & pixmap) { _buttonPixmap = pixmap; }

	void setRegExp(const QString & regExp);

	QString getRegExp() const { return _regExp; }

	QStringList getText() const { return _text;}

	QString getDefaultText() const { return _text[0]; }

	QPixmap getPixmap() const { return _pixmap; }

	QPixmap getButtonPixmap() const { return _buttonPixmap; }

	QString getPath() const { return _path; }

	QString getName() const { return _name; }

	QString getHtmlRegExp() const;

	QString getHtml() const;

	bool isNull() const;

	QtEmoticon & operator=(const QtEmoticon & source);

private:

	QString _name;

	QString _path;

	QStringList _text;

	QString _regExp;

	QPixmap _pixmap;

	QPixmap _buttonPixmap;
};

#endif	//OWQTEMOTICON_H
