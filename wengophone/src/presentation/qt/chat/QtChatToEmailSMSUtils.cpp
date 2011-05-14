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
* Helper Class for Chat to Email, Chat to SMS needs
* @author Chris Jimenez C 
* @date 2009.05.11
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtChatToEmailSMSUtils.h"
#include <QtXml/QtXml>
#include <util/Logger.h>


const QtChatToEmailSMSUtils::Type QtChatToEmailSMSUtils::checkMesageType(const QString & message)
{

	QDomDocument doc("pushedInfo");
	QString fixedMessage = fixXML(message);
	QString ErrorMsg;
	if (!doc.setContent(fixedMessage, &ErrorMsg, 0, 0)) {
		LOG_DEBUG(ErrorMsg.toStdString());
		return QtChatToEmailSMSUtils::Unknown;
	}

	QDomElement tmpElement;
	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	QString tagName;

	while (!n.isNull()) {
		tmpElement = n.toElement();
		if (!tmpElement.isNull()) {
			tagName = tmpElement.tagName();
			if (tagName == "pushedInfo") {
				QDomNode n1 = n.firstChild();
				while (!n1.isNull()) {
					QDomElement e1 = n1.toElement();
					if (!e1.isNull()) {
						if (e1.tagName() == "type") {
							if(e1.text() == "email"){
								return QtChatToEmailSMSUtils::ChatToEmail;
							}else if(e1.text() == "sms"){
								return QtChatToEmailSMSUtils::ChatToSMS;
							}
						}
					}
					n1 = n1.nextSibling();
				}
			}
		}
		n = n.nextSibling();
	}

	return QtChatToEmailSMSUtils::Unknown;

}


const QtChatToEmailMessageData QtChatToEmailSMSUtils::getChatToMailMessageData(const QString & message)
{
	QtChatToEmailMessageData data;
	data.setFrom("");
	QDomDocument doc("pushedInfo");
	QString fixedMessage = fixXML(message);
	QString ErrorMsg;
	if (!doc.setContent(fixedMessage, &ErrorMsg, 0, 0)) {
		LOG_DEBUG(ErrorMsg.toStdString());
		
		return data;
	}

	QDomElement tmpElement;
	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	QString tagName;

	while (!n.isNull()) {
		tmpElement = n.toElement();
		if (!tmpElement.isNull()) {
			tagName = tmpElement.tagName();
			if (tagName == "pushedInfo") {
				return readPushedEmailInfo(n);
			}
		}
		n = n.nextSibling();
	}

	return data;
	

}

const QtChatToSMSMessageData QtChatToEmailSMSUtils::getChatToSMSMessageData(const QString & message)
{
	QtChatToSMSMessageData data;
	data.setFrom("");
	QDomDocument doc("pushedInfo");
	QString fixedMessage = fixXML(message);
	QString ErrorMsg;
	if (!doc.setContent(fixedMessage, &ErrorMsg, 0, 0)) {
		LOG_DEBUG(ErrorMsg.toStdString());
		
		return data;
	}

	QDomElement tmpElement;
	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	QString tagName;

	while (!n.isNull()) {
		tmpElement = n.toElement();
		if (!tmpElement.isNull()) {
			tagName = tmpElement.tagName();
			if (tagName == "pushedInfo") {
				return readPushedSMSInfo(n);
			}
		}
		n = n.nextSibling();
	}

	return data;
	

}

//VOXOX - CJC - 2009.06.15 
const QtChatToEmailMessageData QtChatToEmailSMSUtils::readPushedEmailInfo(const QDomNode & node) {
	QtChatToEmailMessageData data;
	data.setFrom("");
	QDomNode n = node.firstChild();
	QDomElement element;
	QString attributeName;
	while (!n.isNull()) {
		if (!n.toElement().isNull()) {
			if (n.toElement().tagName() == "contents") {
				element = node.toElement();
				if (!element.isNull()) {
					return readEmailContent(n);
				}
			}
		}
		n = n.nextSibling();
	}
	return data;
}
//VOXOX - CJC - 2009.06.15 
const QtChatToSMSMessageData QtChatToEmailSMSUtils::readPushedSMSInfo(const QDomNode & node) {
	QtChatToSMSMessageData data;
	data.setFrom("");
	QDomNode n = node.firstChild();
	QDomElement element;
	QString attributeName;
	while (!n.isNull()) {
		if (!n.toElement().isNull()) {
			if (n.toElement().tagName() == "contents") {
				element = node.toElement();
				if (!element.isNull()) {
					return readSMSContent(n);
				}
			}
		}
		n = n.nextSibling();
	}
	return data;
}




const QtChatToEmailMessageData QtChatToEmailSMSUtils::readEmailContent(const QDomNode & node) {
	
	QDomNode n1 = node.firstChild();
	QStringList textList;
	QtChatToEmailMessageData data;
	data.setFrom("");
	while (!n1.isNull()) {
		QDomElement e1 = n1.toElement();
		if (!e1.isNull()) {
			if (e1.tagName() == "id") {
				data.setId(e1.text());
			}
			if (e1.tagName() == "from") {
				data.setFrom(e1.text());
			}
			if (e1.tagName() == "subject") {
				data.setSubject(e1.text());
			}
			if (e1.tagName() == "body") {
				data.setBody(e1.text());
			}
		
		}
		n1 = n1.nextSibling();
	}
	return data;
}


const QtChatToSMSMessageData QtChatToEmailSMSUtils::readSMSContent(const QDomNode & node) {
	
	QDomNode n1 = node.firstChild();
	QStringList textList;
	QtChatToSMSMessageData data;
	data.setFrom("");
	while (!n1.isNull()) {
		QDomElement e1 = n1.toElement();
		if (!e1.isNull()) {
			if (e1.tagName() == "id") {
				data.setId(e1.text());
			}
			if (e1.tagName() == "from") {
				data.setFrom(e1.text());
			}
			if (e1.tagName() == "body") {
				data.setBody(e1.text());
			}
		
		}
		n1 = n1.nextSibling();
	}
	return data;
}



//TODO: QT does NOT provide an unescape() but they do provide an escape().  Move this to QtUtil project.
const QString QtChatToEmailSMSUtils::fixXML(QString text)
{
    QString result = text;

	result = result.replace("&quot;", "\"");
	result = result.replace("&amp;",  "&" );
	result = result.replace("&gt;",   ">" );
	result = result.replace("&lt;",   "<" );
	result = result.replace("&apos;", "\'");
        
    return result;
}
