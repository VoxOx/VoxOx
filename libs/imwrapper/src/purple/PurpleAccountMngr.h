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

#ifndef PURPLEACCOUNTMNGR_H
#define PURPLEACCOUNTMNGR_H

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountList.h>

#include <thread/RecursiveMutex.h>

#include <util/Trackable.h>

#include <list>

extern "C" 
{
#include <libpurple/account.h>
}

void *purple_wg_get_handle();

class PurplePresenceMngr;
class PurpleChatMngr;
class PurpleConnectMngr;
class PurpleContactListMngr;

//=============================================================================

class PurpleAccountMngrCallbackData
{ 
public:
	PurpleAccountMngrCallbackData()			{ initVars();	}

	PurpleAccount*	getPurpleAccount()	const			{ return _purpleAccount;	}
	int				getCallbackId()		const			{ return _callbackId;		}
	std::string		getPassword()		const			{ return _password;			}

	void setPurpleAccount( PurpleAccount*	  val )		{ _purpleAccount = val;	}
	void setCallbackId   ( int				  val )		{ _callbackId    = val;	}
	void setPassword	 ( const std::string& val )		{ _password		 = val;	}	

protected:
	void initVars()
	{
		setPurpleAccount( NULL );
		setCallbackId   ( 0 );
		setPassword	    ( "" );
	}

private:
	PurpleAccount*	_purpleAccount;
	int				_callbackId;
	std::string		_password;
};

//=============================================================================

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class PurpleAccountMngr
{
public:

	static PurpleAccountMngr *getInstance();

	void Init();

	void reset();
	
	void AddIMAccount(IMAccount &account);

	void RemoveIMAccount(IMAccount &account);
	void UpdateIMAccount(IMAccount &account);

	void importContactsFromMySpaceServer(IMAccount &imAccount);
	static bool importContactsFromMySpaceServerCbk(void * data);

	static bool removeIMAccountCbk(void * data);
	static bool updateIMAccountCbk(void * data);

	static IMAccount *FindIMAccount(const char *login, EnumIMProtocol::IMProtocol protocol);
	static IMAccount *GetFirstIMAccount();

	/* ********** PurpleAccountCallback *********** */
	static void NotifyAddedCbk(PurpleAccount *account, const char *remote_user,
							const char *id, const char *alias, const char *message);

	static void RequestAddCbk(PurpleAccount *account, const char *remote_user,
							const char *id, const char *alias, const char *message);

	static void AuthRequestCbk(PurpleAccount *account, const char *remote_user,
							const char *id, const char *alias,
							const char *message, gboolean on_list,
							void *authorize_cb, void *deny_cb, void *user_data);
	/* ****************************************** */

	static RecursiveMutex _mutex;
protected:
	PurpleAccount* findPurpleAccount( const IMAccount &imAccount );

private:
	PurpleAccountMngr();

	//VOXOX - JRT - 2009.08.21 - These should be in a base class
	static PurpleAccount* getPurpleAccount( const IMAccount& imAccount );
		   int			  timeoutAdd      ( void* cbData, GSourceFunc callbackFunc );
	static void			  timeoutRemove   ( PurpleAccountMngrCallbackData* cbData );


	static PurpleAccountMngr*		_staticInstance;
	static PurplePresenceMngr*		_presenceMngr;
	static PurpleChatMngr*			_chatMngr;
	static PurpleConnectMngr*		_connectMngr;
	static PurpleContactListMngr*	_clistMngr;

	static IMAccountList			_purpleIMAccountList;
};

#endif	//PURPLEACCOUNTMNGR_H
