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

#include "GaimIMPresence.h"

#include "GaimEnumPresenceState.h"
#include "GaimPresenceMngr.h"

extern "C" {
#include "gaim/account.h"
#include "gaim/buddyicon.h"
#include "gaim/core.h"
#include "gaim/privacy.h"
}

#include <util/File.h>
#include <util/Logger.h>
#include <util/OWPicture.h>

struct AuthRequest_s
{
	GaimAccountRequestAuthorizationCb accept_cb;
	GaimAccountRequestAuthorizationCb deny_cb;
	void *user_data;
};

typedef struct misc_s
{
	GaimIMPresence * instance;
	void * data_ptr;
	int data_int;
	int cbk_id;
}	misc_t;


Mutex GaimIMPresence::_mutex;

GaimIMPresence::GaimIMPresence(IMAccount account)
	: IMPresence(account)
{
}

GaimIMPresence::~GaimIMPresence() {
	GaimPresenceMngr::getInstance()->RemoveIMPresence(_imAccount);
}

const char *GaimIMPresence::getGaimPresenceId(void *gaimPresence)
{
	GaimPresence *gPresence = (GaimPresence *) gaimPresence;
	GaimStatus *gStatus;
	GaimStatusType *gStatusType;
	GaimStatusPrimitive gStatusPrim;

	gStatus = gaim_presence_get_active_status(gPresence);
	gStatusType = gaim_status_get_type(gStatus);
	gStatusPrim = gaim_status_type_get_primitive(gStatusType);

	return gaim_primitive_get_id_from_type(gStatusPrim);
}

bool GaimIMPresence::changeMyPresenceCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMPresence::_mutex);

	GaimIMPresence * imPresence = ((misc_t *)data)->instance;
	EnumPresenceState::PresenceState state = 
		(EnumPresenceState::PresenceState)((misc_t *)data)->data_int;
	const char * note = (const char *)((misc_t *)data)->data_ptr; 
	GaimAccount *gAccount = gaim_accounts_find(imPresence->getIMAccount().getLogin().c_str(), 
		GaimIMPrcl::GetPrclId(imPresence->getIMAccount().getProtocol()));

	if (gAccount && !gaim_account_is_connecting(gAccount))
	{
		if (!gaim_account_is_connected(gAccount)) {
			gaim_account_set_enabled(gAccount, gaim_core_get_ui(), TRUE);
		}

		if (strlen(note) == 0) {
			gaim_account_set_status(gAccount, GaimPreState::GetStatusId(state, 
				imPresence->getIMAccount().getProtocol()), TRUE, NULL);
		} else {
			gaim_account_set_status(gAccount, GaimPreState::GetStatusId(state, 
				imPresence->getIMAccount().getProtocol()), TRUE, "message", note, NULL);
		}
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr);
	free((misc_t *)data);
	
	return TRUE;
}

void GaimIMPresence::changeMyPresence(EnumPresenceState::PresenceState state, 
									  const std::string & note)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	data->data_int = state;
	if (note.empty()) {
		data->data_ptr = strdup("");
	} else {
		data->data_ptr = strdup(note.c_str());
	}

	Mutex::ScopedLock lock(GaimIMPresence::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMPresence::changeMyPresenceCbk, data);
}

bool GaimIMPresence::changeMyAliasCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMPresence::_mutex);

	GaimIMPresence * imPresence = ((misc_t *)data)->instance;
	const char * nickname = (const char *)((misc_t *)data)->data_ptr;
	GaimAccount *gAccount = gaim_accounts_find(imPresence->getIMAccount().getLogin().c_str(), 
		GaimIMPrcl::GetPrclId(imPresence->getIMAccount().getProtocol()));

	if (gAccount && gaim_account_is_connected(gAccount)) {
		gaim_account_set_alias(gAccount,nickname, TRUE);
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr);
	free((misc_t *)data);
	
	return TRUE;
}

void GaimIMPresence::changeMyAlias(const std::string & nickname)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	if (nickname.empty()) {
		data->data_ptr = strdup("");
	} else {
		data->data_ptr = strdup(nickname.c_str());
	}

	Mutex::ScopedLock lock(GaimIMPresence::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMPresence::changeMyAliasCbk, data);
}

bool GaimIMPresence::changeMyIconCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMPresence::_mutex);

	GaimIMPresence * imPresence = ((misc_t *)data)->instance;
	const char * iconPath = (const char *)((misc_t *)data)->data_ptr;
	GaimAccount *gAccount = gaim_accounts_find(imPresence->getIMAccount().getLogin().c_str(), 
		GaimIMPrcl::GetPrclId(imPresence->getIMAccount().getProtocol()));

	gaim_account_set_buddy_icon(gAccount, iconPath);

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMPresence::changeMyIcon(const OWPicture & picture)
{
	FileWriter file(File::createTemporaryFile());
	file.write(picture.getData());
	file.close();

	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	data->data_ptr = strdup(file.getFullPath().c_str());

	Mutex::ScopedLock lock(GaimIMPresence::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMPresence::changeMyIconCbk, data);
}

bool GaimIMPresence::subscribeToPresenceOfCbk(void * data) 
{
	Mutex::ScopedLock lock(GaimIMPresence::_mutex);

	GaimIMPresence * imPresence = ((misc_t *)data)->instance;
	const char * contactId = (const char *)((misc_t *)data)->data_ptr;
	GaimAccount *gAccount = gaim_accounts_find(imPresence->getIMAccount().getLogin().c_str(), 
		GaimIMPrcl::GetPrclId(imPresence->getIMAccount().getProtocol()));
	GaimBuddy *gBuddy = NULL;

	if (gAccount && (gBuddy = gaim_find_buddy(gAccount, contactId))) {
		GaimPresence *gPresence = gaim_buddy_get_presence(gBuddy);
		const char *gPresenceId = getGaimPresenceId(gPresence);
		const char *buddy_alias =
			gBuddy->server_alias && *gBuddy->server_alias ? gBuddy->server_alias : gBuddy->alias;

		imPresence->presenceStateChangedEvent(*imPresence,
			GaimPreState::GetPresenceState(gPresenceId),
			!buddy_alias ? String::null : buddy_alias,
			gaim_buddy_get_name(gBuddy));
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr);
	free((misc_t *)data);
	
	return TRUE;
}

void GaimIMPresence::subscribeToPresenceOf(const std::string & contactId)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	data->data_ptr = strdup(contactId.c_str());

	Mutex::ScopedLock lock(GaimIMPresence::_mutex);
	data->cbk_id = gaim_timeout_add(0, 
		(GSourceFunc) GaimIMPresence::subscribeToPresenceOfCbk, data);
}

void GaimIMPresence::unsubscribeToPresenceOf(const std::string & contactId)
{
}

bool GaimIMPresence::blockContactCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMPresence::_mutex);

	GaimIMPresence * imPresence = ((misc_t *)data)->instance;
	const char * contactId = (const char *)((misc_t *)data)->data_ptr;
	GaimAccount *gAccount = gaim_accounts_find(imPresence->getIMAccount().getLogin().c_str(),
		GaimIMPrcl::GetPrclId(imPresence->getIMAccount().getProtocol()));

	if (gAccount)
	{
		gaim_privacy_deny_add(gAccount, contactId, FALSE);
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr);
	free((misc_t *)data);
	
	return TRUE;
}

void GaimIMPresence::blockContact(const std::string & contactId)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	data->data_ptr = strdup(contactId.c_str());

	Mutex::ScopedLock lock(GaimIMPresence::_mutex);
	data->cbk_id = gaim_timeout_add(0, 
		(GSourceFunc) GaimIMPresence::blockContactCbk, data);
}

bool GaimIMPresence::unblockContactCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMPresence::_mutex);

	GaimIMPresence * imPresence = ((misc_t *)data)->instance;
	const char * contactId = (const char *)((misc_t *)data)->data_ptr;
	GaimAccount *gAccount = gaim_accounts_find(imPresence->getIMAccount().getLogin().c_str(),
		GaimIMPrcl::GetPrclId(imPresence->getIMAccount().getProtocol()));

	if (gAccount)
	{
		gaim_privacy_permit_add(gAccount, contactId, FALSE);
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMPresence::unblockContact(const std::string & contactId)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	data->data_ptr = strdup(contactId.c_str());

	Mutex::ScopedLock lock(GaimIMPresence::_mutex);
	data->cbk_id = gaim_timeout_add(0, 
		(GSourceFunc) GaimIMPresence::unblockContactCbk, data);
}

bool GaimIMPresence::authorizeContactCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMPresence::_mutex);

	GaimIMPresence * imPresence = ((misc_t *)data)->instance;
	const char * contactId = (const char *)((misc_t *)data)->data_ptr;
	int authorized = ((misc_t *)data)->data_int;
	GaimAccount *gAccount = gaim_accounts_find(imPresence->getIMAccount().getLogin().c_str(),
		GaimIMPrcl::GetPrclId(imPresence->getIMAccount().getProtocol()));

	if (gAccount && imPresence->authRequestMap.find(contactId) != imPresence->authRequestMap.end()){
		if (authorized) {
			imPresence->authRequestMap[contactId]->accept_cb(
				imPresence->authRequestMap[contactId]->user_data);
		} else {
			imPresence->authRequestMap[contactId]->deny_cb(
				imPresence->authRequestMap[contactId]->user_data);
		}
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMPresence::authorizeContact(const std::string & contactId, 
									  bool authorized, const std::string message)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	data->data_ptr = strdup(contactId.c_str());
	data->data_int = authorized;

	Mutex::ScopedLock lock(GaimIMPresence::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMPresence::authorizeContactCbk, data);
}

void GaimIMPresence::addAuthRequestInList(const std::string &contactId, void *acceptCbk, 
										  void *denyCbk, void *userData)
{
	AuthRequest *authReq = new AuthRequest();

	authReq->accept_cb = (GaimAccountRequestAuthorizationCb) acceptCbk;
	authReq->deny_cb = (GaimAccountRequestAuthorizationCb) denyCbk;
	authReq->user_data = userData;

	authRequestMap[contactId] = authReq;
}
