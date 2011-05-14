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

#ifndef OWIMCHAT_H
#define OWIMCHAT_H

#include "IMAccount.h"
#include "IMContactSet.h"

#include <util/Event.h>
#include <util/Interface.h>
#include <util/Trackable.h>

#include <string>

class IMChatSession;

//=============================================================================

class GroupChatInfo //: public Trackable
{
public:
	GroupChatInfo()		{ initVars();	}

	//To avoid 'cannot access private member declared in EVENT class.
	GroupChatInfo( const GroupChatInfo& gcInfo)
	{
		*this = gcInfo;
	}
	
	Event<void ( const GroupChatInfo& sender )> respondToInviteEvent;

	//For use with incoming GroupChat Info
	GroupChatInfo( bool incoming, const char* who, const char* name, const char* message )
	{
		initVars();

		setIncoming( incoming );
		setWho     ( who	  );
		setName    ( name	  );
		setMessage ( message  );
	}

	bool isIncoming()		 const						{ return _incoming;	}
	std::string	getWho()	 const						{ return _who;		}
	std::string	getName()	 const						{ return _name;		}
	std::string getMessage() const						{ return _message;	}

	int			getId()		  const						{ return _id;		}	//For match to Purple objects
	std::string getChatRoom() const						{ return _chatroom;	}
	std::string	getAlias()	  const						{ return _alias;	}
	std::string getGroup()	  const						{ return _group;	}
	int			getExchange() const						{ return _exchange;	}

	bool		acceptedInvite() const					{ return _acceptedInvite;	}
	std::string	getAccountId()	 const					{ return _accountId;		}


	void setId       ( int  val )						{ _id		 = val;	}
	void setIncoming ( bool val )						{ _incoming  = val;	}
	void setWho      ( const std::string& val )			{ _who       = val;	}
	void setName	 ( const std::string& val )			{ _name		 = val;	}
	void setMessage  ( const std::string& val )			{ _message   = val;	}
	void setAccountId( const std::string& val )			{ _accountId = val;	}

	void setChatRoom ( const std::string& val )			{ _chatroom = val;	}
	void setAlias    ( const std::string& val )			{ _alias	= val;	}
	void setGroup	 ( const std::string& val )			{ _group    = val;	}
	void setExchange ( int			      val )			{ _exchange = val;	}

	void setAcceptedInvite( bool val )					{ _acceptedInvite = val;	}

	std::string getChatRoomDisplayName() const
	{ 
		std::string result = getChatRoom();

		if ( result.empty() )
		{
			result = getAlias();
		}

		return result;
	}

	GroupChatInfo& operator=( const GroupChatInfo& src )
	{
		if ( this != &src )
		{
			setIncoming ( src.isIncoming() );
			setWho      ( src.getWho()     );
			setName	    ( src.getName()    );
			setMessage  ( src.getMessage() );

			setAccountId( src.getAccountId() );
			setId       ( src.getId()		 );
			setChatRoom ( src.getChatRoom()	 );
			setAlias    ( src.getAlias()	 );
			setGroup    ( src.getGroup()	 );
			setExchange ( src.getExchange()	 );

			setAcceptedInvite( src.acceptedInvite() );
		}

		return *this;
	}

protected:
	void initVars()
	{
		setIncoming ( false );
		setWho      ( "" );
		setName	    ( "" );
		setMessage  ( "" );

		setAccountId( "" );
		setId       ( 0  );
		setChatRoom ( "" );
		setAlias    ( "" );
		setGroup    ( "" );
		setExchange ( 0  );

		setAcceptedInvite( false );
	}

private:
	bool		_incoming;

	std::string	_who;
	std::string	_name;
	std::string	_message;

	int			_id;
	std::string	_accountId;
	std::string	_chatroom;
	std::string	_alias;
	std::string _group;
	int			_exchange;

	bool	_acceptedInvite;
};

/**
 * Wrapper for Instant Messaging chat.
 *
 * Low-level functions to implement by the IM backend.
 * Users of imwrapper will use IMChatSession instead.
 *
 * @see IMChatSession
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class IMChat : Interface, public Trackable {
	friend class IMChatSession;
public:

	enum StatusMessage {
		/** Chat message has been received. */
		StatusMessageReceived,

		/** Chat message sending error. 'message' contains the one that produces the error. */
		StatusMessageError,

		/** Information message (e.g "toto has left the chat room"). 'message' contains the information message. */
		StatusMessageInfo
	};

	enum TypingState {
		/** No action is done from the contact. */
		TypingStateNotTyping,

		/** The contact is typing. */
		TypingStateTyping,

		/** The contact stops typing momentarily. */
		TypingStateStopTyping
	};

	//VOXOX - CJC - 2009.05.07 Add types to support chat to email, chat to sms, etc
	enum IMChatType 
	{
		Chat		= 0,
		ChatGroup   = 1,
		ChatToEmail	= 2,
		ChatToSMS	= 3,
	};

	/**
	 * Emitted when a new IMChatSession has been created.
	 *
	 * @param sender this class
	 * @param imChatSession the new IMChatSession
	 */
	Event<void (IMChat & sender, IMChatSession & imChatSession)> newIMChatSessionCreatedEvent;

	//VOXOX - CJC - 2009.05.07 Event that trigger when creating a chat session to IM voxox com, it contains also the contact Id that will be associated with that ask.im.voxox.com.
	//Ej Sending a chat to email message to jorge and using chat.email.voxox.com
	Event<void (IMChat & sender, IMChatSession & imChatSession)> newIMChatToEmailSessionCreatedEvent;

	//VOXOX - CJC - 2009.05.19 Chat to sms session
	Event<void (IMChat & sender, IMChatSession & imChatSession)> newIMChatToSMSSessionCreatedEvent;

	//VOXOX CHANGE CJC SUPPORT SEND MESSAGE WHEN DOING A CHAT SESSION
	Event<void (IMChat & sender, IMChatSession & imChatSession, const std::string & message, bool killChatSession)> newIMChatAndSendMessageSessionCreatedEvent;

	/**
	 * Emitted when a contact has been added to a session.
	 *
	 * @param sender this class
	 * @param imChatSession the session where a contact has been added
	 * @param contactId the added contact
	 */
	Event<void (IMChat & sender, IMChatSession & imChatSession, const IMContact & imContact)> contactAddedEvent;

	/**
	 * Emitted when a contact is removed from a session.
	 *
	 * @param sender this class
	 * @param imChatSession the session where a contact has been removed
	 * @param imChatSession the removed contact
	 */
	Event<void (IMChat & sender, IMChatSession & imChatSession, const std::string & contactId)> contactRemovedEvent;

	/**
	 * Chat message received callback.
	 *
	 * @param session the associated IMChatSession.
	 * @param sender this class
	 * @param contactId message sender
	 * @param message message received
	 */
	Event<void (IMChat & sender, IMChatSession & chatSession, const std::string & contactId, const std::string & message)> messageReceivedEvent;

	/**
	 * Message status event.
	 *
	 * @param sender this class
	 * @param chatSession the associated IMChatSession
	 * @param status new status
	 * @param message @see StatusMessage
	 */
	Event<void (IMChat & sender, IMChatSession & chatSession, StatusMessage status, const std::string & message)> statusMessageReceivedEvent;

	/**
	 * Typing state event.
	 *
	 * @param sender this class
	 * @param chatSession the associated IMChatSession
	 * @param contactId contact who typing state has changed
	 * @param state @see TypingState
	 */
	Event<void (IMChat & sender, IMChatSession & chatSession, const std::string & contactId, TypingState state)> typingStateChangedEvent;

	//VOXOX - JRT - 2009.06.16 - Handle incoming group chat invite.
	Event<void (IMChat & sender, GroupChatInfo& gcInfo ) > incomingGroupChatInviteEvent;	//VOXOX - JRT - 2009.06.16 

	virtual ~IMChat() { }

	/**
	 * Creates a new IMChatSession.
	 *
	 * The new IMChatSession is returned by the Event newIMChatSessionCreatedEvent
	 */
	virtual void createSession(const IMContactSet & imContactList, IMChat::IMChatType imChatType, const std::string& userChatRoomName ) = 0;	//VOXOX - JRT - 2009.06.15 

	virtual void createChatToEmailSession(const IMContactSet & imContactList,const std::string & relatedContactId) = 0;
	virtual void createChatToSMSSession(const IMContactSet & imContactList,const std::string & relatedContactId) = 0;
	virtual void createSessionAndSendMessage(const IMContactSet & imContactList, const std::string & message) = 0;

	//TODO: REDESIGN THIS< RIGHT NOW WE DONT HAVE A LAYER TO CALL CUSTOM PURPLE METHODS, THIS SHOULD NOT BE HERE.
	virtual void callSkypeContact(const IMContactSet & imContactList) = 0;


	void setIMAccount(const IMAccount & imAccount)		{ _imAccount = imAccount;	}
	std::string getIMAccountId() const					{ return const_cast<IMAccount&>(_imAccount).getKey();	}
	const IMAccount & getIMAccount() const				{ return _imAccount; }

	//VOXOX CHANGE CJC SEND MESSAGE AUTOMATIC ACCESS
	void setAutoMessage(std::string autoMessage)		{ _autoMessage = autoMessage;	}
	std::string getAutoMessage() const					{ return _autoMessage;			}

	//VOXOX - CJC - 2009.05.07 Associated contact Id
	void setRelatedContactId(std::string contactId)		{ _relatedContactId = contactId; }
	std::string getRelatedContactId() const				{ return _relatedContactId;	}

	//VOXOX - CJC - 2009.05.07 Type Of the IM, to diferentiate what is IM, or IMToEmil or IMtoSMS
	void		setIMChatType(IMChatType type)			{ _imChatType = type;	}
	IMChatType	getIMChatType() const					{ return _imChatType;	}

	GroupChatInfo&	getGroupChatInfo()					{ return _groupChatInfo;	}

	bool isGroupChat()									{ return (getIMChatType() == IMChat::ChatGroup);	}

protected:

	/**
	 * Says IMChat to close a new session given a IMChatSession.
	 *
	 * @param chatSession the IMChatSession to close
	 */
	virtual void closeSession(IMChatSession & chatSession) = 0;

	/**
	 * Sends a message to all IMContact linked to the chat session.
	 *
	 * @param chatSession the session to send the message to
	 * @param message the message to send
	 */
	virtual void sendMessage(IMChatSession & chatSession, const std::string & message) = 0;

	/**
	 * Changes my typing state to inform all IMContact linked to the chat session.
	 *
	 * @param chatSession the session to send the message to
	 * @param state my current typing state
	 */
	virtual void changeTypingState(IMChatSession & chatSession, TypingState state) = 0;

	/**
	 * Adds a contact to the session.
	 *
	 * @param chatSession the session id
	 * @param protocol the protocol of the contact to add
	 * @param contactId the identifier of the contact
	 */
	virtual void addContact(IMChatSession & chatSession, const std::string & contactId) = 0;

	/**
	 * Removes a contact from the session.
	 *
	 * @param chatSession the session id
	 * @param protocol the protocol of the contact to add
	 * @param contactId the identifier of the contact
	 */
	virtual void removeContact(IMChatSession & chatSession, const std::string & contactId) = 0;

	IMChat(IMAccount imAccount) : _imAccount(imAccount) {}

private:	//VOXOX - JRT - 2009.06.15 - force use of accessors.
	IMAccount		_imAccount;
	std::string		_autoMessage;			//VOXOX CHANGE CJC SEND MESSAGE AUTOMATIC WITHOUT OPENING UI
	std::string		_relatedContactId;		//VOXOX - CJC - 2009.05.07 Associated contact of the chat, this can be use when chatting to 
											//					im.voxox.com but the contact associated with that chat is other

	IMChatType		_imChatType;
	GroupChatInfo	_groupChatInfo;
};

#endif	//OWIMCHAT_H
