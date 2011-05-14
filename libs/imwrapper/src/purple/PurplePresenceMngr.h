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

#ifndef PURPLEPRESENCEMNGR_H
#define PURPLEPRESENCEMNGR_H

#include <list>

#include "PurpleIMPresence.h"

#include <thread/RecursiveMutex.h>

extern "C" {
#include <libpurple/account.h>
}

class PurpleAccountMngr;

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class PurplePresenceMngr
{
public:

	static PurplePresenceMngr *getInstance();

	void Init();

	void reset();

	static PurpleIMPresence *AddIMPresence(IMAccount &account);
	void RemoveIMPresence(IMAccount &account);
	void UpdateIMPresence(IMAccount &account);
	static PurpleIMPresence *FindIMPresence(IMAccount &account);

	/* ********** PurplePrivacyCallback *********** */
	static void PermitAddedCbk(PurpleAccount *account, const char *name);
	static void PermitRemovedCbk(PurpleAccount *account, const char *name);
	static void DenyAddedCbk(PurpleAccount *account, const char *name);
	static void DenyRemovedCbk(PurpleAccount *account, const char *name);
	static void UpdateAccountStatus(PurpleAccount *account, PurpleStatus *old, PurpleStatus *newstatus);
	/* ******************************************** */

private:

	PurplePresenceMngr();
	static PurplePresenceMngr *_staticInstance;
	static PurpleAccountMngr *_accountMngr;

	/* ********** PRESENCE_LIST *********** */
	static std::list<PurpleIMPresence *> _purpleIMPresenceList;
	typedef std::list<PurpleIMPresence *>::iterator PurpleIMPresenceIterator;
	/* ********** PRESENCE_LIST *********** */

	static RecursiveMutex _mutex;

};

#endif	//PURPLEPRESENCEMNGR_H
