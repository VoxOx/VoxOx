/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef PURPLEIMCONTACTLIST_H
#define PURPLEIMCONTACTLIST_H

#include <string>

#include <imwrapper/IMContactList.h>
//#include <imwrapper/IMContact.h>	//VOXOX - JRT - 2009.05.20 - Hold off for now.
#include <thread/Mutex.h>

extern "C" 
{
#include <libpurple/account.h>
}

class  PurpleIMContactList;	//Fwd declaration

//=============================================================================
//VOXOX - JRT - 2009.08.21 
class PurpleIMContactListCallbackData
{
public:
	PurpleIMContactListCallbackData()		{ initVars();	}

	PurpleIMContactList* getInstance()		const		{ return _instance;		}
	int					 getCallbackId()	const		{ return _callbackId;	}
	std::string			 getContactId()		const		{ return _contactId;	}
	std::string			 getGroupName()		const		{ return _groupName;	}
	std::string			 getOldGroupName()  const		{ return _oldGroupName;	}
	std::string			 getNewGroupName()  const		{ return _newGroupName;	}

	void setInstance	 ( PurpleIMContactList* val )	{ _instance		= val;	}
	void setCallbackId	 ( int				     val )	{ _callbackId	= val;	}
	void setContactId	 ( const std::string&   val )	{ _contactId	= val;	}
	void setGroupName	 ( const std::string&   val )	{ _groupName	= val;	}
	void setOldGroupName ( const std::string&   val )	{ _oldGroupName = val;	}
	void setNewGroupName ( const std::string&   val )	{ _newGroupName = val;	}

protected:
	void initVars()
	{
		setInstance		( NULL );
		setCallbackId	(  0 );
		setContactId	( "" );
		setGroupName	( "" );
		setOldGroupName ( "" );
		setNewGroupName ( "" );
	}

private:
	PurpleIMContactList* _instance;
	int					 _callbackId;
	std::string			 _contactId;
	std::string			 _groupName;
	std::string			 _oldGroupName;
	std::string			 _newGroupName;
};

//=============================================================================

class PurpleIMContactList : public IMContactList
{
	friend class PurpleIMFactory;

public:
	PurpleIMContactList(IMAccount imAccount);

	virtual void addContact   (const std::string & groupName, const std::string & contactId);
	virtual void removeContact(const std::string & groupName, const std::string & contactId);

	virtual void addGroup     (const std::string & groupName);
	virtual void removeGroup  (const std::string & groupName);

	virtual void changeGroupName   (const std::string & oldGroupName, const std::string & newGroupName);
	virtual void moveContactToGroup(const std::string & newGroupName, const std::string & oldGroupName, const std::string & contactId);

//	virtual void getContactVcard( const std::string& contactId );	//VOXOX - JRT - 2009.05.18 - Hold off

	const IMAccount & getIMAccount() const { return _imAccount; }



	static bool addContactCbk		 (void * data);
	static bool removeContactCbk	 (void * data);
	static bool addGroupCbk			 (void * data);
	static bool removeGroupCbk		 (void * data);
	static bool changeGroupNameCbk	 (void * data);
	static bool moveContactToGroupCbk(void * data);
//	static bool getContactVcardCbk(void * data);			//VOXOX - JRT - 2009.05.18 - Hold off

	//VOXOX - JRT - 2009.08.21 - These should be in a base class
	static PurpleAccount* getPurpleAccount( PurpleIMContactList* imContactList );
		   int			  timeoutAdd( void* cbData, GSourceFunc callbackFunc );
	static void			  timeoutRemove( PurpleIMContactListCallbackData* cbData );


	static Mutex _mutex;
	static Mutex _mutexContact;	//VOXOX - JRT - 2009.07.14 - avoid deadlock.
};

#endif //PURPLEIMCONTACTLIST_H
