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

#ifndef OWQTFLAG_H
#define OWQTFLAG_H

#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtGui/QPixmap>

/**
 *
 * @ingroup presentation
 * @author Mr K.
 */
class QtFlag: public QObject  {
Q_OBJECT
public:

	QtFlag(int width = 16, int height = 11);

	QtFlag(const QtFlag & source);

	~QtFlag();

	void setCountryName(const QStringList & countryName) { _countryName = countryName; }
	
	void setCountryCode(const QString & countryCode) { _countryCode = countryCode; }
	
	void setPath(const QString & path) { _path = path; }
	
	void setFlagName(const QString & flagName) { _flagName = flagName; }

	void setPixmap(const QPixmap & pixmap) { _pixmap = pixmap; }

	void setButtonPixmap(const QPixmap & pixmap) { _buttonPixmap = pixmap; }

	void setRegExp(const QString & regExp);

	QString getRegExp() const { return _regExp; }
	
	QString getCountryCode() const { return _countryCode;}
	
	QStringList getCountryName() const { return _countryName;}

	QString getDefaultCountryCode() const { return _countryCode; }
	
	QString getDefaultCountryName() const { return _countryName[0]; }

	QPixmap getPixmap() const { return _pixmap; }

	QPixmap getButtonPixmap() const { return _buttonPixmap; }

	QString getPath() const { return _path; }
	
	QString getFlagNameFile() const { return _flagName; }

	QString getHtmlRegExp() const;

	QString getHtml() const;

	bool isNullCountryCode() const;
	
	bool isNullCountryName() const;

	QtFlag & operator=(const QtFlag & source);

private:

	QString _path;
	
	QString _flagName;
	
	QStringList _countryName;
	QString _countryCode;

	QString _regExp;

	QPixmap _pixmap;

	QPixmap _buttonPixmap;
};

#endif	//OWQTFLAG_H
