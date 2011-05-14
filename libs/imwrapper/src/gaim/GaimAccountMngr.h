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

#ifndef GAIMACCOUNTMNGR_H
#define GAIMACCOUNTMNGR_H

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountList.h>

#include <thread/RecursiveMutex.h>

#include <util/Trackable.h>

#include <list>

extern "C" {
#include <gaim/account.h>
}

void *gaim_wg_get_handle();

class GaimPresenceMngr;
class GaimChatMngr;
class GaimConnectMngr;
class GaimContactListMngr;

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class GaimAccountMngr
{
public:

	static GaimAccountMngr *getInstance();

	void Init();

	void reset();
	
	void AddIMAccount(IMAccount &account);

	void RemoveIMAccount(IMAccount &account);
	void UpdateIMAccount(IMAccount &account);

	static bool removeIMAccountCbk(void * data);
	static bool updateIMAccountCbk(void * data);

	static IMAccount *FindIMAccount(const char *login, EnumIMProtocol::IMProtocol protocol);
	static IMAccount *GetFirstIMAccount();

	/* ********** GaimAccountCallback *********** */
	static void NotifyAddedCbk(GaimAccount *account, const char *remote_user,
							const char *id, const char *alias, const char *message);

	static void RequestAddCbk(GaimAccount *account, const char *remote_user,
							const char *id, const char *alias, const char *message);

	static void AuthRequestCbk(GaimAccount *account, const char *remote_user,
							const char *id, const char *alias,
							const char *message, gboolean on_list,
							void *authorize_cb, void *deny_cb, void *user_data);
	/* ****************************************** */

	static RecursiveMutex _mutex;

private:

	GaimAccountMngr();
	static GaimAccountMngr *_staticInstance;
	static GaimPresenceMngr *_presenceMngr;
	static GaimChatMngr *_chatMngr;
	static GaimConnectMngr *_connectMngr;
	static GaimContactListMngr *_clistMngr;

	static IMAccountList _gaimIMAccountList;
};

#endif	//GAIMACCOUNTMNGR_H
