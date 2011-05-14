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

#include "PurpleIMPresence.h"

#include "PurpleEnumPresenceState.h"
#include "PurplePresenceMngr.h"

extern "C" {
#include "libpurple/account.h"
#include "libpurple/buddyicon.h"
#include "libpurple/core.h"
#include "libpurple/privacy.h"
#include <libpurple/blist.h>
}

#include <util/File.h>
#include <util/Logger.h>
#include <util/OWPicture.h>
#include <thread/Thread.h>

#ifdef _WIN32
#include <windows.h>
#endif

struct AuthRequest_s
{
	PurpleAccountRequestAuthorizationCb accept_cb;
	PurpleAccountRequestAuthorizationCb deny_cb;
	void *user_data;
};

//typedef struct misc_s
//{
//	PurpleIMPresence * instance;
//	void * data_ptr;
//	int data_int;
//	int cbk_id;
//}	misc_t;


Mutex PurpleIMPresence::_mutex;

//-----------------------------------------------------------------------------

PurpleIMPresence::PurpleIMPresence(IMAccount account) : IMPresence(account)
{
}

//-----------------------------------------------------------------------------

PurpleIMPresence::~PurpleIMPresence() 
{
	PurplePresenceMngr::getInstance()->RemoveIMPresence(_imAccount);
}

//-----------------------------------------------------------------------------

const char *PurpleIMPresence::getPurplePresenceId(void *purplePresence)
{
	PurplePresence *gPresence = (PurplePresence *) purplePresence;
	PurpleStatus *gStatus;
	PurpleStatusType *gStatusType;
	PurpleStatusPrimitive gStatusPrim;
	const char *gStatusName;

	gStatus = purple_presence_get_active_status(gPresence);
	gStatusType = purple_status_get_type(gStatus);
	gStatusName	= purple_status_get_attr_string(gStatus,"status_text");
	gStatusPrim = purple_status_type_get_primitive(gStatusType);

	return purple_primitive_get_id_from_type(gStatusPrim);
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE CJC GET BUDDYS STATUS MESSAGE
const char *PurpleIMPresence::getPurpleBuddyStatusMessage(void *purpleBuddy)
{
	PurpleBuddy *gBuddy = (PurpleBuddy *) purpleBuddy;
	PurpleAccount * gAccount = purple_buddy_get_account(gBuddy);
	PurplePluginProtocolInfo *prpl_info = NULL;
	PurpleConnection *gc = purple_account_get_connection(gAccount);
	PurplePlugin *prpl = NULL;
	
	if (gc != NULL)
	        prpl = purple_connection_get_prpl(gc);      

	if (prpl != NULL)
		prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);

	if (prpl_info != NULL && prpl_info->status_text != NULL)
		return prpl_info->status_text(gBuddy);

	return "";
}

//-----------------------------------------------------------------------------

bool PurpleIMPresence::changeMyPresenceCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);

	PurpleIMPresenceCallbackData* cbData = (PurpleIMPresenceCallbackData*)dataIn;

	PurpleIMPresence*				 imPresence = cbData->getPurpleIMPresence();
	EnumPresenceState::PresenceState state		= cbData->getPresenceState();
//	const char * note = (const char *)((misc_t *)data)->data_ptr; 
	PurpleAccount *gAccount = getPurpleAccount( imPresence->getIMAccount() );

	if (gAccount && !purple_account_is_connecting(gAccount))
	{
		if (!purple_account_is_connected(gAccount)) 
		{
			purple_account_set_enabled(gAccount, purple_core_get_ui(), TRUE);
		}

		//VOXOX - JRT - 2009.09.12 - easier debugging.
		const char* newPresenceState = PurplePreState::GetStatusId(state, imPresence->getIMAccount().getProtocol());

		if ( cbData->getNote().empty() ) 
		{
			purple_account_set_status(gAccount, newPresenceState, TRUE, NULL);
		} 
		else 
		{
			purple_account_set_status(gAccount, newPresenceState, TRUE, "message", cbData->getNote().c_str(), NULL);
		}
	}

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::changeMyPresence(EnumPresenceState::PresenceState state, const std::string & note)
{
	PurpleIMPresenceCallbackData* cbData = new PurpleIMPresenceCallbackData;

	cbData->setPurpleIMPresence( this  );
	cbData->setPresenceState   ( state );
	cbData->setNote			   ( note  );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMPresence::changeMyPresenceCbk ) );
//	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMPresence::changeMyPresenceCbk, data);
}

//-----------------------------------------------------------------------------

bool PurpleIMPresence::changeMyAliasCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);

	PurpleIMPresenceCallbackData* cbData = (PurpleIMPresenceCallbackData*)dataIn;

	PurpleIMPresence* imPresence = cbData->getPurpleIMPresence();
	PurpleAccount*	  gAccount   = getPurpleAccount( imPresence->getIMAccount() );

	if (gAccount && purple_account_is_connected(gAccount)) 
	{
		purple_account_set_alias( gAccount, cbData->getNickname().c_str() );
	}

	timeoutRemove( cbData );
	delete cbData;
	
	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::changeMyAlias(const std::string & nickname)
{
	PurpleIMPresenceCallbackData* cbData = new PurpleIMPresenceCallbackData;

	cbData->setPurpleIMPresence( this  );
	cbData->setNickname		   ( nickname  );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMPresence::changeMyAliasCbk ) );
//	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMPresence::changeMyAliasCbk, data);
}

//-----------------------------------------------------------------------------

bool PurpleIMPresence::changeMyIconCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);

	PurpleIMPresenceCallbackData* cbData = (PurpleIMPresenceCallbackData*)dataIn;

	PurpleIMPresence* imPresence = cbData->getPurpleIMPresence();
	PurpleAccount*	  gAccount   = getPurpleAccount( imPresence->getIMAccount() );

	OWPicture picture    = OWPicture::pictureFromFile( cbData->getIconPath().c_str() );
	size_t   size        = picture.getData().length();
	gpointer pictureData = g_memdup(picture.getData().c_str(),size);

//#ifdef _WIN32		//JRT-XXX
//	char msg[200];
//	sprintf_s( msg, 200, "PurpleIMPresence::changeMyIconCbk: data len = %d, account: %s\n\n", size, imPresence->getIMAccount().getLogin().c_str() );
//	OutputDebugString( msg );
//#endif
                              
	purple_account_set_buddy_icon_path(gAccount, cbData->getIconPath().c_str() );

	//VOXOX - CJC - 2009.06.23 Fix bug on image not getting send!
	purple_buddy_icons_set_account_icon(gAccount, (guchar*)pictureData, size);

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::changeMyIcon(const OWPicture & picture)
{
	FileWriter file(File::createTemporaryFile());
	file.write(picture.getData());
	file.close();

	PurpleIMPresenceCallbackData* cbData = new PurpleIMPresenceCallbackData;

	cbData->setPurpleIMPresence( this  );
	cbData->setIconPath( file.getFullPath() );

//#ifdef _WIN32		//JRT-XXX
//	size_t   size = picture.getData().length();
//
//	if ( size < 12000 )
//		int xxx = 1;
//
//	char msg[200];
//	sprintf_s( msg, 200, "PurpleIMPresence::changeMyIcon: data len = %d, account: %s\n\n", size, getIMAccount().getLogin().c_str() );
//	OutputDebugString( msg );
//#endif

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMPresence::changeMyIconCbk ) );
//	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMPresence::changeMyIconCbk, data);
}

//-----------------------------------------------------------------------------

bool PurpleIMPresence::subscribeToPresenceOfCbk(void * dataIn) 
{
	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);

	PurpleIMPresenceCallbackData* cbData = (PurpleIMPresenceCallbackData*)dataIn;

	PurpleIMPresence* imPresence = cbData->getPurpleIMPresence();
	PurpleAccount*	  gAccount   = getPurpleAccount( imPresence->getIMAccount() );

//	const char * contactId = (const char *)((misc_t *)data)->data_ptr;
	PurpleBuddy *gBuddy = NULL;

	if (gAccount && (gBuddy = purple_find_buddy(gAccount, cbData->getContactId().c_str()) ) ) 
	{
		PurplePresence *gPresence	  = purple_buddy_get_presence(gBuddy);
		const char	   *gPresenceId   = getPurplePresenceId(gPresence);
		const char	   *buddy_alias   = gBuddy->server_alias && *gBuddy->server_alias ? gBuddy->server_alias : gBuddy->alias;
		const char	   *statusMessage = PurpleIMPresence::getPurpleBuddyStatusMessage(gBuddy);

		//VOXOX CHANGE SUPPORT STATUS MESSAGE
		imPresence->presenceStateChangedEvent(*imPresence,
												PurplePreState::GetPresenceState(gPresenceId),
												!buddy_alias ? String::null : buddy_alias,
												!statusMessage ? String::null : statusMessage,
												purple_buddy_get_name(gBuddy));


	     /*
		mIMPresence->presenceStateChangedEvent(*mIMPresence,
				PurplePreState::GetPresenceState(gPresenceId),
				!buddy_alias ? String::null : buddy_alias,
				purple_buddy_get_name(gBuddy));*/

	}

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::subscribeToPresenceOf(const std::string & contactId)
{
	PurpleIMPresenceCallbackData* cbData = new PurpleIMPresenceCallbackData;

	cbData->setPurpleIMPresence( this  );
	cbData->setContactId( contactId );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMPresence::subscribeToPresenceOfCbk ) );
//	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMPresence::subscribeToPresenceOfCbk, data);
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::unsubscribeToPresenceOf(const std::string & contactId)
{
}

//-----------------------------------------------------------------------------

bool PurpleIMPresence::blockContactCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);

	PurpleIMPresenceCallbackData* cbData = (PurpleIMPresenceCallbackData*)dataIn;

	PurpleIMPresence* imPresence = cbData->getPurpleIMPresence();
	PurpleAccount*	  gAccount   = getPurpleAccount( imPresence->getIMAccount() );

	if (gAccount)
	{
		purple_privacy_deny_add(gAccount, cbData->getContactId().c_str(), FALSE);
	}

	timeoutRemove( cbData );
	delete cbData;
	 
	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::blockContact(const std::string & contactId)
{
	PurpleIMPresenceCallbackData* cbData = new PurpleIMPresenceCallbackData;

	cbData->setPurpleIMPresence( this  );
	cbData->setContactId( contactId );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMPresence::blockContactCbk ) );
//	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMPresence::blockContactCbk, data);
}

//-----------------------------------------------------------------------------

bool PurpleIMPresence::unblockContactCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);

	PurpleIMPresenceCallbackData* cbData = (PurpleIMPresenceCallbackData*)dataIn;

	PurpleIMPresence* imPresence = cbData->getPurpleIMPresence();
	PurpleAccount*	  gAccount   = getPurpleAccount( imPresence->getIMAccount() );

	if (gAccount)
	{
		 purple_privacy_deny_remove(gAccount, cbData->getContactId().c_str(), FALSE);//VOXOX - SEMR - 2009.06.23 Remove contact from deny list
	}

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::unblockContact(const std::string & contactId)
{
	PurpleIMPresenceCallbackData* cbData = new PurpleIMPresenceCallbackData;

	cbData->setPurpleIMPresence( this  );
	cbData->setContactId( contactId );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMPresence::unblockContactCbk ) );
//	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMPresence::unblockContactCbk, data);
}

//-----------------------------------------------------------------------------

bool PurpleIMPresence::authorizeContactCbk(void * dataIn)
{
	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);

	PurpleIMPresenceCallbackData* cbData = (PurpleIMPresenceCallbackData*)dataIn;

	PurpleIMPresence* imPresence = cbData->getPurpleIMPresence();
	PurpleAccount*	  gAccount   = getPurpleAccount( imPresence->getIMAccount() );

	std::string contactId = cbData->getContactId();
//	int authorized = ((misc_t *)data)->data_int;

	if (gAccount && imPresence->authRequestMap.find(contactId) != imPresence->authRequestMap.end())
	{
		if ( cbData->getAuthorized() ) 
		{
			imPresence->authRequestMap[contactId]->accept_cb( imPresence->authRequestMap[contactId]->user_data);
		} 
		else 
		{
			imPresence->authRequestMap[contactId]->deny_cb( imPresence->authRequestMap[contactId]->user_data);
		}
	}

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

bool PurpleIMPresence::reRequestAuthorizationCbk(void * dataIn)//VOXOX - CJC - 2009.07.31 
{
	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);

	PurpleIMPresenceCallbackData* cbData = (PurpleIMPresenceCallbackData*)dataIn;

	PurpleIMPresence* imPresence = cbData->getPurpleIMPresence();
	PurpleAccount*	  gAccount   = getPurpleAccount( imPresence->getIMAccount() );

//	const char * contactId = (const char *)((misc_t *)data)->data_ptr;
//	int authorized = ((misc_t *)data)->data_int;
	
	PurpleBuddy *gBuddy = NULL;

	gBuddy = purple_find_buddy( gAccount, cbData->getContactId().c_str() );
	
	if(gAccount && gBuddy)
	{
		purple_account_add_buddy(gAccount, gBuddy);	//VOXOX - CJC - 2009.07.31 
	}

	timeoutRemove( cbData );
	delete cbData;

	return TRUE;
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::authorizeContact(const std::string & contactId, bool authorized, const std::string message)
{
	PurpleIMPresenceCallbackData* cbData = new PurpleIMPresenceCallbackData;

	cbData->setPurpleIMPresence( this  );
	cbData->setContactId( contactId );
	cbData->setAuthorized( authorized );
	
	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMPresence::authorizeContactCbk ) );
//	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMPresence::authorizeContactCbk, data);
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::reRequestAuthorization(const std::string & contactId) //VOXOX - CJC - 2009.07.31 
{
	PurpleIMPresenceCallbackData* cbData = new PurpleIMPresenceCallbackData;

	cbData->setPurpleIMPresence( this  );
	cbData->setContactId( contactId );
	cbData->setAuthorized( true );

	cbData->setCallbackId( timeoutAdd( cbData, (GSourceFunc) PurpleIMPresence::reRequestAuthorizationCbk ) );
//	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);
//	data->cbk_id = purple_timeout_add(0, (GSourceFunc) PurpleIMPresence::reRequestAuthorizationCbk, data);
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::addAuthRequestInList(const std::string &contactId, void *acceptCbk, 
										  void *denyCbk, void *userData)
{
	AuthRequest *authReq = new AuthRequest();

	authReq->accept_cb = (PurpleAccountRequestAuthorizationCb) acceptCbk;
	authReq->deny_cb = (PurpleAccountRequestAuthorizationCb) denyCbk;
	authReq->user_data = userData;

	authRequestMap[contactId] = authReq;
}

//-----------------------------------------------------------------------------
//static
PurpleAccount* PurpleIMPresence::getPurpleAccount( const IMAccount& imAccount )
{
	return purple_accounts_find( imAccount.getLogin().c_str(), PurpleIMPrcl::GetPrclId( imAccount.getProtocol() ) );
}

//-----------------------------------------------------------------------------

int PurpleIMPresence::timeoutAdd( void* cbData, GSourceFunc callbackFunc )
{
	Mutex::ScopedLock lock(PurpleIMPresence::_mutex);
	return purple_timeout_add(0, callbackFunc, cbData);
}

//-----------------------------------------------------------------------------

void PurpleIMPresence::timeoutRemove( PurpleIMPresenceCallbackData* cbData )
{
	int waitTime	  = 1;
	int totalWaitTime = 0;

	while ( cbData->getCallbackId() == 0 )
	{
		Thread::msleep( waitTime );
		totalWaitTime += waitTime;

#ifdef _WIN32
		char msg[200];
		sprintf_s( msg, "PurpleIMConnect::timeoutRemove - waiting for valid callbackId. %d ms\n", totalWaitTime );
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
