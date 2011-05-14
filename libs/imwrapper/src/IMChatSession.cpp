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

#include <imwrapper/IMChatSession.h>

#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/QtEnumIMProtocol.h>
#include <imwrapper/IMContact.h>
#include <imwrapper/IMAccount.h>

#include <util/Logger.h>
#include <util/String.h>

using namespace std;

IMChatSession::IMChatMessage::IMChatMessage(const IMContact & imContact, const string & message)
	: _imContact(imContact),
	_message(message) 
{
}

IMChatSession::IMChatMessage::~IMChatMessage() 
{
}

//=============================================================================

IMChatSession::IMChatSession(IMChat & imChat, bool userCreated)
	: _imChat(imChat),
	_userCreated(userCreated) 
{
	_imChatType = IMChat::Chat;	//VOXOX - JRT - 2009.07.09 

	_imChat.messageReceivedEvent		+= boost::bind(&IMChatSession::messageReceivedEventHandler,		  this, _1, _2, _3, _4);
	_imChat.statusMessageReceivedEvent	+= boost::bind(&IMChatSession::statusMessageReceivedEventHandler, this, _1, _2, _3, _4);
	_imChat.typingStateChangedEvent		+= boost::bind(&IMChatSession::typingStateChangedEventHandler,	  this, _1, _2, _3, _4);
	_imChat.contactAddedEvent			+= boost::bind(&IMChatSession::contactAddedEventHandler,		  this, _1, _2, _3);
	_imChat.contactRemovedEvent			+= boost::bind(&IMChatSession::contactRemovedEventHandler,		  this, _1, _2, _3);

	_imChat.incomingGroupChatInviteEvent += boost::bind(&IMChatSession::incomingGroupChatInviteEventHandler, this, _1, _2 );//VOXOX - JRT - 2009.06.16 
}

IMChatSession::~IMChatSession() 
{
	//VOXOX - JRT - 2009.07.13 - Appears to be causing app freezes.  Use RecursiveMutex when ready to test. (it was Mutex)
//JRT-XXX	RecursiveMutex::ScopedLock scopedLock(_mutexMsgList);		//VOXOX - JRT - 2009.07.11 

	//VOXOX - ASV - 09-02-2009: adding a mutex to prevent a crash when voxox quits.
	//TODO: check if this makes the app hang when receiving an incoming message.
	RecursiveMutex::ScopedLock scopedLock(_mutexMsgList); 
	
	for (IMChatMessageList::const_iterator it = _receivedIMChatMessageList.begin(); it != _receivedIMChatMessageList.end(); ++it) 
	{
		delete (*it);
	}
}

void IMChatSession::close() 
{
	LOG_DEBUG("closing IMChatSession");
	imChatSessionWillDieEvent(*this);
	_imChat.closeSession(*this);
	delete this;
}

void IMChatSession::addIMContact(const IMContact & imContact) 
{
	Mutex::ScopedLock scopedLock(_mutex);

	//The IMContact must be of the same protocol of the IMAccount
	if (_imChat.getIMAccountId() == imContact.getIMAccountId()) 
	{
		LOG_DEBUG("adding a new IMContact=" + imContact.getContactId());
		_imChat.addContact(*this, imContact.getContactId());
	}
}

void IMChatSession::removeIMContact(const IMContact & imContact) 
{
	Mutex::ScopedLock scopedLock(_mutex);

	LOG_DEBUG("removing an IMContact=" + imContact.getContactId());
	_imChat.removeContact(*this, imContact.getContactId());
}

void IMChatSession::removeAllIMContact() 
{
	Mutex::ScopedLock scopedLock(_mutex);

	for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) 
	{
		_imChat.removeContact(*this, (*it).getContactId());
	}
}

void IMChatSession::sendMessage(const std::string & message) 
{
	_imChat.sendMessage(*this, message);
	messageSentEvent(*this, message);
}

void IMChatSession::changeTypingState(IMChat::TypingState state) 
{
	std::string message;
	//TODO: enum class for IMChat::TypingState
	switch (state) {
		case IMChat::TypingStateTyping:
			message = "typing";
			break;

		case IMChat::TypingStateStopTyping:
			message = "stop typing";
			break;

		default:
			message = "not typing";
			break;
	}

	_imChat.changeTypingState(*this, state);
}

void IMChatSession::messageReceivedEventHandler(IMChat & sender, IMChatSession & imChatSession, const std::string & contactId, const std::string & message) 
{
	if (imChatSession == *this) 
	{
		IMContact* imContact = NULL;

//		switch( sender.getIMChatType() )
		switch( imChatSession.getIMChatType() )	//VOXOX - JRT - 2009.07.09 
		{
		case IMChat::ChatGroup:			//VOXOX - JRT - 2009.07.09 
			imContact = _imContactSet.findBy( _imChat.getIMAccountId(), contactId );
			break;

		case IMChat::Chat:				//VOXOX - JRT - 2009.07.09 
		default:
			imContact = _imContactSet.findBy( _imChat.getIMAccountId(), contactId );
			break;
		}

		if ( imContact )
		{
			//VOXOX - JRT - 2009.07.13 - Appears to be causing app freezes.  Use RecursiveMutex when ready to test. (it was Mutex)
			RecursiveMutex::ScopedLock scopedLock(_mutexMsgList);		//VOXOX - JRT - 2009.07.20 
			_receivedIMChatMessageList.push_back(new IMChatMessage(*imContact, message));
			messageReceivedEvent(*this);
		} 
		else 
		{
			LOG_ERROR("this session does not know contact=" + contactId);
		}
	}
}

void IMChatSession::statusMessageReceivedEventHandler(IMChat & sender, IMChatSession & imChatSession, IMChat::StatusMessage status, const std::string & message) 
{
	if (imChatSession == *this) 
	{
		statusMessageReceivedEvent(*this, status, message);
	}
}

void IMChatSession::typingStateChangedEventHandler(IMChat & sender, IMChatSession & imChatSession, const std::string & contactId, IMChat::TypingState state) 
{
	if (imChatSession == *this) 
	{
		IMContact* imContact = _imContactSet.findBy( _imChat.getIMAccountId(), contactId );
		if ( imContact )
		{
			typingStateChangedEvent(*this, *imContact, state);
		} 
		else 
		{
			LOG_ERROR("this session does not know contact=" + contactId);
		}
	}
}

void IMChatSession::incomingGroupChatInviteEventHandler( IMChat& sender, GroupChatInfo& gcInfo )
{
	incomingGroupChatInviteEvent( *this, gcInfo );
}

bool IMChatSession::operator == (const IMChatSession & imChatSession) const 
{
	return (getId() == imChatSession.getId());
}

int IMChatSession::getId() const 
{
	return (long)this;
}

void IMChatSession::contactAddedEventHandler(IMChat & sender, IMChatSession & imChatSession, const IMContact & imContact) 
{
	Mutex::ScopedLock scopedLock(_mutex);

	if (imChatSession == *this) 
	{
		contactAddedEvent(*this, *_imContactSet.findByContact(imContact));
	}
}

void IMChatSession::contactRemovedEventHandler(IMChat & sender, IMChatSession & imChatSession, const std::string & contactId) 
{
	Mutex::ScopedLock scopedLock(_mutex);

	if (imChatSession == *this) 
	{
		IMContact* imContact = _imContactSet.findBy( _imChat.getIMAccountId(), contactId );
		if ( imContact )
		{
			contactRemovedEvent(*this, *imContact);
			//_imContactSet.erase(foundIMContact);
			LOG_DEBUG("IMContact=" + contactId + " removed (but in fact not) from IMContactList");
		}
	}
}

bool IMChatSession::canDoMultiChat() const 
{
	EnumIMProtocol::IMProtocol proto = _imChat._imAccount.getProtocol();

	return QtEnumIMProtocolMap::getInstance().canDoMultiChat( proto );
}

bool IMChatSession::isGroupChat()	//VOXOX - JRT - 2009.07.08 
{
//	return this->getIMChat().isGroupChat();
	return (getIMChatType() == IMChat::ChatGroup);
}


const IMChatSession::IMChatMessageList IMChatSession::getReceivedMessage(unsigned int fromIndex) 
{
	IMChatSession::IMChatMessageList result;

	if (_receivedIMChatMessageList.size() > 0) 
	{
		if (fromIndex < _receivedIMChatMessageList.size()) 
		{
			//VOXOX - JRT - 2009.07.13 - Appears to be causing app freezes.  Use RecursiveMutex when ready to test. (it was Mutex)
			RecursiveMutex::ScopedLock scopedLock(_mutexMsgList);		//VOXOX - JRT - 2009.07.20 

			for(unsigned int i = fromIndex; i < _receivedIMChatMessageList.size(); i++) 
			{
				result.push_back(_receivedIMChatMessageList.at(i));
			}
		}
	}

	return result;
}

void IMChatSession::setReceivedMessage(std::string & contact, std::string & message, Date date, Time time) 
{
	//TODO: convert to local time from (currently) PST/PDT.
	IMContact* temp = new IMContact(EnumIMProtocol::IMProtocolUnknown, contact);
	temp->setAlias(contact);
	IMChatSession::IMChatMessage* tobeadded = new IMChatMessage(*temp,message);
	tobeadded->setDate(date);
	tobeadded->setTime(time);

	//VOXOX - JRT - 2009.07.13 - Appears to be causing app freezes.  Use RecursiveMutex when ready to test. (it was Mutex)
	RecursiveMutex::ScopedLock scopedLock(_mutexMsgList);		//VOXOX - JRT - 2009.07.20 

	_receivedIMChatMessageList.push_back(tobeadded);
}

bool IMChatSession::isContactInSession(const IMContact & imContact) const 
{
	return (_imContactSet.findByContact( imContact ) != NULL);
}
