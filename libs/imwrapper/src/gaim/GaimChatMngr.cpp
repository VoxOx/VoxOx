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

#include "GaimChatMngr.h"

#include "GaimAccountMngr.h"
#include "GaimEnumIMProtocol.h"
#include "GaimIMFactory.h"

#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMContact.h>

#include <util/Logger.h>

extern "C" {
#include <gaim/util.h>
}

#ifdef OS_WINDOWS
	//#include <Windows.h>
	//#include <Winbase.h>
	#define snprintf _snprintf
	//#define sleep(i) Sleep(i * 1000)
//#else
	//#include <unistd.h>
#endif


/* **************** UTIL ****************** */
int GetGaimConversationId(const char *name)
{
	int id = 0;
	char *str_id = (char *) name;

	if (name == NULL)
		return id;

	while (str_id && *str_id && (*str_id > '9' || *str_id < '0'))
		str_id++;

	if (*str_id == '\0')
		return id;

	id = strtol(str_id, (char **) NULL, 10);
	return id;
}


/* ***************** GAIM CALLBACK ***************** */
void C_CreateIncomingConversationCbk(GaimConversation *conv)
{
	GaimChatMngr::CreateConversationCbk(conv, false);
}

void C_CreateConversationCbk(GaimConversation *conv)
{
	GaimChatMngr::CreateConversationCbk(conv, true);
}

static void C_DestroyConversationCbk(GaimConversation *conv)
{
	GaimChatMngr::DestroyConversationCbk(conv);
}

static void C_WriteChatCbk(GaimConversation *conv, const char *who,
						const char *message, GaimMessageFlags flags,
						time_t mtime)
{
	GaimChatMngr::WriteChatCbk(conv, who, message, flags, mtime);
}

static void C_WriteIMCbk(GaimConversation *conv, const char *who,
						const char *message, GaimMessageFlags flags, time_t mtime)
{
	GaimChatMngr::WriteIMCbk(conv, who, message, flags, mtime);
}

static void C_WriteConvCbk(GaimConversation *conv, const char *name, const char *alias,
						const char *message, GaimMessageFlags flags,
						time_t mtime)
{
	GaimChatMngr::WriteConvCbk(conv, name, alias, message, flags, mtime);
}

static void C_ChatAddUsersCbk(GaimConversation *conv, GList *users, gboolean new_arrivals)
{
	GaimChatMngr::ChatAddUsersCbk(conv, users, new_arrivals);
}

static void C_ChatRenameUserCbk(GaimConversation *conv, const char *old_name,
							const char *new_name, const char *new_alias)
{
	GaimChatMngr::ChatRenameUserCbk(conv, old_name, new_name, new_alias);
}

static void C_ChatRemoveUsersCbk(GaimConversation *conv, GList *users)
{
	GaimChatMngr::ChatRemoveUsersCbk(conv, users);
}

static void C_ChatUpdateUserCbk(GaimConversation *conv, const char *user)
{
	GaimChatMngr::ChatUpdateUserCbk(conv, user);
}

void C_PresentConvCbk(GaimConversation *conv)
{
    GaimChatMngr::PresentConvCbk(conv);
}

gboolean C_HasFocusCbk(GaimConversation *conv)
{
	return GaimChatMngr::HasFocusCbk(conv);
}

static gboolean C_CustomSmileyAddCbk(GaimConversation *conv, const char *smile, gboolean remote)
{
	return GaimChatMngr::CustomSmileyAddCbk(conv, smile, remote);
}

static void C_CustomSmileyWriteCbk(GaimConversation *conv, const char *smile,
								const guchar *data, gsize size)
{
	GaimChatMngr::CustomSmileyWriteCbk(conv, smile, data, size);
}

static void C_CustomSmileyCloseCbk(GaimConversation *conv, const char *smile)
{
	GaimChatMngr::CustomSmileyCloseCbk(conv, smile);
}

static void C_SendConfirm(GaimConversation *conv, const char *message)
{
	GaimChatMngr::SendConfirm(conv, message);
}

GaimConversationUiOps chat_wg_ops = {
	C_CreateConversationCbk,
	C_DestroyConversationCbk,	/* destroy_conversation */
	C_WriteChatCbk,				/* write_chat */
	C_WriteIMCbk,				/* write_im */
	C_WriteConvCbk,				/* write_conv */
	C_ChatAddUsersCbk,			/* chat_add_users */
	C_ChatRenameUserCbk,		/* chat_rename_user */
	C_ChatRemoveUsersCbk,		/* chat_remove_users */
	C_ChatUpdateUserCbk,		/* chat_update_user */
	C_PresentConvCbk,			/* present */
	C_HasFocusCbk,				/* has_focus */
	C_CustomSmileyAddCbk,		/* custom_smiley_add */
	C_CustomSmileyWriteCbk,		/* custom_smiley_write */
	C_CustomSmileyCloseCbk,		/* custom_smiley_close */
	C_SendConfirm,				/* send_confirm */
};


/* **************** MISCELLEANOUS CALLBACK ****************** */

void chat_joined_cb(GaimConversation *conv)
{
	GaimChatMngr::ChatJoinedCbk(conv);
}

int chat_invite_request_cb(GaimAccount *account, const char *who,
						const char *message, void *data)
{
	return 1;
}

void update_buddy_typing_cb(GaimAccount *account, const char *who)
{
	GaimConversation *gConv = NULL;

	gConv = gaim_find_conversation_with_account(GAIM_CONV_TYPE_IM, who, account);
	if (!gConv)
		return;

	GaimChatMngr::UpdateBuddyTyping(gConv, gaim_conv_im_get_typing_state(GAIM_CONV_IM(gConv)));
}

void received_im_msg_cb(GaimAccount *account, char *sender, char *message,
						GaimConversation *conv, int flags)
{
	GaimConversationUiOps *ops = &chat_wg_ops;

	if (conv != NULL)
		return;

	ops->create_conversation = C_CreateIncomingConversationCbk;
	gaim_conversation_new(GAIM_CONV_TYPE_IM, account, sender);
	ops->create_conversation = C_CreateConversationCbk;
}

void init_chat_event()
{
	void *handle = gaim_wg_get_handle();

	gaim_signal_connect(gaim_conversations_get_handle(), "buddy-typing",
		handle, GAIM_CALLBACK(update_buddy_typing_cb), NULL);
	
	gaim_signal_connect(gaim_conversations_get_handle(), "buddy-typing-stopped",
		handle, GAIM_CALLBACK(update_buddy_typing_cb), NULL);

	gaim_signal_connect(gaim_conversations_get_handle(), "chat-invited",
		handle, GAIM_CALLBACK(chat_invite_request_cb), NULL);

	gaim_signal_connect(gaim_conversations_get_handle(), "chat-joined",
		handle, GAIM_CALLBACK(chat_joined_cb), NULL);

	gaim_signal_connect(gaim_conversations_get_handle(), "chat-left",
		handle, GAIM_CALLBACK(chat_joined_cb), NULL);

	gaim_signal_connect(gaim_conversations_get_handle(), "received-im-msg",
		handle, GAIM_CALLBACK(received_im_msg_cb), NULL);
}

/* ************************************************** */

GaimChatMngr * GaimChatMngr::_staticInstance = NULL;
GaimAccountMngr *GaimChatMngr::_accountMngr = NULL;
std::list<GaimIMChat *> GaimChatMngr::_gaimIMChatList;
RecursiveMutex GaimChatMngr::_mutex;

GaimChatMngr::GaimChatMngr()
{
}

void GaimChatMngr::Init()
{
	_accountMngr = GaimAccountMngr::getInstance();
	init_chat_event();
	gaim_conversations_set_ui_ops(&chat_wg_ops);
}

GaimChatMngr *GaimChatMngr::getInstance()
{
	if (!_staticInstance)
	{
		_staticInstance = new GaimChatMngr();
	}
	return _staticInstance;
}

void GaimChatMngr::ChatJoinedCbk(GaimConversation *conv)
{
	mConvInfo_t *mConv = (mConvInfo_t *) conv->ui_data;

	if (mConv->pending_invit)
	{
		for (GList *l = mConv->pending_invit; l != NULL; l = l->next)
		{
			serv_chat_invite(gaim_conversation_get_gc(conv),
				gaim_conv_chat_get_id(GAIM_CONV_CHAT(conv)),
				"Join my conference...", (char *)l->data);
		}
	}
}

std::string GaimChatMngr::cleanContactId(std::string contactId, EnumIMProtocol::IMProtocol prtcl)
{
	if (prtcl == EnumIMProtocol::IMProtocolJabber) {
		std::string::size_type index = contactId.find('/');

		if (index != std::string::npos)
			return contactId.substr(0, index);
	}
	
	return contactId;
}

void GaimChatMngr::CreateConversationCbk(GaimConversation *conv, bool userCreated)
{
	GaimAccount *gAccount = gaim_conversation_get_account(conv);
	GaimConversationType chatType = gaim_conversation_get_type(conv);
	const char *gPrclId = gaim_account_get_protocol_id(gAccount);
	IMAccount *account = _accountMngr->FindIMAccount(gaim_account_get_username(gAccount),
		GaimIMPrcl::GetEnumIMProtocol(gPrclId));
	GaimIMChat *mChat = FindIMChat(*account);
	mConvInfo_t *mConv;

	if (chatType == GAIM_CONV_TYPE_IM)
	{
		IMChatSession *chatSession = NULL;

		// Check if it's a jabber contact, and remove his ressource
		std::string contactId = cleanContactId(std::string(gaim_conversation_get_name(conv)),
			GaimIMPrcl::GetEnumIMProtocol(gPrclId));

		IMContact imContact(*account, contactId);

		mConv = mChat->CreateChatSession(userCreated, *mChat);
		mConv->gaim_conv_session = conv;
 		conv->ui_data = mConv;

		chatSession = (IMChatSession *) mConv->conv_session;

		((IMContactSet &) chatSession->getIMContactSet()).insert(imContact);
		mChat->newIMChatSessionCreatedEvent(*mChat, *chatSession);

		mChat->contactAddedEvent(*mChat, *chatSession, imContact);
	}
	else if (chatType == GAIM_CONV_TYPE_CHAT)
	{
		int id = GetGaimConversationId(gaim_conversation_get_name(conv));

		if ((mConv = mChat->FindChatStructById(id)) == NULL)
		{
			mConv = mChat->CreateChatSession(userCreated, *mChat);
		}

		if (mConv->gaim_conv_session)
			gaim_conversation_destroy((GaimConversation *)mConv->gaim_conv_session);

		mConv->gaim_conv_session = conv;
		conv->ui_data = mConv;
	}
}

void GaimChatMngr::DestroyConversationCbk(GaimConversation *conv)
{
	// TODO: free (mConvInfo_t *) conv->ui_data;
	LOG_DEBUG("wgconv : gaim_wgconv_destroy()\n");

}

void GaimChatMngr::WriteChatCbk(GaimConversation *conv, const char *who,
							const char *message, GaimMessageFlags flags,
							time_t mtime)
{
	gaim_conversation_write(conv, who, message, flags, mtime);
}

void GaimChatMngr::WriteIMCbk(GaimConversation *conv, const char *who,
							const char *message, GaimMessageFlags flags,
							time_t mtime)
{
	gaim_conversation_write(conv, who, message, flags, mtime);
}

void GaimChatMngr::WriteConvCbk(GaimConversation *conv, const char *name, const char *alias,
							const char *message, GaimMessageFlags flags,
							time_t mtime)
{
	if ((flags & GAIM_MESSAGE_RECV))
	{
		mConvInfo_t *mConv = (mConvInfo_t *)conv->ui_data;
		GaimIMChat *mIMChat = FindIMChatByGaimConv(conv);

		// Prevent from double message during a chat session
		if (strcmp(gaim_account_get_username(conv->account), name) == 0)
			return;

		// Check if it's a jabber contact, and remove his ressource
		GaimAccount *gAccount = gaim_conversation_get_account(conv);
		const char *gPrclId = gaim_account_get_protocol_id(gAccount);
		std::string contactId = cleanContactId(name,
			GaimIMPrcl::GetEnumIMProtocol(gPrclId));

		mIMChat->messageReceivedEvent(*mIMChat, 
			*((IMChatSession *)(mConv->conv_session)),
			contactId, std::string(message));
	}
}

void GaimChatMngr::ChatAddUsersCbk(GaimConversation *conv, GList *users, gboolean new_arrivals)
{
	GList *l;
	GaimAccount *gAccount = gaim_conversation_get_account(conv);
	const char *gPrclId = gaim_account_get_protocol_id(gAccount);
	IMAccount *account = _accountMngr->FindIMAccount(gaim_account_get_username(gAccount),
								GaimIMPrcl::GetEnumIMProtocol(gPrclId));
	mConvInfo_t *mConv = NULL;
	IMChatSession *chatSession = NULL;
	GaimIMChat *mChat = FindIMChat(*account);

	if (!mChat)
		LOG_FATAL("Can't find IMChat !");

	mConv = (mConvInfo_t *) conv->ui_data;
	chatSession = (IMChatSession *) mConv->conv_session;

	for (l = users; l != NULL; l = l->next)
	{
		GaimConvChatBuddy *gCCBuddy = (GaimConvChatBuddy *) l->data;

		if (gCCBuddy && strcmp(gaim_account_get_username(gAccount), (char *) gCCBuddy->name))
		{
			std::string buddy((char *) gCCBuddy->name);
			IMContact imContact(*account, buddy);

			if (chatSession->getIMContactSet().find(imContact) != chatSession->getIMContactSet().end())
			{
				LOG_DEBUG("IMContact for " + imContact.getContactId() + " already in IMContactSet");
			}
			else
			{
				((IMContactSet &) chatSession->getIMContactSet()).insert(imContact);

				if (chatSession->getIMContactSet().size() == 1)
					mChat->newIMChatSessionCreatedEvent(*mChat, *chatSession);

				LOG_DEBUG("IMContact " + imContact.getContactId() + " added to IMContactSet");
			}
			mChat->contactAddedEvent(*mChat, *chatSession, imContact);
		}
	}
}

void GaimChatMngr::ChatRenameUserCbk(GaimConversation *conv, const char *old_name,
									const char *new_name, const char *new_alias)
{
}

void GaimChatMngr::ChatRemoveUsersCbk(GaimConversation *conv, GList *users)
{
	GList *l;
	GaimAccount *gAccount = gaim_conversation_get_account(conv);
	const char *gPrclId = gaim_account_get_protocol_id(gAccount);
	IMAccount *account = _accountMngr->FindIMAccount(gaim_account_get_username(gAccount),
								GaimIMPrcl::GetEnumIMProtocol(gPrclId));
	mConvInfo_t *mConv = NULL;
	IMChatSession *chatSession = NULL;
	GaimIMChat *mChat = FindIMChat(*account);

	if (!mChat)
		LOG_FATAL("Can't find IMChat !");

	mConv = (mConvInfo_t *) conv->ui_data;
	chatSession = (IMChatSession *) mConv->conv_session;

	for (l = users; l != NULL; l = l->next)
	{
		if (strcmp(gaim_account_get_username(gAccount), (char *) l->data))
		{
			std::string buddy((char *) l->data);
			IMContact imContact(*account, buddy);

			if (chatSession->getIMContactSet().find(imContact) == chatSession->getIMContactSet().end())
			{
				LOG_ERROR("IMContact for " + imContact.getContactId() + " not in IMContactSet");
			}
			else
			{
				LOG_DEBUG("IMContact " + imContact.getContactId() + " removed from IMContactSet");
				mChat->contactRemovedEvent(*mChat, *chatSession, buddy);
			}
		}
	}
}

void GaimChatMngr::ChatUpdateUserCbk(GaimConversation *conv, const char *user)
{
}

void GaimChatMngr::PresentConvCbk(GaimConversation *conv)
{
}

gboolean GaimChatMngr::HasFocusCbk(GaimConversation *conv)
{
	return TRUE;
}

gboolean GaimChatMngr::CustomSmileyAddCbk(GaimConversation *conv, const char *smile,
									   gboolean remote)
{
	return TRUE;
}

void GaimChatMngr::CustomSmileyWriteCbk(GaimConversation *conv, const char *smile,
									const guchar *data, gsize size)
{
}

void GaimChatMngr::CustomSmileyCloseCbk(GaimConversation *conv, const char *smile)
{
}

void GaimChatMngr::SendConfirm(GaimConversation *conv, const char *message)
{
}


void GaimChatMngr::UpdateBuddyTyping(GaimConversation *conv, GaimTypingState state)
{
	IMChat::TypingState mState;

	mConvInfo_t *mConv = (mConvInfo_t *)conv->ui_data;
	GaimIMChat *mIMChat = FindIMChatByGaimConv(conv);

	if (!mIMChat)
		return;

	switch (state)
	{
		case GAIM_TYPING:
			mState = IMChat::TypingStateTyping;
			break;

		case GAIM_TYPED:
			mState = IMChat::TypingStateStopTyping;
			break;

		default:
			mState = IMChat::TypingStateNotTyping;
			break;
	}

	// Get contact id. If it's a jabber contact, remove its ressource.
	const char *name = gaim_conversation_get_name(conv);
	GaimAccount *gAccount = gaim_conversation_get_account(conv);
	const char *gPrclId = gaim_account_get_protocol_id(gAccount);
	std::string contactId = cleanContactId(name,
		GaimIMPrcl::GetEnumIMProtocol(gPrclId));

	mIMChat->typingStateChangedEvent(*mIMChat, *((IMChatSession *)(mConv->conv_session)),
									contactId, mState);

}

/* **************** IMWRAPPER/GAIM INTERFACE ****************** */
GaimIMChat *GaimChatMngr::FindIMChatByGaimConv(void *gConv)
{
	GaimAccount *gAccount = gaim_conversation_get_account((GaimConversation *) gConv);
	const char *gPrclId = gaim_account_get_protocol_id(gAccount);
	IMAccount *account = _accountMngr->FindIMAccount(gaim_account_get_username(gAccount),
													GaimIMPrcl::GetEnumIMProtocol(gPrclId));
	GaimIMChat *mChat = FindIMChat(*account);

	return mChat;
}

/* **************** MANAGE CHAT_LIST ****************** */
GaimIMChat *GaimChatMngr::FindIMChat(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMChatIterator i;
	for (i = _gaimIMChatList.begin(); i != _gaimIMChatList.end(); i++)
	{
		if (GaimIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			return (*i);
		}
	}

	return NULL;
}

GaimIMChat *GaimChatMngr::AddIMChat(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMChat *mIMChat = FindIMChat(account);

	if (mIMChat == NULL)
	{
		mIMChat = new GaimIMChat(account);

		_gaimIMChatList.push_back(mIMChat);
	}

	return mIMChat;
}

void GaimChatMngr::RemoveIMChat(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMChatIterator i;
	for (i = _gaimIMChatList.begin(); i != _gaimIMChatList.end(); i++)
	{
		if (GaimIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			_gaimIMChatList.erase(i);
			break;
		}
	}
}

void GaimChatMngr::UpdateIMChat(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMChatIterator i;
	for (i = _gaimIMChatList.begin(); i != _gaimIMChatList.end(); i++)
	{
		if (GaimIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			(*i)->setIMAccount(account);
			break;
		}
	}
}

void GaimChatMngr::reset() {
	RecursiveMutex::ScopedLock lock(_mutex);

	_gaimIMChatList.clear();
}

/* ****************************************************** */
