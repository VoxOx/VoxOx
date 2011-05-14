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

#include "GaimPresenceMngr.h"
#include "GaimAccountMngr.h"
#include "GaimIMFactory.h"

extern "C" {
#include "gaim/privacy.h"
#include "gaim/prefs.h"
}

#include <util/Logger.h>

/* ************ STATUS MANAGEMENT ************** */

void update_account_status(GaimAccount *account, GaimStatus *old, GaimStatus *newstatus)
{
	GaimPresenceMngr::UpdateAccountStatus(account, old, newstatus);
}

void init_account_status_changed_event()
{
	void *handle = gaim_wg_get_handle();

	gaim_signal_connect(gaim_accounts_get_handle(), "account-status-changed",
		handle, GAIM_CALLBACK(update_account_status), NULL);
}


/* ***************** GAIM CALLBACK ***************** */
static void C_PermitAddedCbk(GaimAccount *account, const char *name)
{
	GaimPresenceMngr::PermitAddedCbk(account, name);
}

static void C_PermitRemovedCbk(GaimAccount *account, const char *name)
{
	GaimPresenceMngr::PermitRemovedCbk(account, name);
}

static void C_DenyAddedCbk(GaimAccount *account, const char *name)
{
	GaimPresenceMngr::DenyAddedCbk(account, name);
}

static void C_DenyRemovedCbk(GaimAccount *account, const char *name)
{
	GaimPresenceMngr::DenyRemovedCbk(account, name);
}

GaimPrivacyUiOps privacy_wg_ops =	{
										C_PermitAddedCbk,
										C_PermitRemovedCbk,
										C_DenyAddedCbk,
										C_DenyRemovedCbk
									};

/* ************************************************** */

GaimAccountMngr *GaimPresenceMngr::_accountMngr = NULL;
GaimPresenceMngr *GaimPresenceMngr::_staticInstance = NULL;
std::list<GaimIMPresence *> GaimPresenceMngr::_gaimIMPresenceList;
RecursiveMutex GaimPresenceMngr::_mutex;

GaimPresenceMngr::GaimPresenceMngr()
{
}

void GaimPresenceMngr::Init()
{
	_accountMngr = GaimAccountMngr::getInstance();
	init_account_status_changed_event();
	
	gaim_prefs_set_string("/core/away/idle_reporting", "none");
	gaim_prefs_set_string("/core/away/auto_reply", "never");
	gaim_prefs_set_bool("/core/away/away_when_idle", FALSE);
}

GaimPresenceMngr *GaimPresenceMngr::getInstance()
{
	if (!_staticInstance)
		_staticInstance = new GaimPresenceMngr();

	return _staticInstance;
}

void GaimPresenceMngr::UpdateAccountStatus(GaimAccount *gAccount, 
										   GaimStatus *old, 
										   GaimStatus *newstatus)
{
	GaimIMPresence *mIMPresence = NULL;
	const char *gPrclId = gaim_account_get_protocol_id(gAccount);
	IMAccount *account = _accountMngr->FindIMAccount(gaim_account_get_username(gAccount),
		GaimIMPrcl::GetEnumIMProtocol(gPrclId));

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

void GaimPresenceMngr::PermitAddedCbk(GaimAccount *account, const char *name)
{
	//LOG_DEBUG("GaimPresenceMngr : PermitAddedCbk()\n");
}

void GaimPresenceMngr::PermitRemovedCbk(GaimAccount *account, const char *name)
{
	//LOG_DEBUG("GaimPresenceMngr : PermitRemovedCbk()\n");
}

void GaimPresenceMngr::DenyAddedCbk(GaimAccount *account, const char *name)
{
	//LOG_DEBUG("GaimPresenceMngr : DenyAddedCbk()\n");
}

void GaimPresenceMngr::DenyRemovedCbk(GaimAccount *account, const char *name)
{
	//LOG_DEBUG("GaimPresenceMngr : DenyRemovedCbk()\n");
}


/* **************** MANAGE PRESENCE LIST (Buddy list) ****************** */

GaimIMPresence *GaimPresenceMngr::FindIMPresence(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMPresenceIterator i;
	for (i = _gaimIMPresenceList.begin(); i != _gaimIMPresenceList.end(); i++)
	{
		if (GaimIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			return (*i);
		}
	}

	return NULL;
}

GaimIMPresence *GaimPresenceMngr::AddIMPresence(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMPresence *mIMPresence = FindIMPresence(account);

	if (mIMPresence == NULL)
	{
		mIMPresence = new GaimIMPresence(account);

		_gaimIMPresenceList.push_back(mIMPresence);
	}

	return mIMPresence;
}

void GaimPresenceMngr::RemoveIMPresence(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMPresenceIterator it;
	for (it = _gaimIMPresenceList.begin(); it != _gaimIMPresenceList.end(); it++)
	{
		if (GaimIMFactory::equals((*it)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			_gaimIMPresenceList.erase(it);
			break;
		}
	}
}

void GaimPresenceMngr::UpdateIMPresence(IMAccount &account) {
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMPresenceIterator it;
	for (it = _gaimIMPresenceList.begin(); it != _gaimIMPresenceList.end(); it++)
	{
		if (GaimIMFactory::equals((*it)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			(*it)->setIMAccount(account);
			break;
		}
	}
}

void GaimPresenceMngr::reset() {
	RecursiveMutex::ScopedLock lock(_mutex);

	_gaimIMPresenceList.clear();
}

/* *********************************************************************** */
