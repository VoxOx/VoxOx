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

#ifndef GAIMCONTACTLISTMNGR_H
#define GAIMCONTACTLISTMNGR_H

#include <imwrapper/IMContactList.h>

#include "GaimIMContactList.h"

#include <thread/RecursiveMutex.h>

#include <list>

extern "C" {
#include "gaim/blist.h"
}

class GaimPresenceMngr;
class GaimAccountMngr;

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class GaimContactListMngr
{
public:

	static GaimContactListMngr *getInstance();

	void Init();

	void reset();
	
	GaimIMContactList *AddIMContactList(IMAccount &account);
	void RemoveIMContactList(IMAccount &account);
	void UpdateIMContactList(IMAccount &account);
	static GaimIMContactList *FindIMContactList(IMAccount &account);

	/* ********** GaimBuddyListCallback *********** */
	static void NewListCbk(GaimBuddyList *blist);
	static void NewNodeCbk(GaimBlistNode *node);
	static void ShowCbk(GaimBuddyList *list);
	static void UpdateCbk(GaimBuddyList *list, GaimBlistNode *node);
	static void RemoveCbk(GaimBuddyList *list, GaimBlistNode *node);
	static void DestroyCbk(GaimBuddyList *list);
	static void SetVisibleCbk(GaimBuddyList *list, gboolean show);
	static void RequestAddBuddyCbk(GaimAccount *account, const char *username,
									const char *group, const char *alias);
	static void RequestAddChatCbk(GaimAccount *account, GaimGroup *group,
									const char *alias, const char *name);
	static void RequestAddGroupCbk(void);
	static void UpdateBuddyIcon(GaimBuddy *gBuddy);
	/* ******************************************** */

private:

	GaimContactListMngr();
	static GaimContactListMngr *_staticInstance;
	static GaimPresenceMngr *_presenceMngr;
	static GaimAccountMngr *_accountMngr;

	static void UpdateBuddy(GaimBuddyList *list, GaimBuddy *gBuddy);
	static const char *FindBuddyGroup(GaimBuddy *gBuddy);
	static void NewBuddyAdded(GaimBuddy *gBuddy);
	static void NewGroupAdded(GaimGroup *gGroup);

	static std::list<GaimIMContactList *> _gaimIMContactListList;
	typedef std::list<GaimIMContactList *>::iterator GaimIMContactListIterator;

	static RecursiveMutex _mutex;

};

#endif	//GAIMCONTACTLISTMNGR_H
