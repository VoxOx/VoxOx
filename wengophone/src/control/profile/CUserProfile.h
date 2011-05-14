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

#ifndef OWCUSERPROFILE_H
#define OWCUSERPROFILE_H

#include <control/chat/CChatHandler.h>
#include <control/contactlist/CContactList.h>
#include <control/wenbox/CWenboxPlugin.h>

#include <model/account/SipAccount.h>
#include <model/phoneline/EnumMakeCallError.h>

#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMAccountList.h>

#include <util/Trackable.h>

#include <set>

class CHistory;
class CPhoneLine;
class CSms;
class CSoftUpdate;
class CWengoPhone;
class CWsCallForward;
class History;
class IMAccount;
class IPhoneLine;
class PhoneCall;
class PUserProfile;
class UserProfile;
class WengoAccount;
class WsCallForward;
class WsSms;
class WsSoftUpdate;
class PresenceHandler;
class WsContactNumber;
class WsTranslation;

class WsContactProfile;		//VOXOX - JRT - 2009.08.03 
class CWsContactProfile;	//VOXOX - JRT - 2009.08.03 


/**
 * Control layer for UserProfile.
 *
 * FIXME: 'init()' must be called on UserProfile before constructing CUserProfile.
 *
 * @author Philippe Bernery
 */
class CUserProfile : public Trackable {
public:

	Event <void (CUserProfile & sender, EnumMakeCallError::MakeCallError, std::string phoneNumber)> makeCallErrorEvent;//VOXOX CHANGE by Rolando - 2009.05.29 - added parameter phoneNumber to check what call failed

	CUserProfile(UserProfile & userProfile, CWengoPhone & cWengoPhone);

	~CUserProfile();

	/**
	 * @see UserProfile::disconnect
	 */
	void disconnect();

	/**
	 * @see UserProfile::makeCall()
	 */
	void makeContactCall(const std::string & contactId);

	/**
	 * @see UserProfile::makeCall()
	 */
	void makeCall(const std::string & phoneNumber);


	/**
	 * @see UserProfile::startIM
	 */
	
	//VOXOX - CJC - 2009.06.03 
	void initChat();

	void startIM(const std::string & contactId);

	//VOXOX - CJC - 2009.05.07 Start Chat to Email
	//VOXOX - CJC - 2009.06.11 
	void startIMToEmail( const std::string& email );
	void startIMToEmail( const std::string& contactId, const std::string& email);	//VOXOX - JRT - 2009.07.26 

	//VOXOX - CJC - 2009.05.07 Start Chat to SMS
	void startIMToSMS( const std::string & number);
	void startIMToSMS( const std::string& contactId, const std::string & number);	//VOXOX - JRT - 2009.07.26 

	//VOXOX CHANGE CJC SEND FILE
	void sendFile(const std::string & contactId);

	void reRequestAuthorization(const std::string & contactId);//VOXOX - CJC - 2009.07.30 

	//VOXOX - CJC - 2009.06.23 
	void sendFax( const std::string & faxNumber);
	void sendFax( const std::string& contactId, const std::string & faxNumber);	//VOXOX - JRT - 2009.07.26 

	//VOXOX CHANGE CJC CONTACT PROFILE ONCHAT
	void openContactProfile(const std::string & contactId);

	//VOXOX CHANGE CJC STARTIM AND SEND MESSAGE
	void startIMAndSendMessage(const std::string & contactId,const std::string & message);

	//VOXOX - JRT - 2009.05.25 - Added so we can perform local and remote updates.
	void updateContact( const ContactProfile& contactProfile );
	void updateUserProfile();			//VOXOX - JRT - 2009.08.03 

	//VOXOX - JRT - 2009.06.14 
	void inviteToGroupChat( const std::string& chatRoom, const std::string& imAccountId, const IMContactSet& imContactSet ); 

	bool updateContactProfileFromJSON( const std::string& data, int& jsId, int& qtId );

	void doDefaultAction( const std::string& contactId );		//VOXOX - JRT - 2009.07.26 
	void callContact	( const std::string& contactId );		//VOXOX - JRT - 2009.07.26 

	void checkForSoftUpdate( bool manualCheck );				//VOXOX - JRT - 2009.10.27 

	/**
	 * @see UserProfile::setSipAccount
	 */
	void setSipAccount(const SipAccount & sipAccount);

	/**
	 * Gets the active phone call.
	 *
	 * Used for playing DTMF.
	 *
	 * @return active phone call or NULL
	 */
	PhoneCall * getActivePhoneCall() const;

	/**
	 * Gets the CHistory.
	 *
	 * @return the CHistory
	 */
	CHistory * getCHistory() const {
		return _cHistory;
	}

	/**
	 * Gets the CContactList.
	 *
	 * @return the CContactList
	 */
	CContactList & getCContactList() {
		return _cContactList;
	}

	/**
	 * @return the CWsCallForward object.
	 */
	CWsCallForward * getCWsCallForward() {
		return _cWsCallForward;
	}

	/**
	 * @return the CWengoPhone object.
	 */
	CWengoPhone & getCWengoPhone() {
		return _cWengoPhone;
	}

	/**
	 * Gets the UserProfile.
	 *
	 * TODO: This method should not exist (because it allows the GUI to access
	 * the model directly) and so should be removed ASAP.
	 */
	UserProfile & getUserProfile() {
		return _userProfile;
	}

	/**
	 * Gets the Presentation layer.
	 */
	PUserProfile * getPresentation() {
		return _pUserProfile;
	}

	WsTranslation*				getWsTranslation();//VOXOX - CJC - 2010.01.17 
	std::string					getTranslationLanguageXML();//VOXOX - CJC - 2010.01.28 
	CPhoneLine * getCPhoneLine() {
		return _cPhoneLine;
	}

private:

	/**
	 * @see UserProfile::loginStateChangedEvent
	 */
	void loginStateChangedEventHandler(SipAccount & sender,
		EnumSipLoginState::SipLoginState state);

	/**
	 * @see UserProfile::networkDiscoveryStateChangedEvent
	 */
	void networkDiscoveryStateChangedEventHandler(SipAccount & sender,
		SipAccount::NetworkDiscoveryState state);

	/**
	 * @see PresenceHandler::authorizationRequestEvent
	 */
	void authorizationRequestEventHandler(PresenceHandler & sender,
		const IMContact & imContact, const std::string & message);

	/**
	 * @see PresenceHandler::authorizationRequestEvent
	 */
	void incomingSubscribeEventHandler(PresenceHandler & sender,
		const std::string & imaccountId,int sid,const std::string & from,const std::string & evtType);

	/**
	 * @see UserProfile::historyLoadedEvent
	 */
	void historyLoadedEventHandler(History & sender);

	/**
	 * @see UserProfile::phoneLineCreatedEvent
	 */
	void phoneLineCreatedEventHandler(UserProfile & sender, IPhoneLine & phoneLine);

	/**
	 * @see UserProfile::wsSmsCreatedEvent
	 */
	void wsSmsCreatedEventHandler(UserProfile & sender, WsSms & wsSms);

	/**
	 * @see UserProfile::wsSoftUpdateCreatedEvent
	 */
	void wsSoftUpdateCreatedEventHandler(UserProfile & sender, WsSoftUpdate & wsSoftUpdate, bool manualCheck );	//VOXOX - JRT - 2009.10.27 

	/**
	 * @see UserProfile::wsCallForwardCreatedEvent
	 */
	void wsCallForwardCreatedEventHandler(UserProfile & sender, WsCallForward & wsCallForward);

	void contactNumberEventHandler(WsContactNumber & sender,const std::string & contactNumber,const std::string & contactId);

	/**
	 * @see disconnect()
	 */
	void disconnectThreadSafe();

	/**
	 * @see makeCall()
	 */
	void makeContactCallThreadSafe(std::string contactId);

	/**
	 * @see makeCall()
	 */
	void makeCallThreadSafe(std::string phoneNumber);

	/**
	 * @see startIM
	 */
	void startIMThreadSafe(std::string contactId);
	//VOXOX - CJC - 2009.06.03 
	void initChatThreadSafe();

	//VOXOX - CJC - 2009.05.07 Start IM To Email tread safe
	void startIMToEmailThreadSafe(std::string email);

	//VOXOX - CJC - 2009.05.19 to sms
	void startIMToSMSThreadSafe(std::string number);

	//VOXOX CHANGE CJC START IM AND SEND MESSAGE
	void startIMAndSendMessageThreadSafe(std::string contactId,std::string  message);


	//VOXOX CHANGE SEND FILE
	void sendFileThreadSafe(std::string contactId);

	void reRequestAuthorizationThreadSafe(std::string contactId);//VOXOX - CJC - 2009.07.30 

	void sendFaxThreadSafe(std::string faxNumber);
	
	//VOXOX CHANGE contactProfile
	void openContactProfileThreadSafe(std::string contactId);

	//VOXOX - JRT - 2009.05.25 - Save profile on server.
	void updateContactThreadSafe(const ContactProfile contactProfile);
	void updateUserProfileThreadSafe();		//VOXOX - JRT - 2009.08.03 

	void inviteToGroupChatThreadSafe( const std::string  chatRoom, const std::string  imAccountId, const IMContactSet  imContactSet ); 
//	void createGroupChatThreadSafe( const std::string  chatRoom, const std::string  imAccountId ); 
	//End JRT
	void createWsContactProfile();	//VOXOX - JRT - 2009.08.03 


	/**
	 * @see setSipAccount
	 */
	void setSipAccountThreadSafe(SipAccount sipAccount);

	UserProfile & _userProfile;

	CWengoPhone & _cWengoPhone;

	PUserProfile * _pUserProfile;

	CContactList _cContactList;

	CHistory * _cHistory;

	CWsCallForward * _cWsCallForward;

	CWenboxPlugin _cWenboxPlugin;

	CChatHandler _cChatHandler;

	CSms * _cSms;

	CSoftUpdate * _cSoftUpdate;

	CPhoneLine * _cPhoneLine;

	CWsContactProfile*	_cWsContactProfile;		//VOXOX - JRT - 2009.08.03 

};

#endif	//OWCUSERPROFILE_H
