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

#include "PurplePresenceMngr.h"
#include "PurpleAccountMngr.h"
#include "PurpleIMFactory.h"

extern "C" {
#include "libpurple/privacy.h"
#include "libpurple/prefs.h"
}

#include <util/Logger.h>

/* ************ STATUS MANAGEMENT ************** */

void update_account_status(PurpleAccount *account, PurpleStatus *old, PurpleStatus *newstatus)
{
	PurplePresenceMngr::UpdateAccountStatus(account, old, newstatus);
}

void init_account_status_changed_event()
{
	void *handle = purple_wg_get_handle();

	purple_signal_connect(purple_accounts_get_handle(), "account-status-changed",
		handle, PURPLE_CALLBACK(update_account_status), NULL);
}


/* ***************** PURPLE CALLBACK ***************** */
static void C_PermitAddedCbk(PurpleAccount *account, const char *name)
{
	PurplePresenceMngr::PermitAddedCbk(account, name);
}

static void C_PermitRemovedCbk(PurpleAccount *account, const char *name)
{
	PurplePresenceMngr::PermitRemovedCbk(account, name);
}

static void C_DenyAddedCbk(PurpleAccount *account, const char *name)
{
	PurplePresenceMngr::DenyAddedCbk(account, name);
}

static void C_DenyRemovedCbk(PurpleAccount *account, const char *name)
{
	PurplePresenceMngr::DenyRemovedCbk(account, name);
}

PurplePrivacyUiOps privacy_wg_ops =	{
										C_PermitAddedCbk,
										C_PermitRemovedCbk,
										C_DenyAddedCbk,
										C_DenyRemovedCbk
									};

/* ************************************************** */

PurpleAccountMngr *PurplePresenceMngr::_accountMngr = NULL;
PurplePresenceMngr *PurplePresenceMngr::_staticInstance = NULL;
std::list<PurpleIMPresence *> PurplePresenceMngr::_purpleIMPresenceList;
RecursiveMutex PurplePresenceMngr::_mutex;

PurplePresenceMngr::PurplePresenceMngr()
{
}

void PurplePresenceMngr::Init()
{
	_accountMngr = PurpleAccountMngr::getInstance();
	init_account_status_changed_event();
	
	purple_prefs_set_string("/core/away/idle_reporting", "none");
	purple_prefs_set_string("/core/away/auto_reply", "never");
	purple_prefs_set_bool("/core/away/away_when_idle", FALSE);
}

PurplePresenceMngr *PurplePresenceMngr::getInstance()
{
	if (!_staticInstance)
		_staticInstance = new PurplePresenceMngr();

	return _staticInstance;
}

void PurplePresenceMngr::UpdateAccountStatus(PurpleAccount *gAccount, 
										   PurpleStatus *old, 
										   PurpleStatus *newstatus)
{
	PurpleIMPresence *mIMPresence = NULL;
	const char *gPrclId = purple_account_get_protocol_id(gAccount);
	IMAccount *account = _accountMngr->FindIMAccount(purple_account_get_username(gAccount),
		PurpleIMPrcl::GetEnumIMProtocol(gPrclId));

	if (!account)
		return;

	mIMPresence = FindIMPresence(*account);
	if (!mIMPresence)
	{
		LOG_FATAL("IMPresence not found!");
		return;
	}

	mIMPresence->myPresenceStatusEvent(*mIMPresence, EnumPresenceState::MyPresenceStatusOk, "");

}

void PurplePresenceMngr::PermitAddedCbk(PurpleAccount *account, const char *name)
{
	//LOG_DEBUG("PurplePresenceMngr : PermitAddedCbk()\n");
}

void PurplePresenceMngr::PermitRemovedCbk(PurpleAccount *account, const char *name)
{
	//LOG_DEBUG("PurplePresenceMngr : PermitRemovedCbk()\n");
}

void PurplePresenceMngr::DenyAddedCbk(PurpleAccount *account, const char *name)
{
	//LOG_DEBUG("PurplePresenceMngr : DenyAddedCbk()\n");
}

void PurplePresenceMngr::DenyRemovedCbk(PurpleAccount *account, const char *name)
{
	//LOG_DEBUG("PurplePresenceMngr : DenyRemovedCbk()\n");
}


/* **************** MANAGE PRESENCE LIST (Buddy list) ****************** */

PurpleIMPresence *PurplePresenceMngr::FindIMPresence(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleIMPresenceIterator i;
	for (i = _purpleIMPresenceList.begin(); i != _purpleIMPresenceList.end(); i++)
	{
		if (PurpleIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			return (*i);
		}
	}

	return NULL;
}

PurpleIMPresence *PurplePresenceMngr::AddIMPresence(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleIMPresence *mIMPresence = FindIMPresence(account);

	if (mIMPresence == NULL)
	{
		mIMPresence = new PurpleIMPresence(account);

		_purpleIMPresenceList.push_back(mIMPresence);
	}

	return mIMPresence;
}

void PurplePresenceMngr::RemoveIMPresence(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleIMPresenceIterator it;
	for (it = _purpleIMPresenceList.begin(); it != _purpleIMPresenceList.end(); it++)
	{
		if (PurpleIMFactory::equals((*it)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			_purpleIMPresenceList.erase(it);
			break;
		}
	}
}

void PurplePresenceMngr::UpdateIMPresence(IMAccount &account) {
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleIMPresenceIterator it;
	for (it = _purpleIMPresenceList.begin(); it != _purpleIMPresenceList.end(); it++)
	{
		if (PurpleIMFactory::equals((*it)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			(*it)->setIMAccount(account);
			break;
		}
	}
}

void PurplePresenceMngr::reset() {
	RecursiveMutex::ScopedLock lock(_mutex);

	_purpleIMPresenceList.clear();
}

/* *********************************************************************** */
