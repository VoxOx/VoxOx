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

#ifndef PURPLEIMCHAT_H
#define PURPLEIMCHAT_H

#include <imwrapper/IMChat.h>
#include <thread/Mutex.h>

class IMAccount;
class IMChatSession;
class IMContactSet;

class PurpleIMChat;

extern "C" {
#include <libpurple/connection.h>
}

//=============================================================================

typedef struct mConvInfo_s
{
	IMChatSession		*conv_session;			//VOXOX - JRT - 2009.07.09 - Let's make this USEABLE!
	int					 conv_id;				//VOXOX - JRT - 2009.07.09 - Typically just in-memory address of conv_session.
	PurpleConversation	*purple_conv_session;	//VOXOX - JRT - 2009.07.09 - Let's make this USEABLE!
	GList				*pending_invites;		//VOXOX - JRT - 2009.07.09 
} mConvInfo_t;

//=============================================================================

class PurpleIMChatCallbackData
{
public:
	PurpleIMChatCallbackData()						{ initVars();	}
	
	virtual ~PurpleIMChatCallbackData()	
	{
		if ( _imContactSet )
		{
			delete _imContactSet;
			_imContactSet = NULL;
		}
	}

	PurpleIMChat*		getPurpleIMChat()  const				{ return _chat;				}
	int					getCallbackId()    const				{ return _callbackId;		}
	IMContactSet*		getIMContactSet()  const				{ return _imContactSet;		}
	IMChatSession*		getIMChatSession() const				{ return _imChatSession;	}
	std::string			getMessage()	   const				{ return _message;			}
	std::string			getContactId()	   const				{ return _contactId;		}
	IMChat::TypingState getTypingState()   const				{ return _typingState;		}

	void setPurpleIMChat ( PurpleIMChat*		 val )			{ _chat			 = val;	}
	void setCallbackId   ( int					 val )			{ _callbackId	 = val;	}
	void setIMContactSet ( IMContactSet*		 val )			{ _imContactSet  = val;	}
	void setIMChatSession( IMChatSession*		 val )			{ _imChatSession = val;	}
	void setMessage      ( const std::string&	 val )			{ _message		 = val;	}
	void setContactId    ( const std::string&	 val )			{ _contactId	 = val;	}
	void setTypingState  ( IMChat::TypingState	 val )			{ _typingState   = val;	}

protected:
	void initVars()
	{
		setPurpleIMChat ( NULL );
		setCallbackId   ( 0 );
		setIMContactSet ( NULL );
		setIMChatSession( NULL );
		setMessage      ( "" );
		setContactId    ( "" );
		setTypingState  ( IMChat::TypingStateNotTyping );
	}

private:
	PurpleIMChat*		_chat;
	int					_callbackId;
	IMContactSet*		_imContactSet;
	IMChatSession*		_imChatSession;
	std::string			_message;
	std::string			_contactId;
	IMChat::TypingState	_typingState;
};

//=============================================================================

/**
 * Purple IM chat.
 *
 * @ingroup model
 * @author Julien Bossart
 */
class PurpleIMChat : public IMChat
{
	friend class PurpleIMFactory;

public:
	PurpleIMChat(IMAccount account);

	virtual void createSession(const IMContactSet & imContactList, IMChat::IMChatType imChatType, const std::string& userChatRoomName );	//VOXOX - JRT - 2009.06.15 

	virtual void createChatToEmailSession	(const IMContactSet & imContactList, const std::string & relatedContactId);
	virtual void createChatToSMSSession		(const IMContactSet & imContactList, const std::string & relatedContactId);
	virtual void createSessionAndSendMessage(const IMContactSet & imContactList, const std::string & message);

	virtual void callSkypeContact(const IMContactSet & imContactList);

	virtual void closeSession	  (IMChatSession & chatSession);
	virtual void sendMessage	  (IMChatSession & chatSession, const std::string & message);
	virtual void changeTypingState(IMChatSession & chatSession, TypingState state);
	virtual void addContact		  (IMChatSession & chatSession, const std::string & contactId);
	virtual void removeContact	  (IMChatSession & chatSession, const std::string & contactId);


	static bool createSessionCbk(void * data);

	static bool createSessionAndSendMessageCbk(void * data);
	static bool createChatToEmailSessionCbk(void * data);
	static bool createChatToSMSSessionCbk(void * data);

	static bool callSkypeContactCbk(void * data);

	static bool closeSessionCbk(void * data);
	static bool sendMessageCbk(void * data);
	static bool changeTypingStateCbk(void * data);
	static bool addContactCbk(void * data);
	static bool removeContactCbk(void * data);

	static mConvInfo_t *CreateChatSession(bool userCreated, PurpleIMChat & imChat);
	static void createPurpleChat(PurpleConnection *gGC, int id, GList *users, PurpleIMChat & imChat, PurpleAccount* gAccount); //VOXOX - JRT - 2009.07.10 

	/* ************ MANAGE CHAT SESSION LIST ************ */
	static std::list<mConvInfo_t *> _PurpleChatSessionList;
	typedef std::list<mConvInfo_t *>::iterator PurpleChatSessionIterator;
	static void AddChatSessionInList(mConvInfo_t *conv);
	static void RemoveChatSessionFromList(int convId);
	static bool IsChatSessionInList(int convId);
	static mConvInfo_t *FindChatStructById(int convId);
	static mConvInfo_t *FindChatStructByName( const char* chatName );	//VOXOX - JRT - 2009.06.19 

	/* ************************************************** */

	//VOXOX - JRT - 2009.08.21 - These should be in a base class
	static PurpleAccount* getPurpleAccount( const IMAccount& imAccount );
		   int			  timeoutAdd      ( void* cbData, GSourceFunc callbackFunc );
	static void			  timeoutRemove   ( PurpleIMChatCallbackData* cbData );


	static Mutex _mutex;
};

#endif	//PURPLEIMCHAT_H
