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

#ifndef PURPLECONTACTLISTMNGR_H
#define PURPLECONTACTLISTMNGR_H

#include <imwrapper/IMContactList.h>

#include "PurpleIMContactList.h"

#include <thread/RecursiveMutex.h>

#include <list>

extern "C" {
#include "libpurple/blist.h"
}

class PurplePresenceMngr;
class PurpleAccountMngr;

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class PurpleContactListMngr
{
public:

	static PurpleContactListMngr *getInstance();

	void Init();

	void reset();
	
	PurpleIMContactList *AddIMContactList(IMAccount &account);
	void RemoveIMContactList(IMAccount &account);
	void UpdateIMContactList(IMAccount &account);
	static PurpleIMContactList *FindIMContactList(IMAccount &account);

	/* ********** PurpleBuddyListCallback *********** */
	static void SetInitialLoad(int nOnOff);			//VOXOX - JRT - 2009.04.11 
	static void NewListCbk(PurpleBuddyList *blist);
	static void NewNodeCbk(PurpleBlistNode *node);
	static void ShowCbk(PurpleBuddyList *list);
	static void UpdateCbk(PurpleBuddyList *list, PurpleBlistNode *node);
	static void RemoveCbk(PurpleBuddyList *list, PurpleBlistNode *node);
	static void DestroyCbk(PurpleBuddyList *list);
	static void SetVisibleCbk(PurpleBuddyList *list, gboolean show);
	static void RequestAddBuddyCbk(PurpleAccount *account, const char *username,
									const char *group, const char *alias);
	static void RequestAddChatCbk(PurpleAccount *account, PurpleGroup *group,
									const char *alias, const char *name);
	static void RequestAddGroupCbk(void);
	static void UpdateBuddyIcon(PurpleBuddy *gBuddy);
	/* ******************************************** */

private:

	PurpleContactListMngr();
	static PurpleContactListMngr *_staticInstance;
	static PurplePresenceMngr *_presenceMngr;
	static PurpleAccountMngr *_accountMngr;

	static void UpdateBuddy(PurpleBuddyList *list, PurpleBuddy *gBuddy);
	static const char *FindBuddyGroup(PurpleBuddy *gBuddy);
	static void NewBuddyAdded(PurpleBuddy *gBuddy);
	static void NewGroupAdded(PurpleGroup *gGroup);

	static std::list<PurpleIMContactList *> _purpleIMContactListList;
	typedef std::list<PurpleIMContactList *>::iterator PurpleIMContactListIterator;

	static RecursiveMutex _mutex;

};

#endif	//PURPLECONTACTLISTMNGR_H
