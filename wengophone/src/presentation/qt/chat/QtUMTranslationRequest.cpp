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


#include "stdafx.h"		//VOXOX - JRT - 2009.04.05
#include "QtUMTranslationRequest.h"
#include <util/SafeDelete.h>
#include <QtCore/QUuid.h> //VOXOX -ASV- 2010.01.20


QtUMTranslationRequest::QtUMTranslationRequest(const QString & contactId, const QString & senderName, const QString & protocolName,const QString & originalMessage,QtUMTranslationRequest::TranslationRequestMode mode)
{
	QString key = QUuid::createUuid().toString();
	setKey(key);
	setContactId(contactId);
	setSenderName(senderName);
	setProtocolName(protocolName);
	setOriginalMessage(originalMessage);
	setTranslationRequestMode(mode);
	
}



QtUMTranslationRequest::~QtUMTranslationRequest() {

}


QtUMTranslationRequest::TranslationRequestMode QtUMTranslationRequest::getTranslationRequestMode(){

	return _mode;
}

void QtUMTranslationRequest::setTranslationRequestMode(QtUMTranslationRequest::TranslationRequestMode mode){

	_mode = mode;
}
