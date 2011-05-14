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

#include "GaimIMChat.h"

#include "GaimEnumIMProtocol.h"

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMContactSet.h>
#include <imwrapper/IMContact.h>

#include <util/Logger.h>
#include <cutil/global.h>

extern "C" {
#include <gaim/conversation.h>
#include <gaim/util.h>
}

#ifdef OS_WINDOWS
	#define snprintf _snprintf
#else
	#include <cstdio>
#endif

typedef struct misc_s
{
	GaimIMChat * instance;
	void * data_ptr1;
	void * data_ptr2;
	int data_int;
	int cbk_id;
}	misc_t;

std::list<mConvInfo_t *> GaimIMChat::_GaimChatSessionList;
Mutex GaimIMChat::_mutex;

GaimIMChat::GaimIMChat(IMAccount account)
	: IMChat(account)
{

}

mConvInfo_t *GaimIMChat::CreateChatSession(bool userCreated, GaimIMChat & imChat)
{
	mConvInfo_t *mConv = new mConvInfo_t();
	IMChatSession *chatSession = new IMChatSession(imChat, userCreated);

	mConv->conv_session = chatSession;
	mConv->conv_id = chatSession->getId();
	mConv->pending_invit = NULL;

	AddChatSessionInList(mConv);

	return mConv;
}

bool GaimIMChat::createSessionCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMChat::_mutex);

	GaimIMChat * imChat = ((misc_t *)data)->instance;
	IMContactSet & imContactSet = *(IMContactSet *)((misc_t *)data)->data_ptr1;
	GaimAccount *gAccount = gaim_accounts_find(imChat->getIMAccount().getLogin().c_str(),
		GaimIMPrcl::GetPrclId(imChat->getIMAccount().getProtocol()));

	IMContactSet::const_iterator it;

	if (imContactSet.empty())
	{
		LOG_FATAL("imContactSet is empty");
	}
	else if (imContactSet.size() == 1)
	{
		it = imContactSet.begin();
		std::string contactId = (*it).getContactId();

		GaimConversation *gConv = NULL;
		if ((gConv = gaim_find_conversation_with_account(GAIM_CONV_TYPE_IM,
			contactId.c_str(), gAccount)))
		{
			mConvInfo_t *mConv = (mConvInfo_t *)gConv->ui_data;
			imChat->newIMChatSessionCreatedEvent(*imChat, *((IMChatSession *)(mConv->conv_session)));

			gaim_timeout_remove(((misc_t *)data)->cbk_id);
			delete (IMContactSet *)((misc_t *)data)->data_ptr1;
			free((misc_t *)data);

			return FALSE;
		}

		gaim_conversation_new(GAIM_CONV_TYPE_IM, gAccount, contactId.c_str());
	}
	else
	{
		GList *mlist = NULL;
		GaimConnection *gGC = gaim_account_get_connection(gAccount);

		for (it = imContactSet.begin(); it != imContactSet.end(); it++)
		{
			mlist = g_list_append(mlist, (char *) it->getContactId().c_str());
		}

		createGaimChat(gGC, 0, mlist, *imChat);
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);
	delete (IMContactSet *)((misc_t *)data)->data_ptr1;
	free((misc_t *)data);

	return TRUE;
}

void GaimIMChat::createSession(const IMContactSet & imContactSet)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	data->data_ptr1 = new IMContactSet(imContactSet);

	Mutex::ScopedLock lock(GaimIMChat::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMChat::createSessionCbk, data);
}

bool GaimIMChat::closeSessionCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMChat::_mutex);

	IMChatSession & chatSession = *(IMChatSession *)((misc_t *)data)->data_ptr1;
	mConvInfo_t *mConv = FindChatStructById(chatSession.getId());
	GaimConversation *gConv = (GaimConversation *)mConv->gaim_conv_session;

	if (mConv != NULL)
	{
		gaim_conversation_destroy(gConv);
		RemoveChatSessionFromList(chatSession.getId());
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free((misc_t *)data);

	return TRUE;
}

void GaimIMChat::closeSession(IMChatSession & chatSession)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->data_ptr1 = &chatSession;

	Mutex::ScopedLock lock(GaimIMChat::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMChat::closeSessionCbk, data);
}


bool GaimIMChat::sendMessageCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMChat::_mutex);

	IMChatSession & chatSession = *(IMChatSession *)((misc_t *)data)->data_ptr1;
	const char * message = (const char *)((misc_t *)data)->data_ptr2;
	char *cleanMess = (char *) message;
	bool cleaned = false;

	mConvInfo_t *mConv = FindChatStructById(chatSession.getId());
	GaimConversation *gConv = (GaimConversation *)mConv->gaim_conv_session;

	// special case for ICQ
	GaimAccount *gAccount = gaim_conversation_get_account(gConv);
	GaimPlugin *prpl = gaim_find_prpl(gaim_account_get_protocol_id(gAccount));
	if (prpl)
	{
		GaimPluginProtocolInfo *prpl_info = GAIM_PLUGIN_PROTOCOL_INFO(prpl);
		if (prpl_info->list_icon != NULL)
		{
			if (!strcmp("icq", prpl_info->list_icon(gAccount, NULL))) {
				cleanMess = (char *) gaim_markup_strip_html(message);
				cleaned = true;
			}
		}
	}

	if (gaim_conversation_get_type(gConv) == GAIM_CONV_TYPE_IM)
	{
		gaim_conv_im_send_with_flags(GAIM_CONV_IM(gConv),
			cleanMess, GAIM_MESSAGE_SEND);
	}
	else if (gaim_conversation_get_type(gConv) == GAIM_CONV_TYPE_CHAT)
	{
		gaim_conv_chat_send_with_flags(GAIM_CONV_CHAT(gConv),
			cleanMess, GAIM_MESSAGE_SEND);
	}
	
	if (cleaned) {
		g_free(cleanMess);
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr2);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMChat::sendMessage(IMChatSession & chatSession, const std::string & message)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->data_ptr1 = &chatSession;
	data->data_ptr2 = strdup(message.c_str());

	Mutex::ScopedLock lock(GaimIMChat::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMChat::sendMessageCbk, data);
}

bool GaimIMChat::changeTypingStateCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMChat::_mutex);

	IMChatSession & chatSession = *(IMChatSession *)((misc_t *)data)->data_ptr1;
	IMChat::TypingState state = (IMChat::TypingState) ((misc_t *)data)->data_int;
	GaimTypingState gState = GAIM_NOT_TYPING;
	mConvInfo_t *mConv = FindChatStructById(chatSession.getId());
	GaimConversation *gConv = NULL;

	if (mConv) {
		if ((gConv = (GaimConversation *)mConv->gaim_conv_session)) {

			switch (state)
			{
				case IMChat::TypingStateTyping:
					gState = GAIM_TYPING;
					break;

				case IMChat::TypingStateStopTyping:
				//gState = GAIM_TYPED;
					gState = GAIM_NOT_TYPING;
					break;

				default:
					gState = GAIM_NOT_TYPING;
					break;
			}

			serv_send_typing(gaim_conversation_get_gc(gConv),
				gaim_conversation_get_name(gConv), gState);
		}
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free((misc_t *)data);

	return TRUE;
}

void GaimIMChat::changeTypingState(IMChatSession & chatSession, 
								   IMChat::TypingState state)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->data_ptr1 = &chatSession;
	data->data_int = state;

	Mutex::ScopedLock lock(GaimIMChat::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMChat::changeTypingStateCbk, data);
}

bool GaimIMChat::addContactCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMChat::_mutex);

	GaimIMChat * imChat = ((misc_t *)data)->instance;
	IMChatSession & chatSession = *(IMChatSession *)((misc_t *)data)->data_ptr1;
	const char * contactId = (const char *)((misc_t *)data)->data_ptr2;
	mConvInfo_t *mConv = FindChatStructById(chatSession.getId());
	GaimAccount *gAccount = gaim_accounts_find(imChat->getIMAccount().getLogin().c_str(),
		GaimIMPrcl::GetPrclId(imChat->getIMAccount().getProtocol()));
	int BuddyNbr = chatSession.getIMContactSet().size();
	GaimConversation *gConv = NULL;

	if (mConv)
		gConv = (GaimConversation *)mConv->gaim_conv_session;
	else
		LOG_FATAL("ConvInfo not created !!!");

	if (gaim_conversation_get_type(gConv) == GAIM_CONV_TYPE_IM)
	{
		GList *mlist = NULL;
		IMContactSet &chatContact = (IMContactSet &)chatSession.getIMContactSet();
		IMContactSet::const_iterator it = chatContact.begin();
		const std::string & firstContactId = it->getContactId();
		GaimConnection *gGC;

		gConv = (GaimConversation *) mConv->gaim_conv_session;
		gGC = gaim_conversation_get_gc(gConv);
		mlist = g_list_append(mlist, (char *) contactId);
		mlist = g_list_append(mlist, (char *) firstContactId.c_str());

		createGaimChat(gGC, chatSession.getId(), mlist, *imChat);
	}
	else if (gaim_conversation_get_type(gConv) == GAIM_CONV_TYPE_CHAT)
	{
		serv_chat_invite(gaim_conversation_get_gc(gConv), gaim_conv_chat_get_id(GAIM_CONV_CHAT(gConv)),
			NULL, contactId);
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr2);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMChat::addContact(IMChatSession & chatSession, const std::string & contactId)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	data->data_ptr1 = &chatSession;
	data->data_ptr2 = strdup(contactId.c_str());

	Mutex::ScopedLock lock(GaimIMChat::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMChat::addContactCbk, data);
}

bool GaimIMChat::removeContactCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMChat::_mutex);

	IMChatSession & chatSession = *(IMChatSession *)((misc_t *)data)->data_ptr1;
	const char * contactId = (const char *)((misc_t *)data)->data_ptr2;
	mConvInfo_t *mConv = FindChatStructById(chatSession.getId());
	GaimConvChat *conv = GAIM_CONV_CHAT((GaimConversation *)(mConv->gaim_conv_session));

	gaim_conv_chat_remove_user(conv, contactId, NULL);

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr2);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMChat::removeContact(IMChatSession & chatSession, const std::string & contactId)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->data_ptr1 = &chatSession;
	data->data_ptr2 = strdup(contactId.c_str());

	Mutex::ScopedLock lock(GaimIMChat::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMChat::removeContactCbk, data);
}

void GaimIMChat::AddChatSessionInList(mConvInfo_t *conv)
{
	if (!IsChatSessionInList(conv->conv_id))
	{
		_GaimChatSessionList.push_back(conv);
	}
}

void GaimIMChat::RemoveChatSessionFromList(int convId)
{
	GaimChatSessionIterator it;
	for (it = _GaimChatSessionList.begin(); it != _GaimChatSessionList.end(); it++)
	{
		if ((*it)->conv_id == convId)
		{
			_GaimChatSessionList.erase(it);
			break;
		}
	}
}

mConvInfo_t *GaimIMChat::FindChatStructById(int convId)
{
	GaimChatSessionIterator it;
	for (it = _GaimChatSessionList.begin(); it != _GaimChatSessionList.end(); it++)
	{
		if ((*it)->conv_id == convId)
		{
			return (*it);
		}
	}

	return NULL;
}

void GaimIMChat::createGaimChat(GaimConnection *gGC, int id, GList *users, GaimIMChat & imChat)
{
	char chatName[100];
	mConvInfo_t *mConv = FindChatStructById(id);
	GaimConversation *gConv;
	EnumIMProtocol::IMProtocol protocol = GaimIMPrcl::GetEnumIMProtocol(gGC->account->protocol_id);

	if (mConv == NULL)
		mConv = CreateChatSession(true, imChat);

	snprintf(chatName, sizeof(chatName), "Chat%d", mConv->conv_id);

	if (protocol == EnumIMProtocol::IMProtocolMSN)
	{
		serv_chat_create(gGC, chatName, users);
		gConv = gaim_find_conversation_with_account(GAIM_CONV_TYPE_CHAT, chatName, gGC->account);

		if (!gConv)
			LOG_FATAL("Chat doesn't exist !!");

		mConv->gaim_conv_session = gConv;
		gConv->ui_data = mConv;
	}
	else
	{
		GHashTable *components;

		components = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

		g_hash_table_replace(components, g_strdup("room"), g_strdup(chatName));

		if (protocol == EnumIMProtocol::IMProtocolYahoo)
		{
			g_hash_table_replace(components, g_strdup("topic"), g_strdup("Join my conference..."));
			g_hash_table_replace(components, g_strdup("type"), g_strdup("Conference"));
		}
		else if ((protocol == EnumIMProtocol::IMProtocolAIM)
			|| (protocol == EnumIMProtocol::IMProtocolICQ))
		{
			g_hash_table_replace(components, g_strdup("exchange"), g_strdup("16"));
		}

		mConv->pending_invit = users;

		serv_join_chat(gGC, components);
		g_hash_table_destroy(components);
	}
}

bool GaimIMChat::IsChatSessionInList(int convId)
{
	return FindChatStructById(convId);
}
