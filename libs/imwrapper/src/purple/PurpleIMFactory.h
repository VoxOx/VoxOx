/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#ifndef OWPURPLEIMFACTORY_H
#define OWPURPLEIMFACTORY_H

#include <imwrapper/IMWrapperFactory.h>
#include <imwrapper/IMAccount.h>

#include <glib.h>
#include <glib/gtypes.h>

class PurpleAccountMngr;
class PurpleChatMngr;
class PurpleConnectMngr;
class PurpleContactListMngr;
class PurplePresenceMngr;

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class PurpleIMFactory : public IMWrapperFactory {

	friend gpointer PurpleMainEventLoop(gpointer data);

public:

	PurpleIMFactory();
	~PurpleIMFactory();

	virtual void init(const std::string & pathToProfile);

	virtual void terminate();

	virtual IMConnect *createIMConnect(IMAccount & account);

	virtual IMChat *createIMChat(IMAccount & account);

	virtual IMPresence *createIMPresence(IMAccount & account);

	virtual IMContactList *createIMContactList(IMAccount & account);

	virtual void removeIMAccount(IMAccount imAccount);

	//VOXOX CHANGE CJC ADD FRIEND FROM MYSPACE SERVER
	virtual void importContactsFromMySpaceServer(IMAccount imAccount); 

	virtual void imAccountUpdated(IMAccount imAccount);

	static bool equals(const IMAccount & imAccount,
		std::string login, EnumIMProtocol::IMProtocol protocol);

	static void PurpleSetCallbacks();

	static void PurpleQuitCallback();

private:

	PurpleAccountMngr *AccountMngr;
	PurpleConnectMngr *ConnectMngr;
	PurplePresenceMngr *PresenceMngr;
	PurpleChatMngr *ChatMngr;
	PurpleContactListMngr *ContactListMngr;

	void PurpleIMInit(const std::string & pathToProfile);

	void PurpleWrapperInit();

	static PurpleIMFactory *_instance;
};

#endif //OWPURPLEIMFACTORY_H
