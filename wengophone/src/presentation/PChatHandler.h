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

#ifndef OWPCHATHANDLER_H
#define OWPCHATHANDLER_H

#include <presentation/Presentation.h>
#include <string>

class IMChatSession;
class IMContact;
class GroupChatInfo;



/**
 *
 * @ingroup presentation
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class PChatHandler : public Presentation {
public:

	/**
	 * @see ChatHandler::newIMChatSessionCreatedEvent
	 */
	virtual void newIMChatSessionCreatedEvent(IMChatSession & imChatSession) = 0;

	virtual void newSendMessageCreatedEvent(IMChatSession & imChatSession, std::string & message) = 0;//VOXOX - CJC - 2009.10.14 

	//VOXOX - CJC - 2009.05.07 Add support for chat to email messages
	virtual void newIMChatToEmailSessionCreatedEvent(IMChatSession & imChatSession, std::string & email) = 0;

	//VOXOX - CJC - 2009.05.19 Chat to sms
	virtual void newIMChatToSMSSessionCreatedEvent(IMChatSession & imChatSession, std::string & number) = 0;

	//VOXOX - CJC - 2009.05.27 
	virtual void newSendFileCreatedEvent(IMContact & imContact) = 0;
	
	//VOXOX - CJC - 2009.05.27 
//	virtual void newContactProfileCreatedEvent(IMContact & imContact) = 0;	//VOXOX - JRT - 2009.06.30 
	//VOXOX - CJC - 2009.06.03 
	virtual void newInitChatCreatedEvent() = 0;

	virtual void deletePresentation() = 0;

	virtual void newSendFaxCreatedEvent(std::string & faxNumber) = 0;

	virtual void incomingGroupChatInviteEvent( GroupChatInfo& gcInfo ) = 0;	//VOXOX - JRT - 2009.07.10 

};

#endif	//OWPCHATHANDLER_H
