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

#ifndef GAIMPRESENCEMNGR_H
#define GAIMPRESENCEMNGR_H

#include <list>

#include "GaimIMPresence.h"

#include <thread/RecursiveMutex.h>

extern "C" {
#include <gaim/account.h>
}

class GaimAccountMngr;

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class GaimPresenceMngr
{
public:

	static GaimPresenceMngr *getInstance();

	void Init();

	void reset();

	static GaimIMPresence *AddIMPresence(IMAccount &account);
	void RemoveIMPresence(IMAccount &account);
	void UpdateIMPresence(IMAccount &account);
	static GaimIMPresence *FindIMPresence(IMAccount &account);

	/* ********** GaimPrivacyCallback *********** */
	static void PermitAddedCbk(GaimAccount *account, const char *name);
	static void PermitRemovedCbk(GaimAccount *account, const char *name);
	static void DenyAddedCbk(GaimAccount *account, const char *name);
	static void DenyRemovedCbk(GaimAccount *account, const char *name);
	static void UpdateAccountStatus(GaimAccount *account, GaimStatus *old, GaimStatus *newstatus);
	/* ******************************************** */

private:

	GaimPresenceMngr();
	static GaimPresenceMngr *_staticInstance;
	static GaimAccountMngr *_accountMngr;

	/* ********** PRESENCE_LIST *********** */
	static std::list<GaimIMPresence *> _gaimIMPresenceList;
	typedef std::list<GaimIMPresence *>::iterator GaimIMPresenceIterator;
	/* ********** PRESENCE_LIST *********** */

	static RecursiveMutex _mutex;

};

#endif	//GAIMPRESENCEMNGR_H
