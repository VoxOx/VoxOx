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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtChatHistoryWidget.h"


#include <presentation/qt/chat/QtChatUtils.h>

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/profile/AvatarList.h>

#include <control/chat/CChatHandler.h>
#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/Account.h>

#include <util/Logger.h>
#include <util/WebBrowser.h>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTime>

#include <QtGui/QtGui>

#include <qtutil/SafeConnect.h>

#include <util/Base64.h>
#include <util/SafeDelete.h>

// If previous message from last sender is more than MAX_SECONDS_BETWEEN_CONSECUTIVE_MESSAGES
// old, don't group new message with it.
static int MAX_SECONDS_BETWEEN_CONSECUTIVE_MESSAGES = 3 * 60;

QtChatHistoryWidget::QtChatHistoryWidget(QWidget * parent) : QWebView(parent) {
	//setLineWrapMode(QTextEdit::WidgetWidth);

	
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	String themeDir = config.getChatTheme();
	String variant = config.getChatThemeVariant();
	variant.decodeFromXMLSpecialCharacters();

	_theme = new QtChatTheme(this);
	setTheme(QString::fromUtf8(themeDir.c_str()),QString::fromUtf8(variant.c_str()));


	//VOXOX - CJC - 2009.05.06 Force Keyboard copyng
	_copy = new QShortcut(QKeySequence::Copy,this);
	SAFE_CONNECT(_copy, SIGNAL(activated()), SLOT(copySlot()));

	page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
	page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);

	page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);

	SAFE_CONNECT(this, SIGNAL(linkClicked(const QUrl &)), SLOT(urlClicked(const QUrl &)));
	
	//QWebSettings *defaultSettings = QWebSettings::globalSettings();
	//
	////Add support for flash and other pluggins
	//defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);
	//defaultSettings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
	//defaultSettings->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, false);
	//defaultSettings->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, false);
	//defaultSettings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, false);
	//
	//
	////Add support for flash and other pluggins
	//defaultSettings->setObjectCacheCapacities(1024,0,1024);
	//defaultSettings->clearIconDatabase(); 
	

	//clear();
}

QtChatHistoryWidget::~QtChatHistoryWidget() {
	/*QWebSettings *defaultSettings = QWebSettings::globalSettings();
	defaultSettings->clearIconDatabase(); */
	OWSAFE_DELETE(_copy);
	OWSAFE_DELETE(_theme);
	
}
//VOXOX - CJC - 2009.05.06 Force to copy with keyboard
void QtChatHistoryWidget::copySlot() {

	// VOXOX -ASV- 08-26-2009: we set the qstring text to the clipboard instead of 
	// using the triggerAction so we remove the style from the font
	//page()->triggerAction(QWebPage::Copy);  	
	 QApplication::clipboard()->setText(page()->selectedText());

}

void QtChatHistoryWidget::setTheme(const QString& themeDir,const QString & themeVariant) {
	_theme->load(themeDir,themeVariant);

}

void QtChatHistoryWidget::init(const QString & contactId, const QString & senderName, const QString & accountId, const QTime & time, const QDate & date){

	initHeader(contactId,senderName,accountId,time,date);
	
	_theme->initTemplate();

	setHtml(_theme->getTemplate(), QUrl(_theme->getThemePath()));

}


void QtChatHistoryWidget::initHeader(const QString & contactId, const QString & senderName, const QString & accountId, const QTime & time, const QDate & date){

	QString html = _theme->getHeader();
	if(html!=""){

		QRegExp regExpFindTimeBrackets("timeOpened\\{[^\\}]*\\}");
			//We found something
			if(regExpFindTimeBrackets.indexIn(html) != -1){
				html = html.replace(regExpFindTimeBrackets,QString("timeOpened"));
			}

		_theme->setKeywordValue(html, "sourceName", accountId);
		_theme->setKeywordValue(html, "destinationName", senderName);
		_theme->setKeywordValue(html, "destinationName", senderName);
		_theme->setKeywordValue(html, "destinationDisplayName", senderName);
		_theme->setKeywordValue(html, "chatName", senderName);
		
		_theme->setKeywordValue(html, "timeOpened", time.toString("HH:mm:ss AP"));
		QString data;
		QString dataself;
		if (!_avatarMap.contains(contactId)) {
			data = QString::fromStdString(AvatarList::getInstance().getDefaultContactAvatarPicture(QtEnumIMProtocol::IMProtocolUnknown).getData());
			dataself = QString::fromStdString(AvatarList::getInstance().getDefaultContactAvatarPicture(QtEnumIMProtocol::IMProtocolUnknown).getData());
		}else{
			data = _avatarMap[contactId];
			dataself = _avatarMap[QString("self")];
		}

		_theme->setKeywordValue(html, "outgoingIconPath", QString("data:image/x-icon;base64,") + QString::fromStdString(Base64::encode(dataself.toStdString())));
	
		_theme->setKeywordValue(html, "incomingIconPath", QString("data:image/x-icon;base64,") + QString::fromStdString(Base64::encode(data.toStdString())));
	
		_theme->initHeader(html);
	}
}


void QtChatHistoryWidget::saveHistoryAsHtml() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString contentToSave = page()->currentFrame()->toHtml();

	QString filePath = QFileDialog::getSaveFileName(this, tr("Save As"), 
		QString::fromStdString(config.getLastChatHistorySaveDir()), "HTML (*.htm *.html)");

	if(filePath.length() > 0) {

		if(!filePath.endsWith(QString(".htm"), Qt::CaseInsensitive) && 
			!filePath.endsWith(QString(".html"), Qt::CaseInsensitive)) {

			filePath.append(QString(".html"));
		}

		QFile fileToSave(filePath);
		config.set(Config::LAST_CHAT_HISTORY_SAVE_DIR_KEY, QFileInfo(fileToSave).absolutePath().toStdString());
		fileToSave.open(QIODevice::WriteOnly);
		fileToSave.write(contentToSave.toStdString().c_str(), (long long)contentToSave.length());
		fileToSave.close();
	}
}

void QtChatHistoryWidget::urlClicked(const QUrl & link) {
	//setSource(QUrl(QString::null));
	QString urlToOpen = link.toString();//VOXOX CHANGE by Rolando - 2009.10.21
	if(!urlToOpen.isEmpty()){//VOXOX CHANGE by Rolando - 2009.10.21 
		WebBrowser::openUrl(urlToOpen.toStdString());//VOXOX CHANGE by Rolando - 2009.10.21
	}	
}


void QtChatHistoryWidget::insertMessage(const QString & contactId, const QString & senderName,const QString & protocolName, const QString & message, const QTime & time, const QDate & date) {
	
	// Prepare message
	QString htmlMessage = QtChatUtils::decodeMessage(message);

	// Get html
	QtChatTheme::Position position;
	int timeDelta = _lastMessageTime.secsTo(time);
	if (_lastSenderName == senderName && timeDelta < MAX_SECONDS_BETWEEN_CONSECUTIVE_MESSAGES) {
		position = QtChatTheme::Next;
	} else {
		position = QtChatTheme::First;
		_lastSenderName = senderName;
	}
	_lastMessageTime = time;

	QtChatTheme::Direction direction;
	if (contactId == "self") { // FIXME: do not duplicate "self"
		direction = QtChatTheme::Outgoing;
	} else {
		direction = QtChatTheme::Incoming;
	}

	QString html = _theme->getContent(direction, position);

	//VOXOX CHANGE CJC replace time brackets, we dont use this
	QRegExp regExpFindTimeBrackets("time\\{[^\\}]*\\}");
			//We found something
			if(regExpFindTimeBrackets.indexIn(html) != -1){
				html = html.replace(regExpFindTimeBrackets,QString("time"));
	}

	_theme->setKeywordValue(html, "senderId", QtChatUtils::fixHTMLforJavascript(contactId));
	_theme->setKeywordValue(html, "sender",  QtChatUtils::fixHTMLforJavascript(senderName));
	_theme->setKeywordValue(html, "senderScreenName", QtChatUtils::fixHTMLforJavascript(senderName));
	_theme->setKeywordValue(html, "senderDisplayName", QtChatUtils::fixHTMLforJavascript(senderName));

	_theme->setKeywordValue(html, "time", time.toString("HH:mm:ss AP"));
	_theme->setKeywordValue(html, "shortTime", time.toString("HH:mm AP"));
	_theme->setKeywordValue(html, "timeDate", QString(date.toString("d/MM/yy"))+" "+QString(time.toString("HH:mm AP")));
	_theme->setKeywordValue(html, "message", QtChatUtils::fixHTMLforJavascript(htmlMessage));
	_theme->setKeywordValue(html, "service", QtChatUtils::fixHTMLforJavascript(protocolName));

	
	if (!_avatarMap.contains(contactId)) {
		std::string data = AvatarList::getInstance().getDefaultContactAvatarPicture(QtEnumIMProtocol::IMProtocolUnknown).getData();	//VOXOX - JRT - 2009.06.11 
		_theme->setKeywordValue(html, "userIconPath", QString("data:image/x-icon;base64,") + QString::fromStdString(Base64::encode(data)));
		_avatarMap[contactId] = QString::fromStdString(data);
	}else{
		QString data = _avatarMap[contactId];
		_theme->setKeywordValue(html, "userIconPath", QString("data:image/x-icon;base64,") + QString::fromStdString(Base64::encode(data.toStdString())));
	}

	addMessage(html,position);

	
}

void QtChatHistoryWidget::insertTranslatedMessage(const QString & contactId, const QString & senderName,const QString & protocolName, const QString & message, const QString & translatedMessage, const QTime & time, const QDate & date) {
	
	// Prepare message
	QString htmlOriginalMessage = QtChatUtils::decodeMessage(message);

	QString htmlTranslatedMessage = QtChatUtils::decodeMessage(translatedMessage);


	

	QString translatedHTML  =  QString("<div style=\"display:inline-block\"><div onclick=\"document.getElementById('%1').style.display = (document.getElementById('%1').style.display == 'none') ? 'block' : 'none'\">%2</div><div id=\"%1\" style=\"display:none\" onclick=\"document.getElementById('%1').style.display = (document.getElementById('%1').style.display == 'none') ? 'block' : 'none'\">%3</div></div>").arg(QUuid::createUuid().toString()).arg(htmlOriginalMessage).arg(htmlTranslatedMessage);
	insertMessage(contactId,senderName,protocolName,translatedHTML,time,date);
	
}
void QtChatHistoryWidget::insertStatusMessage(const QString & message, const QTime& time) {

	QString html = _theme->getStatusMessage();
	_theme->setKeywordValue(html, "message", QtChatUtils::fixHTMLforJavascript(message));
	_theme->setKeywordValue(html, "time", time.toString("HH:mm"));
	_theme->setKeywordValue(html, "shortTime", time.toString("HH:mm"));
	_lastSenderName = ""; //VOXOX - CJC - 2009.09.17 Force to be a new message after adding a status message
	appendMessage(html);
}

void QtChatHistoryWidget::setAvatarPixmap(const QString& name, const QString  data) {
	_avatarMap[name] = data;
}


void QtChatHistoryWidget::addMessage(const QString& html,const QtChatTheme::Position & pos) {

	if(pos == QtChatTheme::Next){
		appendNextMessage(html);
	}else{
		appendMessage(html);
	}
}


void QtChatHistoryWidget::appendHTML(const QString & html){

	QString script = QString("appendHTML('%1');").arg(html);
	
	QVariant log = page()->mainFrame()->evaluateJavaScript(script);
	
	LOG_DEBUG(log.toString().toStdString());

}

void QtChatHistoryWidget::appendMessage(const QString & html){

	QString script = QString("appendMessage('%1');").arg(html);
	
	QVariant log = page()->mainFrame()->evaluateJavaScript(script);
	
	LOG_DEBUG(log.toString().toStdString());

}


void QtChatHistoryWidget::appendNextMessage(const QString & html){

	QString script = QString("appendNextMessage('%1');").arg(html);
	
	QVariant log = page()->mainFrame()->evaluateJavaScript(script);
	
	LOG_DEBUG(log.toString().toStdString());

}


void QtChatHistoryWidget::setMainStyle(const QString & styleName){
	
	_theme->setVariantCss(styleName);

	QString script = QString("setStylesheet('mainStyle', '%1');").arg(_theme->getSelectedVariantPath());
	
	QVariant log = page()->mainFrame()->evaluateJavaScript(script);
	
	LOG_DEBUG(log.toString().toStdString());
	
}




void QtChatHistoryWidget::clear() {

	setHtml("", QUrl(_theme->getThemePath()));

}


QStringList QtChatHistoryWidget::getSelectedThemeVariantList(const QString & selectedTheme){

	return _theme->getSelectedThemeVariantList(selectedTheme);
}

QStringList QtChatHistoryWidget::getThemeList(){

	return _theme->getThemeList();
}
