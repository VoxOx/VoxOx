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

#include "stdafx.h"
#include "IMAccountManager.h"

#include <imwrapper/IMWrapperFactory.h>

#include <util/Logger.h>	//VOXOX - JRT - 2009.08.27 

IMAccountManager::IMAccountManager() 
{
}

IMAccountManager::~IMAccountManager() 
{
}

IMAccountList IMAccountManager::getIMAccountsOfProtocol(EnumIMProtocol::IMProtocol protocol) 	//VOXOX - JRT - 2009.04.09 - Removed const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMAccountList result;

	_imAccountList.getIMAccountsOfProtocol( protocol, result );

	return result;
}

IMAccountList IMAccountManager::getIMAccountsOfQtProtocol(QtEnumIMProtocol::IMProtocol protocol) //VOXOX - CJC - 2009.12.17 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMAccountList result;

	_imAccountList.getIMAccountsOfQtProtocol( protocol, result );

	return result;
}

IMAccountList IMAccountManager::getIMAccountsOfProtocolVector(std::vector<EnumIMProtocol::IMProtocol> protocols) 	//VOXOX - JRT - 2009.04.09 - Removed const
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMAccountList result;
	_imAccountList.getIMAccountsOfProtocolVector(protocols, result );
	return result;
}

IMAccountList IMAccountManager::getIMAccountListCopy() const 
{
	IMAccountList temp;
	temp = _imAccountList;
	return _imAccountList;
}

// VOXOX -ASV- 07-14-2009: We need to get the real object to set its state
IMAccount * IMAccountManager::getIMAccountNoClone( const IMAccount& imAccount ) 
{
	RecursiveMutex::ScopedLock lock(_mutex);
	
	IMAccount* result = NULL;
	IMAccount* imAccountNoClone = _imAccountList.findByAccount( imAccount );
	
	return imAccountNoClone;
}

//VOXOX CHANGE by Rolando - 2009.07.14 
IMAccount * IMAccountManager::getIMAccountNoClone(const std::string& imAccountId) 
{
	RecursiveMutex::ScopedLock lock(_mutex);
	
	IMAccount* result = _imAccountList.findByKeyNoClone( imAccountId );	//Does locking;
	if(result)
	{
		IMAccount* imAccountNoClone = _imAccountList.findByAccount( *result );
		return imAccountNoClone;
	}
	else
	{
		return NULL;
	}	
}

IMAccount * IMAccountManager::getIMAccount( const IMAccount& imAccount ) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMAccount* result = NULL;
	IMAccount* temp = _imAccountList.findByAccount( imAccount );

	if ( temp )
	{
		result = temp->clone();		//VOXOX - JRT - 2009.04.10 - TODO - Seems unnecessary to clone this,
	}								//				but it is called in about 180 places in the code!

	return result;
}

IMAccount * IMAccountManager::getIMAccount(const std::string& imAccountId) 	//VOXOX - JRT - 2009.04.09 - Removed const
{
	return _imAccountList.findByKeyAndClone( imAccountId );	//Does locking.
}

IMAccount* IMAccountManager::getIMAccount( int userNetworkId )
{
	RecursiveMutex::ScopedLock lock(_mutex);

	IMAccount* result = _imAccountList.findByUserNetworkId( userNetworkId );

	return result;
}

IMAccount* IMAccountManager::getIMAccount( QtEnumIMProtocol::ServerProtocolId protocolId )
{
	RecursiveMutex::ScopedLock lock(_mutex);

 	IMAccount* result = _imAccountList.findByServerProtocolId( protocolId );
	return result;
}

bool IMAccountManager::contains( const IMAccount& imAccount) 
{
	RecursiveMutex::ScopedLock lock(_mutex);
 	return _imAccountList.contains( imAccount );
}

bool IMAccountManager::containsVoxOxAccount() 
{
	RecursiveMutex::ScopedLock lock(_mutex);
	return _imAccountList.containsVoxOxAccount();
}


bool IMAccountManager::addIMAccount(const IMAccount & imAccount) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	bool result = false;

	IMAccount* pAcct = _imAccountList.findByAccount( imAccount );
	if ( pAcct == NULL )
	{
		_imAccountList.Add( const_cast<IMAccount&>(imAccount) );
		imAccountAddedEvent(*this, imAccount.getKey() );
		result = true;
	}

	return result;
}

bool IMAccountManager::removeIMAccount(const IMAccount & imAccount) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	bool result = false;

	//JRT - TODO: just Delete() and check result?
	IMAccount* pAcct = _imAccountList.findByAccount( imAccount );

	if ( pAcct )
	{
		// TODO: Hack. See IMWrapperFactory::removeIMAccount doc
		IMWrapperFactory::getFactory().removeIMAccount(imAccount);

		imAccountRemovedEvent(*this, imAccount.getKey() );
		_imAccountList.Delete( *pAcct );

		result = true;
	}

	return result;
}

//VOXOX CHANGE by Rolando - 2009.08.28 
void IMAccountManager::updateIMAccountLogin(std::string newLogin, IMAccount * imAccount){
	RecursiveMutex::ScopedLock lock(_mutex);

	if ( _imAccountList.contains( *imAccount ) )
	{
		IMAccount * newIMAccount = getIMAccount(*imAccount);
		if(newIMAccount)
		{
			newIMAccount->changeLogin(newLogin);
			IMAccountList::iterator iter = _imAccountList.find(imAccount->getKey() );

			if(iter != _imAccountList.end())
			{
				_imAccountList.Delete(iter->second);
				_imAccountList.Add(const_cast<IMAccount&>(*newIMAccount));
				imAccountUpdatedEvent(*this, newIMAccount->getKey());
			}
		}
	}
}

bool IMAccountManager::addFriendsFromMySpace(const IMAccount & imAccount) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	bool result = false;

	if ( _imAccountList.contains( imAccount ) )
	{
		IMWrapperFactory::getFactory().importContactsFromMySpaceServer(imAccount);

		imAccountUpdatedEvent(*this, imAccount.getKey());

		result = true;
	}

	return result;
}


bool IMAccountManager::updateIMAccount(const IMAccount & imAccount) 
{
	LOG_DEBUG("updating IMAccount");
	RecursiveMutex::ScopedLock lock(_mutex);

	bool result = false;

	IMAccount* pAcct = _imAccountList.findByAccount( imAccount );
	if ( pAcct )
	{
		*pAcct = imAccount;
		IMWrapperFactory::getFactory().imAccountUpdated(imAccount);
		imAccountUpdatedEvent(*this, imAccount.getKey());
		result = true;
	}

	return result;
}

void IMAccountManager::emitIMAccountAddedEvent() 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	for (IMAccountList::const_iterator it = _imAccountList.begin(); it != _imAccountList.end(); ++it) 
	{
		imAccountAddedEvent(*this, it->second.getKey());
	}
}

bool IMAccountManager::setImAccountServerProtocolId( const IMAccount & imAccount )
{
	RecursiveMutex::ScopedLock lock(_mutex);

	bool result = false;
	IMAccount* pAcct = _imAccountList.findByAccount( imAccount );

	if ( pAcct )
	{
		pAcct->setServerProtocolId( imAccount.getServerProtocolId() );
		result = true;
	}

	return result;
}

bool IMAccountManager::setImAccountUserNetworkId( const IMAccount & imAccount )
{
	RecursiveMutex::ScopedLock lock(_mutex);

	bool result = false;
	IMAccount* pAcct = _imAccountList.findByAccount( imAccount );

	if ( pAcct )
	{
		pAcct->setUserNetworkId( imAccount.getUserNetworkId() );
		result = true;
	}

	return result;
}

bool IMAccountManager::setImAccountUserSelectedPresenceState( const IMAccount & imAccount )
{
	RecursiveMutex::ScopedLock lock(_mutex);

	bool result = false;
	IMAccount* pAcct = _imAccountList.findByAccount( imAccount );

	if ( pAcct )
	{
		pAcct->updateInitialPresenceState( imAccount.getInitialPresenceState() );
		result = true;
	}

	return result;
}

//VOXOX - JRT - 2009.09.15 
void IMAccountManager::setInitialPresence( EnumPresenceState::PresenceState presenceState )
{
	for (IMAccountList::iterator it = _imAccountList.begin(); it != _imAccountList.end(); ++it) 
	{
		(*it).second.updateInitialPresenceState( presenceState );
	}
}


