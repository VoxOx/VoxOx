/*
	VOXOX!!!!

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

#ifndef CONTACT_PROFILE_XML_SERIALIZER_H
#define CONTACT_PROFILE_XML_SERIALIZER_H

#include <serializer/VoxXMLSerializerBase.h>
#include <string>

class Profile;
//class UserProfile;
class ProfileData;
class ContactProfile;
class TiXmlHandle;

/**
 * Serialize a ContactProfile and UserProfile objects.
 *
 * @author Jeff Theinert
 */
class ProfileXMLSerializer2 : public XMLSerializerBase
{
public:
	ProfileXMLSerializer2( Profile& profile );

	virtual std::string serialize();

	bool unserialize( TiXmlHandle& rootElt   );
	bool unserialize( const std::string& xml );

	void unserializeAvatar2(TiXmlHandle & avatarElt );	//VOXOX - JRT - 2009.08.14 - Public because server returns in XML node outside <PROFILE>

	void setMethod( const std::string& method  )		{ _method = method;	}

protected:
	std::string	getMethod() const						{ return _method;	}

	//Pure virtual
	virtual std::string serializeIdentity()							= 0;
	virtual void		unserializeIdentity( TiXmlHandle& rootElt ) = 0;

	std::string serializeName();
	std::string serializeAddress();
	std::string serializeAvatar();
	std::string serializeEmail();
	std::string serializePhone();
	std::string serializeWebSite();
	std::string serializeWorkPlace();
//	std::string serializeMergedContacts();

	void		unserializeAvatar		 ( TiXmlHandle & rootElt );
	void		unserializeName		     ( TiXmlHandle & rootElt );
	void		unserializeAddress		 ( TiXmlHandle & rootElt );
	void		unserializeEmail		 ( TiXmlHandle & rootElt );
	void		unserializePhone		 ( TiXmlHandle & rootElt );
	void		unserializeWebSite		 ( TiXmlHandle & rootElt );
	void		unserializeWorkPlace	 ( TiXmlHandle & rootElt );
//	void		unserializeMergedContacts( TiXmlHandle & rootElt );

	Profile&	_profile;
	std::string	_method;
};

//=============================================================================

class ContactProfileXMLSerializer2 : public ProfileXMLSerializer2
{
public:
	ContactProfileXMLSerializer2( ContactProfile& profile );

protected:
	virtual std::string serializeIdentity();
	virtual void        unserializeIdentity( TiXmlHandle& rootElt );

	ContactProfile& _profile;
};

//=============================================================================

class UserProfileXMLSerializer2 : public ProfileXMLSerializer2
{
public:
	UserProfileXMLSerializer2( ProfileData& profile );

protected:
	virtual std::string serializeIdentity();
	virtual void        unserializeIdentity( TiXmlHandle& rootElt );

	ProfileData& _profile;
};

#endif //CONTACT_PROFILE_XML_SERIALIZER_H
