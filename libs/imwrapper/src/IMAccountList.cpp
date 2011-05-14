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

#include <imwrapper/IMAccountList.h>

//-----------------------------------------------------------------------------

IMAccount*  IMAccountList::findByAccount( const IMAccount& rAcct  )
{
	IMAccount* pRet = NULL;

	pRet = Lookup( const_cast<IMAccount&>(rAcct).getKey() );	//Does locking

	//VOXOX - JRT - 2009.07.14 
	if ( pRet == NULL )
	{
		pRet = findByUUID( rAcct.getUUID() );
	}

	return pRet;
}

//-----------------------------------------------------------------------------

IMAccount * IMAccountList::findByKey(const std::string & keyIn)
{
	IMAccount *result = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );		//VOXOX - JRT - 2009.07.13

	std::string key = String::toLowerCase( keyIn );	//VOXOX - JRT - 2009.10.29 

	iterator it = find( key );

	if ( it != end() )
	{
		result = &(*it).second;
	}

	return result;
}

//-----------------------------------------------------------------------------
	
IMAccount* IMAccountList::findByKeyAndClone( const std::string& imAccountId )
{
	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	IMAccount* result = NULL;
	IMAccount* temp = findByKey( imAccountId );

	if ( temp )
	{
		result = temp->clone();		//VOXOX - JRT - 2009.04.10 - TODO - Seems unnecessary to clone this,
	}								//				but it is called in about 180 places in the code!

	return result;
}

//VOXOX CHANGE by Rolando - 2009.07.14 
IMAccount* IMAccountList::findByKeyNoClone( const std::string& imAccountId )
{
	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX CHANGE by Rolando - 2009.07.14

	IMAccount* result = NULL;
	IMAccount* temp = findByKey( imAccountId );

	if ( temp )
	{
		result = temp;//VOXOX CHANGE by Rolando - 2009.07.14 
	}


	return result;
}

//-----------------------------------------------------------------------------

IMAccount* IMAccountList::findByLoginInfo( const std::string& loginValue, EnumIMProtocol::IMProtocol protocol )
{
	IMAccount* pRet = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( iterator it = begin();  it != end(); ++it) 
	{
		if ((it->second.getLogin() == loginValue) &&		
			(it->second.getProtocol() == protocol) ) 
		{
			pRet = &(it->second);
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.07.14 - needed for proper updates in case where user editted 
//							 IM Account login info.
IMAccount* IMAccountList::findByUUID( const std::string& uuid )
{
	IMAccount* pRet = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );

	for ( iterator it = begin();  it != end(); ++it) 
	{
		if ( it->second.getUUID() == uuid)		
		{
			pRet = &(it->second);
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

IMAccount* IMAccountList::findByServerProtocolId( QtEnumIMProtocol::ServerProtocolId tgtProtocolId )
{
	IMAccount* pRet = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( iterator it = begin();  it != end(); ++it) 
	{
		if (it->second.getServerProtocolId() == tgtProtocolId ) 
		{
			pRet = &(it->second);
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

IMAccount* IMAccountList::findByUserNetworkId( int tgtUserNetworkId )
{
	IMAccount* pRet = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( iterator it = begin();  it != end(); ++it) 
	{
		if (it->second.getUserNetworkId() == tgtUserNetworkId ) 
		{
			pRet = &(it->second);
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

IMAccount* IMAccountList::getFirstIMAccount()
{
	IMAccount* pRet = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	IMAccountList::iterator it = begin();

	if (it != end()) 
	{
		pRet = &(it->second);
	}

	return pRet;
}

//-----------------------------------------------------------------------------

IMAccount* IMAccountList::findFirstValidAccount( const IMContactSet& rImContactSet )
{
	IMAccount* pRet = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( iterator upIt = begin(); upIt != end(); ++upIt) 
	{
		for (IMContactSet::const_iterator imIt = rImContactSet.begin(); imIt != rImContactSet.end(); ++imIt) 
		{
			if ( upIt->second.getKey() == (*imIt).getIMAccountId()) 
			{
				pRet = &(upIt->second);
				break;
			}
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

bool IMAccountList::contains( const IMAccount& rAcct )
{
	return ( findByKey( const_cast<IMAccount&>(rAcct).getKey() ) != NULL );	//Does locking
}

//-----------------------------------------------------------------------------
	
bool IMAccountList::containsVoxOxAccount()
{
	return (getVoxOxAccount() != NULL);
}

//-----------------------------------------------------------------------------

IMAccount* IMAccountList::getVoxOxAccount() const
{
	IMAccount* pRet = NULL;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); ++it) 
	{
		if ( it->second.isVoxOxAccount()) 
		{
			pRet = const_cast<IMAccount*>(&(it->second));
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

bool IMAccountList::update( IMAccount& imAccount )
{
	bool bUpdated = false;

	IMAccount* pAccount = Lookup( imAccount.getKey() );

	if( pAccount )
	{
		*pAccount = imAccount;
		bUpdated  = true;
	}

	return bUpdated;
}

//-----------------------------------------------------------------------------

EnumPresenceState::PresenceState IMAccountList::getPresenceState() const 
{
	EnumPresenceState::PresenceState psRet = EnumPresenceState::PresenceStateUnknown;

	if ( size() == 0 )
	{
		psRet = EnumPresenceState::PresenceStateUnavailable;
	}
	else
	{
		RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

		// Check whether all states are the same
		for ( const_iterator it = begin(); it != end(); ++it) 
		{
			if ( it == begin() )
			{
				psRet = it->second.getPresenceState();
			}
			else
			{
				if ( it->second.getPresenceState() != psRet )
				{
					psRet = EnumPresenceState::PresenceStateMulti;
					break;
				}
			}
		}
	}

	return psRet;
}

//-----------------------------------------------------------------------------
//Same as getPresenceState() except we a check for Protocol.
EnumPresenceState::PresenceState IMAccountList::getIMPresenceState() const 
{
	EnumPresenceState::PresenceState psRet = EnumPresenceState::PresenceStateUnknown;
	bool isIMAccountFound = false;

	if ( size() == 0 )
	{
		psRet = EnumPresenceState::PresenceStateUnavailable;
	}
	else
	{
		psRet = EnumPresenceState::PresenceStateUnknown;		//VOXOX - JRT TODO - 2009.04.09 - NOTE: This default is different from others.

		RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

		for ( const_iterator it = begin(); it != end(); ++it) 
		{
			if ( it == begin() )
			{
				if( it->second.getProtocol() != EnumIMProtocol::IMProtocolWengo && 
					it->second.getProtocol() != EnumIMProtocol::IMProtocolSIP)
				{
					psRet = it->second.getPresenceState();
					isIMAccountFound = true;
				}
			}
			else
			{
				if( it->second.getProtocol() != EnumIMProtocol::IMProtocolWengo && 
					it->second.getProtocol() != EnumIMProtocol::IMProtocolSIP)
				{
					if (it->second.getPresenceState() != psRet && isIMAccountFound) 
					{
						psRet = EnumPresenceState::PresenceStateMulti;
						break;
					}

					psRet			 = it->second.getPresenceState();
					isIMAccountFound = true;
				}
			}
		}
	}

	return psRet;
}

//-----------------------------------------------------------------------------

EnumPresenceState::PresenceState IMAccountList::getVoxOxIMPresenceState() const 
{
	EnumPresenceState::PresenceState psRet = EnumPresenceState::PresenceStateUnknown;

	if ( size() == 0 )
	{
		psRet = EnumPresenceState::PresenceStateUnavailable;
	}
	else
	{
		psRet = EnumPresenceState::PresenceStateUnavailable;	//In case VoxOx Account not found.

		RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

		for ( const_iterator it = begin(); it != end(); ++it) 
		{
			if ( it->second.isVoxOxAccount() )
			{
				psRet = it->second.getPresenceState();
				break;
			}
		}
	}

	return psRet;
}

//-----------------------------------------------------------------------------

EnumPresenceState::PresenceState IMAccountList::getSIPPresenceState() const 
{
	EnumPresenceState::PresenceState psRet = EnumPresenceState::PresenceStateUnknown;
	bool isSIPAccountFound = false;

	if ( size() == 0 )
	{
		psRet = EnumPresenceState::PresenceStateUnavailable;
	}
	else
	{
		psRet = EnumPresenceState::PresenceStateUnknown;		//VOXOX - JRT TODO - 2009.04.09 - NOTE: This default is different from others.

		RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

		for ( const_iterator it = begin(); it != end(); ++it) 
		{
			if ( it == begin() )
			{
				if( it->second.getProtocol() == EnumIMProtocol::IMProtocolWengo || 
					it->second.getProtocol() == EnumIMProtocol::IMProtocolSIP)
				{
					psRet = it->second.getPresenceState();
					isSIPAccountFound = true;

				}
			}
			else
			{
				if( it->second.getProtocol() == EnumIMProtocol::IMProtocolWengo || 
					it->second.getProtocol() == EnumIMProtocol::IMProtocolSIP)
				{
					if (it->second.getPresenceState() != psRet && isSIPAccountFound) 
					{
						psRet = EnumPresenceState::PresenceStateMulti;
						break;
					}
					psRet = it->second.getPresenceState();
					isSIPAccountFound = true;
				}
			}
		}
	}

	return psRet;
}

//-----------------------------------------------------------------------------

bool IMAccountList::isConnected() const 
{
	bool bConnected = false;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); ++it) 
	{
		if ( it->second.isConnected() ) 
		{
			bConnected = true;
		}
	}

	return bConnected;
}

//-----------------------------------------------------------------------------

bool IMAccountList::isIMConnected() const 
{
	bool bConnected = false;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); ++it) 
	{
		if( it->second.getProtocol() != EnumIMProtocol::IMProtocolWengo && 
			it->second.getProtocol() != EnumIMProtocol::IMProtocolSIP)
		{
			if ( it->second.isConnected() ) 
			{
				bConnected = true;
				break;
			}
		}
	}

	return bConnected;
}

//-----------------------------------------------------------------------------

bool IMAccountList::isSIPConnected() const 
{
	bool bConnected = false;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); ++it) 
	{
		if( it->second.getProtocol() == EnumIMProtocol::IMProtocolWengo || 
			it->second.getProtocol() == EnumIMProtocol::IMProtocolSIP)
		{
			if ( it->second.isConnected() ) 
			{
				bConnected = true;
				break;
			}
		}
	}

	return bConnected;
}

//-----------------------------------------------------------------------------

bool IMAccountList::isVoxOxConnected() const 
{
	bool bConnected = false;

	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); ++it) 
	{
		if( it->second.isVoxOxAccount() )
		{
			bConnected = it->second.isConnected();
			break;
		}
	}

	return bConnected;
}

//-----------------------------------------------------------------------------

void IMAccountList::getIMAccountsOfProtocol(EnumIMProtocol::IMProtocol protocol, IMAccountList& rResult )
{
	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); ++it ) 
	{
		// FIXME: line 50 and 51 should not be there.
		// There are here for compatibility with previous version of WengoPhone.
		EnumIMProtocol::IMProtocol accountProtocol = it->second.getProtocol();

		if  ( ( accountProtocol == protocol) ||
			( ( protocol       == EnumIMProtocol::IMProtocolSIPSIMPLE) && (accountProtocol == EnumIMProtocol::IMProtocolSIP) ) ||
			( (accountProtocol == EnumIMProtocol::IMProtocolSIPSIMPLE) && (protocol        == EnumIMProtocol::IMProtocolSIP) )  )
		{
			rResult.Add( const_cast<IMAccount&>(it->second) );
		}
	}
}

void IMAccountList::getIMAccountsOfQtProtocol(QtEnumIMProtocol::IMProtocol protocol, IMAccountList& rResult )
{
	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for ( const_iterator it = begin(); it != end(); ++it ) 
	{
		// FIXME: line 50 and 51 should not be there.
		// There are here for compatibility with previous version of WengoPhone.
		QtEnumIMProtocol::IMProtocol accountProtocol = it->second.getQtProtocol();

		if ( accountProtocol == protocol) 
		{
			rResult.Add( const_cast<IMAccount&>(it->second) );
		}
	}
}
//-----------------------------------------------------------------------------

void IMAccountList::getIMAccountsOfProtocolVector( std::vector<EnumIMProtocol::IMProtocol> protocols, IMAccountList& rResult )
{
	RecursiveMutex::ScopedLock lock( _mutex );	//VOXOX - JRT - 2009.07.13 

	for (IMAccountList::const_iterator it = begin(); it != end(); ++it) 
	{
		for( std::vector<EnumIMProtocol::IMProtocol>::const_iterator itv = protocols.begin(); itv != protocols.end(); ++itv ) 
		{
			if (it->second.getProtocol() == (*itv)) 
			{
				rResult.Add( const_cast<IMAccount&>(it->second) );
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
