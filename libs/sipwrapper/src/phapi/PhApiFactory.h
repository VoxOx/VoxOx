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

#ifndef PHAPIFACTORY_H
#define PHAPIFACTORY_H

#include <sipwrapper/SipWrapperFactory.h>
#include <imwrapper/IMWrapperFactory.h>

class IMAccount;
class IMConnect;
class IMChat;
class IMPresence;
class PhApiWrapper;
class PhApiIMChat;

/**
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class PhApiFactory : public SipWrapperFactory, public IMWrapperFactory {
public:

	PhApiFactory();

	virtual SipWrapper * createSipWrapper();

	virtual void init(const std::string & pathToProfile);

	virtual void terminate();

	virtual IMConnect * createIMConnect(IMAccount & account);

	virtual IMChat * createIMChat(IMAccount & account);

	virtual IMPresence * createIMPresence(IMAccount & account);

	virtual IMContactList * createIMContactList(IMAccount & account);

	virtual void removeIMAccount(IMAccount imAccount) {}

	//VOXOX CHANTE CJC ADD SUPPORT FOR MYSPACE SERVER CONTACTS
	virtual void importContactsFromMySpaceServer(IMAccount imAccount) {}

	virtual void imAccountUpdated(IMAccount imAccount) {}

private:

	PhApiWrapper * _phApiWrapperInstance;

};

#endif	//PHAPIFACTORY_H
