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

#include "PurpleAccountMngr.h"
#include "PurplePresenceMngr.h"
#include "PurpleChatMngr.h"
#include "PurpleConnectMngr.h"
#include "PurpleContactListMngr.h"
#include "PurpleIMFactory.h"

#include <util/Logger.h>
#include <thread/Thread.h>

#ifdef WIN32
#include <windows.h>
#endif

//-----------------------------------------------------------------------------

void *purple_wg_get_handle()
{
	static int handle;

	return &handle;
}

/* ***************** PURPLE CALLBACK ***************** */
static void C_NotifyAddedCbk(PurpleAccount *account, const char *remote_user,
							const char *id, const char *alias,
							const char *message)
{
	PurpleAccountMngr::NotifyAddedCbk(account, remote_user, id, alias, message);
}

static void C_RequestAddCbk(PurpleAccount *account, const char *remote_user,
							const char *id, const char *alias,
							const char *message)
{
	PurpleAccountMngr::RequestAddCbk(account, remote_user, id, alias, message);
}

static void * C_AuthRequestCbk(PurpleAccount *account, const char *remote_user,
							const char *id, const char *alias,
							const char *message, gboolean on_list,
							PurpleAccountRequestAuthorizationCb authorize_cb, PurpleAccountRequestAuthorizationCb deny_cb, void *user_data)
{
	PurpleAccountMngr::AuthRequestCbk(account, remote_user, id, alias, message, 
		on_list, (void *)authorize_cb, (void *)deny_cb, user_data);

	return user_data;
}

PurpleAccountUiOps acc_wg_ops =
{
	C_NotifyAddedCbk,
	NULL,
	C_RequestAddCbk,
	C_AuthRequestCbk,
};

PurpleAccountUiOps null_acc_wg_ops =
{
	NULL, NULL, NULL, NULL,
};

//-----------------------------------------------------------------------------

IMAccountList			PurpleAccountMngr::_purpleIMAccountList;
PurpleAccountMngr*		PurpleAccountMngr::_staticInstance	= NULL;
PurplePresenceMngr*		PurpleAccountMngr::_presenceMngr	= NULL;
PurpleChatMngr*			PurpleAccountMngr::_chatMngr		= NULL;
PurpleConnectMngr*		PurpleAccountMngr::_connectMngr		= NULL;
PurpleContactListMngr*	PurpleAccountMngr::_clistMngr		= NULL;
RecursiveMutex			PurpleAccountMngr::_mutex;

//-----------------------------------------------------------------------------

PurpleAccountMngr::PurpleAccountMngr()
{
}

//-----------------------------------------------------------------------------

PurpleAccountMngr *PurpleAccountMngr::getInstance()
{
	if (!_staticInstance)
	{
		_staticInstance = new PurpleAccountMngr();
	}

	return _staticInstance;
}

//-----------------------------------------------------------------------------

void PurpleAccountMngr::Init()
{
	_presenceMngr	= PurplePresenceMngr::getInstance();
	_chatMngr		= PurpleChatMngr::getInstance();
	_connectMngr	= PurpleConnectMngr::getInstance();
	_clistMngr		= PurpleContactListMngr::getInstance();
}

//-----------------------------------------------------------------------------

void PurpleAccountMngr::NotifyAddedCbk(PurpleAccount *account, const char *remote_user,
										const char *id, const char *alias, const char *message)
{
	LOG_DEBUG("PurpleAccountMngr : NotifyAddedCbk()");
}

//-----------------------------------------------------------------------------

void PurpleAccountMngr::RequestAddCbk(PurpleAccount *account, const char *remote_user,
									const char *id, const char *alias, const char *message)
{
	LOG_DEBUG("PurpleAccountMngr : RequestAddCbk()");
}

//-----------------------------------------------------------------------------

void PurpleAccountMngr::AuthRequestCbk(PurpleAccount *account, const char *remote_user,
									const char *id, const char *alias,
									const char *message, gboolean on_list,
									void *authorize_cb, void *deny_cb, void *user_data)
{
	const char *gPrclId = purple_account_get_protocol_id(account);
	IMAccount *mAccount = FindIMAccount(purple_account_get_username(account), PurpleIMPrcl::GetEnumIMProtocol(gPrclId));
	PurpleIMPresence *mPresence = NULL;
	
	if (mAccount)
	{
		mPresence = _presenceMngr->FindIMPresence(*mAccount);
		mPresence->addAuthRequestInList(std::string(remote_user), authorize_cb, deny_cb, user_data);
		mPresence->authorizationRequestEvent(*mPresence, remote_user, message != NULL ? message : "");
	}
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// MANAGE ACCOUNT LIST (Buddy list)
//-----------------------------------------------------------------------------

IMAccount *PurpleAccountMngr::GetFirstIMAccount()
{
//	RecursiveMutex::ScopedLock lock(_mutex);			//VOXOX - JRT - 2009.07.12 - locking done in _purpleIMAccountList
	return _purpleIMAccountList.getFirstIMAccount();
}

//-----------------------------------------------------------------------------

IMAccount *PurpleAccountMngr::FindIMAccount(const char *login, EnumIMProtocol::IMProtocol protocol)
{
//	RecursiveMutex::ScopedLock lock(_mutex);			//VOXOX - JRT - 2009.07.12 - locking done in _purpleIMAccountList

	IMAccount* result = NULL;

	if ( login )	//VOXOX - JRT - 2009.09.15 - Fix crash.  Not sure why we get NULL login.
	{
		result = _purpleIMAccountList.findByLoginInfo( std::string(login), protocol );
	}

	return result;
}

//-----------------------------------------------------------------------------

void PurpleAccountMngr::AddIMAccount(IMAccount &account)
{
//	RecursiveMutex::ScopedLock lock(_mutex);			//VOXOX - JRT - 2009.07.12 - locking done in _purpleIMAccountList

	IMAccount* pAccount = _purpleIMAccountList.findByLoginInfo( account.getLogin(), account.getProtocol() );

	if ( pAccount == NULL)
	{
		_purpleIMAccountList.Add( account );
	}
}

//-----------------------------------------------------------------------------

bool PurpleAccountMngr::removeIMAccountCbk( void *dataIn )
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleAccountMngrCallbackData* cbData = (PurpleAccountMngrCallbackData*) dataIn;

	purple_accounts_delete( cbData->getPurpleAccount() );

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleAccountMngr::RemoveIMAccount(IMAccount &imAccount)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleAccount* gAccount = getPurpleAccount( imAccount );

	if (gAccount) 
	{
		_presenceMngr->RemoveIMPresence(imAccount);
		_chatMngr->RemoveIMChat(imAccount);
		_clistMngr->RemoveIMContactList(imAccount);
		_connectMngr->RemoveIMConnect(imAccount);

		_purpleIMAccountList.Delete( imAccount );

		PurpleAccountMngrCallbackData* cbData = new PurpleAccountMngrCallbackData;

		cbData->setPurpleAccount( gAccount );
		cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleAccountMngr::removeIMAccountCbk ) );
	}
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE CJC TO BE ABLE TO IMPORT THE CONTACTS FROM THE SERVER
void PurpleAccountMngr::importContactsFromMySpaceServer(IMAccount &imAccount)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleAccount* gAccount = getPurpleAccount( imAccount );

	if (gAccount) 
	{
		PurpleAccountMngrCallbackData* cbData = new PurpleAccountMngrCallbackData;

		cbData->setPurpleAccount( gAccount );
		cbData->setCallbackId   ( timeoutAdd( cbData, (GSourceFunc) PurpleAccountMngr::importContactsFromMySpaceServerCbk ) );
	}
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE CJC TO BE ABLE TO IMPORT THE CONTACTS FROM THE SERVER
bool PurpleAccountMngr::importContactsFromMySpaceServerCbk(void * dataIn)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleAccountMngrCallbackData* cbData = (PurpleAccountMngrCallbackData*) dataIn;

	purple_account_add_buddies( cbData->getPurpleAccount(), NULL);

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

bool PurpleAccountMngr::updateIMAccountCbk(void * dataIn)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleAccountMngrCallbackData* cbData = (PurpleAccountMngrCallbackData*) dataIn;

	purple_account_set_password( cbData->getPurpleAccount(), cbData->getPassword().c_str() );

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleAccountMngr::UpdateIMAccount(IMAccount &imAccount)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	PurpleAccount* gAccount = getPurpleAccount( imAccount );

	if (gAccount) 
	{
		_presenceMngr->UpdateIMPresence(imAccount);
		_chatMngr->UpdateIMChat(imAccount);
		_clistMngr->UpdateIMContactList(imAccount);
		_connectMngr->UpdateIMConnect(imAccount);

		_purpleIMAccountList.update( imAccount );

		PurpleAccountMngrCallbackData* cbData = new PurpleAccountMngrCallbackData;

		cbData->setPurpleAccount( gAccount );
		cbData->setPassword( imAccount.getPassword() );
	
		cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleAccountMngr::updateIMAccountCbk ) );
	}
}

//-----------------------------------------------------------------------------

void PurpleAccountMngr::reset() 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	_presenceMngr->reset();
	_chatMngr->reset();
	_clistMngr->reset();
	_connectMngr->reset();

	_purpleIMAccountList.DeleteAll();	//VOXOX - JRT - 2009.07.12 - does locking.
}

//-----------------------------------------------------------------------------
//static
PurpleAccount* PurpleAccountMngr::getPurpleAccount( const IMAccount& imAccount )
{
	return purple_accounts_find( imAccount.getLogin().c_str(), PurpleIMPrcl::GetPrclId( imAccount.getProtocol() ) );
}

//-----------------------------------------------------------------------------

int PurpleAccountMngr::timeoutAdd( void* cbData, GSourceFunc callbackFunc )
{
	Mutex::ScopedLock lock(PurpleIMChat::_mutex);
	return purple_timeout_add(0, callbackFunc, cbData);
}

//-----------------------------------------------------------------------------

void PurpleAccountMngr::timeoutRemove( PurpleAccountMngrCallbackData* cbData )
{
	int waitTime = 1;
	int totalWaitTime = 0;
	while ( cbData->getCallbackId() == 0 )
	{
		Thread::msleep( waitTime );
		totalWaitTime += waitTime;

#ifdef _WIN32
		char msg[200];
		sprintf_s( msg, "PurpleIMChat::timeoutRemove - waiting for valid callbackId. %d ms\n", totalWaitTime );
		OutputDebugString( msg );
#endif
	}

#ifdef _WIN32
	if ( totalWaitTime > 0 )
	{
		char msg[200];
		sprintf_s( msg, "\n" );
		OutputDebugString( msg );
	}
#endif

	purple_timeout_remove( cbData->getCallbackId() );
}

//-----------------------------------------------------------------------------
