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

#ifndef OWIMACCOUNTMANAGER_H
#define OWIMACCOUNTMANAGER_H

#include <imwrapper/IMAccountList.h>
#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/QtEnumIMProtocol.h>

#include <thread/RecursiveMutex.h>

#include <util/NonCopyable.h>
#include <util/Event.h>

#include <vector>
#include <string>

class UserProfile;
//class IMAccount;
//class IMAccountList;

/**
 * Manage the IMAccountList.
 *
 * This class access the given IMAccountList in a thread-safe.
 *
 * @author Philippe Bernery
 */
class IMAccountManager : NonCopyable 
{
	friend class UserProfile;
	friend class UserProfileFileStorage;
	friend class UserProfileFileStorage1;

public:

	/**
	 * A new IMAccount has been added.
	 *
	 * @param sender this class
	 * @param imAccountId the id of the added IMAccount.
	 */
	Event< void (IMAccountManager & sender, std::string imAccountId) > imAccountAddedEvent;

	/**
	 * Emitted when an IMAccount has been removed.
	 *
	 * @param sender this class
	 * @param imAccountId the id of the removed IMAccount.
	 */
	Event< void (IMAccountManager & sender, std::string imAccountId) > imAccountRemovedEvent;

	/**
	 * Emitted when an IMAccount has been updated.
	 *
	 * @param sender this class
	 * @param imAccountId the id of the updates IMAccount.
	 */
	Event< void (IMAccountManager & sender, std::string imAccountId) > imAccountUpdatedEvent;

	/**
	 * @param protocol the desired protocol
	 * @return IMAccountList list of IMAccount  with protocol 'protocol'
	 */
	IMAccountList getIMAccountsOfProtocol(EnumIMProtocol::IMProtocol protocol);	//VOXOX - JRT - 2009.04.09 

	IMAccountList getIMAccountsOfQtProtocol(QtEnumIMProtocol::IMProtocol protocol);//VOXOX - CJC - 2009.12.17 
	/**
	 * @param protocol vector of desired protocols
	 * @return IMAccountList list of IMAccount  with protocols from 'protocol'
	 */
	IMAccountList getIMAccountsOfProtocolVector(std::vector<EnumIMProtocol::IMProtocol> protocols);	//VOXOX - JRT - 2009.04.09 - Removed const
	
	/**
	 * @return a copy of the IMAccountList
	 */
	IMAccountList getIMAccountListCopy() const;

	/**
	 * Gets a clone of the IMAccount with id 'imAccountId'.
	 * @return the IMAccount or NULL if not found
	 */
	IMAccount* getIMAccountNoClone( const IMAccount&   imAccount  );// VOXOX -ASV- 07-14-2009: We need to get the real object to set its state
	IMAccount* getIMAccountNoClone( const std::string& imAccountId);//VOXOX CHANGE by Rolando - 2009.07.14 
	IMAccount* getIMAccount( const std::string& imAccountId  );
	IMAccount* getIMAccount( const IMAccount&   imAccount    );
	IMAccount* getIMAccount( int userNetworkId );			//VOXOX - JRT - 2009.06.05 
	IMAccount* getIMAccount( QtEnumIMProtocol::ServerProtocolId protocolId );		//VOXOX - JRT - 2009.06.05 

	/**
	 * @return true if the IMAccountList contains the IMAccount with id 'imAccountId'
	 */
	bool contains(const std::string& imAccountId);
	bool contains(const IMAccount& rAccount );

	bool containsVoxOxAccount();

	const IMAccountList& getIMAccountListConst() const		{ return _imAccountList;	}

	bool  setImAccountServerProtocolId			( const IMAccount & imAccount );		//VOXOX - JRT - 2009.06.03 
	bool  setImAccountUserNetworkId				( const IMAccount & imAccount );		//VOXOX - JRT - 2009.06.09 
	bool  setImAccountUserSelectedPresenceState	( const IMAccount & imAccount );		//VOXOX - JRT - 2009.06.09 

	void  setInitialPresence( EnumPresenceState::PresenceState presenceState );


private:
	IMAccountManager();
	virtual ~IMAccountManager();

	/**
	 * @name Method used by UserProfile
	 * @{
	 */

	/**
	 * Adds an IMAccount.
	 *
	 * @return true if actually added
	 */
	bool addIMAccount(const IMAccount & imAccount);

	/**
	 * Removes an IMAccount.
	 *
	 * @return true if actually removed
	 */
	bool removeIMAccount(const IMAccount & imAccount);

	//VOXOX CHANGE by Rolando - 2009.08.28 
	void updateIMAccountLogin(std::string newLogin, IMAccount * imAccount);

	bool addFriendsFromMySpace(const IMAccount & imAccount);

	/**
	 * Updates an IMAccount.
	 *
	 * @return true if actually updated
	 */
	bool updateIMAccount(const IMAccount & imAccount);

	/**
	 * Reemits imAccountAddedEvent for all IMAccounts.
	 */
	void emitIMAccountAddedEvent();
	
	/**
	 * @}
	 */

	IMAccountList _imAccountList;

	mutable RecursiveMutex _mutex;
};

#endif //OWIMACCOUNTMANAGER_H
