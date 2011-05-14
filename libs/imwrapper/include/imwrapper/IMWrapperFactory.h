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

#ifndef OWIMWRAPPERFACTORY_H
#define OWIMWRAPPERFACTORY_H

#include <imwrapper/IMAccount.h>

#include <util/Interface.h>

class IMChat;
class IMConnect;
class IMPresence;
class IMContactList;

/**
 * Factories for the Instant Messaging wrapper component.
 *
 * Depending on the factory instanciated, the IM implementation component will use Gaim
 * or what ever you prefer.
 *
 * @author Philippe Bernery
 * @author Tanguy Krotoff
 */
class IMWrapperFactory : Interface {
public:

	virtual ~IMWrapperFactory() {
	}

	/**
	 * Sets the IM implementation to use.
	 *
	 * @param factory factory to create the SIP implementation component
	 */
	static void setFactory(IMWrapperFactory * factory) {
		_factory = factory;
	}

	/**
	 * Gets the factory that will instanciate a IM implementation plugin.
	 *
	 * @return IM implementation
	 */
	static IMWrapperFactory & getFactory() {
		return *_factory;
	}

	/**
	 * Initializes the factory.
	 */
	virtual void init(const std::string & pathToProfile) = 0;

	/**
	 * Terminates the factory.
	 */
	virtual void terminate() = 0;

	/**
	 * Instantiates a IM connect implementation.
	 *
	 * @param account IM account
	 * @return IM connect implementation instance
	 */
	virtual IMConnect * createIMConnect(IMAccount & account) = 0;

	/**
	 * Instantiates a IM chat implementation.
	 *
	 * @param account IM account
	 * @return IM chat implementation instance
	 */
	virtual IMChat * createIMChat(IMAccount & account) = 0;

	/**
	 * Instantiates a IM presence implementation.
	 *
	 * @param account IM account
	 * @return IM presence implementation instance
	 */
	virtual IMPresence * createIMPresence(IMAccount & account) = 0;

	/**
	 * Instantiates a IMContactList implementation.
	 *
	 * @param account an IMAccount
	 * @return the new IMContactList instance.
	 */
	virtual IMContactList * createIMContactList(IMAccount & account) = 0;

	/** 
	 * Tells the wrapper to remove all class associated with the given IMAccount. 
	 * TODO: this is a workaround until the refactoring from CoIpManager branch. 
	 */ 
	virtual void removeIMAccount(IMAccount imAccount) = 0; 
	
	//VOXOX CHANGE CJC ADD FRIEND FROM MYSPACE SERVER
	virtual void importContactsFromMySpaceServer(IMAccount imAccount) = 0; 

	/**
	 * Tells IMWrapper that an IMAccount has been updated.
	 */
	virtual void imAccountUpdated(IMAccount imAccount) = 0;

private:

	static IMWrapperFactory * _factory;
};

#endif	//OWIMWRAPPERFACTORY_H
