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

#include "PurpleIMChat.h"

#include "PurpleEnumIMProtocol.h"

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMContactSet.h>
#include <imwrapper/IMContact.h>

#include <util/Logger.h>
#include <cutil/global.h>
#include <thread/Thread.h>

extern "C" 
{
#include <libpurple/conversation.h>
#include <libpurple/util.h>
}

#ifdef OS_WINDOWS
	#define snprintf _snprintf
	#include <windows.h>		//For OutputDebugString
#else
	#include <cstdio>
#endif

//typedef struct misc_s
//{
//	PurpleIMChat * instance;
//	void * data_ptr1;
//	void * data_ptr2;
//	int data_int;
//	int cbk_id;
//}	misc_t;

std::list<mConvInfo_t *> PurpleIMChat::_PurpleChatSessionList;
Mutex PurpleIMChat::_mutex;

//-----------------------------------------------------------------------------

PurpleIMChat::PurpleIMChat(IMAccount account)
	: IMChat(account)
{
	setIMChatType ( IMChat::Chat );
	setAutoMessage( "" );		//No Automessage
	setRelatedContactId( "" );	//No Related Contact
}

//-----------------------------------------------------------------------------

mConvInfo_t *PurpleIMChat::CreateChatSession(bool userCreated, PurpleIMChat & imChat)
{
	mConvInfo_t	  *mConv	   = new mConvInfo_t();
	IMChatSession *chatSession = new IMChatSession(imChat, userCreated);

	mConv->conv_session    = chatSession;
	mConv->conv_id		   = chatSession->getId();
	mConv->pending_invites = NULL;

	imChat.getGroupChatInfo().setId( chatSession->getId() );	//VOXOX - JRT - 2009.06.19 - so we can find it later.

	AddChatSessionInList(mConv);

	return mConv;
}

//-----------------------------------------------------------------------------

bool PurpleIMChat::createSessionCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMChat::_mutex);

	PurpleIMChatCallbackData* cbData = (PurpleIMChatCallbackData*) dataIn;

	PurpleIMChat*  imChat		= cbData->getPurpleIMChat();
	IMContactSet*  imContactSet = cbData->getIMContactSet();
//	PurpleAccount* gAccount		= purple_accounts_find(imChat->getIMAccount().getLogin().c_str(),
//														PurpleIMPrcl::GetPrclId(imChat->getIMAccount().getProtocol()));
	PurpleAccount* gAccount		= getPurpleAccount( imChat->getIMAccount() );

	IMContactSet::const_iterator it;

	if (imContactSet->empty())
	{
		LOG_FATAL("imContactSet is empty");
	}
	else
	{
		switch( imChat->getIMChatType() )
		{
		case IMChat::Chat:
		{
			it = imContactSet->begin();
			std::string contactId = (*it).getContactId();

			PurpleConversation *gConv = NULL;
			gConv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, contactId.c_str(), gAccount);

			//If converation already exists, we still do the CreatedEvent so UI responds properly,
			//	then we remove the task from queue.
			if ( gConv )
			{
				mConvInfo_t *mConv = (mConvInfo_t *)gConv->ui_data;
				imChat->newIMChatSessionCreatedEvent(*imChat, *(mConv->conv_session));	//VOXOX - JRT - 2009.07.09 

				timeoutRemove( cbData );
				delete cbData;

				return FALSE;
			}

			purple_conversation_new(PURPLE_CONV_TYPE_IM, gAccount, contactId.c_str());
			break;
		}

		case IMChat::ChatGroup:
		{
			//Capture invitees.  They will be handled later.
			PurpleConnection *gGC = purple_account_get_connection(gAccount);
			GList *mlist = NULL;

			for (it = imContactSet->begin(); it != imContactSet->end(); it++)
			{
				mlist = g_list_append(mlist, strdup(it->getContactId().c_str()) );
			}

			//Does this group chat already exist?
			std::string chatRoomName = imChat->getGroupChatInfo().getAlias();	//VOXOX - JRT - 2009.06.18 - TODO: use getChatRoom()

			PurpleConversation *gConv = NULL;
			gConv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, chatRoomName.c_str(), gAccount);

			//If converation already exists, we still do the CreatedEvent so UI responds properly,
			//	then we remove the task from queue.
			if ( gConv )
			{
				mConvInfo_t *mConv = (mConvInfo_t *)gConv->ui_data;
				imChat->newIMChatSessionCreatedEvent(*imChat, *(mConv->conv_session));	//VOXOX - JRT - 2009.07.09 

				timeoutRemove( cbData );
				delete cbData;

				return FALSE;
			}

			//Create new chat.
//			purple_conversation_new(PURPLE_CONV_TYPE_CHAT, gAccount, contactId.c_str());
			createPurpleChat(gGC, 0, mlist, *imChat, gAccount );	//VOXOX - JRT - 2009.07.10 
			break;
		}

		default:
			LOG_FATAL("IMChat:IMChatType is improper: ", imChat->getIMChatType());
		}
	}

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------
//VOXOX - CJC - 2009.05.07 Add session form AskImVoxOxCom
bool PurpleIMChat::createChatToEmailSessionCbk(void * dataIn)
{
	#if (defined OS_WINDOWS)
		Mutex::ScopedLock lock(PurpleIMChat::_mutex);
	#endif

	PurpleIMChatCallbackData* cbData = (PurpleIMChatCallbackData*) dataIn;

	PurpleIMChat*  imChat		= cbData->getPurpleIMChat();
	IMContactSet*  imContactSet	= cbData->getIMContactSet();
	PurpleAccount* gAccount		= getPurpleAccount( imChat->getIMAccount() );
	
	IMContactSet::const_iterator askImContact;

	if (imContactSet->empty())
	{
		LOG_FATAL("imContactSet is empty");
	}
	else if (imContactSet->size() == 1)
	{
		askImContact = imContactSet->begin();
		std::string askId = (*askImContact).getContactId();

		PurpleConversation *gConv = NULL;

		if ((gConv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, askId.c_str(), gAccount)))
		{
			mConvInfo_t *mConv = (mConvInfo_t *)gConv->ui_data;
			imChat->newIMChatToEmailSessionCreatedEvent(*imChat, *(mConv->conv_session));	//VOXOX - JRT - 2009.07.09 

			timeoutRemove( cbData );
			delete cbData;

			return FALSE;
		}

		purple_conversation_new(PURPLE_CONV_TYPE_IM, gAccount, askId.c_str());
	}

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------
//VOXOX - CJC - 2009.05.07 Add session form AskImVoxOxCom
bool PurpleIMChat::createChatToSMSSessionCbk( void * dataIn )
{
	#if (defined OS_WINDOWS)
		Mutex::ScopedLock lock(PurpleIMChat::_mutex);
	#endif

	PurpleIMChatCallbackData* cbData = (PurpleIMChatCallbackData*) dataIn;

	PurpleIMChat*  imChat		= cbData->getPurpleIMChat();
	IMContactSet*  imContactSet	= cbData->getIMContactSet();
	PurpleAccount* gAccount		= getPurpleAccount( imChat->getIMAccount() );
	
	IMContactSet::const_iterator askImContact;

	if (imContactSet->empty())
	{
		LOG_FATAL("imContactSet is empty");
	}
	else if (imContactSet->size() == 1)
	{
		askImContact = imContactSet->begin();
		std::string askId = (*askImContact).getContactId();

		PurpleConversation *gConv = NULL;
		if ((gConv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, askId.c_str(), gAccount)))
		{
			mConvInfo_t *mConv = (mConvInfo_t *)gConv->ui_data;
			imChat->newIMChatToSMSSessionCreatedEvent(*imChat, *(mConv->conv_session));	//VOXOX - JRT - 2009.07.09 

			timeoutRemove( cbData );
			delete cbData;

			return FALSE;
		}

		purple_conversation_new(PURPLE_CONV_TYPE_IM, gAccount, askId.c_str());
	}

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE CJC ADD SKYPE CALL SUPPORT
//TODO: THIS SHOULD NOT BE HERE
bool PurpleIMChat::callSkypeContactCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMChat::_mutex);

	PurpleIMChatCallbackData* cbData = (PurpleIMChatCallbackData*) dataIn;

	PurpleIMChat*  imChat		= cbData->getPurpleIMChat();
	IMContactSet*  imContactSet = cbData->getIMContactSet();
	PurpleAccount* gAccount		= getPurpleAccount( imChat->getIMAccount() );

	PurplePluginProtocolInfo* prpl_info = NULL;
	PurpleConnection*		  gc		= purple_account_get_connection(gAccount);
	PurplePlugin*			  prpl		= NULL;

	IMContactSet::const_iterator it;


	if (imContactSet->size() == 1)
	{
		it = imContactSet->begin();
		std::string contactId = (*it).getContactId();
		
		if (gc != NULL)
			prpl = purple_connection_get_prpl(gc);      

		if (prpl != NULL)
			prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);

		if (prpl_info != NULL && prpl_info->set_info != NULL)
			 prpl_info->set_info(NULL,contactId.c_str());
	}
	
	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMChat::createSession(const IMContactSet & imContactSet, IMChat::IMChatType imChatType, const std::string& userChatRoomName )
{
	PurpleIMChatCallbackData* cbData = new PurpleIMChatCallbackData;

	cbData->setPurpleIMChat( this );
	cbData->setIMContactSet( new IMContactSet(imContactSet) );

	setIMChatType	( imChatType	   );
	getGroupChatInfo().setAlias( userChatRoomName );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMChat::createSessionCbk ) );

//	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMChat::createSessionCbk, data);
}

//-----------------------------------------------------------------------------

void PurpleIMChat::createChatToEmailSession(const IMContactSet & imContactSet,const std::string & relatedContactId)
{
	PurpleIMChatCallbackData* cbData = new PurpleIMChatCallbackData;

	cbData->setPurpleIMChat( this );
	cbData->setIMContactSet( new IMContactSet(imContactSet) );

	setRelatedContactId(relatedContactId);
	setIMChatType(IMChat::ChatToEmail);

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMChat::createChatToEmailSessionCbk ) );
//	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMChat::createChatToEmailSessionCbk, data);
}

//-----------------------------------------------------------------------------
//VOXOX - CJC - 2009.05.19 Create chat to sms session
void PurpleIMChat::createChatToSMSSession(const IMContactSet & imContactSet,const std::string & relatedContactId)
{
	PurpleIMChatCallbackData* cbData = new PurpleIMChatCallbackData;

	cbData->setPurpleIMChat( this );
	cbData->setIMContactSet( new IMContactSet(imContactSet) );

	setRelatedContactId(relatedContactId);
	setIMChatType(IMChat::ChatToSMS);

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMChat::createChatToSMSSessionCbk ) );
//	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMChat::createChatToSMSSessionCbk, data);
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE CJC ADD SKYPE CALL SUPPORT
//TODO: THIS SHOULD NOT BE HERE
void PurpleIMChat::callSkypeContact(const IMContactSet & imContactSet)
{
	PurpleIMChatCallbackData* cbData = new PurpleIMChatCallbackData;

	cbData->setPurpleIMChat( this );
	cbData->setIMContactSet( new IMContactSet(imContactSet) );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMChat::callSkypeContactCbk ) );

//	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMChat::callSkypeContactCbk, data);
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE CJC ADD SUPPORT FOR MULTIPLE CHAT MESSAGES
void PurpleIMChat::createSessionAndSendMessage(const IMContactSet & imContactSet, const std::string & message)
{
	PurpleIMChatCallbackData* cbData = new PurpleIMChatCallbackData;

	cbData->setPurpleIMChat( this );
	cbData->setIMContactSet( new IMContactSet(imContactSet) );
	cbData->setMessage     ( message );

	setAutoMessage( message );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMChat::createSessionAndSendMessageCbk ) );

//	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMChat::createSessionAndSendMessageCbk, data);
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE CJC SEND MESSAGE AUTOMATICALLY WHEN OPENING AN ACCOUNT
bool PurpleIMChat::createSessionAndSendMessageCbk(void * dataIn)
{
#if (defined OS_WINDOWS)
	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
#endif

	PurpleIMChatCallbackData* cbData = (PurpleIMChatCallbackData*) dataIn;

	PurpleIMChat*  imChat		= cbData->getPurpleIMChat();
	IMContactSet*  imContactSet = cbData->getIMContactSet();
	PurpleAccount* gAccount		= getPurpleAccount( imChat->getIMAccount() );

//	const char * message = (const char *)((misc_t *)data)->data_ptr2;
//	std::string realMessage = message;
	//VOXOX CHANGE CJC TO BE ABLE TO SEND THIS VALUES TO PURPLECHATMNGR AND SEND MESSAGE AUTOMATICALLY


	IMContactSet::const_iterator it;

	if (imContactSet->empty())
	{
		LOG_FATAL("imContactSet is empty");
	}
	else if (imContactSet->size() == 1)
	{
		it = imContactSet->begin();
		std::string contactId = (*it).getContactId();

		PurpleConversation *gConv = NULL;

		if ((gConv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, contactId.c_str(), gAccount)))
		{
			mConvInfo_t *mConv = (mConvInfo_t *)gConv->ui_data;
			imChat->newIMChatAndSendMessageSessionCreatedEvent(*imChat, *(mConv->conv_session), cbData->getMessage(), false);	//VOXOX - JRT - 2009.07.09 

			timeoutRemove( cbData );
			delete cbData;

			return FALSE;
		}

		purple_conversation_new(PURPLE_CONV_TYPE_IM, gAccount, contactId.c_str());
	}

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

bool PurpleIMChat::closeSessionCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMChat::_mutex);

	PurpleIMChatCallbackData* cbData = (PurpleIMChatCallbackData*) dataIn;

	IMChatSession* chatSession = cbData->getIMChatSession();
	mConvInfo_t *mConv = FindChatStructById(chatSession->getId());
	PurpleConversation *gConv = mConv->purple_conv_session;		//VOXOX - JRT - 2009.07.09 

	if (mConv != NULL)
	{
		purple_conversation_destroy(gConv);
		RemoveChatSessionFromList(chatSession->getId());
	}

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMChat::closeSession(IMChatSession & chatSession)
{
	PurpleIMChatCallbackData* cbData = new PurpleIMChatCallbackData;

	cbData->setPurpleIMChat( this );

	cbData->setIMChatSession( &chatSession );
	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMChat::closeSessionCbk ) );

//	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMChat::closeSessionCbk, data);
}

//-----------------------------------------------------------------------------

bool PurpleIMChat::sendMessageCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMChat::_mutex);

	PurpleIMChatCallbackData* cbData = (PurpleIMChatCallbackData*) dataIn;

	IMChatSession* chatSession = cbData->getIMChatSession();
//	const char * message = (const char *)((misc_t *)data)->data_ptr2;
//	char *cleanMess = (char *) message;
//	bool cleaned = false;
	std::string cleanMsg = cbData->getMessage();

	mConvInfo_t *mConv = FindChatStructById(chatSession->getId());
	PurpleConversation *gConv = mConv->purple_conv_session;		//VOXOX - JRT - 2009.07.09 

	// special case for ICQ
	PurpleAccount *gAccount = purple_conversation_get_account(gConv);
	PurplePlugin *prpl = purple_find_prpl(purple_account_get_protocol_id(gAccount));
	if (prpl)
	{
		PurplePluginProtocolInfo *prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);
		if (prpl_info->list_icon != NULL)
		{
			if (!strcmp("icq", prpl_info->list_icon(gAccount, NULL))) 
			{
//				cleanMess = (char *) purple_markup_strip_html(message);
				cleanMsg= purple_markup_strip_html( cbData->getMessage().c_str() );
//				cleaned   = true;
			}
		}
	}

	if (purple_conversation_get_type(gConv) == PURPLE_CONV_TYPE_IM)
	{
		purple_conv_im_send_with_flags(PURPLE_CONV_IM(gConv), cleanMsg.c_str(), PURPLE_MESSAGE_SEND);
	}
	else if (purple_conversation_get_type(gConv) == PURPLE_CONV_TYPE_CHAT)
	{
		purple_conv_chat_send_with_flags(PURPLE_CONV_CHAT(gConv), cleanMsg.c_str(), PURPLE_MESSAGE_SEND);
	}
	
//	if (cleaned) 
//	{
//		g_free(cleanMess);
//	}

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMChat::sendMessage(IMChatSession & chatSession, const std::string & message)
{
	PurpleIMChatCallbackData* cbData = new PurpleIMChatCallbackData;

	cbData->setPurpleIMChat( this );

	cbData->setIMChatSession( &chatSession );
	cbData->setMessage      ( message      );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMChat::sendMessageCbk ) );
//	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMChat::sendMessageCbk, data);
}

//-----------------------------------------------------------------------------

bool PurpleIMChat::changeTypingStateCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMChat::_mutex);

	PurpleIMChatCallbackData* cbData = (PurpleIMChatCallbackData*) dataIn;

	IMChatSession*		chatSession = cbData->getIMChatSession();
	IMChat::TypingState state		= cbData->getTypingState();
	PurpleTypingState	gState		 = PURPLE_NOT_TYPING;
	mConvInfo_t			*mConv		 = FindChatStructById(chatSession->getId());
	PurpleConversation	*gConv		 = NULL;

	if (mConv) 
	{
//		if ((gConv = (PurpleConversation *)mConv->purple_conv_session))		//VOXOX - JRT - 2009.07.09 This is poor coding technique!
		gConv = mConv->purple_conv_session;

		if ( gConv ) 
		{
			switch (state)
			{
			case IMChat::TypingStateTyping:
				gState = PURPLE_TYPING;
				break;

			case IMChat::TypingStateStopTyping:
			//gState = PURPLE_TYPED;
				gState = PURPLE_NOT_TYPING;
				break;

			default:
				gState = PURPLE_NOT_TYPING;
				break;
			}

			serv_send_typing(purple_conversation_get_gc(gConv), purple_conversation_get_name(gConv), gState);
		}
	}

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMChat::changeTypingState(IMChatSession & chatSession,  IMChat::TypingState state)
{
	PurpleIMChatCallbackData* cbData = new PurpleIMChatCallbackData;

	cbData->setPurpleIMChat( this );
	
	cbData->setIMChatSession( &chatSession );
	cbData->setTypingState  ( state );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMChat::changeTypingStateCbk ) );
//	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMChat::changeTypingStateCbk, data);
}

//-----------------------------------------------------------------------------

bool PurpleIMChat::addContactCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMChat::_mutex);

	PurpleIMChatCallbackData* cbData = (PurpleIMChatCallbackData*) dataIn;

	PurpleIMChat*	imChat		= cbData->getPurpleIMChat();
	IMChatSession*	chatSession = cbData->getIMChatSession();
	PurpleAccount*	gAccount	= getPurpleAccount( imChat->getIMAccount() );

//	const char * contactId = (const char *)((misc_t *)data)->data_ptr2;
	mConvInfo_t *mConv = FindChatStructById(chatSession->getId());
	int BuddyNbr = chatSession->getIMContactSet().size();
	PurpleConversation *gConv = NULL;

	if (mConv)
		gConv = mConv->purple_conv_session;		//VOXOX - JRT - 2009.07.09 
	else
		LOG_FATAL("ConvInfo not created !!!");

	if (purple_conversation_get_type(gConv) == PURPLE_CONV_TYPE_IM)
	{
		GList *mlist = NULL;
		IMContactSet& chatContact = const_cast<IMContactSet&>(chatSession->getIMContactSet());
		IMContactSet::const_iterator it = chatContact.begin();
		const std::string & firstContactId = it->getContactId();
		PurpleConnection *gGC;

		gConv = mConv->purple_conv_session;	//VOXOX - JRT - 2009.07.09 
		gGC = purple_conversation_get_gc(gConv);
		mlist = g_list_append(mlist, const_cast<char*>(cbData->getContactId().c_str()) );
		mlist = g_list_append(mlist, (char *) firstContactId.c_str());

		createPurpleChat(gGC, chatSession->getId(), mlist, *imChat, gAccount);	//VOXOX - JRT - 2009.07.10 
	}
	else if (purple_conversation_get_type(gConv) == PURPLE_CONV_TYPE_CHAT)
	{
		serv_chat_invite(purple_conversation_get_gc(gConv), purple_conv_chat_get_id(PURPLE_CONV_CHAT(gConv)), NULL, cbData->getContactId().c_str() );
	}

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMChat::addContact(IMChatSession & chatSession, const std::string & contactId)
{
	PurpleIMChatCallbackData* cbData = new PurpleIMChatCallbackData;

	cbData->setPurpleIMChat( this );
	cbData->setIMChatSession( &chatSession );
	cbData->setContactId    ( contactId    );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMChat::addContactCbk ) );
//	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMChat::addContactCbk, data);
}

//-----------------------------------------------------------------------------

bool PurpleIMChat::removeContactCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMChat::_mutex);

	PurpleIMChatCallbackData* cbData = (PurpleIMChatCallbackData*) dataIn;

	IMChatSession* chatSession = cbData->getIMChatSession();
//	const char * contactId = (const char *)((misc_t *)data)->data_ptr2;
	mConvInfo_t *mConv = FindChatStructById(chatSession->getId());
	PurpleConvChat *conv = PURPLE_CONV_CHAT( mConv->purple_conv_session );	//VOXOX - JRT - 2009.07.09 

	purple_conv_chat_remove_user(conv, cbData->getContactId().c_str(), NULL);

	timeoutRemove( cbData );

	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMChat::removeContact(IMChatSession & chatSession, const std::string & contactId)
{
	PurpleIMChatCallbackData* cbData = new PurpleIMChatCallbackData;

	cbData->setPurpleIMChat( this );
	cbData->setIMChatSession( &chatSession );
	cbData->setContactId    ( contactId    );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMChat::removeContactCbk ) );
//	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMChat::removeContactCbk, data);
}

//-----------------------------------------------------------------------------

void PurpleIMChat::AddChatSessionInList(mConvInfo_t *conv)
{
	if (!IsChatSessionInList(conv->conv_id))
	{
		_PurpleChatSessionList.push_back(conv);
	}
}

//-----------------------------------------------------------------------------

void PurpleIMChat::RemoveChatSessionFromList(int convId)
{
	PurpleChatSessionIterator it;
	for (it = _PurpleChatSessionList.begin(); it != _PurpleChatSessionList.end(); it++)
	{
		if ((*it)->conv_id == convId)
		{
			_PurpleChatSessionList.erase(it);
			break;
		}
	}
}

//-----------------------------------------------------------------------------

mConvInfo_t *PurpleIMChat::FindChatStructById(int convId)
{
	PurpleChatSessionIterator it;
	for (it = _PurpleChatSessionList.begin(); it != _PurpleChatSessionList.end(); it++)
	{
		if ((*it)->conv_id == convId)
		{
			return (*it);
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.06.19 
mConvInfo_t *PurpleIMChat::FindChatStructByName( const char* chatName )
{
	mConvInfo_t* result = NULL;
	PurpleChatSessionIterator it;

	for (it = _PurpleChatSessionList.begin(); it != _PurpleChatSessionList.end(); it++)
	{
		PurpleConversation* conv = (*it)->purple_conv_session;	//VOXOX - JRT - 2009.07.09 

		if ( conv )
		{
			if ( strcmp( conv->name, chatName) == 0 )
			{
				result = *it;
				break;
			}
		}

		IMChatSession* chatSession = (*it)->conv_session;	//VOXOX - JRT - 2009.07.10 

		if ( chatSession )
		{
			if ( strcmp( chatSession->getGroupChatInfo().getChatRoom().c_str(), chatName) == 0 )
			{
				result = *it;
				break;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------

void PurpleIMChat::createPurpleChat(PurpleConnection *gGC, int id, GList *users, PurpleIMChat & imChat, PurpleAccount* gAccount)	//VOXOX - JRT - 2009.07.10 
{
	char chatName[256];
	mConvInfo_t *mConv = FindChatStructById(id);
	PurpleConversation *gConv = NULL;
	EnumIMProtocol::IMProtocol protocol = PurpleIMPrcl::GetEnumIMProtocol(gGC->account->protocol_id);

	if (mConv == NULL)
	{
		//Create IMChatSession and copy some values from IMChat.
		mConv = CreateChatSession(true, imChat);
		mConv->conv_session->setIMChatType( imChat.getIMChatType() );
//		mConv->conv_session->getGroupChatInfo() = imChat.getGroupChatInfo();
	}

	//VOXOX - JRT - 2009.06.15 - purple_conversation_get_name() uses this to id (numeric)!
	//		Since id is ChatSession Address (0x), if you change this, then some 'find's will fail.
	//		You will need your own data members to track alias, etc.
	//		NOTE: This may be JABBER ONLY.
//	snprintf(chatName, sizeof(chatName), "Chat%d", mConv->conv_id);		
	strcpy( chatName, imChat.getGroupChatInfo().getAlias().c_str() );

	if (protocol == EnumIMProtocol::IMProtocolMSN)
	{
		//VOXOX - JRT - 2009.06.18 - TODO: check on use of 'chatname'; move to switch in following else-clause.
		gConv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, chatName, gGC->account);

		if (!gConv)
			LOG_FATAL("Chat doesn't exist !!");

		mConv->purple_conv_session = gConv;
	//	gConv->ui_data = mConv;
	}
	else
	{
		GHashTable *components;

		components = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

		char *room   = NULL;
		char *server = NULL;

		g_hash_table_replace(components, g_strdup("room"), g_strdup(chatName));

		switch( protocol )
		{
		case EnumIMProtocol::IMProtocolYahoo:
			g_hash_table_replace(components, g_strdup("topic"), g_strdup("Join my conference..."));
			g_hash_table_replace(components, g_strdup("type"),  g_strdup("Conference"));
			break;

		case EnumIMProtocol::IMProtocolAIM:
		case EnumIMProtocol::IMProtocolICQ:
			//Defaults for:
			//	"room"		- empty or passed in value (NULL below)
			//  "exchange"	- "4"
			components = serv_chat_info_defaults( gGC, NULL );
			g_hash_table_replace(components, g_strdup("exchange"), g_strdup("16"));			//VOXOX - JRT - 2009.06.14 - why do we use "16"?
			g_hash_table_replace(components, g_strdup("room"),     g_strdup(chatName));

			room   = (char*)g_hash_table_lookup(components, "room");
//			server = (char*)g_hash_table_lookup(components, "server");		//TODO: 

			break;

		case EnumIMProtocol::IMProtocolJabber:
			std::string userChatRoomName = (imChat.getGroupChatInfo().getAlias().empty() ? chatName : imChat.getGroupChatInfo().getAlias());

			//This will return defaults for:
			//	"server" - conference.im.voxox.com
			//  "room"	 - user-defined, or parsed from room@server/handle
			//	"handle" - user id without domain.  "user@domain.com" -> "user"
			components = serv_chat_info_defaults( gGC, NULL );	//VOXOX - JRT - 2009.06.14 - Get defaults, like server (conference.im.voxox.com)
			g_hash_table_replace(components, g_strdup("room"),   g_strdup(chatName));

			room   = (char*)g_hash_table_lookup(components, "room");
			server = (char*)g_hash_table_lookup(components, "server");

			break;
		}

		std::string fullRoomName = room;

		if ( server != NULL )		//VOXOX - JRT - 2009.10.12 - AIM gives NULL server.
		{
		fullRoomName += "@";
		fullRoomName += server;
		}

		mConv->conv_session->getGroupChatInfo().setChatRoom( fullRoomName );
		mConv->pending_invites = users;
			
//		purple_conversation_new(PURPLE_CONV_TYPE_CHAT, gAccount, fullRoomName.c_str());	//JRT-XXX

		serv_join_chat(gGC, components);
		g_hash_table_destroy(components);
	}
}

//-----------------------------------------------------------------------------

bool PurpleIMChat::IsChatSessionInList(int convId)
{
	return (FindChatStructById(convId) != NULL);
}

//-----------------------------------------------------------------------------
//static
PurpleAccount* PurpleIMChat::getPurpleAccount( const IMAccount& imAccount )
{
	return purple_accounts_find( imAccount.getLogin().c_str(), PurpleIMPrcl::GetPrclId( imAccount.getProtocol() ) );
}

//-----------------------------------------------------------------------------

int PurpleIMChat::timeoutAdd( void* cbData, GSourceFunc callbackFunc )
{
	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
	return purple_timeout_add(0, callbackFunc, cbData);
}

//-----------------------------------------------------------------------------
//static
void PurpleIMChat::timeoutRemove( PurpleIMChatCallbackData* cbData )
{
	int waitTime	  = 1;
	int totalWaitTime = 0;

	while ( cbData->getCallbackId() == 0 )
	{
		Thread::msleep( waitTime );
		totalWaitTime += waitTime;

#ifdef _WIN32
		char msg[200];
		sprintf_s( msg, "PurpleIMChat::timeoutRemove - waiting for valid callbackId. %d ms\n", totalWaitTime );
		OutputDebugString( msg );
#endif
	}

#ifdef _WIN32
	if ( totalWaitTime > 0 )
	{
		char msg[200];
		sprintf_s( msg, "\n" );
		OutputDebugString( msg );
	}
#endif

	purple_timeout_remove( cbData->getCallbackId() );
}

//-----------------------------------------------------------------------------
