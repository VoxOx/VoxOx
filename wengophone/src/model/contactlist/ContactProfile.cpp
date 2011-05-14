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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "ContactProfile.h"

#include "ContactPresenceStateUnknown.h"
#include "ContactPresenceStateOnline.h"
#include "ContactPresenceStateOffline.h"
#include "ContactPresenceStateAway.h"
#include "ContactPresenceStateDoNotDisturb.h"
#include "ContactPresenceStateUnavailable.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/AvatarList.h>

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMContact.h>
#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMChat.h>

#define LOGGER_COMPONENT "ContactList"
#include <util/Logger.h>
#include <util/OWPicture.h>
//#include <util/Uuid.h>		//VOXOX - JRT - 2009.04.21 - Not needed.

#include <iostream>

using namespace std;

ContactProfile::PresenceStates ContactProfile::_presenceStateMap;

static ContactPresenceStateUnknown contactPresenceStateUnknown;
static ContactPresenceStateOnline contactPresenceStateOnline;
static ContactPresenceStateOffline contactPresenceStateOffline;
static ContactPresenceStateAway contactPresenceStateAway;
static ContactPresenceStateDoNotDisturb contactPresenceStateDoNotDisturb;
static ContactPresenceStateUnavailable contactPresenceStateUnavailable;

ContactProfile::ContactProfile() {
//	_sex = EnumSex::SexUnknown;
	setSex( EnumSex::SexUnknown );	//VOXOX - JRT - 2009.05.22 
	_preferredIMContact = NULL;

//	_uuid = Uuid::generateString();	//VOXOX - JRT - 2009.04.21 - Let's leave this empty because it is our KEY and we need to set it explicitly.
	_uuid = "";

	//Default state (ContactPresenceStateUnknown)
	_presenceState = &contactPresenceStateUnknown;

	_presenceStateMap[contactPresenceStateUnknown.getCode()]		= &contactPresenceStateUnknown;
	_presenceStateMap[contactPresenceStateOnline.getCode()]			= &contactPresenceStateOnline;
	_presenceStateMap[contactPresenceStateOffline.getCode()]		= &contactPresenceStateOffline;
	_presenceStateMap[contactPresenceStateAway.getCode()]			= &contactPresenceStateAway;
	_presenceStateMap[contactPresenceStateDoNotDisturb.getCode()]	= &contactPresenceStateDoNotDisturb;
	_presenceStateMap[contactPresenceStateUnavailable.getCode()]	= &contactPresenceStateUnavailable;
}

ContactProfile::ContactProfile(const ContactProfile & contactProfile)
: Profile() {
	copy(contactProfile);
}

ContactProfile::~ContactProfile() {
}

ContactProfile & ContactProfile::operator=(const ContactProfile & contactProfile) {
	if (&contactProfile != this) {
		copy(contactProfile);
	}

	return *this;
}

void ContactProfile::copy(const ContactProfile & contactProfile) {
	Profile::copy(contactProfile);
//	_uuid = contactProfile._uuid;	//VOXOX - JRT - 2009.04.21 - let's not overwrite the UUID!
	if ( _uuid.empty() )
	{
		_uuid = contactProfile._uuid;	//VOXOX - JRT - 2009.04.21 
	}

	_preferredIMContact = contactProfile._preferredIMContact;
	_imContactSet		= contactProfile._imContactSet;

//	_groupId = contactProfile._groupId;
	_contactGroupInfoSet = const_cast<ContactProfile&>(contactProfile).getContactGroupInfoSet();	//VOXOX - JRT - 2009.05.06 JRT-GRPS
	_presenceState		 = contactProfile._presenceState;
	getGroups()			 = const_cast<ContactProfile&>(contactProfile).getGroups();					//VOXOX - JRT - 2009.05.10 	
}

bool ContactProfile::operator == (const ContactProfile & contactProfile) const {
	return (_uuid == contactProfile._uuid);
}

void ContactProfile::addIMContact(const IMContact & imContact) {
//	pair<IMContactSet::const_iterator, bool> result = _imContactSet.insert(imContact);
	_imContactSet.add( imContact );	//VOXOX - JRT - 2009.07.11 
	updatePresenceState();
}

void ContactProfile::removeIMContact(const IMContact & imContact) 
{
	//VOXOX - JRT - 2009.06.06 
	_imContactSet.Delete( imContact );

	updatePresenceState();
}

bool ContactProfile::hasIMContact(const IMContact & imContact) const {
	return (_imContactSet.size() > 0);
}

IMContact * ContactProfile::getIMContact(const IMContact & imContact) const {
	return _imContactSet.findByContact(imContact);	//VOXOX - JRT - 2009.06.06 
}

//void ContactProfile::setGroupId(const std::string & groupId) {	//VOXOX - JRT - 2009.05.06 JRT-GRPS
//	_groupId = groupId;
//	profileChangedEvent(*this);
//}

//VOXOX - JRT - 2009.05.06 JRT-GRPS
void ContactProfile::addToGroup( const ContactGroup& group )
{
	ContactGroupInfo info( group.getKey(), group.getType() );
	getContactGroupInfoSet().Add( info );
	profileChangedEvent(*this);
}

void ContactProfile::addToGroup( const std::string& key, EnumGroupType::GroupType type )	//TODO JRT-GRPS: parse type from key.
{
	ContactGroupInfo info( key, type );
	getContactGroupInfoSet().Add( info );
	profileChangedEvent(*this);
}

void ContactProfile::removeFromGroup( const ContactGroup& group )
{
	getContactGroupInfoSet().Delete( group.getKey() );	//Does locking
	profileChangedEvent(*this);
}

void ContactProfile::removeFromGroup( const std::string& key )
{
	getContactGroupInfoSet().Delete( key );		//Does locking
	profileChangedEvent(*this);
}

//VOXOX - JRT - 2009.08.09 - Not called
//void ContactProfile::changeUserGroup( const std::string& oldGroupId, const ContactGroup& group )	//VOXOX - JRT - 2009.08.09 - Multiple groups
//{
////	removeFromGroup( this->getFirstUserGroupId() );	
//	removeFromGroup( oldGroupId );		//VOXOX - JRT - 2009.08.09 - Multiple Groups.
//	addToGroup( group );
//}

bool ContactProfile::isInGroup( const std::string& key )
{
	return (getContactGroupInfoSet().FindByKey( key ) != NULL);		//Does locking
}

bool ContactProfile::isInUserGroup( const std::string& name )	//VOXOX - JRT - 2009.08.07 
{
	ContactGroup grp( name, EnumGroupType::GroupType_User );
	return (getContactGroupInfoSet().FindByKey( grp.getKey() ) != NULL);		//Does locking
}

int ContactProfile::getUserGroupCount()	 //VOXOX - JRT - 2009.09.03 
{
	return getContactGroupInfoSet().GetUserCount();
}

std::string ContactProfile::getFirstUserGroupId()	// const
{ 
	std::string groupId = getContactGroupInfoSet().GetFirstUserGroupKey();	//Does locking

	if ( groupId == "" )
	{
		groupId = getGroupsConst().getFirstUserGroupKey();
	}

	return groupId;
}

bool ContactProfile::hasIM() const 
{
	//VOXOX - JRT - 2009.07.21 
	return QtEnumIMProtocolMap::getInstance().hasIM( getQtIMProtocol() );

	//All accounts should support offline messages
//	return true;

	/*if(getIsIMAccountVoxox()){
		return true;
	}else if(getIMProtocol() == EnumIMProtocol::IMProtocolFacebook || getIMProtocol() == EnumIMProtocol::IMProtocolMYSPACE || 
		
	}
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	if(config.getSipChatWithoutPresence())
	{
	  String imContact = getFirstAvailableSIPNumber();
	  if(!imContact.empty())
	    return true;
	}

	return (getPresenceState() != EnumPresenceState::PresenceStateOffline &&
		getPresenceState() != EnumPresenceState::PresenceStateUnknown && 
		getPresenceState() != EnumPresenceState::PresenceStateUnavailable);*/
//}
}

bool ContactProfile::hasCall() {	//VOXOX - JRT - 2009.04.05 - Removed const
	if (!getPreferredNumber().empty()) {
		return true;
	} else {
		return false;
	}
}

bool ContactProfile::hasVideo() const {
	return hasAvailableWengoId();
}

bool ContactProfile::hasVoiceMail() const {
	// For now, we assume only Wengo accounts support voice mail
	return !getFirstWengoId().empty();
}

bool ContactProfile::hasFileTransfer() const 
{
	if (!getFirstWengoId().empty() && isAvailable()) 
	{
		IMContact imContact = getFirstAvailableWengoIMContact();

//		if ((imContact.getPresenceState() != EnumPresenceState::PresenceStateOffline) &&
//			(imContact.getPresenceState() != EnumPresenceState::PresenceStateUnknown) &&
//			(imContact.getPresenceState() != EnumPresenceState::PresenceStateUnavailable)) {
		if ( !EnumPresenceState::isOfflineEx( imContact.getPresenceState() ) )	//VOXOX - JRT - 2009.09.10 
		{
				return true;
		}
	}
	return false;
}

std::string ContactProfile::getPreferredNumber() {	//VOXOX - JRT - 2009.04.05 - Removed const
	string result;

	if (!_preferredNumber.empty()) {
		result = _preferredNumber;
	
//	//VOXOXCHANGE ADD VOXOX NUMBER
//	} else if (!_voxoxPhone.empty()) {
//		result = _voxoxPhone;
//	//VOXOXCHANGE REMOVE WENGOID
//	/*} else if (hasAvailableWengoId()) {
//			result = getFirstAvailableWengoId();*/
//	} else if (hasAvailableSIPNumber()) {
//			result = getFirstAvailableSIPNumber();
//	} else if (!_mobilePhone.empty()) {
//		result = _mobilePhone;
//	} else if (!_homePhone.empty()) {
//		result = _homePhone;
//	} else if (!_workPhone.empty()) {
//		result = _workPhone;
//	} else if (!_otherPhone.empty()) {
//		result = _otherPhone;
//	}

	//TODO: JRT - 2009.03.31 - move this logic to new Telephones class
	//VOXOX - JRT - 2009.03.31 - Use new Telephone class.
	//VOXOXCHANGE ADD VOXOX NUMBER
	} else if (!getVoxOxPhone().empty()) {
		result = getVoxOxPhone();
	//VOXOXCHANGE REMOVE WENGOID
	/*} else if (hasAvailableWengoId()) {
			result = getFirstAvailableWengoId();*/
	/*} else if (hasAvailableSIPNumber()) {
			result = getFirstAvailableSIPNumber();*/
	} else if (!getMobilePhone().empty()) {
		result = getMobilePhone();
	} else if (!getHomePhone().empty()) {
		result = getHomePhone();
	} else if (!getWorkPhone().empty()) {
		result = getWorkPhone();
	} else if (!getOtherPhone().empty()) {
		result = getOtherPhone();
	}
	//End VOXOX

	return result;
}
//VOXOX - CJC - 2009.06.12 
std::string ContactProfile::getPreferredEmail() {
	string result;
	//VOXOX - TODO: Get prefered email when method is implemented before checking if emails exists CJC - 2009.06.12 
	 if (!getPersonalEmail().empty()) {
		result = getPersonalEmail();
	} else if (!getWorkEmail().empty()) {
		result = getWorkEmail();
	} else if (!getOtherEmail().empty()) {
		result = getOtherEmail();
	} 

	return result;
}


/**
 * Helper to assign scores to presence states
 * FIXME: This should be moved in EnumPresenceState, as a getScore() method
 */
static int getStateScore(EnumPresenceState::PresenceState state) {
	typedef std::map<EnumPresenceState::PresenceState, int> StateScoreMap;
	static StateScoreMap stateScoreMap;

	if (stateScoreMap.empty()) {
		stateScoreMap[EnumPresenceState::PresenceStateOffline] = 1;
		stateScoreMap[EnumPresenceState::PresenceStateDoNotDisturb] = 2;
		stateScoreMap[EnumPresenceState::PresenceStateAway] = 3;
		stateScoreMap[EnumPresenceState::PresenceStateOnline] = 4;
	}

	StateScoreMap::const_iterator it = stateScoreMap.find(state);
	if (it == stateScoreMap.end()) {
		return 0;
	}
	return it->second;
};

//VOXOX - JRT - 2009.07.11 - TODO: move to IMContactSet for better thread syncing
IMContact * ContactProfile::getPreferredIMContact() const 
{
	const int offlineScore = getStateScore(EnumPresenceState::PresenceStateOffline);

	IMContact * bestIMContact	 = NULL;
	IMContact * bestWengoContact = NULL;
	int bestIMContactScore		 = -1;
	int bestWengoContactScore	 = -1;

	// Find best IM contact and best Wengo contact
	IMContactSet::const_iterator it = _imContactSet.begin(), end = _imContactSet.end();

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	bool ignorePresence = config.getSipChatWithoutPresence();

	for (; it!=end; ++it) 
	{
		EnumPresenceState::PresenceState state = it->getPresenceState();
		int contactScore = getStateScore(state);

		if (!ignorePresence && (contactScore <= offlineScore))
		  continue;


		IMContact* contact = const_cast<IMContact*>( &(*it) );

		if (contact->getProtocol() == EnumIMProtocol::IMProtocolWengo) 
		{
			if (contactScore > bestWengoContactScore) 
			{
				bestWengoContact = contact;
				bestWengoContactScore = contactScore;
			}
		} 
		else 
		{
			if (contactScore > bestIMContactScore) 
			{
				bestIMContact = contact;
				bestIMContactScore = contactScore;
			}
		}
	}

	// Get score of _preferredIMContact, if any
	int preferredIMContactScore;
	if (_preferredIMContact) {
		preferredIMContactScore = getStateScore(_preferredIMContact->getPresenceState());
	} else {
		preferredIMContactScore = -1;
	}

	// Choose the best one
	IMContact* bestContact = 0;
	if (bestWengoContactScore >= bestIMContactScore) {
		if (preferredIMContactScore >= bestWengoContactScore) {
			bestContact =  _preferredIMContact;
		} else {
			bestContact = bestWengoContact;
		}
	} else {
		if (preferredIMContactScore >= bestIMContactScore) {
			bestContact = _preferredIMContact;
		} else {
			bestContact = bestIMContact;
		}
	}
	////

	return bestContact;
}

EnumPresenceState::PresenceState ContactProfile::getPresenceState() const {
	return _presenceState->getCode();
}

//VOXOX - CJC - 2009.05.05 Get if contact is blocked
// VOXOX CHANGE by ASV 05-06-2009: fix for crash when deleting contact.
bool ContactProfile::isBlocked() {
	IMContact *imContact = getPreferredIMContact();
	if (imContact) {
		return imContact->isBlocked();
	} else {
		return false;
	}	
}

//VOXOX - JRT - 2009.07.11 - TODO: move to IMContactSet for better thread syncing
EnumPresenceState::PresenceState ContactProfile::computePresenceState() const {
	// Find best state
	EnumPresenceState::PresenceState bestState = EnumPresenceState::PresenceStateUnknown;
	int bestStateScore = 0;

	IMContactSet::const_iterator 
		it = _imContactSet.begin(),
		end = _imContactSet.end();

	for (; it!=end; ++it) {
		EnumPresenceState::PresenceState contactState = it->getPresenceState();
		int contactStateScore = getStateScore(contactState);

		if (contactStateScore > bestStateScore) {
			bestState = contactState;
			bestStateScore = contactStateScore;
		}
	}

	return bestState;
}

void ContactProfile::updatePresenceState() {
	EnumPresenceState::PresenceState presenceState = computePresenceState();

	LOG_DEBUG("PresenceState=" + String::fromNumber(presenceState));

	PresenceStates::iterator it = _presenceStateMap.find(presenceState);
	if (it == _presenceStateMap.end()) {
		LOG_FATAL("unknown PresenceState=" + String::fromNumber(presenceState));
	}

	ContactPresenceState * state = it->second;
	if (state->getCode() == presenceState) {
		if (_presenceState->getCode() != state->getCode()) {
			_presenceState = state;
			_presenceState->execute(*this);
			LOG_DEBUG("presence state changed=" + EnumPresenceState::toString(_presenceState->getCode()));
			profileChangedEvent(*this);
			return;
		}
	}
}

//VOXOX - JRT - 2009.07.11 - TODO: move to IMContactSet for better thread syncing
const OWPicture& ContactProfile::getIcon() const 
{
	for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); it++) 
	{
		const OWPicture& icon = it->getIcon();
		if (!icon.getData().empty()) 
		{
			return icon;
		}
	}

	return AvatarList::getInstance().getDefaultContactAvatarPicture(getQtIMProtocol());	//VOXOX - JRT - 2009.06.11 
}

//OWPicture ContactProfile::getIcon() const 
//{
//	OWPicture icon;
//	for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); it++) 
//	{
//		icon = it->getIcon();
//		if (!icon.getData().empty()) 
//		{
//			return icon;
//		}
//	}
//
//	return AvatarList::getInstance().getDefaultContactAvatarPicture(getQtIMProtocol());	//VOXOX - JRT - 2009.06.11 
//}

const std::string ContactProfile::getContactId() const{
	return const_cast<IMContactSet&>(_imContactSet).getFirstContactId();
}

const std::string ContactProfile::getKey() const 
{
	return const_cast<IMContactSet&>(_imContactSet).getFirstKey();
}


EnumIMProtocol::IMProtocol ContactProfile::getIMProtocol() const 
{
	return _imContactSet.getIMProtocol();
}

QtEnumIMProtocol::IMProtocol ContactProfile::getQtIMProtocol() const 
{
	return _imContactSet.getQtIMProtocol();
}

string ContactProfile::getDisplayName() const {
	string result;
	string pseudo;
	int find;
	string contactId;
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if (! getFirstName().empty() || !getLastName().empty()) 
	{
		result += getFirstName();

		if (!result.empty() && !getLastName().empty()) 
		{
			result += " ";
		}

		result += getLastName();
		return result;
	}

	// Take the alias of the first IMContact
	IMContact* imContact = _imContactSet.getFirstValidAlias();	//VOXOX - JRT - 2009.07.11 

	if ( imContact )
	{
		pseudo	  = imContact->getAlias();
		contactId = imContact->getDisplayContactId();
	}

	// If no alias set, we take the first contact id:
	if (contactId.empty()) 
	{
		contactId = _imContactSet.getFirstDisplayContactId();		//VOXOX - JRT - 2009.07.11 
	}

	if (result.empty()) 
	{
		result = contactId;
	}

	// FIXME: I don't know how this happens, but sometimes the pseudo is set to
	// "(null)"! Since I don't have time to investigate that bug (and don't
	// know how to reproduce it), I resort to this hack for now.
	// See ticket #1495
	if (!pseudo.empty() && pseudo != "(null)") {
		result = pseudo;
	}

	find = result.find("@"+config.getJabberVoxoxServer());
	if(find!=string::npos){
	//Remove VoxOx Jabber server from name
		result= result.substr(0,find);
	}

	return result;
}

std::string ContactProfile::getShortDisplayName() const 
{
	std::string toReturn = getFirstName();
	if (toReturn.empty()) 
	{
		toReturn = getLastName();
	}

	return std::string(toReturn);
}

std::string ContactProfile::getBestShortDisplayName()
{
	std::string result = getShortDisplayName();

	if ( result.empty() )
	{
		result = getDisplayName();
	}

	return result;

}

//VOXOX CHANGE CJC GET STATUS MESSAGE
string ContactProfile::getStatusMessage() const 
{
	string statusMessage = _imContactSet.getFirstStatusMessage();	//VOXOX - JRT - 2009.07.11 
	

	//// Take the alias of the first IMContact
	//for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) 
	//{
	//	if (!(*it).getStatusMessage().empty()) {
	//		statusMessage = (*it).getStatusMessage();
	//		break;
	//	}
	//}

	return statusMessage;
}

std::string ContactProfile::getContactSocialAddress() const {	//VOXOX - CJC - 2009.06.09 
	IMContact* contact = getPreferredIMContact();			
	EnumIMProtocol::IMProtocol imProtocol = EnumIMProtocol::IMProtocolUnknown;	

	if ( contact )
	{
		imProtocol = contact->getProtocol();	
	}

	std::string address   = "";
	std::string contactId = getPreferredIMContact()->getContactId();

	//Add tooltip to button for this contacts
	if(imProtocol == EnumIMProtocol::IMProtocolFacebook)
	{
		address = "http://www.facebook.com/profile.php?id=" + contactId;
	}
	else if(imProtocol == QtEnumIMProtocol::IMProtocolMYSPACE)
	{
		address = "http://www.myspace.com/"+ contactId;					
	}
	else if(imProtocol == QtEnumIMProtocol::IMProtocolTwitter)
	{
		address = "http://twitter.com/"+ contactId;
	}

	return address;

}

IMContact * ContactProfile::getFirstAvailableIMContact(IMChatSession & imChatSession) const 
{
	IMContact * result = _imContactSet.getFirstAvailable( imChatSession.getIMChat().getIMAccountId() );

	//for (IMContactSet::const_iterator it = _imContactSet.begin() ; it != _imContactSet.end() ; ++it) 
	//{
	//	if (((*it).getPresenceState() != EnumPresenceState::PresenceStateOffline)
	//		&& (((*it).getIMAccountId()) == imChatSession.getIMChat().getIMAccountId())) 
	//	{
	//		result = (IMContact *)&(*it);
	//		break;
	//	}
	//}

	return result;
}

std::string ContactProfile::getFirstAvailableSIPNumber() const 
{
	std::string result = _imContactSet.getFirstAvailableSIPNumber();

	//for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) 
	//{
	//	if ((*it).getPresenceState() != EnumPresenceState::PresenceStateOffline && (*it).isIMAccountVoxOx()) 
	//	{
	//		result = (*it).getContactId();
	//		break;
	//	}
	//}

	return result;
}

bool ContactProfile::hasAvailableSIPNumber() const {
	return (!getFirstAvailableSIPNumber().empty());
}

bool ContactProfile::hasPstnCall()
{
	return (hasCall() && !hasFreeCall() );
}

IMContact ContactProfile::getFirstAvailableWengoIMContact() const 
{
	IMContact result = _imContactSet.getFirstAvailableWengoIMContact();

	//for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) 
	//{
	//	if ((*it).getPresenceState() != EnumPresenceState::PresenceStateOffline && (*it).isIMAccountVoxOx()) 
	//	{
	//		result = *it;
	//		break;
	//	}
	//}

	return result;
}

std::string ContactProfile::getFirstAvailableWengoId() const {
	return getFirstAvailableWengoIMContact().getContactId();
}

std::string ContactProfile::getFirstWengoId() const {
	std::string result = _imContactSet.getFirstWengoId();

	//for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) 
	//{
	//	if ((*it).isIMAccountVoxOx()) {
	//		result = (*it).getContactId();
	//		break;
	//	}
	//}

	return result;
}

bool ContactProfile::hasAvailableWengoId() const 
{
	return (!getFirstAvailableWengoId().empty());
}

bool ContactProfile::hasAvailableSIPNumberAndNoWengoId() const {

	bool hasSIP = _imContactSet.hasAvailableSIPNumberAndNoWengoId();
	
	//for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) 
	//{
	//	if (((*it).getProtocol() == EnumIMProtocol::IMProtocolWengo)) {
	//		return false;
	//	} else if (((*it).getProtocol() == EnumIMProtocol::IMProtocolSIPSIMPLE) ||
	//		((*it).getProtocol() == EnumIMProtocol::IMProtocolSIP)) {
	//		hasSIP = true;
	//	}
	//}
	
	return hasSIP;
}

IMContact* ContactProfile::getFirstIMContact() const
{
	IMContact* imContact = _imContactSet.getFirst();

	//for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) 
	//{
	//	imContact = const_cast<IMContact*>(&(*it));	
	//	break;
	//}

	return imContact;
}

bool ContactProfile::getIsIMAccountVoxox() const 
{
	bool result = false;
	IMContact* imContact = getFirstIMContact();

	if ( imContact )
	{
		result = imContact->isIMAccountVoxOx();
	}

	return result;
}

bool ContactProfile::getIsIMAccountGtalk() const 
{
	bool result = false;
	IMContact* imContact = getFirstIMContact();

	if ( imContact )
	{
		result = imContact->isIMAccountGtalk();
	}

	return result;
}

bool ContactProfile::isValid() const	//VOXOX - JRT - 2009.06.05 
{
	bool bValid = (getPreferredIMContact() != NULL);

	return bValid;
}


std::string ContactProfile::getFirstFreePhoneNumber() const 
{
	std::string result;

	if (hasAvailableWengoId()) 
	{
		result = getFirstAvailableWengoId();
	} 
	else if (hasAvailableSIPNumber()) 
	{
		result = getFirstAvailableSIPNumber();
	}
	else
	{
		result = _imContactSet.getFirstSkypeContactId();
		//VOXOX CHANGE ADD CALL SKYPE SUPPORT 
		//for (IMContactSet::const_iterator it = _imContactSet.begin(); it != _imContactSet.end(); ++it) 
		//{
		//	if ((*it).getPresenceState() != EnumPresenceState::PresenceStateOffline && (*it).getProtocol() == EnumIMProtocol::IMProtocolSkype) 
		//	{
		//		result = (*it).getContactId();
		//		break;
		//	}
		//}
	}

	return result;
}

std::string ContactProfile::getFirstVoiceMailNumber() const {
	return getFirstWengoId();
}

bool ContactProfile::hasFreeCall(){		//JRT - 2009.04.05 - Removed const
	return (!getFirstFreePhoneNumber().empty());
}

void ContactProfile::setWengoPhoneId(const std::string & wengoPhoneId) {
	Profile::setWengoPhoneId(wengoPhoneId);
	addIMContact(IMContact(EnumIMProtocol::IMProtocolWengo, wengoPhoneId));
}

bool ContactProfile::isAvailable() const {
	return ((getPresenceState() != EnumPresenceState::PresenceStateOffline)
		&& ((getPresenceState() != EnumPresenceState::PresenceStateUnknown)));
}

void ContactProfile::setUUID(const std::string & newUUID) {
	_uuid = newUUID;
}
