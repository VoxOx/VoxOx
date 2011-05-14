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

#ifndef OWQTCHATHISTORYWIDGET_H
#define OWQTCHATHISTORYWIDGET_H

#include <imwrapper/EnumIMProtocol.h>

#include <QtCore/QTime>
#include <QtCore/QDate>
#include <QtCore/QUrl>
#include <QtGui/QTextBrowser>

#include <QtWebKit/QtWebKit>

#include <presentation/qt/chat/QtChatTheme.h>

class QShortcut;

/**
 * Implements the chat text zone with a custom context menu.
 *
 * @author Nicolas Couturier
 * @author Mathieu Stute
 * @author Xavier Desjardins
 */
class QtChatHistoryWidget : public QWebView {
	Q_OBJECT
public:

	QtChatHistoryWidget(QWidget * parent);

	~QtChatHistoryWidget();

	void init(const QString & contactId, const QString & senderName, const QString & accountId, const QTime & time, const QDate & date);

	void setTheme(const QString & themeDir,const QString & themeVariant);
	
	void insertWithJavascriptNext(QString & html);

	void clear();

	void insertMessage(const QString & contactId, const QString & senderName, const QString & protocolName, const QString & message, const QTime & time, const QDate & date);

	void insertTranslatedMessage(const QString & contactId, const QString & senderName, const QString & protocolName, const QString & message, const QString & translatedMessage, const QTime & time, const QDate & date);//VOXOX - CJC - 2010.01.19 
	
	void insertStatusMessage(const QString & statusMessage, const QTime& time);
	

	QStringList getSelectedThemeVariantList(const QString & selectedTheme);

	QStringList getThemeList();

	void saveHistoryAsHtml();

	void setAvatarPixmap(const QString& name, const QString data);

	void setMainStyle(const QString & stylePath);

private Q_SLOTS:

	/**
	 * An URL has been clicked we must open a web browser
	 *
	 * @param link the clicked url
	 */
	void urlClicked(const QUrl & link);





	

	void copySlot();
	
private:

	void initHeader(const QString & contactId, const QString & senderName, const QString & accountId, const QTime & time, const QDate & date);

	void appendHTML(const QString & html);

	void addMessage(const QString& html, const QtChatTheme::Position& pos = QtChatTheme::First);

	void appendMessage(const QString & html);

	void appendNextMessage(const QString & html);

	EnumIMProtocol::IMProtocol _protocol;

	QString _lastSenderName;

	QTime _lastMessageTime;

	QMap <QString, QString> _avatarMap;

	QtChatTheme* _theme;

	QShortcut * _copy;
	
};

#endif	//OWQTCHATHISTORYWIDGET_H
