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
#ifndef QTCHATTHEME_H
#define QTCHATTHEME_H

#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtGui/QWidget>	//VOXOX - JRT - 2009.09.09

/**
 * Embryonic implementation of theme support. Severely limited by Qt HTML
 * renderer for now. Hope WebKit will save us.
 */
class QtChatTheme: public QObject{
	Q_OBJECT
public:

	QtChatTheme(QWidget * parent);

	virtual ~QtChatTheme();

	enum Direction {
		Incoming, Outgoing
	};

	enum Position {
		First, Next
	};

	QString getFooter() const;

	QString getHeader() const;

	QString loadFile(const QString& path);

	QString themeBaseDir();

	void initHeader(const QString & header);

	void setVariantCss(const QString & cssFile);

	QString getTemplate() const;

	QString getStatusMessage() const;

	QString getContent(Direction, Position) const;

	void initTemplate();//VOXOX - CJC - 2009.08.24 

	//QString getStyleSheet() const;

	QString getThemePath() const;

	QString getSelectedVariantPath() const;

	QString removeBreaklines(const QString & html);

	//VOXOX CHANGE CJC TO SUPPORT DIFERENT NEXT CONTENTS
	//bool getNextContentInsideContent() const;

	void load(const QString & themeDir,const QString & themeVariant);

	void setKeywordValue(QString & html, const QString & keyword, const QString & value);

	QStringList getThemeList();

	QStringList getSelectedThemeVariantList(QString selectedTheme);

private:
	QString _incomingContent;
	QString _incomingNextContent;
	QString _outgoingContent;
	QString _outgoingNextContent;
	QString _footer;
	QString _header;
	QString _template;
	QString _statusMessage;
	QString _styleSheet;

	//VOXOX CHANGE CJC TO SUPPORT DIFERENT NEXT CONTENTS
	bool _nextContentInsideContent;

	QString _themePath;

	QString _selectedVariantPath;
};

#endif // QTCHATUTILS_H
