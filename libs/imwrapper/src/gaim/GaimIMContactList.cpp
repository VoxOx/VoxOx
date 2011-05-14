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

#include "GaimIMContactList.h"

#include "GaimEnumIMProtocol.h"

#include <util/Logger.h>

extern "C" {
#include <gaim/blist.h>
}

typedef struct misc_s
{
	GaimIMContactList * instance;
	void * data_ptr1;
	void * data_ptr2;
	void * data_ptr3;
	int cbk_id;
}	misc_t;

Mutex GaimIMContactList::_mutex;

GaimIMContactList::GaimIMContactList(IMAccount account)
	: IMContactList(account)
{
}

bool GaimIMContactList::addContactCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMContactList::_mutex);

	GaimIMContactList * imContactList = ((misc_t *)data)->instance;
	const char * groupName =(const char *)((misc_t *)data)->data_ptr1;
	const char * contactId =(const char *)((misc_t *)data)->data_ptr2;
	GaimAccount *gAccount = gaim_accounts_find(imContactList->getIMAccount().getLogin().c_str(),
								GaimIMPrcl::GetPrclId(imContactList->getIMAccount().getProtocol()));
	GaimGroup *gGroup = NULL;
	GaimBuddy *gBuddy = NULL;

	if ((gGroup = gaim_find_group(groupName)) == NULL) {
		gGroup = gaim_group_new(groupName);
		gaim_blist_add_group(gGroup, NULL);
	}

	gBuddy = gaim_buddy_new(gAccount, contactId, contactId);
	gaim_blist_add_buddy(gBuddy, NULL, gGroup, NULL);
	gaim_account_add_buddy(gAccount, gBuddy);

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr1);
	free(((misc_t *)data)->data_ptr2);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMContactList::addContact(const std::string & groupName, 
								   const std::string & contactId)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	data->data_ptr1 = strdup(groupName.c_str());
	data->data_ptr2 = strdup(contactId.c_str());

	Mutex::ScopedLock lock(GaimIMContactList::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMContactList::addContactCbk, data);
}

bool GaimIMContactList::removeContactCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMContactList::_mutex);

	GaimIMContactList * imContactList = ((misc_t *)data)->instance;
	const char * groupName =(const char *)((misc_t *)data)->data_ptr1;
	const char * contactId =(const char *)((misc_t *)data)->data_ptr2;
	GaimAccount *gAccount = gaim_accounts_find(imContactList->getIMAccount().getLogin().c_str(),
								GaimIMPrcl::GetPrclId(imContactList->getIMAccount().getProtocol()));
	GaimGroup *gGroup = NULL;
	GaimBuddy *gBuddy = NULL;

	if (groupName)
	{
		gGroup = gaim_find_group(groupName);
		gBuddy = gaim_find_buddy_in_group(gAccount, contactId, gGroup);
	}
	else
	{
		gBuddy = gaim_find_buddy(gAccount, contactId);
	}

	if (gBuddy) {
		gaim_account_remove_buddy(gAccount, gBuddy, gGroup);
		gaim_blist_remove_buddy(gBuddy);
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	if (((misc_t *)data)->data_ptr1) {
		free(((misc_t *)data)->data_ptr1);
	}
	free(((misc_t *)data)->data_ptr2);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMContactList::removeContact(const std::string & groupName, 
									  const std::string & contactId)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	if (!groupName.empty())
		data->data_ptr1 = strdup(groupName.c_str());
	data->data_ptr2 = strdup(contactId.c_str());

	Mutex::ScopedLock lock(GaimIMContactList::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMContactList::removeContactCbk, data);
}

bool GaimIMContactList::addGroupCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMContactList::_mutex);

	const char * groupName = (const char *)((misc_t *)data)->data_ptr1;
	GaimGroup *gGroup = gaim_find_group(groupName);

	if (!gGroup) {
		gGroup = gaim_group_new(groupName);
		gaim_blist_add_group(gGroup, NULL);
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr1);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMContactList::addGroup(const std::string & groupName)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->data_ptr1 = strdup(groupName.c_str());

	Mutex::ScopedLock lock(GaimIMContactList::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMContactList::addGroupCbk, data);
}

bool GaimIMContactList::removeGroupCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMContactList::_mutex);

	const char * groupName = (const char *)((misc_t *)data)->data_ptr1;
	GaimGroup *gGroup = gaim_find_group(groupName);

	if (gGroup) {
		gaim_blist_remove_group(gGroup);
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr1);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMContactList::removeGroup(const std::string & groupName)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->data_ptr1 = strdup(groupName.c_str());

	Mutex::ScopedLock lock(GaimIMContactList::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMContactList::removeGroupCbk, data);
}

bool GaimIMContactList::changeGroupNameCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMContactList::_mutex);

	const char * newGroupName = (const char *)((misc_t *)data)->data_ptr1;
	const char * oldGroupName = (const char *)((misc_t *)data)->data_ptr2;
	GaimGroup *gGroup = gaim_find_group(oldGroupName);
	
	if (gGroup) {
		gaim_blist_rename_group(gGroup, newGroupName);
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr1);
	free(((misc_t *)data)->data_ptr2);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMContactList::changeGroupName(const std::string & oldGroupName, 
										const std::string & newGroupName)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->data_ptr1 = strdup(newGroupName.c_str());
	data->data_ptr2 = strdup(oldGroupName.c_str());

	Mutex::ScopedLock lock(GaimIMContactList::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMContactList::changeGroupNameCbk, data);
}

bool GaimIMContactList::moveContactToGroupCbk(void * data)
{
	Mutex::ScopedLock lock(GaimIMContactList::_mutex);

	GaimIMContactList * imContactList = ((misc_t *)data)->instance;
	const char * newGroupName = (const char *)((misc_t *)data)->data_ptr1;
	const char * oldGroupName = (const char *)((misc_t *)data)->data_ptr2;
	const char * contactId = (const char *)((misc_t *)data)->data_ptr3;
	GaimAccount *gAccount = gaim_accounts_find(imContactList->getIMAccount().getLogin().c_str(),
		GaimIMPrcl::GetPrclId(imContactList->getIMAccount().getProtocol()));
	GaimGroup *gOldGroup = gaim_find_group(oldGroupName);

	if (gOldGroup) {
		GaimBuddy *gBuddy = gaim_find_buddy_in_group(gAccount, contactId, gOldGroup);
		if (gBuddy) {
			GaimGroup *gNewGroup = gaim_find_group(newGroupName);
			if (gNewGroup) {
				gNewGroup = gaim_group_new(newGroupName);
				gaim_blist_add_buddy(gBuddy, NULL, gNewGroup, NULL);
			}
		}
	}

	gaim_timeout_remove(((misc_t *)data)->cbk_id);

	free(((misc_t *)data)->data_ptr1);
	free(((misc_t *)data)->data_ptr2);
	free(((misc_t *)data)->data_ptr3);
	free((misc_t *)data);

	return TRUE;
}

void GaimIMContactList::moveContactToGroup(const std::string & newGroupName,
	const std::string & oldGroupName, const std::string & contactId)
{
	misc_t * data = (misc_t *) malloc(sizeof(misc_t));
	memset(data, 0, sizeof(misc_t));

	data->instance = this;
	data->data_ptr1 = strdup(newGroupName.c_str());
	data->data_ptr2 = strdup(oldGroupName.c_str());
	data->data_ptr3 = strdup(contactId.c_str());

	Mutex::ScopedLock lock(GaimIMContactList::_mutex);
	data->cbk_id = gaim_timeout_add(0, (GSourceFunc) GaimIMContactList::moveContactToGroupCbk, data);
}
