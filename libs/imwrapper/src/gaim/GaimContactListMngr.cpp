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

#include "GaimContactListMngr.h"
#include "GaimPresenceMngr.h"
#include "GaimAccountMngr.h"
#include "GaimEnumIMProtocol.h"
#include "GaimEnumPresenceState.h"
#include "GaimIMFactory.h"

extern "C" {
#include "gaim/blist.h"
#include "gaim/status.h"
}

#include <util/Logger.h>
#include <util/OWPicture.h>

/* ************ BUDDY ICON MANAGEMENT ************** */

void update_buddy_icon(GaimBuddy *buddy)
{
	GaimContactListMngr::UpdateBuddyIcon(buddy);
}

void init_buddy_icon_changed_event()
{
	void *handle = gaim_wg_get_handle();

	gaim_signal_connect(gaim_blist_get_handle(), "buddy-icon-changed",
		handle, GAIM_CALLBACK(update_buddy_icon), NULL);
}


/* ***************** GAIM CALLBACK ***************** */
static void C_NewListCbk(GaimBuddyList *blist)
{
	GaimContactListMngr::NewListCbk(blist);
}

static void C_NewNodeCbk(GaimBlistNode *node)
{
	GaimContactListMngr::NewNodeCbk(node);
}

static void C_ShowCbk(GaimBuddyList *list)
{
	GaimContactListMngr::ShowCbk(list);
}

static void C_UpdateCbk(GaimBuddyList *list, GaimBlistNode *node)
{
	GaimContactListMngr::UpdateCbk(list, node);
}

static void C_RemoveCbk(GaimBuddyList *list, GaimBlistNode *node)
{
	GaimContactListMngr::RemoveCbk(list, node);
}

static void C_DestroyCbk(GaimBuddyList *list)
{
	GaimContactListMngr::DestroyCbk(list);
}

static void C_SetVisibleCbk(GaimBuddyList *list, gboolean show)
{
	GaimContactListMngr::SetVisibleCbk(list, show);
}

static void C_RequestAddBuddyCbk(GaimAccount *account, const char *username,
	const char *group, const char *alias)
{
	GaimContactListMngr::RequestAddBuddyCbk(account, username, group, alias);
}

static void C_RequestAddChatCbk(GaimAccount *account, GaimGroup *group,
	const char *alias, const char *name)
{
	GaimContactListMngr::RequestAddChatCbk(account, group, alias, name);
}

static void C_RequestAddGroupCbk(void)
{
	GaimContactListMngr::RequestAddGroupCbk();
}

GaimBlistUiOps blist_wg_ops =
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
	C_RequestAddGroupCbk
};

GaimBlistUiOps null_blist_wg_ops =
{
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
};

/* ************************************************** */

GaimContactListMngr *GaimContactListMngr::_staticInstance = NULL;
GaimPresenceMngr *GaimContactListMngr::_presenceMngr = NULL;
GaimAccountMngr *GaimContactListMngr::_accountMngr = NULL;
std::list<GaimIMContactList *> GaimContactListMngr::_gaimIMContactListList;
RecursiveMutex GaimContactListMngr::_mutex;

GaimContactListMngr::GaimContactListMngr()
{
}

void GaimContactListMngr::Init()
{
	_presenceMngr = GaimPresenceMngr::getInstance();
	_accountMngr = GaimAccountMngr::getInstance();
	gaim_set_blist(gaim_blist_new());
	gaim_blist_load();
	init_buddy_icon_changed_event();
}

GaimContactListMngr *GaimContactListMngr::getInstance()
{
	if (!_staticInstance)
		_staticInstance = new GaimContactListMngr();

	return _staticInstance;
}


void GaimContactListMngr::NewListCbk(GaimBuddyList *blist)
{
	//LOG_DEBUG("GaimContactListMngr : NewListCbk()\n");
}

const char *GaimContactListMngr::FindBuddyGroup(GaimBuddy *gBuddy)
{
	GaimGroup *gGroup = gaim_buddy_get_group(gBuddy);
	
	if (gGroup)
		return gGroup->name;
	else
		return NULL;
}

void GaimContactListMngr::NewBuddyAdded(GaimBuddy *gBuddy)
{
	GaimAccount *gAccount = gaim_buddy_get_account(gBuddy);
	const char *gPrclId = gaim_account_get_protocol_id(gAccount);
	IMAccount *account = _accountMngr->FindIMAccount(gaim_account_get_username(gAccount),
		GaimIMPrcl::GetEnumIMProtocol(gPrclId));
	
	if (account == NULL)
		return;

	GaimIMContactList *mIMContactList = FindIMContactList(*account);
	if (mIMContactList)
	{
		const char *groupName = FindBuddyGroup(gBuddy);

		if (groupName) {
			mIMContactList->newContactAddedEvent(*mIMContactList,
				groupName, gaim_buddy_get_name(gBuddy));
		}
	}
}

void GaimContactListMngr::NewGroupAdded(GaimGroup *gGroup)
{
	// GAIM CONTACT LIST GROUPS ARE NOT ASSOCIATED WITH ANY ACCOUNTS
	// THAT'S WHY WE TAKE THE FIRST FOUND IM_CONTACTLIST 
	// TO SEND NEW_GROUP_ADDDED EVENT

	IMAccount *account = _accountMngr->GetFirstIMAccount();

	if (!account)
		return;

	GaimIMContactList *gIMContactList = FindIMContactList(*account);
	if (gIMContactList)
	{
		gIMContactList->newContactGroupAddedEvent(*gIMContactList, gGroup->name);
	}
}

void GaimContactListMngr::NewNodeCbk(GaimBlistNode *node)
{
	switch (node->type)
	{
		case GAIM_BLIST_BUDDY_NODE:
			NewBuddyAdded((GaimBuddy *) node);
			break;

		case GAIM_BLIST_GROUP_NODE:
			NewGroupAdded((GaimGroup *) node);
			break;

		case GAIM_BLIST_CONTACT_NODE:
			break;

		case GAIM_BLIST_CHAT_NODE:
			break;

		default:
			break;
	}
}


void GaimContactListMngr::ShowCbk(GaimBuddyList *list)
{
	//LOG_DEBUG("GaimContactListMngr : ShowCbk()\n");
}



void GaimContactListMngr::UpdateBuddy(GaimBuddyList *list, GaimBuddy *gBuddy)
{
	IMAccount *account = NULL;
	GaimAccount	*gAccount = gaim_buddy_get_account(gBuddy);
	GaimPresence *gPresence = gaim_buddy_get_presence(gBuddy);
	const char *gPrclId = gaim_account_get_protocol_id(gAccount);
	
	account = _accountMngr->FindIMAccount(gaim_account_get_username(gAccount),
		GaimIMPrcl::GetEnumIMProtocol(gPrclId));
	
	if (account)
	{
		GaimIMContactList *mIMBList = FindIMContactList(*account);
		GaimIMPresence *mIMPresence = _presenceMngr->FindIMPresence(*account);
	
		if (mIMBList)
		{
			const char * groupName = FindBuddyGroup(gBuddy);
			if (groupName)
			{
				mIMBList->contactMovedEvent(*mIMBList,
					groupName, gaim_buddy_get_name(gBuddy));
			}
		}

		if (mIMPresence)
		{
			const char *buddy_alias =
				gBuddy->server_alias && *gBuddy->server_alias ? gBuddy->server_alias : gBuddy->alias;
			const char *gPresenceId = GaimIMPresence::getGaimPresenceId(gPresence);

			mIMPresence->presenceStateChangedEvent(*mIMPresence,
				GaimPreState::GetPresenceState(gPresenceId),
				!buddy_alias ? String::null : buddy_alias,
				gaim_buddy_get_name(gBuddy));
		}
	}
}

void GaimContactListMngr::UpdateBuddyIcon(GaimBuddy *buddy)
{
	IMAccount *account = NULL;
	GaimIMPresence *mIMPresence = NULL;
	GaimAccount	*gAccount = gaim_buddy_get_account(buddy);
	const char *gPrclId = gaim_account_get_protocol_id(gAccount);
	
	account = _accountMngr->FindIMAccount(gaim_account_get_username(gAccount),
		GaimIMPrcl::GetEnumIMProtocol(gPrclId));

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
	const char *data = (const char *)gaim_buddy_icon_get_data(buddy->icon, &size);
		
	if (data && size > 0)
		picture = OWPicture::pictureFromData(std::string(data, size));

	mIMPresence->contactIconChangedEvent(*mIMPresence, gaim_buddy_get_name(buddy), picture);
}

void GaimContactListMngr::UpdateCbk(GaimBuddyList *list, GaimBlistNode *node)
{
	switch (node->type)
	{
		case GAIM_BLIST_BUDDY_NODE:
			UpdateBuddy(list, (GaimBuddy *)node);
			break;

		default:
			break;
	}
}

void GaimContactListMngr::RemoveCbk(GaimBuddyList *list, GaimBlistNode *node)
{
	//LOG_DEBUG("GaimContactListMngr : RemoveCbk()\n");
}

void GaimContactListMngr::DestroyCbk(GaimBuddyList *list)
{
	//LOG_DEBUG("GaimContactListMngr : DestroyCbk()");
}

void GaimContactListMngr::SetVisibleCbk(GaimBuddyList *list, gboolean show)
{
	//LOG_DEBUG("GaimContactListMngr : SetVisibleCbk()\n");
}

void GaimContactListMngr::RequestAddBuddyCbk(GaimAccount *account, const char *username,
	const char *group, const char *alias)
{
	//LOG_DEBUG("GaimContactListMngr : RequestAddBuddyCbk()\n");
}

void GaimContactListMngr::RequestAddChatCbk(GaimAccount *account, GaimGroup *group,
	const char *alias, const char *name)
{
	//LOG_DEBUG("GaimContactListMngr : RequestAddChatCbk()\n");
}

void GaimContactListMngr::RequestAddGroupCbk(void)
{
	//LOG_DEBUG("GaimContactListMngr : RequestAddGroupCbk()\n");
}

/* **************** MANAGE CONTACT LIST (Buddy list) ****************** */

GaimIMContactList *GaimContactListMngr::FindIMContactList(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMContactListIterator i;
	for (i = _gaimIMContactListList.begin(); i != _gaimIMContactListList.end(); i++)
	{
		if (GaimIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			return (*i);
		}
	}

	return NULL;
}

GaimIMContactList *GaimContactListMngr::AddIMContactList(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMContactList *mIMContactList = FindIMContactList(account);

	if (mIMContactList == NULL)
	{
		mIMContactList = new GaimIMContactList(account);
		_gaimIMContactListList.push_back(mIMContactList);
	}

	return mIMContactList;
}

void GaimContactListMngr::RemoveIMContactList(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMContactListIterator i;
	for (i = _gaimIMContactListList.begin(); i != _gaimIMContactListList.end(); i++)
	{
		if (GaimIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			_gaimIMContactListList.erase(i);
			break;
		}
	}
}

void GaimContactListMngr::UpdateIMContactList(IMAccount &account) {
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMContactListIterator i;
	for (i = _gaimIMContactListList.begin(); i != _gaimIMContactListList.end(); i++)
	{
		if (GaimIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			(*i)->setIMAccount(account);
			break;
		}
	}
}

void GaimContactListMngr::reset() {
	RecursiveMutex::ScopedLock lock(_mutex);

	_gaimIMContactListList.clear();
}
