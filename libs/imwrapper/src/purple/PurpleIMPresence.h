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

#ifndef PURPLEIMPRESENCE_H
#define PURPLEIMPRESENCE_H

#include <imwrapper/IMPresence.h>
#include <thread/Mutex.h>

#include "PurpleEnumIMProtocol.h"

extern "C" 
{
#include <libpurple/account.h>
}

typedef struct AuthRequest_s AuthRequest;

class PurpleIMPresence;

//=============================================================================

class PurpleIMPresenceCallbackData
{
public:
	PurpleIMPresenceCallbackData()			{ initVars();	}

	PurpleIMPresence*				 getPurpleIMPresence()	const		{ return _imPresence;	}
	int								 getCallbackId()		const		{ return _callbackId;	}
	EnumPresenceState::PresenceState getPresenceState()		const		{ return _presenceState;}
	std::string						 getNote()				const		{ return _note;			}
	std::string						 getNickname()			const		{ return _nickname;		}
	std::string						 getIconPath()			const		{ return _iconPath;		}
	std::string						 getContactId()			const		{ return _contactId;	}
	bool							 getAuthorized()		const		{ return _authorized;	}

	void setPurpleIMPresence( PurpleIMPresence* val )					{ _imPresence	 = val;	}
	void setCallbackId		( int				val )					{ _callbackId	 = val;	}
	void setPresenceState   ( EnumPresenceState::PresenceState val )	{ _presenceState = val;	}
	void setNote			( const std::string&			   val )	{ _note			 = val; }
	void setNickname		( const std::string&			   val )	{ _nickname		 = val; }
	void setIconPath		( const std::string&			   val )	{ _iconPath		 = val; }
	void setContactId		( const std::string&			   val )	{ _contactId	 = val; }
	void setAuthorized		( bool							   val )	{ _authorized	 = val;	}

protected:
	void initVars()
	{
		setPurpleIMPresence( NULL );
		setCallbackId	   ( 0 );
		setPresenceState   ( EnumPresenceState::PresenceStateOnline );	//VOXOX - JRT - 2009.09.08 - TODO: should this be Unknown?
		setNote			   ( "" );
		setNickname		   ( "" );
		setIconPath		   ( "" );
		setContactId	   ( "" );
		setAuthorized	   ( false );
	}

private:
	PurpleIMPresence*					_imPresence;
	int									_callbackId;
	EnumPresenceState::PresenceState	_presenceState;
	std::string							_note;
	std::string							_nickname;
	std::string							_iconPath;
	std::string							_contactId;
	bool								_authorized;
};

//=============================================================================

/**
 * Stub for Instant Messaging presence.
 *
 * @ingroup model
 * @author Julien Bossart
 */
class PurpleIMPresence : public IMPresence 
{
	friend class PurpleIMFactory;

public:

	PurpleIMPresence(IMAccount account);

	virtual ~PurpleIMPresence();

	virtual void changeMyPresence(EnumPresenceState::PresenceState state, const std::string & note);
	virtual void changeMyAlias(const std::string & nickname);
	virtual void changeMyIcon(const OWPicture & picture);
	virtual void subscribeToPresenceOf(const std::string & contactId);
	virtual void unsubscribeToPresenceOf(const std::string & contactId);
	virtual void blockContact(const std::string & contactId);
	virtual void unblockContact(const std::string & contactId);
	virtual void authorizeContact(const std::string & contactId, bool authorized, const std::string message);

	virtual void reRequestAuthorization(const std::string & contactId);

	static const char *getPurplePresenceId(void *purplePresence);
	//VOXOX CHANGE CJC GET BUDDYS STATUS MESSAGE
	static const char *getPurpleBuddyStatusMessage(void *purpleBuddy);

	void addAuthRequestInList(const std::string & contactId, void *acceptCbk, void *denyCbk, void *userData);

	const IMAccount & getIMAccount() const { return _imAccount; }

	static bool changeMyPresenceCbk(void * data);
	static bool changeMyAliasCbk(void * data);
	static bool changeMyIconCbk(void * data);
	static bool subscribeToPresenceOfCbk(void * data);
	static bool unsubscribeToPresenceOfCbk(void * data);
	static bool blockContactCbk(void * data);
	static bool unblockContactCbk(void * data);
	static bool authorizeContactCbk(void * data);

	static bool reRequestAuthorizationCbk(void * data);//VOXOX - CJC - 2009.07.31 

		//VOXOX - JRT - 2009.08.21 - These should be in a base class
	static PurpleAccount* getPurpleAccount( const IMAccount& imAccount );
		   int			  timeoutAdd( void* cbData, GSourceFunc callbackFunc );
	static void			  timeoutRemove( PurpleIMPresenceCallbackData* cbData );


	std::map<const std::string, AuthRequest *> authRequestMap;

	static Mutex _mutex;
};

#endif	//PURPLEIMPRESENCE_H
