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

#include <imwrapper/IMContactSet.h>
#include <imwrapper/IMContact.h>

using namespace std;

IMContactSet::IMContactSet()
{
}

IMContactSet::IMContactSet( const IMContactSet& src )
{
	operator=(src);
}

IMContactSet::~IMContactSet()
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	clear();
}
	
IMContactSet& IMContactSet::operator=( const IMContactSet& src )
{
	if ( this != &src )
	{
		RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

		clear();

		for (IMContactSet::const_iterator it = src.begin(); it != src.end(); it++) 
		{
			add( (*it) );
		}
	}

	return *this;
}

void IMContactSet::add( const IMContact &imContact )
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	pair<IMContactSet::const_iterator, bool> result = insert(imContact);
}

bool IMContactSet::Delete( const IMContact& imContact )
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	bool result = false;

	for (IMContactSet::const_iterator it = begin(); it != end(); it++) 
	{
		if ( (*it) == imContact )
		{
			erase( *it );
			result = true;
			break;
		}
	}

	return result;
}

std::string IMContactSet::getFirstContactId()
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	std::string contactId = "";

	for (IMContactSet::const_iterator it = begin(); it != end(); it++) 
	{
		contactId = it->getContactId();
		break;
	}
		
	return contactId;
}

std::string IMContactSet::getFirstDisplayContactId() const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	std::string contactId = "";

	for (IMContactSet::const_iterator it = begin(); it != end(); it++) 
	{
		contactId = it->getDisplayContactId();
		break;
	}
		
	return contactId;
}

std::string IMContactSet::getFirstKey() const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	std::string key = "";

	for (IMContactSet::const_iterator it = begin(); it != end(); it++) 
	{
		key = it->getKey();
		break;
	}
		
	return key;
}

std::string IMContactSet::getFirstStatusMessage() const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	std::string result	  = "";
	IMContact*  imContact = getFirst();

	if ( imContact )
	{
		result = imContact->getStatusMessage();
	}
		
	return result;
}

IMContact* IMContactSet::getFirstAvailable( const std::string imAccountId ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	IMContact* result = NULL;

	for ( const_iterator it = begin(); it != end(); ++it) 
	{
//		if ( (*it).getPresenceState() != EnumPresenceState::PresenceStateOffline )
		if ( !(*it).isOffline() )		//VOXOX - JRT - 2009.09.10 
		{
			if ( (*it).getIMAccountId()  == imAccountId )
			{
				result = const_cast<IMContact*>(&(*it));
				break;
			}
		}
	}

	return result;
}

IMContact* IMContactSet::getFirstValidAlias() const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	IMContact* result = NULL;

	for ( const_iterator it = begin(); it != end(); ++it) 
	{
		if (!(*it).getAlias().empty()) 
		{
			result = const_cast<IMContact*>(&(*it));
			break;
		}
	}

	return result;
}

std::string IMContactSet::getFirstAvailableSIPNumber() const 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	std::string result = "";

	for ( const_iterator it = begin(); it != end(); ++it) 
	{
//		if ((*it).getPresenceState() != EnumPresenceState::PresenceStateOffline && (*it).isIMAccountVoxOx()) 
		if ( !(*it).isOffline() && (*it).isIMAccountVoxOx()) //VOXOX - JRT - 2009.09.10 
		{
			result = (*it).getContactId();
			break;
		}
	}

	return result;
}

IMContact IMContactSet::getFirstAvailableWengoIMContact() const //VOXOX - JRT - 2009.07.11 - TODO why is this not a ptr/reference?
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	IMContact result;

	for (const_iterator it = begin(); it != end(); ++it) 
	{
//		if ((*it).getPresenceState() != EnumPresenceState::PresenceStateOffline )
		if ( !(*it).isOffline() )	//VOXOX - JRT - 2009.09.10 
		{
			if ( (*it).isIMAccountVoxOx() ) 
			{
				result = *it;
				break;
			}
		}
	}

	return result;
}

std::string IMContactSet::getFirstWengoId() const 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	std::string result = "";

	for ( const_iterator it = begin(); it != end(); ++it) 
	{
		if ((*it).isIMAccountVoxOx()) 
		{
			result = (*it).getContactId();
			break;
		}
	}

	return result;
}

bool IMContactSet::hasAvailableSIPNumberAndNoWengoId() const 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	bool hasSIP = false;
	
	for ( const_iterator it = begin(); it != end(); ++it) 
	{
		if (((*it).getProtocol() == EnumIMProtocol::IMProtocolWengo)) 
		{
			hasSIP = false;
			break;				//VOXOX - JRT - 2009.07.11 - Is this break correct?
		} 
		else if (((*it).getProtocol() == EnumIMProtocol::IMProtocolSIPSIMPLE) ||
			     ((*it).getProtocol() == EnumIMProtocol::IMProtocolSIP)) 
		{
			hasSIP = true;
		}
	}
	
	return hasSIP;
}

std::string IMContactSet::getFirstSkypeContactId() const 
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	std::string	result = "";

	for ( const_iterator it = begin(); it != end(); ++it) 
	{
//		if ((*it).getPresenceState() != EnumPresenceState::PresenceStateOffline )
		if ( !(*it).isOffline() )	//VOXOX - JRT - 2009.09.10 
		{
			if ( (*it).getProtocol() == EnumIMProtocol::IMProtocolSkype) 
			{
				result = (*it).getContactId();
				break;
			}
		}
	}

	return result;
}

IMContact* IMContactSet::getFirst() const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	IMContact* result = NULL;

	for (IMContactSet::const_iterator it = begin(); it != end(); it++) 
	{
		result = &(const_cast<IMContact&>(*it));
		break;
	}
		
	return result;
}

IMContact* IMContactSet::findByContact( const IMContact& imContact ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	IMContact* result = NULL;

	for (IMContactSet::const_iterator it = begin(); it != end(); it++) 
	{
		if ( (*it) == imContact )
		{
			result = &(const_cast<IMContact&>(*it));
			break;
		}
	}
		
	return result;
}

IMContact*	IMContactSet::findBy( const std::string& accountId, const std::string& contactId, bool tryUserOnly) const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	IMContact* result = NULL;

	for (IMContactSet::const_iterator it = begin(); it != end(); it++) 
	{
		if ( (*it).getIMAccountId() == accountId  )
		{
			if ( (*it).getContactId() == contactId ) 
			{
				result = &(const_cast<IMContact&>(*it));
				break;
			}
			else
			{
				if ( tryUserOnly )
				{
					 //VOXOX - JRT - 2009.06.16 - TODO: we need a JID class!  so we don't have to do this.
					std::string::size_type index = contactId.find('@');

					if (index != std::string::npos) 
					{
						std::string temp = contactId.substr(0, index);

						if ( (*it).getContactId() == temp )
						{
							result = &(const_cast<IMContact&>(*it));
							break;
						}
					} 
				}
			}
		}
	}
		
	return result;
}

IMContact*	IMContactSet::findByKey( const std::string& key ) const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	IMContact* result = NULL;

	for (IMContactSet::const_iterator it = begin(); it != end(); it++) 
	{
		if ( (*it).getKey() == key) 
		{
			result = &(const_cast<IMContact&>(*it));
			break;
		}
	}
		
	return result;
}

EnumIMProtocol::IMProtocol IMContactSet::getIMProtocol()  const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	EnumIMProtocol::IMProtocol eProtocol = EnumIMProtocol::IMProtocolUnknown;

	for (IMContactSet::const_iterator it = begin(); it != end(); it++) 
	{
		eProtocol = ((*it).getProtocol());
		break;
	}

	return eProtocol;
}


QtEnumIMProtocol::IMProtocol IMContactSet::getQtIMProtocol() const
{
	RecursiveMutex::ScopedLock lock(_mutex);	//VOXOX - JRT - 2009.07.11 

	QtEnumIMProtocol::IMProtocol eProtocol = QtEnumIMProtocol::IMProtocolUnknown;

	for (IMContactSet::const_iterator it = begin(); it != end(); it++) 
	{
		eProtocol = ((*it).getQtProtocol());
		break;
	}

	return eProtocol;
}

string IMContactSet::serialize() 
{
	string result;

	for (const_iterator it = begin() ; it != end() ; ++it)
	{
		//result += ((IMContact &)(*it)).serialize();
	}

	return result;
}
	
bool IMContactSet::unserialize(const string & data) 
{
	return false;
}

//IMContact * IMContactSet::getPreferredIMContact() const 
//{
//	const int offlineScore = getStateScore(EnumPresenceState::PresenceStateOffline);
//
//	IMContact * bestIMContact	 = NULL;
//	IMContact * bestWengoContact = NULL;
//	int bestIMContactScore		 = -1;
//	int bestWengoContactScore	 = -1;
//
//	// Find best IM contact and best Wengo contact
//	IMContactSet::const_iterator it = begin(), end = end();
//
//	Config & config = ConfigManager::getInstance().getCurrentConfig();
//	bool ignorePresence = config.getSipChatWithoutPresence();
//
//	for (; it!=end; ++it) 
//	{
//		EnumPresenceState::PresenceState state = it->getPresenceState();
//		int contactScore = getStateScore(state);
//
//		if (!ignorePresence && (contactScore <= offlineScore))
//		  continue;
//
//
//		IMContact* contact = const_cast<IMContact*>( &(*it) );
//
//		if (contact->getProtocol() == EnumIMProtocol::IMProtocolWengo) 
//		{
//			if (contactScore > bestWengoContactScore) 
//			{
//				bestWengoContact = contact;
//				bestWengoContactScore = contactScore;
//			}
//		} 
//		else 
//		{
//			if (contactScore > bestIMContactScore) 
//			{
//				bestIMContact = contact;
//				bestIMContactScore = contactScore;
//			}
//		}
//	}
//
//	// Get score of _preferredIMContact, if any
//	int preferredIMContactScore;
//	if (_preferredIMContact) {
//		preferredIMContactScore = getStateScore(_preferredIMContact->getPresenceState());
//	} else {
//		preferredIMContactScore = -1;
//	}
//
//	// Choose the best one
//	IMContact* bestContact = 0;
//	if (bestWengoContactScore >= bestIMContactScore) {
//		if (preferredIMContactScore >= bestWengoContactScore) {
//			bestContact =  _preferredIMContact;
//		} else {
//			bestContact = bestWengoContact;
//		}
//	} else {
//		if (preferredIMContactScore >= bestIMContactScore) {
//			bestContact = _preferredIMContact;
//		} else {
//			bestContact = bestIMContact;
//		}
//	}
//	////
//
//	return bestContact;
//}
