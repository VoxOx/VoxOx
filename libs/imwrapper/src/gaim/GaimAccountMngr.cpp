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

#include "GaimAccountMngr.h"
#include "GaimPresenceMngr.h"
#include "GaimChatMngr.h"
#include "GaimConnectMngr.h"
#include "GaimContactListMngr.h"
#include "GaimIMFactory.h"

#include <util/Logger.h>

typedef struct misc_s
{
	void * data_ptr1;
	void * data_ptr2;
	int cbk_id;
}	misc_t;

void *gaim_wg_get_handle()
{
	static int handle;

	return &handle;
}

/* ***************** GAIM CALLBACK ***************** */
static void C_NotifyAddedCbk(GaimAccount *account, const char *remote_user,
							const char *id, const char *alias,
							const char *message)
{
	GaimAccountMngr::NotifyAddedCbk(account, remote_user, id, alias, message);
}

static void C_RequestAddCbk(GaimAccount *account, const char *remote_user,
							const char *id, const char *alias,
							const char *message)
{
	GaimAccountMngr::RequestAddCbk(account, remote_user, id, alias, message);
}

static void C_AuthRequestCbk(GaimAccount *account, const char *remote_user,
							const char *id, const char *alias,
							const char *message, gboolean on_list,
							GCallback authorize_cb, GCallback deny_cb, void *user_data)
{
	GaimAccountMngr::AuthRequestCbk(account, remote_user, id, alias, message, 
		on_list, (void *)authorize_cb, (void *)deny_cb, user_data);
}

GaimAccountUiOps acc_wg_ops =
{
	C_NotifyAddedCbk,
	NULL,
	C_RequestAddCbk,
	C_AuthRequestCbk,
};

GaimAccountUiOps null_acc_wg_ops =
{
	NULL, NULL, NULL, NULL,
};

/* ************************************************* */
GaimAccountMngr *GaimAccountMngr::_staticInstance = NULL;
IMAccountList GaimAccountMngr::_gaimIMAccountList;
GaimPresenceMngr *GaimAccountMngr::_presenceMngr = NULL;
GaimChatMngr *GaimAccountMngr::_chatMngr = NULL;
GaimConnectMngr *GaimAccountMngr::_connectMngr = NULL;
GaimContactListMngr *GaimAccountMngr::_clistMngr = NULL;
RecursiveMutex GaimAccountMngr::_mutex;

GaimAccountMngr::GaimAccountMngr()
{
}

GaimAccountMngr *GaimAccountMngr::getInstance()
{
	if (!_staticInstance)
		_staticInstance = new GaimAccountMngr();

	return _staticInstance;
}

void GaimAccountMngr::Init()
{
	_presenceMngr = GaimPresenceMngr::getInstance();
	_chatMngr = GaimChatMngr::getInstance();
	_connectMngr = GaimConnectMngr::getInstance();
	_clistMngr = GaimContactListMngr::getInstance();
}

void GaimAccountMngr::NotifyAddedCbk(GaimAccount *account, const char *remote_user,
										const char *id, const char *alias,
										const char *message)
{
	LOG_DEBUG("GaimAccountMngr : NotifyAddedCbk()");
}

void GaimAccountMngr::RequestAddCbk(GaimAccount *account, const char *remote_user,
									const char *id, const char *alias,
									const char *message)
{
	LOG_DEBUG("GaimAccountMngr : RequestAddCbk()");
}

void GaimAccountMngr::AuthRequestCbk(GaimAccount *account, const char *remote_user,
									const char *id, const char *alias,
									const char *message, gboolean on_list,
									void *authorize_cb, void *deny_cb, void *user_data)
{
	const char *gPrclId = gaim_account_get_protocol_id(account);
	IMAccount *mAccount = FindIMAccount(gaim_account_get_username(account),
									GaimIMPrcl::GetEnumIMProtocol(gPrclId));
	GaimIMPresence *mPresence = NULL;
	
	if (!mAccount)
		return;
	
	mPresence = _presenceMngr->FindIMPresence(*mAccount);
	mPresence->addAuthRequestInList(std::string(remote_user), authorize_cb, deny_cb, user_data);
	mPresence->authorizationRequestEvent(*mPresence, remote_user, message != NULL ? message : "");
}

/* **************** MANAGE ACCOUNT LIST (Buddy list) ****************** */

IMAccount *GaimAccountMngr::GetFirstIMAccount()
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMAccountList::iterator it = _gaimIMAccountList.begin();

	if (it == _gaimIMAccountList.end()) {
		return NULL;
	} else {
		return &(*it);
	}
}

IMAccount *GaimAccountMngr::FindIMAccount(const char *login, EnumIMProtocol::IMProtocol protocol)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	for (IMAccountList::iterator i = _gaimIMAccountList.begin(); i != _gaimIMAccountList.end(); i++)
	{
		if (GaimIMFactory::equals((*i), std::string(login), protocol))
		{
			return &(*i);
		}
	}

	return NULL;
}

void GaimAccountMngr::AddIMAccount(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);
	bool found = false;

	for (IMAccountList::iterator i = _gaimIMAccountList.begin();
		i != _gaimIMAccountList.end(); i++) {
		if (GaimIMFactory::equals((*i), 
			std::string(account.getLogin().c_str()), account.getProtocol())) {
			found = true;
			break;
		}
	}
	
	if (!found) {
		_gaimIMAccountList.push_back(account);
	}
}

bool GaimAccountMngr::removeIMAccountCbk(void *data)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimAccount	*gAccount = (GaimAccount *)((misc_t *)data)->data_ptr1;

	gaim_accounts_delete(gAccount);

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free((misc_t *)data);

	return TRUE;
}

void GaimAccountMngr::RemoveIMAccount(IMAccount &imAccount)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimAccount	*gAccount = gaim_accounts_find(imAccount.getLogin().c_str(),
		GaimIMPrcl::GetPrclId(imAccount.getProtocol()));
	if (!gAccount) {
		return;
	}

	_presenceMngr->RemoveIMPresence(imAccount);
	_chatMngr->RemoveIMChat(imAccount);
	_clistMngr->RemoveIMContactList(imAccount);
	_connectMngr->RemoveIMConnect(imAccount);

	IMAccountList::iterator it =
		std::find(_gaimIMAccountList.begin(), _gaimIMAccountList.end(), imAccount);
	if (it != _gaimIMAccountList.end()) {
		_gaimIMAccountList.erase(it);
	}

	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->data_ptr1 = gAccount;

	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimAccountMngr::removeIMAccountCbk, data);
}

bool GaimAccountMngr::updateIMAccountCbk(void * data)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimAccount * gAccount = (GaimAccount *)((misc_t *)data)->data_ptr1;
	const char * password = (const char *)((misc_t *)data)->data_ptr2;

	gaim_account_set_password(gAccount, password);
	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr2);
	free((misc_t *)data);

	return TRUE;
}

void GaimAccountMngr::UpdateIMAccount(IMAccount &imAccount)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimAccount	*gAccount = gaim_accounts_find(imAccount.getLogin().c_str(),
		GaimIMPrcl::GetPrclId(imAccount.getProtocol()));
	if (!gAccount) {
		return;
	}

	_presenceMngr->UpdateIMPresence(imAccount);
	_chatMngr->UpdateIMChat(imAccount);
	_clistMngr->UpdateIMContactList(imAccount);
	_connectMngr->UpdateIMConnect(imAccount);

	IMAccountList::iterator it =
		std::find(_gaimIMAccountList.begin(), _gaimIMAccountList.end(), imAccount);
	if (it != _gaimIMAccountList.end()) {
		(*it) = imAccount;
	}

	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->data_ptr1 = gAccount;
	data->data_ptr2 = strdup(imAccount.getPassword().c_str());

	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimAccountMngr::updateIMAccountCbk, data);
}

void GaimAccountMngr::reset() {
	RecursiveMutex::ScopedLock lock(_mutex);

	_presenceMngr->reset();
	_chatMngr->reset();
	_clistMngr->reset();
	_connectMngr->reset();

	_gaimIMAccountList.clear();
}

/* ******************************************************************** */
