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




#ifndef OWQTUMTRANSLATIONREQUEST_H
#define OWQTUMTRANSLATIONREQUEST_H

#include <QtGui/QWidget>	//VOXOX - JRT - 2009.09.09
#include <util/Trackable.h>


class QtUMTranslationRequest:public QObject, public Trackable{
	Q_OBJECT
public:


	enum TranslationRequestMode { SendMessage,RecieveMessage};

	QtUMTranslationRequest(const QString & contactId, const QString & senderName, const QString & protocolName,const QString & originalMessage,TranslationRequestMode mode);

	~QtUMTranslationRequest();

	QString getKey(){return _key;}

	QString getTranslatedMessage(){return _translatedMessage;}

	QString getOriginalMessage(){return _originalMessage;}

	TranslationRequestMode getTranslationRequestMode();

	QString getContactId(){return _contactId;}

	QString getSenderName(){return _senderName;}

	QString getProtocolName(){return _protocolName;}

	bool succeeded(){return _success;}

	void setKey(QString key){_key = key;}

	void setTranslatedMessage(QString translatedMessage){_translatedMessage = translatedMessage;}

	void setOriginalMessage(QString originalMessage){_originalMessage = originalMessage;}

	void setTranslationRequestMode(TranslationRequestMode mode);

	void setSucceeded(bool success){_success =  success;}

	void setContactId(QString contactId){_contactId = contactId;}

	void setSenderName(QString senderName){_senderName = senderName;}

	void setProtocolName(QString protocolName){_protocolName = protocolName;}



private:

	QString _key;

	QString _translatedMessage;

	QString _originalMessage;

	QString _contactId;

	QString _senderName;

	QString _protocolName;

	TranslationRequestMode _mode;

	bool _success;


};

#endif	//OWQtUMTranslationRequest_H
