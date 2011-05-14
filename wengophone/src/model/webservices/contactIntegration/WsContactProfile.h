/*
 * VoxOx
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

#ifndef VOX_WS_CONTACT_PROFILE_H
#define VOX_WS_CONTACT_PROFILE_H

#include <model/webservices/WengoWebService.h>
#include <serializer/VoxQueryResultXMLSerializer.h>
#include "../../profile/VoxListTemplate.h"	

#include <thread/RecursiveMutex.h>
#include <thread/Thread.h>

#include <util/VoxApiTimer.h>

/**
 * Sends info to webservices about user and contact Profiles.
 */

class UserProfile;
class ContactProfile;
class ProfileData;
class QueryResult;

class WsContactProfileThread;

class TiXmlNode;
class TiXmlHandle;

//=============================================================================

class ProfileRequest
{
public:
	ProfileRequest();

	virtual ~ProfileRequest();

	ProfileRequest& operator= ( const ProfileRequest& src );
	bool			operator==( const ProfileRequest& src );

	//Gets -----------------------------------------------------------------
	QueryResult::EnumMethod getMethod()			const			{ return _eMethod;			}
	std::string				getContactKey()		const			{ return _contactKey;		}
	std::string				getXml()			const			{ return _xml;				}
	std::string				getAvatarCheckSum()	const			{ return _avatarChecksum;	}
	int						getUserNetworkId()	const			{ return _userNetworkId;	}
	int						getTimestamp()		const			{ return _timestamp;		}

	//Sets -----------------------------------------------------------------
	void setMethod		  ( QueryResult::EnumMethod val )		{ _eMethod			= val;	}
	void setContactKey	  ( const std::string&		val	)		{ _contactKey		= val;	}
	void setXml			  ( const std::string&		val	)		{ _xml				= val;	}
	void setAvatarChecksum( const std::string&		val )		{ _avatarChecksum	= val;	}
	void setUserNetworkId ( int						val )		{ _userNetworkId	= val;	}
	void setTimestamp	  ( int						val )		{ _timestamp		= val;	}


protected:
	void initVars();

private:
	QueryResult::EnumMethod _eMethod;
	std::string				_contactKey;
	std::string				_xml;
	std::string				_avatarChecksum;
	int						_userNetworkId;
	int						_timestamp;

};

//=============================================================================


//=============================================================================

class ProfileRequestList : public VoxListTemplate<ProfileRequest>
{
public:
	void addSetContactProfile( const std::string& contactKey, const std::string& xml );
	void addGetContactProfile( const std::string& contactKey, int userNetworkId, int timeStamp );

	void addSetUserProfile   ( const std::string& contactKey, const std::string& xml );
	void addGetUserProfile   ( int timeStamp, const std::string& avatarChecksum );

	ProfileRequest	getFirst();
	void			removeFirst();

};


//=============================================================================


//=============================================================================

class WsContactProfile : public WengoWebService 
{
public:
	WsContactProfile( UserProfile& userProfile);

	virtual ~WsContactProfile();

	/**
	 * Event WengoPhone should be updated.
	 *
	 * @param sender this class
	 * @param Contact Numer
	 */
	Event<void (WsContactProfile& sender, int id, const std::string& contactId, int userNetworkId, int timestamp )> wsContactProfileSetEvent;
	Event<void (WsContactProfile& sender, int id, ContactProfile& contactProfile)>								    wsContactProfileGetEvent;

	Event<void (WsContactProfile& sender, int id, int timestamp )			> wsUserProfileSetEvent;
	Event<void (WsContactProfile& sender, int id, ProfileData& userProfile) > wsUserProfileGetEvent;

	/**
	 * Sets profile on server
	 */
	void setContactProfile( const ContactProfile& contactProfile);
	void getContactProfile( const std::string& contactKey, int userNetworkId, int timeStamp );

	void setUserProfile( const UserProfile& userProfile );
	void getUserProfile( int timeStamp, const std::string& avatarChecksum );

	void executeRequest( ProfileRequest& pr );

protected:
	void setContactProfilePrivate( const std::string& contactKey, const std::string& contactXml ); 
	void getContactProfilePrivate( const std::string& contactKey, int userNetworkId, int timeStamp );

	void setUserProfilePrivate	 ( const std::string& contactXml); 
	void getUserProfilePrivate	 ( int timeStamp, const std::string& avatarChecksum );

private:
	void		execute( const std::string& method );
	void		setContactProfile( const std::string& contactKey, const std::string& xml );
	void		setUserProfile   ( const std::string& contactKey, const std::string& xml );

	void		answerReceived( const std::string& answer, int id);
	void		fromXml		  ( const std::string& xml, ContactProfile& contactProfile, ProfileData& userProfile );
	void		fromXmlGeneric( TiXmlNode * syncNode, QueryResult::EnumMethod eMethod );
	void		profileFromXml( TiXmlNode* profileNode, ContactProfile& contactProfile, ProfileData& userProfile, QueryResult::EnumMethod eMethod );
	void		photoFromXml  ( TiXmlNode* profileNode, ContactProfile& contactProfile, ProfileData& userProfile, QueryResult::EnumMethod eMethod );

	std::string toXml		  ( const ContactProfile& profile, const std::string& method );
	std::string toXml		  ( const UserProfile&    profile, const std::string& method );
	TiXmlNode*	findSyncNode( TiXmlHandle docHandle );

	std::string getSecurityKey();
	std::string initParameters( const std::string& method );
	bool		applyContactProfile( const ContactProfile& profile );	//VOXOX - JRT - 2009.07.22 

//	RecursiveMutex			_mutex;
	UserProfile&			_userProfile;
	QueryResult				_queryResult;

	ProfileRequestList		_reqList;
	WsContactProfileThread*	_thread;

	static std::string _saveUserProfileMethod;
	static std::string _getUserProfileMethod;
	static std::string _saveContactProfileMethod;
	static std::string _getContactProfileMethod;



	VoxApiTimer*			_apiTimer;
};

//=============================================================================


//=============================================================================

class WsContactProfileThread : public Thread 
{
public:
	WsContactProfileThread( WsContactProfile& wsContactProfile, ProfileRequestList& reqList );
	
	virtual ~WsContactProfileThread();

	void init();		//Starts the thread
		
	void removeCurrentProfileRequest();

protected:
	void run();		//Run the thread.  Internal method, this should not be used from outside.
	bool canProcess();

private:
	bool				_started;
	bool				_reqActive;

	int					_startCount;
	int					_endCount;

	WsContactProfile&	_wsContactProfile;
	ProfileRequestList& _reqList;
};

//=============================================================================

#endif //VOX_WS_CONTACT_PROFILE_H
