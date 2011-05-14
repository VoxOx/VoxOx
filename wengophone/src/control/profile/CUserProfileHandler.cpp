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
#include "CUserProfileHandler.h"

#include <presentation/PFactory.h>
#include <presentation/PUserProfileHandler.h>

#include <control/profile/CUserProfile.h>
#include <control/phoneline/CPhoneLine.h>

#include <model/account/wengo/WengoAccount.h>
#include <model/connect/ConnectHandler.h>
#include <model/profile/UserProfile.h>
#include <model/profile/UserProfileHandler.h>
#include <model/WengoPhone.h>

#include <thread/ThreadEvent.h>
#include <util/SafeDelete.h>

#include <exception>

CUserProfileHandler::CUserProfileHandler(UserProfileHandler & userProfileHandler, CWengoPhone & cWengoPhone)
	: _userProfileHandler(userProfileHandler),
	_cWengoPhone(cWengoPhone) {

	_cUserProfile = NULL;

	_pUserProfileHandler = PFactory::getFactory().createPresentationUserProfileHandler(*this);

	_userProfileHandler.noCurrentUserProfileSetEvent	  += boost::bind(&CUserProfileHandler::noCurrentUserProfileSetEventHandler,		 this, _1);
	_userProfileHandler.currentUserProfileWillDieEvent	  += boost::bind(&CUserProfileHandler::currentUserProfileWillDieEventHandler,	 this, _1);
	_userProfileHandler.userProfileInitializedEvent		  += boost::bind(&CUserProfileHandler::userProfileInitializedEventHandler,		 this, _1, _2);
	_userProfileHandler.sipAccountConnectionFailedEvent   += boost::bind(&CUserProfileHandler::sipAccountConnectionFailedEventHandler,	 this, _1, _2, _3);//VOXOX CHANGE by Rolando - 2009.09.10 
	_userProfileHandler.profileLoadedFromBackupsEvent	  += boost::bind(&CUserProfileHandler::profileLoadedFromBackupsEventHandler,	 this, _1, _2);
	_userProfileHandler.profileCannotBeLoadedEvent		  += boost::bind(&CUserProfileHandler::profileCannotBeLoadedEventHandler,		 this, _1, _2);
	_userProfileHandler.networkDiscoveryStateChangedEvent += boost::bind(&CUserProfileHandler::networkDiscoveryStateChangedEventHandler, this, _1, _2);
}

CUserProfileHandler::~CUserProfileHandler() {
	OWSAFE_DELETE(_pUserProfileHandler);
	OWSAFE_DELETE(_cUserProfile);
}

std::vector<std::string> CUserProfileHandler::getUserProfileNames(bool onlyRealDir) {
	return _userProfileHandler.getUserProfileNames(onlyRealDir);
}

//VOXOX CHANGE by Rolando - 2009.05.22 - added parameter loginInvisible to check if user clicked "login" button with invisible presence
void CUserProfileHandler::setCurrentUserProfile(const std::string & login, const SipAccount & sipAccount, bool loginInvisible) 
{
	_pUserProfileHandler->setCurrentUserProfileEventHandler();	//Notify UI.

	if (sipAccount.getType() == SipAccount::SipAccountTypeWengo) 
	{
		const WengoAccount & wengoAccount = dynamic_cast<const WengoAccount &>(sipAccount);

		typedef ThreadEvent3<void (std::string, WengoAccount, bool), std::string, WengoAccount, bool> MyThreadEvent;
		MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfileHandler::setUserProfileWithWengoAccountThreadSafe, this, _1, _2, _3),
													login, wengoAccount, loginInvisible);

		WengoPhone::getInstance().postEvent(event);
	} 
	else 
	{
		typedef ThreadEvent2<void (std::string, SipAccount), std::string, SipAccount> MyThreadEvent;
		MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfileHandler::setUserProfileWithSipAccountThreadSafe, this, _1, _2),
									login, sipAccount);

		WengoPhone::getInstance().postEvent(event);
	}
}

void CUserProfileHandler::setUserProfileWithSipAccountThreadSafe(std::string profileName, SipAccount sipAccount) {
	_userProfileHandler.setCurrentUserProfile(profileName, sipAccount);
}

//VOXOX CHANGE by Rolando - 2009.05.22 - added parameter loginInvisible to check if user clicked "login" button with invisible presence
void CUserProfileHandler::setUserProfileWithWengoAccountThreadSafe(std::string profileName, WengoAccount wengoAccount, bool loginInvisible) {
	_userProfileHandler.setCurrentUserProfile(profileName, wengoAccount, loginInvisible);
}

void CUserProfileHandler::logOff( bool quitting ) 		//VOXOX - JRT - 2009.07.13 
{
	//VOXOX - JRT - 2009.07.12 - do this first to minimize crash potential.
	// hack to avoid crashes when logging off while an IMAccount is connecting
	UserProfile * currentUserProfile = getUserProfileHandler().getCurrentUserProfile();
	if (currentUserProfile) {
		currentUserProfile->getConnectHandler().userProfileIsLoggingOff();
	}

	// close active call(s)
	if (_cUserProfile) {
		CPhoneLine * cPhoneLine = _cUserProfile->getCPhoneLine();
		if (cPhoneLine) {
			cPhoneLine->closePendingCalls();
		}
	}
	
	if ( !quitting )		//VOXOX - JRT - 2009.07.13 
	{
		setCurrentUserProfile(String::null, SipAccount::empty);
	}
}

bool CUserProfileHandler::userProfileExists(const std::string & name) const {
	return _userProfileHandler.userProfileExists(name);
}

SipAccount CUserProfileHandler::getSipAccountOfUserProfile(const std::string & name) {
	UserProfile * userProfile = _userProfileHandler.getUserProfile(name);
	SipAccount result;

	if (userProfile) {
		result = *userProfile->getSipAccount();
	}

	OWSAFE_DELETE(userProfile);

	return result;
}

void CUserProfileHandler::currentUserProfileReleased() {
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfileHandler::currentUserProfileReleasedThreadSafe, this));

	WengoPhone::getInstance().postEvent(event);
}

void CUserProfileHandler::currentUserProfileReleasedThreadSafe() {
	OWSAFE_DELETE(_cUserProfile);

	PFactory::getFactory().reset();

	_userProfileHandler.currentUserProfileReleased();
}

void CUserProfileHandler::noCurrentUserProfileSetEventHandler(UserProfileHandler & sender) {
	_pUserProfileHandler->noCurrentUserProfileSetEventHandler();
}

void CUserProfileHandler::currentUserProfileWillDieEventHandler(UserProfileHandler & sender) {
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event =
		new MyThreadEvent(boost::bind(&CUserProfileHandler::currentUserProfileWillDieEventHandlerThreadSafe, this));

	WengoPhone::getInstance().postEvent(event);
}

void CUserProfileHandler::currentUserProfileWillDieEventHandlerThreadSafe() {
	_pUserProfileHandler->currentUserProfileWillDieEventHandler();
}

void CUserProfileHandler::userProfileInitializedEventHandler(UserProfileHandler & sender, UserProfile & userProfile) {
	Mutex::ScopedLock scopedLock(_mutex);
	_cUserProfile = new CUserProfile(userProfile, _cWengoPhone);
	_pUserProfileHandler->userProfileInitializedEventHandler();
}

//VOXOX CHANGE by Rolando - 2009.09.10 
void CUserProfileHandler::sipAccountConnectionFailedEventHandler(SipAccount & sipAccount, EnumSipLoginState::SipLoginState state, std::string loginErrorMessage) {
	_pUserProfileHandler->sipAccountConnectionFailedEventHandler(sipAccount, state, loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
}

void CUserProfileHandler::profileLoadedFromBackupsEventHandler(UserProfileStorage &sender, std::string profileName) {
	_pUserProfileHandler->profileLoadedFromBackupsEventHandler(profileName);
}

void CUserProfileHandler::profileCannotBeLoadedEventHandler(UserProfileStorage &sender, std::string profileName) {
	_pUserProfileHandler->profileCannotBeLoadedEventHandler(profileName);
}

void CUserProfileHandler::networkDiscoveryStateChangedEventHandler(SipAccount & sender,
	SipAccount::NetworkDiscoveryState state) {

	if ((state == SipAccount::NetworkDiscoveryStateError) ||
		(state == SipAccount::NetworkDiscoveryStateHTTPError) ||
		(state == SipAccount::NetworkDiscoveryStateSIPError)) {
		_pUserProfileHandler->networkErrorEventHandler(sender);
	}
}

void CUserProfileHandler::updateUserProfile()	//VOXOX - JRT - 2009.08.03 
{ 
	getCUserProfile()->updateUserProfile();	
}

void CUserProfileHandler::ignoreVersion( const SoftUpdateInfo& suInfo )	//VOXOX - JRT - 2009.10.30 
{
	UserProfile * currentUserProfile = getUserProfileHandler().getCurrentUserProfile();
	
	if (currentUserProfile) 
	{
		currentUserProfile->ignoreVersion( suInfo );
	}
}

//VOXOX - JRT - 2009.12.10 
SoftUpdatePrefs& CUserProfileHandler::getSoftUpdatePrefs()	
{ 
	UserProfile * currentUserProfile = getUserProfileHandler().getCurrentUserProfile();
	
	if (currentUserProfile) 
	{
		return currentUserProfile->getSoftUpdatePrefs();
	}
	else
	{
		return _defSoftUpdatePrefs;
	}
}

//VOXOX - JRT - 2009.12.10 
void CUserProfileHandler::saveSoftUpdatePrefs()	
{ 
	getUserProfileHandler().saveCurrentUserProfile();
}
