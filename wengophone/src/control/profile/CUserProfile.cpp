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
#include "CUserProfile.h"

#include <presentation/PFactory.h>
#include <presentation/PUserProfile.h>

#include <control/CWengoPhone.h>
#include <control/chat/CChatHandler.h>
#include <control/contactlist/CContactList.h>
#include <control/history/CHistory.h>
#include <control/phoneline/CPhoneLine.h>

#include <control/webservices/sms/CSms.h>
#include <control/webservices/softupdate/CSoftUpdate.h>
#include <control/webservices/callforward/CWsCallForward.h>
#include <control/webservices/contactIntegration/CWsContactProfile.h>	//VOXOX - JRT - 2009.08.03 

#include <control/wenbox/CWenboxPlugin.h>

#include <model/WengoPhone.h>
#include <model/account/wengo/WengoAccount.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/contactlist/Contact.h>
#include <model/contactlist/ContactList.h>					//VOXOX - JRT - 2009.06.29 - HACK support
#include <model/contactlist/ContactProfileJSONSerializer.h>	//VOXOX - JRT - 2009.06.29 
#include <model/history/History.h>
#include <model/phoneline/IPhoneLine.h>
#include <model/presence/PresenceHandler.h>
#include <model/profile/UserProfile.h>
#include <model/webservices/sms/WsSms.h>
#include <model/webservices/softupdate/WsSoftUpdate.h>
#include <model/webservices/contactInfo/WsContactNumber.h>
#include <model/webservices/contactIntegration/WsContactProfile.h>		//VOXOX - JRT - 2009.08.03 
#include <model/wenbox/WenboxPlugin.h>

#include <sipwrapper/SipWrapper.h>

#include <thread/ThreadEvent.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

CUserProfile::CUserProfile(UserProfile & userProfile, CWengoPhone & cWengoPhone)
	: _userProfile(userProfile),
	_cWengoPhone(cWengoPhone),
	_cContactList(userProfile.getContactList(), cWengoPhone),
	_cWenboxPlugin(*userProfile.getWenboxPlugin(), cWengoPhone),
	_cChatHandler(userProfile.getChatHandler(), *this) 
{
	_cHistory			= NULL;
	_cPhoneLine			= NULL;
	_cSms				= NULL;
	_cSoftUpdate		= NULL;
	_cWsCallForward		= NULL;
	_cWsContactProfile	= NULL;	//VOXOX - JRT - 2009.08.03 


	_pUserProfile = PFactory::getFactory().createPresentationUserProfile(*this);

//JRT-XXX	_userProfile.userProfileInitializedEvent		+= boost::bind(&CUserProfile::userProfileInitializedEventHandler,		this, _1);
	_userProfile.phoneLineCreatedEvent				+= boost::bind(&CUserProfile::phoneLineCreatedEventHandler,				this, _1, _2);
	_userProfile.wsSmsCreatedEvent					+= boost::bind(&CUserProfile::wsSmsCreatedEventHandler,					this, _1, _2);
	_userProfile.wsSoftUpdateCreatedEvent			+= boost::bind(&CUserProfile::wsSoftUpdateCreatedEventHandler,			this, _1, _2, _3);	//VOXOX - JRT - 2009.10.27 
	_userProfile.wsCallForwardCreatedEvent			+= boost::bind(&CUserProfile::wsCallForwardCreatedEventHandler,			this, _1, _2);
	_userProfile.loginStateChangedEvent				+= boost::bind(&CUserProfile::loginStateChangedEventHandler,			this, _1, _2);
	_userProfile.networkDiscoveryStateChangedEvent	+= boost::bind(&CUserProfile::networkDiscoveryStateChangedEventHandler, this, _1, _2);

	_userProfile.getHistory().historyLoadedEvent	+= boost::bind(&CUserProfile::historyLoadedEventHandler,				this, _1);

	_userProfile.getPresenceHandler().authorizationRequestEvent += boost::bind(&CUserProfile::authorizationRequestEventHandler, this, _1, _2, _3);
	_userProfile.getPresenceHandler().incomingSubscribeEvent	+= boost::bind(&CUserProfile::incomingSubscribeEventHandler,	this, _1, _2, _3, _4, _5);

	//Check if a PhoneLine already exist
	if (_userProfile.getActivePhoneLine()) 
	{
		phoneLineCreatedEventHandler(_userProfile, *_userProfile.getActivePhoneLine());
	}

	historyLoadedEventHandler(_userProfile.getHistory());

	/*if (_userProfile.getWsSms()) 
	{
		wsSmsCreatedEventHandler(_userProfile, *_userProfile.getWsSms());
	}*/

	checkForSoftUpdate( false );	//Automatic - //VOXOX - JRT - 2009.10.27 

	createWsContactProfile();	//Call now in case event already fired.

	//VOXOXCHANGE CJC	//VOXOX - JRT - 2009.05.24 TODO: find supporting files and remove from project.
	//Don't need call transfer functionality
	/*if (_userProfile.getWsCallForward()) {
		wsCallForwardCreatedEventHandler(_userProfile, *_userProfile.getWsCallForward());
	}*/
}

CUserProfile::~CUserProfile() 
{
	OWSAFE_DELETE(_pUserProfile);
	OWSAFE_DELETE(_cHistory);
	OWSAFE_DELETE(_cSms);
	OWSAFE_DELETE(_cSoftUpdate);
	OWSAFE_DELETE(_cPhoneLine);

	OWSAFE_DELETE(_cWsContactProfile);	//VOXOX - JRT - 2009.08.03 
}

void CUserProfile::loginStateChangedEventHandler(SipAccount & sender, EnumSipLoginState::SipLoginState state) 
{
	_pUserProfile->loginStateChangedEventHandler(sender, state);
}

void CUserProfile::networkDiscoveryStateChangedEventHandler(SipAccount & sender, SipAccount::NetworkDiscoveryState state) 
{
	_pUserProfile->networkDiscoveryStateChangedEventHandler(sender, state);
}

void CUserProfile::authorizationRequestEventHandler(PresenceHandler & sender, const IMContact & imContact, const std::string & message) 
{
	_pUserProfile->authorizationRequestEventHandler(sender, imContact, message);
}

void CUserProfile::incomingSubscribeEventHandler(PresenceHandler & sender, const std::string & imaccountId, 
												  int sid,const std::string & from,const std::string & evtType) 
{
	_pUserProfile->incomingSubscribeEventHandler(sender,imaccountId, sid, from, evtType);
}

void CUserProfile::phoneLineCreatedEventHandler(UserProfile & sender, IPhoneLine & phoneLine) 
{
	_cPhoneLine = new CPhoneLine(phoneLine, _cWengoPhone);

	LOG_DEBUG("CPhoneLine created");
}

void CUserProfile::wsSmsCreatedEventHandler(UserProfile & sender, WsSms & sms) 
{
	if (!_cSms) 
	{
		_cSms = new CSms(sms, _cWengoPhone);
		LOG_DEBUG("CSms created");
	}
}

void CUserProfile::wsCallForwardCreatedEventHandler(UserProfile & sender, WsCallForward & wsCallForward) 
{
	_cWsCallForward = new CWsCallForward(_cWengoPhone, wsCallForward);
}

void CUserProfile::checkForSoftUpdate( bool manualCheck )
{
	if (_userProfile.getWsSoftUpdate()) 
	{
		wsSoftUpdateCreatedEventHandler(_userProfile, *_userProfile.getWsSoftUpdate(), manualCheck );
	}
}

void CUserProfile::wsSoftUpdateCreatedEventHandler(UserProfile & sender, WsSoftUpdate & wsSoftUpdate, bool manualCheck ) 
{
	if (!_cSoftUpdate) 
	{
		_cSoftUpdate = new CSoftUpdate(wsSoftUpdate, _cWengoPhone);
		LOG_DEBUG("CSoftUpdate created");
	}

	//Since we can now get here manually...
	if (_cSoftUpdate) 
	{
		//Check for WengoPhone update
		if (_userProfile.getSipAccount()) 
		{
			if (_userProfile.getSipAccount()->getType() == SipAccount::SipAccountTypeWengo) 
			{
				wsSoftUpdate.checkForUpdate( manualCheck );
			}
		}
	}
}

void CUserProfile::historyLoadedEventHandler(History & history) {
	_cHistory = new CHistory(history, _cWengoPhone, *this);
}

void CUserProfile::disconnect() {
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::disconnectThreadSafe, this));
	WengoPhone::getInstance().postEvent(event);
}

void CUserProfile::disconnectThreadSafe() {
	_userProfile.disconnect();
}

void CUserProfile::makeContactCall(const std::string & contactId) {
	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::makeContactCallThreadSafe, this, _1), contactId);
	WengoPhone::getInstance().postEvent(event);
}

void CUserProfile::makeContactCallThreadSafe(std::string contactId) {
	Contact * contact = _cContactList.getContact(contactId);
	
	if (contact) 
	{
		//VOXOXCHANGE CJC CALL VOXOX NUMBERS
		if(contact->getIsIMAccountVoxox())
		{
			//We don't get the number if we already have it//TODO CHANGE THIS IF NUMBERS ARE CHANGED FREQUENTLY
			if(!contact->getVoxOxPhone().empty())
			{
				EnumMakeCallError::MakeCallError error = _userProfile.makeCall(*contact);
				if (error != EnumMakeCallError::NoError) 
				{
					makeCallErrorEvent(*this, error, contact->getVoxOxPhone());//VOXOX CHANGE by Rolando - 2009.05.29 - added parameter phonenumber to check what call failed
				}
			}
			else
			{
				if(_userProfile.getWsContactNumber())
				{
					WsContactNumber & wsContactNumber = *_userProfile.getWsContactNumber();
					wsContactNumber.setContactId(contact->getKey());
					wsContactNumber.contactNumberEvent +=	boost::bind(&CUserProfile::contactNumberEventHandler, this, _1, _2, _3);
					wsContactNumber.execute();
				}
			}	

		}else if(contact->getQtIMProtocol() == QtEnumIMProtocol::IMProtocolSkype){
			_userProfile.makeCall(*contact);
		}
		else
		{
			EnumMakeCallError::MakeCallError error = _userProfile.makeCall(*contact);
			if (error != EnumMakeCallError::NoError) 
			{
				makeCallErrorEvent(*this, error, contactId);//VOXOX CHANGE by Rolando - 2009.05.29 - added parameter contactId to check what call failed
			}
		}
	}
	else
	{
		makeCallErrorEvent(*this, EnumMakeCallError::ContactNotFound, contactId);//VOXOX CHANGE by Rolando - 2009.05.29 - added parameter contactId to check what call failed
	}
}
//VOXOXCHANGE CJC
//SETVOXOX NUMBER TO CONTACT
void CUserProfile::contactNumberEventHandler(WsContactNumber & sender, const std::string & contactNumber, const std::string & contactId)
{
	//Check if webservice return a number
	if(contactNumber!="noNumber")
	{
		Contact * contact = _cContactList.getContactById(contactId);
		//Check if we find a contact
		if (contact) 
		{
			contact->setVoxOxPhone(contactNumber);
			
			getCContactList().updateContact(*contact);	//VOXOX - CJC - 2009.06.23 Fix bug not calling voxox contact

			EnumMakeCallError::MakeCallError error = _userProfile.makeCall(*contact);

			if (error != EnumMakeCallError::NoError) 
			{
					makeCallErrorEvent(*this, error, contactNumber);//VOXOX CHANGE by Rolando - 2009.05.29 - added parameter contactNumber to check what call failed
			}
		}
	}
}

void CUserProfile::makeCall(const std::string & phoneNumber) 
{
	typedef ThreadEvent1<void (std::string phoneNumber), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::makeCallThreadSafe, this, _1), phoneNumber);
	WengoPhone::getInstance().postEvent(event);
}

void CUserProfile::makeCallThreadSafe(std::string phoneNumber) 
{
	EnumMakeCallError::MakeCallError error = _userProfile.makeCall(phoneNumber);
	if (error != EnumMakeCallError::NoError) 
	{
		makeCallErrorEvent(*this, error, phoneNumber);//VOXOX CHANGE by Rolando - 2009.05.29 - added parameter phoneNumber to check what call failed
	}
}

void CUserProfile::initChat() {
	typedef ThreadEvent0<void()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::initChatThreadSafe, this));
	WengoPhone::getInstance().postEvent(event);
}


void CUserProfile::startIM(const std::string & contactId) {
	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::startIMThreadSafe, this, _1), contactId);
	WengoPhone::getInstance().postEvent(event);
}

//VOXOX - JRT - 2009.07.26 
void CUserProfile::startIMToEmail( const std::string& contactId, const std::string & emailIn) 
{
	std::string email = emailIn;

	if ( email.empty() )
	{
		Contact* contact = getCContactList().getContactById( contactId );

		if ( contact )
		{
			email = contact->getPreferredEmail();
		}
	}

	if ( !email.empty() )
	{
		startIMToEmail( email );
	}
}

//VOXOX - CJC - 2009.05.07 Start IM To Email
void CUserProfile::startIMToEmail(const std::string & email) 
{
	typedef ThreadEvent1<void (std::string email), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::startIMToEmailThreadSafe, this, _1), email);
	WengoPhone::getInstance().postEvent(event);
}

//VOXOX - CJC - 2009.05.19 im to sms
void CUserProfile::startIMToSMS(const std::string & sms) 
{
	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::startIMToSMSThreadSafe, this, _1), sms);
	WengoPhone::getInstance().postEvent(event);
}

//VOXOX - JRT - 2009.07.26 
void CUserProfile::startIMToSMS( const std::string& contactId, const std::string & smsIn) 
{
	std::string sms = smsIn;

	if ( sms.empty() )
	{
		Contact* contact = getCContactList().getContactById( contactId );

		if ( contact )
		{
			sms = contact->getMobilePhone();
		}
	}

	if ( !sms.empty() )
	{
		typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
		MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::startIMToSMSThreadSafe, this, _1), sms);
		WengoPhone::getInstance().postEvent(event);
	}
}

//VOXOX - JRT - 2009.06.14 
//void CUserProfile::createGroupChat( const std::string& chatRoom, const std::string& imAccountId ) 
//{
//	typedef ThreadEvent2<void ( std::string chatRoom, std::string imAccountId ), std::string, std::string> MyThreadEvent;
//	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::createGroupChatThreadSafe, this, _1, _2), chatRoom, imAccountId);
//	WengoPhone::getInstance().postEvent(event);
//}

//void CUserProfile::createGroupChatThreadSafe( const std::string chatRoom, const std::string imAccountId) 
//{
//	_userProfile.createGroupChat( chatRoom, imAccountId );
//}

void CUserProfile::inviteToGroupChat( const std::string& chatRoom, const std::string& imAccountId, const IMContactSet& imContactSet ) 
{
	typedef ThreadEvent3<void ( std::string chatRoom, std::string imAccountId, IMContactSet imContactSet), std::string, std::string, IMContactSet> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::inviteToGroupChatThreadSafe, this, _1, _2, _3), chatRoom, imAccountId, imContactSet);
	WengoPhone::getInstance().postEvent(event);
}

void CUserProfile::inviteToGroupChatThreadSafe( const std::string chatRoom, const std::string imAccountId, const IMContactSet imContactSet) 
{
	if ( imContactSet.size() > 0 )
	{
		_userProfile.inviteToGroupChat( chatRoom, imAccountId, imContactSet );
	}
}

//VOXOX - CJC - 2009.06.03 
void CUserProfile::initChatThreadSafe() 
{	
	_userProfile.initChat();
}

void CUserProfile::startIMThreadSafe(std::string contactId) 
{
	Contact * contact = _cContactList.getContact(contactId);
	if (contact) 
	{
		_userProfile.startIM(*contact);
	}
}

//VOXOX - CJC - 2009.05.07 Start IM To Email Thread safe
void CUserProfile::startIMToEmailThreadSafe(std::string email) 
{
	_userProfile.startIMToEmail(email);
	
}

//VOXOX - CJC - 2009.05.07 Start IM To Email Thread safe
void CUserProfile::startIMToSMSThreadSafe(std::string number) 
{
	_userProfile.startIMToSMS(number);	
}

//VOXOX CHANGE CJC START IM AND SEND MESSAGE
void CUserProfile::startIMAndSendMessage(const std::string & contactId,const std::string & message) 
{
	typedef ThreadEvent2<void (std::string contactId, std::string message ), std::string,std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::startIMAndSendMessageThreadSafe, this, _1, _2), contactId,message);
	WengoPhone::getInstance().postEvent(event);
}

//VOXOX CHANGE CJC START IM AND SEND MESSAGE
void CUserProfile::startIMAndSendMessageThreadSafe(std::string contactId,std::string message) 
{
	Contact * contact = _cContactList.getContact(contactId);
	if (contact) {
		_userProfile.startIMAndSendMessage(*contact,message);
	}
}
//VOXOX CHANTE OPEN SEND FILE ON CHAT
void CUserProfile::sendFile(const std::string & contactId) 
{
	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::sendFileThreadSafe, this, _1), contactId);
	WengoPhone::getInstance().postEvent(event);
}
//VOXOX CHANGE CJC OPEN SEND FILE ONCHAT
void CUserProfile::sendFileThreadSafe(std::string contactId) 
{
	Contact * contact = _cContactList.getContact(contactId);
	if (contact) 
	{
		_userProfile.sendFile(*contact);
	}
}

void CUserProfile::reRequestAuthorization(const std::string & contactId) //VOXOX - CJC - 2009.07.30 
{
	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::reRequestAuthorizationThreadSafe, this, _1), contactId);
	WengoPhone::getInstance().postEvent(event);
}

void CUserProfile::reRequestAuthorizationThreadSafe(std::string contactId) //VOXOX - CJC - 2009.07.30 
{
	Contact * contact = _cContactList.getContact(contactId);
	if (contact) 
	{
		_userProfile.contactReRequestAuthorization(contactId);
	}
}


//VOXOX - JRT - 2009.07.26 
void CUserProfile::sendFax( const std::string& contactId, const std::string & faxNumberIn )
{
	std::string faxNumber = faxNumberIn;

	if ( faxNumber.empty() )
	{
		Contact* contact = getCContactList().getContactById( contactId );

		if ( contact )
		{
			faxNumber = contact->getFax();
		}
	}

	if ( !faxNumber.empty() )
	{
		sendFax( faxNumber );
	}
}

//VOXOX - CJC - 2009.06.23 
void CUserProfile::sendFax(const std::string & faxNumber) //VOXOX - CJC - 2009.06.23 
{
	typedef ThreadEvent1<void (std::string faxNumber), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::sendFaxThreadSafe, this, _1), faxNumber);
	WengoPhone::getInstance().postEvent(event);
}
//VOXOX - CJC - 2009.06.23 
void CUserProfile::sendFaxThreadSafe(std::string faxNumber) //VOXOX - CJC - 2009.06.23 
{
	_userProfile.sendFax(faxNumber);
	
}
//VOXOX CHANGE CJC OPEN CONTACT PROFILE ONCHAT
void CUserProfile::openContactProfile(const std::string & contactId) 
{
	typedef ThreadEvent1<void (std::string contactId), std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::openContactProfileThreadSafe, this, _1), contactId);
	WengoPhone::getInstance().postEvent(event);
}

//VOXOX CHANGE CHC OPEN CONTACT PROFILE ONCHAT
void CUserProfile::openContactProfileThreadSafe(std::string contactId)
{
	Contact * contact = _cContactList.getContact(contactId);
	if (contact) 
	{
		_userProfile.openContactProfile(*contact);
	}
}

//void CUserProfile::setWengoAccount(const WengoAccount & wengoAccount) {
//	typedef ThreadEvent1<void (WengoAccount wengoAccount), WengoAccount> MyThreadEvent;
//	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::setWengoAccountThreadSafe, this, _1), wengoAccount);
//	WengoPhone::getInstance().postEvent(event);
//}
//
//void CUserProfile::setWengoAccountThreadSafe(WengoAccount wengoAccount) {
//	_userProfile.setWengoAccount(wengoAccount);
//}

void CUserProfile::setSipAccount(const SipAccount & sipAccount) 
{
	typedef ThreadEvent1<void (SipAccount sipAccount), SipAccount> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::setSipAccountThreadSafe, this, _1), sipAccount);
	WengoPhone::getInstance().postEvent(event);
}

void CUserProfile::setSipAccountThreadSafe(SipAccount sipAccount) 
{
	_userProfile.setSipAccount(sipAccount);
}

PhoneCall * CUserProfile::getActivePhoneCall() const 
{
	PhoneCall * result = NULL;

	//FIXME: model must not be used directly by the GUI
	IPhoneLine * phoneLine = _userProfile.getActivePhoneLine();
	if (phoneLine) {
		result = phoneLine->getActivePhoneCall();
	}

	return result;
}


//VOXOX - JRT - 2009.08.03 - Sync current profile to server.
void CUserProfile::createWsContactProfile()
{
	if ( _userProfile.getWsContactProfile() != NULL ) 
	{
		_cWsContactProfile = new CWsContactProfile( _cWengoPhone, *_userProfile.getWsContactProfile() );
	}
}

void CUserProfile::updateUserProfile()
{
	typedef ThreadEvent0<void ()> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::updateUserProfileThreadSafe, this));
	WengoPhone::getInstance().postEvent(event);
}

void CUserProfile::updateUserProfileThreadSafe() 
{
	if ( _cWsContactProfile )
	{
		_cWsContactProfile->setUserProfile( getUserProfile() );
	}
}

//VOXOX - JRT - 2009.05.25 - Added so local and server updates can be done in same call.
void CUserProfile::updateContact(const ContactProfile& contactProfile)
{
	typedef ThreadEvent1<void (ContactProfile contactProfile), ContactProfile> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&CUserProfile::updateContactThreadSafe, this, _1), contactProfile);
	WengoPhone::getInstance().postEvent(event);
}

void CUserProfile::updateContactThreadSafe(const ContactProfile contactProfile) 
{
	//We need to get the ServerProtocolId or UserNetworkId for the IMAccount.
	IMContact* imContact = contactProfile.getPreferredIMContact();

	if ( imContact )
	{
		IMAccount* imAccount = getUserProfile().getIMAccountManager().getIMAccount( imContact->getIMAccount()->getKey() );

		if ( imAccount )
		{
			imContact->setServerProtocolId( imAccount->getServerProtocolId() );
			imContact->setUserNetworkId   ( imAccount->getUserNetworkId()    );	//VOXOX - JRT - 2009.06.09 

			getCContactList().updateContact(contactProfile);		//This does the update.
		}
	}
}


bool CUserProfile::updateContactProfileFromJSON( const std::string& data, int& jsId, int& qtId )
{
	bool result = false;
	ContactProfile profile;

	ContactProfileJSONSerializer serializer( getUserProfile().getContactList(), profile );
	
	if ( serializer.unserialize( data ) )
	{
		if ( profile.getId() <= 0 )	//This is an add.
		{
			jsId = profile.getId();

			getCContactList().addContact(profile);
		}
		else
		{
			Contact* contact = getUserProfile().getContactList().getContactByJSONId( profile.getId() );

			if ( contact )
			{
//				getCContactList().updateContactGroups( *contact, contact->getContactGroupInfoSet(), profile.getContactGroupInfoSet() );	//VOXOX - JRT - 2009.09.23 

				profile.getIMContactSetNonConst() = contact->getIMContactSetNonConst();
			}

			getCContactList().updateContact( profile );
		}

		result = true;
	}

	return result;
}
	
//VOXOX - JRT - 2009.07.26 
void CUserProfile::doDefaultAction( const std::string& contactId )
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	if (config.getGeneralClickStartChat()) 
	{
		startIM( contactId );
	}
	else
	{
		Contact* contact = getCContactList().getContactById( contactId );

		if ( contact )
		{
			// Start free call by default
			if (contact->hasFreeCall() || contact->hasVoiceMail()) 
			{
				std::string phoneNumber = contact->getFirstFreePhoneNumber();
				if (phoneNumber.empty()) 
				{
					phoneNumber = contact->getFirstVoiceMailNumber();
				}

				if (phoneNumber.empty()) 
				{
					LOG_WARN("Could find neither a free phone number, nor a voice mail number");
				}
				else
				{
					makeCall(phoneNumber);
				}
				//End VoxOx
			} 
			else if (config.getGeneralClickCallCellPhone() && contact->hasCall()) 
			{
				makeCall( contactId );
			}
		}
	}
}

void CUserProfile::callContact( const std::string& contactId )
{
	Contact* contact = getCContactList().getContactById( contactId );

	if ( contact )
	{
		//VOXOX - CJC - 2009.06.23 Fix bug not calling voxox contact
		if ( contact->getPreferredIMContact()->isIMAccountVoxOx() )
		{
			//VOXOX - JRT - 2009.09.24 - TODO: Reversed logic?  This prevents block of code prevents retrieval of SIP number for this contact.
			//								   Most of this logic already exists in makeContactCallThreadSafe(), which is called by makeContactCall()
			if ( contact->getVoxOxPhone().empty() )
			{
				makeContactCall( contactId );
			}
			else
			{
				makeCall( contact->getPreferredNumber());	//VOXOX - CJC - 2009.06.11 
			}
		}
		else if(contact->getQtIMProtocol() == QtEnumIMProtocol::IMProtocolSkype){
			makeContactCall(contactId);
		}
		else
		{
			if ( contact->getTelephones().hasValid() )
			{
				makeCall( contact->getPreferredNumber());	//VOXOX - CJC - 2009.06.11 
			}
		}
	}
}
//VOXOX - CJC - 2010.01.17 
WsTranslation* CUserProfile::getWsTranslation()//VOXOX - CJC - 2010.01.17 
{
	return getUserProfile().getWsTranslation();
}
std::string CUserProfile::getTranslationLanguageXML()//VOXOX - CJC - 2010.01.28 
{
	return getUserProfile().getTranslationLanguagesXML();//VOXOX - CJC - 2010.01.28 
}
