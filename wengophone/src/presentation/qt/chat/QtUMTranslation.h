/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**


* CLASS DESCRIPTION 
* @author Chris Jimenez C 
* @date 2010.01.11
*/




#ifndef OWUMTRANSLATION_H
#define OWUMTRANSLATION_H

#include "QtUMTranslationRequest.h"
#include "translation/QtEnumTranslationMode.h"
#include "translation/QtTranslationLanguage.h"

#include <model/webservices/translation/WsTranslation.h>
#include <control/chat/CChatHandler.h>

class QtUMTranslation: public QObject, public Trackable{
	Q_OBJECT
public:

	QtUMTranslation(CChatHandler & cChatHandler);

	~QtUMTranslation();

	bool isTranslationOn();

	bool validateTranslationSettings();

	bool isTranslationForSendingMessages();

	bool isTranslationForReceivingMessages();

	void setTranslationDestLocale(QtTranslationLanguage destLocale);

	void setTranslationSourceLocale(QtTranslationLanguage sourceLocale);

	void setTranslationMode(QtEnumTranslationMode::Mode mode);

	QtTranslationLanguage getTranslationDestLocale();

	QtTranslationLanguage getTranslationSourceLocale();

	QtEnumTranslationMode::Mode getTranslationMode();

	void translateRecievedMessage(const QString & contactId, const QString & senderName, const QString & protocolName,const QString & message);

	void translateAndSendMessage(const QString & contactId, const QString & senderName, const QString & protocolName, const QString & message);

	void initTranslationSettings(const QString & contactId);

	void saveTranslationSettings();

Q_SIGNALS:

	void translateMessageSignal(const QString & contactId, const QString & senderName, const QString & protocolName,const QString & originalText,const QString & translatedText, bool success, bool sendMessageToSession);

	void translateErrorSignal();

	
//protected:
//
//
//private Q_SLOTS:



private:


	QtTranslationLanguage _translationSourceLocate;

	QtTranslationLanguage _translationDestLocate;

	QtEnumTranslationMode::Mode _mode;

	typedef std::map < QString, QtUMTranslationRequest * > UMTranslationRequestMap;

	UMTranslationRequestMap _umRequestMap;

	void removeTranslationRequest(const QString & key);

	QtUMTranslationRequest * getTranslationRequest(const QString & key);
	

	WsTranslation * _wsTranslation;

	CChatHandler & _cChatHandler;

	void messageTranslated(WsTranslation & sender,const std::string id,const std::string originalText,
			const std::string & translatedText,bool success);

	void processTranlatedMessage(const std::string id,const std::string originalText,
			const std::string & translatedText,bool success);

	QtTranslationLanguage getLanguage(const QString & text);

	QtEnumTranslationMode::Mode getMode(const QString & text);

	QString _contactId;




};

#endif	//OWQtUMTranslation_H
