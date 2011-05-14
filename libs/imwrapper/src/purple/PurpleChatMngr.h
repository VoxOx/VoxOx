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

#ifndef PURPLECHATMNGR_H
#define PURPLECHATMNGR_H

#include "PurpleIMChat.h"
#include "PurpleAccountMngr.h"

#include <thread/RecursiveMutex.h>

#include <list>

extern "C" {
#include "libpurple/conversation.h"
}

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class PurpleChatMngr
{
public:

	static PurpleChatMngr *getInstance();

	void Init();

	void reset();
	
	PurpleIMChat *AddIMChat(IMAccount &account);
	void RemoveIMChat(IMAccount &account);
	void UpdateIMChat(IMAccount &account);

	/* *************** PurpleChatCallback **************** */
	static void ChatJoinedCbk(PurpleConversation *conv);
	static void ChatLeftCbk  (PurpleConversation *conv);	//VOXOX - JRT - 2009.06.16 

	static void CreateConversationCbk(PurpleConversation *conv, bool userCreated);
	static void DestroyConversationCbk(PurpleConversation *conv);
	static void WriteChatCbk(PurpleConversation *conv, const char *who,
							const char *message, PurpleMessageFlags flags, time_t mtime);
	static void WriteIMCbk(PurpleConversation *conv, const char *who,
							const char *message, PurpleMessageFlags flags, time_t mtime);
	static void WriteConvCbk(PurpleConversation *conv, const char *name, const char *alias,
							const char *message, PurpleMessageFlags flags, time_t mtime);
	static void ChatAddUsersCbk(PurpleConversation *conv, GList *users,
								gboolean new_arrivals);
	static void ChatRenameUserCbk(PurpleConversation *conv, const char *old_name,
								const char *new_name, const char *new_alias);
	static void ChatRemoveUsersCbk(PurpleConversation *conv, GList *users);
	static void ChatUpdateUserCbk(PurpleConversation *conv, const char *user);
	static void PresentConvCbk(PurpleConversation *conv);
	static gboolean HasFocusCbk(PurpleConversation *conv);
	static gboolean CustomSmileyAddCbk(PurpleConversation *conv, const char *smile,
										gboolean remote);
	static void CustomSmileyWriteCbk(PurpleConversation *conv, const char *smile,
									const guchar *data, gsize size);
	static void CustomSmileyCloseCbk(PurpleConversation *conv, const char *smile);
	static void SendConfirm(PurpleConversation *conv, const char *message);
	/* ************************************************** */

	static void UpdateBuddyTyping( PurpleConversation *conv, PurpleTypingState state );
	static int  HandleChatInvite ( PurpleAccount *account, const char *who, const char* name, const char *message, void *data );	//VOXOX - JRT - 2009.06.16 
//	static void ConversationCreatedCbk(PurpleConversation *conv );		//VOXOX - JRT - 2009.07.09 

private:

	PurpleChatMngr();
	static PurpleChatMngr *_staticInstance;

	static PurpleAccountMngr *_accountMngr;

	static PurpleIMChat *FindIMChatByPurpleConv(void *gConv);

	static std::string cleanContactId(std::string contactId, EnumIMProtocol::IMProtocol prtcl);

	/* ********** CHAT_LIST *********** */
	static std::list<PurpleIMChat *> _purpleIMChatList;
	typedef std::list<PurpleIMChat *>::iterator PurpleIMChatIterator;
	static PurpleIMChat *FindIMChat(IMAccount &account);
	/* *********************************** */

	static RecursiveMutex _mutex;

};

#endif	//PURPLECHATMNGR_H
