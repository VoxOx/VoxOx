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

#include <imwrapper/IMContact.h>
#include <imwrapper/IMAccount.h>

#include <util/String.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

IMContact::IMContact() 
{
	initVars();
	updateKey();
}

IMContact::IMContact(const IMAccount & imAccount, const std::string & contactId) 
{
	initVars();

	_imAccount = imAccount.clone();
	_contactId = contactId;
	_protocol  = imAccount.getProtocol();	//TODO: redundant?

	updateKey();
}

IMContact::IMContact(EnumIMProtocol::IMProtocol protocol, const std::string & contactId) 
{
	initVars();

	_contactId = contactId;
	_protocol  = protocol;

	updateKey();			//VOXOX - JRT - 2009.04.30 
}

IMContact::IMContact(const IMContact & imContact)
	: Trackable() 
{
	copy(imContact);
}

void IMContact::initVars()
{
	_imAccount		  = NULL;
	_presenceState	  = EnumPresenceState::PresenceStateUnknown;
	_protocol		  = EnumIMProtocol::IMProtocolUnknown;
	_blocked		  = false;
	_cleanContactId	  = "";

	_serverProtocolId = QtEnumIMProtocol::ServerProtocolIdNone;
	_userNetworkId    = 0;

	cleanContactId();
}

IMContact & IMContact::operator = (const IMContact & imContact) 
{
	OWSAFE_DELETE(_imAccount);

	copy(imContact);

	return *this;
}

void IMContact::copy(const IMContact & imContact) 
{
	if (imContact._imAccount) 
	{
		_imAccount = imContact._imAccount->clone();
	} 
	else 
	{
		_imAccount = NULL;
	}

	_contactId		 = imContact._contactId;
	_presenceState	 = imContact._presenceState;
	_protocol		 = imContact._protocol;
	_icon			 = imContact._icon;
	_alias			 = imContact._alias;
	_statusMessage	 = imContact._statusMessage;			//VOXOX CHANGE STATUS MESSAGE SUPPORT
	_blocked		 = imContact._blocked;				//VOXOX CHANGE Marin Block option when right clicking contact 4/24/2009 
	_cleanContactId  = imContact._cleanContactId;

	_serverProtocolId = imContact._serverProtocolId;
	_userNetworkId    = imContact._userNetworkId;

	cleanContactId();
	updateKey();

	imContactChangedEvent += imContact.imContactChangedEvent;	//VOXOX - JRT - 2009.04.27 
}

IMContact::~IMContact() 
{
	OWSAFE_DELETE(_imAccount);
}

bool IMContact::operator == (const IMContact & imContact) const 
{
	bool result = false;

	if (_imAccount && imContact._imAccount) 
	{
		if ((*_imAccount) == (*(imContact._imAccount))) 
		{
			if ( getKey() == imContact.getKey()) 		//VOXOX - JRT - 2009.04.30 
			{
				result = true;
			}
		}
	} 
	else 
	{
		if (getProtocol() == imContact.getProtocol()) 
		{
			if (getKey() == imContact.getKey()) 
			{
				result = true;
			}
		}
	}

	return result;
}

bool IMContact::operator < (const IMContact & imContact) const {
	bool result = false;

	if ( getCleanContactId() < imContact.getCleanContactId() ) {
		result = true;
	} 
	else 
	{
		if ( getCleanContactId() == imContact.getCleanContactId()) 
		{
			if (_imAccount && imContact._imAccount) 
			{
				if (_imAccount->getKey() < imContact._imAccount->getKey() )
				{
					result = true;
				}
			} 
			else 
			{
				if (getProtocol() < imContact.getProtocol()) 
				{
					result = true;
				}
			}
		}
	}

	return result;
}

std::string IMContact::cleanContactId() 	//VOXOX - JRT - 2009.04.10  - Removed const so we can set _cleanContactID.
{
	//VOXOX - JRT - 2009.04.10
	//TODO - cleanContactId() is used to as part of operator==
	//		 we should NOT be using getDisplayContactId() which drops the domain.
	//		 It is very likely safe, but not a good idea.
	//		 DisplayName should be considered totally separate from ContactId.
	std::string result;

	if ( _cleanContactId == "" )
	{
		if (_protocol == EnumIMProtocol::IMProtocolWengo) 
		{
			result = getDisplayContactId();
		}
		else
		{
			std::string::size_type index = _contactId.find('/');

			if (index != std::string::npos) 
			{
				result = _contactId.substr(0, index);
			} 
			else 
			{
				result = _contactId;
			}
		}

		_cleanContactId = result;
	}
	else
	{
		result = _cleanContactId;
	}

	return result;
}

std::string IMContact::getIMAccountId() const {
	std::string result;

	if (_imAccount) 
	{
		result = _imAccount->getKey();
	}

	return result;
}

void IMContact::setIMAccount(const IMAccount * imAccount) {
	OWSAFE_DELETE(_imAccount);

	if (imAccount) 
	{
		_imAccount = imAccount->clone();
		_protocol = imAccount->getProtocol();
		updateKey();
	} 
	else 
	{
		_imAccount = NULL;
	}
}

void IMContact::setContactId(const std::string & contactId) {
	_contactId = contactId;

	_cleanContactId = "";
	cleanContactId();
	updateKey();
}

void IMContact::setPresenceState(EnumPresenceState::PresenceState presenceState) {
	if (_presenceState == presenceState) {
		return;
	}
	_presenceState = presenceState;
	imContactChangedEvent(*this);
}

void IMContact::setAlias(const std::string& alias) {
	if (_alias == alias) {
		return;
	//VOXOX - CJC - 2009.05.04 Facebook is deleting statusmessageinfo and alias info after it goes online, maintain it
	}
	else if((getProtocol() == EnumIMProtocol::IMProtocolFacebook) && (_alias!="" || _alias!=String::null) && (alias=="" || _alias==String::null))
	{
		return;
	}
	_alias = alias;
	imContactChangedEvent(*this);
}

//VOXOX CHANGE CJC SUPPORT STATUS MESSAGE
void IMContact::setStatusMessage(const std::string & statusMessage) {
	if (_statusMessage == statusMessage) {
		return;
		//VOXOX - CJC - 2009.05.04 Facebook is deleting statusmessageinfo and alias info after it goes online, maintain it
	}else if((getProtocol() == EnumIMProtocol::IMProtocolFacebook) && (_statusMessage!="" || _statusMessage!=String::null) && (statusMessage=="" || statusMessage==String::null)){
		return;
	}
	_statusMessage = statusMessage;
	imContactChangedEvent(*this);
}

bool IMContact::isIMAccountVoxOx() const {
	if(_imAccount){
	return _imAccount->isVoxOxAccount();
	}
	return false;
}

bool IMContact::isIMAccountGtalk() const {

	if(_imAccount){
		return _imAccount->isGtalkAccount();
	}
	return false;
}

//VOXOX - JRT - 2009.04.24 
bool IMContact::isValid() const
{
	bool bValid = false;

	if ( ( getPresenceState() != EnumPresenceState::PresenceStateUnknown ) ||
		 ( getPresenceState() != EnumPresenceState::PresenceStateOffline ) )
	{
			bValid = true;
	}

	return bValid;
}
//End VoxOx

void IMContact::setIcon(const OWPicture& icon) {
	_icon = icon;
	imContactChangedEvent(*this);
}

std::string IMContact::getDisplayContactId() const {
	//VOXOX - JRT - 2009.04.10 - TODO- This is called on every operator==
	//			Let's just set in the the new _cleanContactId memvar
	//VOXOX CHANGE CJC IF ITS VOXOX ACCOUNT REMOVE EXTENTION
	if (isIMAccountVoxOx()) {
		size_t posFirstOf = _contactId.find("@");
		if (posFirstOf != _contactId.npos) {
			return _contactId.substr(0, posFirstOf);
		}
	}

	return _contactId;
}

void IMContact::updateKey()
{
	_key = _contactId + EnumIMProtocol::toString( getProtocol() );

	if ( _imAccount )
	{
		_key += _imAccount->getKey();	//VOXOX - JRT - 2009.05.29 
	}
}

std::string IMContact::getKey() const
{
	assert( !_key.empty() );
	return _key;
}

QtEnumIMProtocol::IMProtocol IMContact::getQtProtocol()	const	//VOXOX - JRT - 2009.05.29 
{
	QtEnumIMProtocol::IMProtocol qtProtocol = QtEnumIMProtocol::IMProtocolUnknown;

	if ( _imAccount )
	{
		qtProtocol = _imAccount->getQtProtocol();
	}

	return qtProtocol;
}

void IMContact::setServerProtocolId( QtEnumIMProtocol::ServerProtocolId val )
{ 
	_serverProtocolId = val;

	if ( _imAccount )
	{
		const_cast<IMAccount*>(_imAccount)->setServerProtocolId( val );	
	}
}

void IMContact::setUserNetworkId( int val )
{ 
	_userNetworkId = val;

	if ( _imAccount )
	{
		const_cast<IMAccount*>(_imAccount)->setUserNetworkId( val );	
	}
}

int	IMContact::getUserNetworkId() const
{
	int result = _userNetworkId;

	if ( _imAccount )
	{
		if ( _imAccount->getUserNetworkId() > 0 )
		{
			result = _imAccount->getUserNetworkId();
		}
	}

	return result;
}


QtEnumIMProtocol::ServerProtocolId IMContact::getServerProtocolId()	const	//VOXOX - JRT - 2009.05.29 
{
	QtEnumIMProtocol::ServerProtocolId result = _serverProtocolId;

	if ( _imAccount )
	{
		if ( _imAccount->getServerProtocolId() != QtEnumIMProtocol::ServerProtocolIdNone )
		{
			result = _imAccount->getServerProtocolId();
		}
	}

	return result;
}
