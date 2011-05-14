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

#include "PurpleContactListMngr.h"
#include "PurplePresenceMngr.h"
#include "PurpleAccountMngr.h"
#include "PurpleEnumIMProtocol.h"
#include "PurpleEnumPresenceState.h"
#include "PurpleIMFactory.h"

extern "C" {
#include "libpurple/blist.h"
#include "libpurple/status.h"
}

#include <util/Logger.h>
#include <util/OWPicture.h>

/* ************ BUDDY ICON MANAGEMENT ************** */

void update_buddy_icon(PurpleBuddy *buddy)
{
	PurpleContactListMngr::UpdateBuddyIcon(buddy);
}

void init_buddy_icon_changed_event()
{
	void *handle = purple_wg_get_handle();

	purple_signal_connect(purple_blist_get_handle(), "buddy-icon-changed",
		handle, PURPLE_CALLBACK(update_buddy_icon), NULL);
}


/* ***************** PURPLE CALLBACK ***************** */
static void C_NewListCbk(PurpleBuddyList *blist)
{
	PurpleContactListMngr::NewListCbk(blist);
}

//VOXOX - JRT - 2009.04.11 - Add callback to handle 'initial load' notifications
static void C_SetInitialLoad(int nOnOff)
{
	PurpleContactListMngr::SetInitialLoad(nOnOff );
}
//End VoxOx

static void C_NewNodeCbk(PurpleBlistNode *node)
{
	PurpleContactListMngr::NewNodeCbk(node);
}

static void C_ShowCbk(PurpleBuddyList *list)
{
	PurpleContactListMngr::ShowCbk(list);
}

static void C_UpdateCbk(PurpleBuddyList *list, PurpleBlistNode *node)
{
	PurpleContactListMngr::UpdateCbk(list, node);
}

static void C_RemoveCbk(PurpleBuddyList *list, PurpleBlistNode *node)
{
	PurpleContactListMngr::RemoveCbk(list, node);
}

static void C_DestroyCbk(PurpleBuddyList *list)
{
	PurpleContactListMngr::DestroyCbk(list);
}

static void C_SetVisibleCbk(PurpleBuddyList *list, gboolean show)
{
	PurpleContactListMngr::SetVisibleCbk(list, show);
}

static void C_RequestAddBuddyCbk(PurpleAccount *account, const char *username,
	const char *group, const char *alias)
{
	PurpleContactListMngr::RequestAddBuddyCbk(account, username, group, alias);
}

static void C_RequestAddChatCbk(PurpleAccount *account, PurpleGroup *group,
	const char *alias, const char *name)
{
	PurpleContactListMngr::RequestAddChatCbk(account, group, alias, name);
}

static void C_RequestAddGroupCbk(void)
{
	PurpleContactListMngr::RequestAddGroupCbk();
}

PurpleBlistUiOps blist_wg_ops =
{
	C_NewListCbk,
	C_NewNodeCbk,
	C_ShowCbk,
	C_UpdateCbk,
	C_RemoveCbk,
	C_DestroyCbk,
	C_SetVisibleCbk,
	C_RequestAddBuddyCbk,
	C_RequestAddChatCbk,
	C_RequestAddGroupCbk,
	C_SetInitialLoad		//VOXOX - JRT - 2009.04.11 
};

PurpleBlistUiOps null_blist_wg_ops =
{
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
};

/* ************************************************** */

PurpleContactListMngr *PurpleContactListMngr::_staticInstance = NULL;
PurplePresenceMngr *PurpleContactListMngr::_presenceMngr = NULL;
PurpleAccountMngr *PurpleContactListMngr::_accountMngr = NULL;
std::list<PurpleIMContactList *> PurpleContactListMngr::_purpleIMContactListList;
RecursiveMutex PurpleContactListMngr::_mutex;

PurpleContactListMngr::PurpleContactListMngr()
{
}

void PurpleContactListMngr::Init()
{
	_presenceMngr = PurplePresenceMngr::getInstance();
	_accountMngr = PurpleAccountMngr::getInstance();
	purple_set_blist(purple_blist_new());
	purple_blist_load();
	init_buddy_icon_changed_event();
}

PurpleContactListMngr *PurpleContactListMngr::getInstance()
{
	if (!_staticInstance)
		_staticInstance = new PurpleContactListMngr();

	return _staticInstance;
}


void PurpleContactListMngr::NewListCbk(PurpleBuddyList *blist)
{
	//LOG_DEBUG("PurpleContactListMngr : NewListCbk()\n");
}

//VOXOX - JRT - 2009.04.11 - Add callback to handle 'initial load' notifications
void PurpleContactListMngr::SetInitialLoad(int nOnOff )
{
	//We take first because we are simply notifying UI that list parsing has started/stopped
	IMAccount *account = _accountMngr->GetFirstIMAccount();

	if (!account)
		return;

	PurpleIMContactList *mIMContactList = FindIMContactList(*account);
	if (mIMContactList)
	{
		mIMContactList->setInitialLoadEvent(*mIMContactList, nOnOff );
	}
}
//End VoxOx

const char *PurpleContactListMngr::FindBuddyGroup(PurpleBuddy *gBuddy)
{
	PurpleGroup *gGroup = purple_buddy_get_group(gBuddy);
	
	if (gGroup)
		return gGroup->name;
	else
		return NULL;
}

void PurpleContactListMngr::NewBuddyAdded(PurpleBuddy *gBuddy)
{
	PurpleAccount *gAccount = purple_buddy_get_account(gBuddy);
	const char *gPrclId = purple_account_get_protocol_id(gAccount);
	IMAccount *account = _accountMngr->FindIMAccount(purple_account_get_username(gAccount),
		PurpleIMPrcl::GetEnumIMProtocol(gPrclId));
	
	if (account == NULL)
		return;

	PurpleIMContactList *mIMContactList = FindIMContactList(*account);
	if (mIMContactList)
	{
		const char *groupName = FindBuddyGroup(gBuddy);

		if (groupName) 
		{
			mIMContactList->newContactAddedEvent(*mIMContactList, groupName, purple_buddy_get_name(gBuddy));
		}
	}
}

void PurpleContactListMngr::NewGroupAdded(PurpleGroup *gGroup)
{
	// PURPLE CONTACT LIST GROUPS ARE NOT ASSOCIATED WITH ANY ACCOUNTS
	// THAT'S WHY WE TAKE THE FIRST FOUND IM_CONTACTLIST 
	// TO SEND NEW_GROUP_ADDDED EVENT

	IMAccount *account = _accountMngr->GetFirstIMAccount();

	if (!account)
		return;

	PurpleIMContactList *gIMContactList = FindIMContactList(*account);
	if (gIMContactList)
	{
		gIMContactList->newContactGroupAddedEvent(*gIMContactList, gGroup->name);
	}
}

void PurpleContactListMngr::NewNodeCbk(PurpleBlistNode *node)
{
	switch (node->type)
	{
		case PURPLE_BLIST_BUDDY_NODE:
			NewBuddyAdded((PurpleBuddy *) node);
			break;

		case PURPLE_BLIST_GROUP_NODE:
			NewGroupAdded((PurpleGroup *) node);
			break;

		case PURPLE_BLIST_CONTACT_NODE:
			break;

		case PURPLE_BLIST_CHAT_NODE:
			break;

		default:
			break;
	}
}


void PurpleContactListMngr::ShowCbk(PurpleBuddyList *list)
{
	//LOG_DEBUG("PurpleContactListMngr : ShowCbk()\n");
}


void PurpleContactListMngr::UpdateBuddy(PurpleBuddyList *list, PurpleBuddy *gBuddy)
{
	IMAccount *account = NULL;
	PurpleAccount	*gAccount = purple_buddy_get_account(gBuddy);
	PurplePresence *gPresence = purple_buddy_get_presence(gBuddy);
	const char *gPrclId = purple_account_get_protocol_id(gAccount);
	
	account = _accountMngr->FindIMAccount(purple_account_get_username(gAccount),
		PurpleIMPrcl::GetEnumIMProtocol(gPrclId));
	
	if (account)
	{
		PurpleIMContactList *mIMBList = FindIMContactList(*account);
		PurpleIMPresence *mIMPresence = _presenceMngr->FindIMPresence(*account);
	

		if (mIMBList)
		{
			const char * groupName = FindBuddyGroup(gBuddy);
			if (groupName)
			{
				mIMBList->contactMovedEvent(*mIMBList,
					groupName, purple_buddy_get_name(gBuddy));
			}
		}

		if (mIMPresence)
		{
			const char* buddy_alias   = gBuddy->server_alias && *gBuddy->server_alias ? gBuddy->server_alias : gBuddy->alias;
			const char* statusMessage = PurpleIMPresence::getPurpleBuddyStatusMessage(gBuddy);
			const char* gPresenceId   = PurpleIMPresence::getPurplePresenceId(gPresence);

			//VOXOX - JRT - 2009.07.20 - COE in this method.  Is there a way to alert libpurple that we are quitting?
			//VOXOX - JRT - 2009.09.21 - Still here.  Let's see if we can catch problem in the parameters.
			EnumPresenceState::PresenceState presenceState = PurplePreState::GetPresenceState(gPresenceId);

			std::string   alias     = !buddy_alias   ? String::null : buddy_alias;
			std::string   statusMsg = !statusMessage ? String::null : statusMessage;
			const char*   from		= purple_buddy_get_name(gBuddy);

			mIMPresence->presenceStateChangedEvent(*mIMPresence, presenceState, alias, statusMsg, from );
		}
	}
}

void PurpleContactListMngr::UpdateBuddyIcon(PurpleBuddy *buddy)
{
	IMAccount *account = NULL;
	PurpleIMPresence *mIMPresence = NULL;
	PurpleAccount	*gAccount = purple_buddy_get_account(buddy);
	const char *gPrclId = purple_account_get_protocol_id(gAccount);
	
	account = _accountMngr->FindIMAccount(purple_account_get_username(gAccount),
		PurpleIMPrcl::GetEnumIMProtocol(gPrclId));

	if (!account)
		return;

	mIMPresence = _presenceMngr->FindIMPresence(*account);

	if (!mIMPresence)
	{
		LOG_FATAL("IMPresence not found!");
		return;
	}
	
	size_t size;
	OWPicture picture;
	const char *data = (const char *)purple_buddy_icon_get_data(buddy->icon, &size);
		
	if (data && size > 0)
		picture = OWPicture::pictureFromData(std::string(data, size));

	mIMPresence->contactIconChangedEvent(*mIMPresence, purple_buddy_get_name(buddy), picture);
}

void PurpleContactListMngr::UpdateCbk(PurpleBuddyList *list, PurpleBlistNode *node)
{
	switch (node->type)
	{
		case PURPLE_BLIST_BUDDY_NODE:
			UpdateBuddy(list, (PurpleBuddy *)node);
			break;

		default:
			break;
	}
}

void PurpleContactListMngr::RemoveCbk(PurpleBuddyList *list, PurpleBlistNode *node)
{
	//LOG_DEBUG("PurpleContactListMngr : RemoveCbk()\n");
}

void PurpleContactListMngr::DestroyCbk(PurpleBuddyList *list)
{
	//LOG_DEBUG("PurpleContactListMngr : DestroyCbk()");
}

void PurpleContactListMngr::SetVisibleCbk(PurpleBuddyList *list, gboolean show)
{
	//LOG_DEBUG("PurpleContactListMngr : SetVisibleCbk()\n");
}

void PurpleContactListMngr::RequestAddBuddyCbk(PurpleAccount *account, const char *username,
	const char *group, const char *alias)
{
	//LOG_DEBUG("PurpleContactListMngr : RequestAddBuddyCbk()\n");
}

void PurpleContactListMngr::RequestAddChatCbk(PurpleAccount *account, PurpleGroup *group,
	const char *alias, const char *name)
{
	//LOG_DEBUG("PurpleContactListMngr : RequestAddChatCbk()\n");
}

void PurpleContactListMngr::RequestAddGroupCbk(void)
{
	//LOG_DEBUG("PurpleContactListMngr : RequestAddGroupCbk()\n");
}

/* **************** MANAGE CONTACT LIST (Buddy list) ****************** */

PurpleIMContactList *PurpleContactListMngr::FindIMContactList(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleIMContactListIterator i;
	for (i = _purpleIMContactListList.begin(); i != _purpleIMContactListList.end(); i++)
	{
		if (PurpleIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			return (*i);
		}
	}

	return NULL;
}

PurpleIMContactList *PurpleContactListMngr::AddIMContactList(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleIMContactList *mIMContactList = FindIMContactList(account);

	if (mIMContactList == NULL)
	{
		mIMContactList = new PurpleIMContactList(account);
		_purpleIMContactListList.push_back(mIMContactList);
	}

	return mIMContactList;
}

void PurpleContactListMngr::RemoveIMContactList(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleIMContactListIterator i;
	for (i = _purpleIMContactListList.begin(); i != _purpleIMContactListList.end(); i++)
	{
		if (PurpleIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			_purpleIMContactListList.erase(i);
			break;
		}
	}
}

void PurpleContactListMngr::UpdateIMContactList(IMAccount &account) {
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleIMContactListIterator i;
	for (i = _purpleIMContactListList.begin(); i != _purpleIMContactListList.end(); i++)
	{
		if (PurpleIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			(*i)->setIMAccount(account);
			break;
		}
	}
}

void PurpleContactListMngr::reset() {
	RecursiveMutex::ScopedLock lock(_mutex);

	_purpleIMContactListList.clear();
}
