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

#ifndef GAIMCHATMNGR_H
#define GAIMCHATMNGR_H

#include "GaimIMChat.h"
#include "GaimAccountMngr.h"

#include <thread/RecursiveMutex.h>

#include <list>

extern "C" {
#include "gaim/conversation.h"
}

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class GaimChatMngr
{
public:

	static GaimChatMngr *getInstance();

	void Init();

	void reset();
	
	GaimIMChat *AddIMChat(IMAccount &account);
	void RemoveIMChat(IMAccount &account);
	void UpdateIMChat(IMAccount &account);

	/* *************** GaimChatCallback **************** */
	static void ChatJoinedCbk(GaimConversation *conv);

	static void CreateConversationCbk(GaimConversation *conv, bool userCreated);
	static void DestroyConversationCbk(GaimConversation *conv);
	static void WriteChatCbk(GaimConversation *conv, const char *who,
							const char *message, GaimMessageFlags flags, time_t mtime);
	static void WriteIMCbk(GaimConversation *conv, const char *who,
							const char *message, GaimMessageFlags flags, time_t mtime);
	static void WriteConvCbk(GaimConversation *conv, const char *name, const char *alias,
							const char *message, GaimMessageFlags flags, time_t mtime);
	static void ChatAddUsersCbk(GaimConversation *conv, GList *users,
								gboolean new_arrivals);
	static void ChatRenameUserCbk(GaimConversation *conv, const char *old_name,
								const char *new_name, const char *new_alias);
	static void ChatRemoveUsersCbk(GaimConversation *conv, GList *users);
	static void ChatUpdateUserCbk(GaimConversation *conv, const char *user);
	static void PresentConvCbk(GaimConversation *conv);
	static gboolean HasFocusCbk(GaimConversation *conv);
	static gboolean CustomSmileyAddCbk(GaimConversation *conv, const char *smile,
										gboolean remote);
	static void CustomSmileyWriteCbk(GaimConversation *conv, const char *smile,
									const guchar *data, gsize size);
	static void CustomSmileyCloseCbk(GaimConversation *conv, const char *smile);
	static void SendConfirm(GaimConversation *conv, const char *message);
	/* ************************************************** */

	static void UpdateBuddyTyping(GaimConversation *conv, GaimTypingState state);

private:

	GaimChatMngr();
	static GaimChatMngr *_staticInstance;

	static GaimAccountMngr *_accountMngr;

	static GaimIMChat *FindIMChatByGaimConv(void *gConv);

	static std::string cleanContactId(std::string contactId, EnumIMProtocol::IMProtocol prtcl);

	/* ********** CHAT_LIST *********** */
	static std::list<GaimIMChat *> _gaimIMChatList;
	typedef std::list<GaimIMChat *>::iterator GaimIMChatIterator;
	static GaimIMChat *FindIMChat(IMAccount &account);
	/* *********************************** */

	static RecursiveMutex _mutex;

};

#endif	//GAIMCHATMNGR_H
