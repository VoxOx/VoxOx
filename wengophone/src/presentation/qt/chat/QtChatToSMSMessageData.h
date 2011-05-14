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
* CLASS That represent the content of a chat to email message
* @author Chris Jimenez C 
* @date 2009.05.11
*/

#ifndef OWQTCHATTOSMSMESSAGEDATA_H
#define OWQTCHATTOSMSMESSAGEDATA_H

#include <QtCore/QString>


class QtChatToSMSMessageData {
public:

	QString getId(){return _id;}
	QString getFrom(){return _from;}
	QString getBody(){return _body;}
	void setId(QString id){_id = id;}
	void setFrom(QString from){_from = from;}
	void setBody(QString body){_body = body;}

private:

	QString _id;
	QString _from;
	QString _body;


};

#endif	//OWQtChatToEmailSMSUtils_H
