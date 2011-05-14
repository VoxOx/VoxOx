/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007 Wengo
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
#include "UserProfile.h"

#include <coipmanager/CoIpManager.h>

#include <model/WengoPhone.h>
#include <model/account/wengo/WengoAccount.h>
#include <model/chat/ChatHandler.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/connect/ConnectHandler.h>

#include <model/contactlist/Contact.h>
#include <model/contactlist/ContactGroup.h>
#include <model/contactlist/ContactList.h>
#include <model/contactlist/IMContactListHandler.h>
#include <model/contactlist/ContactListJSONSerializer.h>
#include <model/contactlist/ContactGroupJSONSerializer.h>
#include <model/contactlist/ContactProfileJSONSerializer.h>


#include <model/history/History.h>
#include <model/phonecall/PhoneCall.h>
#include <model/phoneline/PhoneLine.h>
#include <model/phoneline/PhoneLineState.h>
#include <model/phoneline/IPhoneLine.h>
#include <model/presence/PresenceHandler.h>
#include <model/profile/AvatarList.h>
#include <model/profile/UserProfileFileStorage.h>
#include <model/history/History.h>

#include <model/webservices/sms/WsSms.h>
#include <model/webservices/softupdate/WsSoftUpdate.h>
#include <model/webservices/info/WsInfo.h>
#include <model/webservices/translation/WsTranslationLanguages.h>	//VOXOX - CJC - 2010.01.28 
#include <model/webservices/translation/WsTranslation.h>//VOXOX - CJC - 2010.01.17 
#include <model/webservices/contactInfo/WsContactNumber.h>
#include <model/webservices/accountIntegration/WsAccount.h>			//VOXOX - JRT - 2009.05.27 
#include <model/webservices/contactIntegration/WsContactProfile.h>	//VOXOX - JRT - 2009.06.10 

#include <model/wenbox/WenboxPlugin.h>

#include <sipwrapper/SipWrapper.h>
#include <imwrapper/IMAccountParametersXMLSerializer.h>
#include <imwrapper/IMAccountListJSONSerializer.h>		//VOXOX - JRT - 2009.09.08 
#include <thread/Thread.h>
#include <thread/ThreadEvent.h>

#include <util/File.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/VoxMd5.h>
#include <util/VoxApiTimer.h>

#include <exception>

//#ifdef _WIN32
//#include <windows.h>	//JRT-XXX
//#endif

using namespace std;

const std::string UserProfile::s_securityKey = "";

//VOXOX - JRT - 2009.07.12 - TODO: Use of _activePhoneLine ptr is problematic.  This should
//	be mangaged and maintained by the PhoneLineList class and accessed via an 
//	accessor class.  This will be VERY important when we support multiple lines.
//
//-----------------------------------------------------------------------------

UserProfile::UserProfile() 
{
	_wsSms		     = NULL;
	_wsInfo			 = NULL;
	_wsTranslation	 = NULL;//VOXOX - CJC - 2010.01.17 
	_wsTranslationLanguages	 = NULL;//VOXOX - CJC - 2010.01.17 
	_wsContactNumber = NULL;
	_wsCallForward	 = NULL;
	_wsSoftUpdate	 = NULL;
	_wsAccount		 = NULL;
	_wsContactProfile = NULL;

	_activePhoneLine = NULL;
	_activePhoneCall = NULL;
	_sipAccount		 = NULL;
	_loginInvisible  = false;
	_needUserNetworks = true;	//VOXOX - JRT - 2009.07.22 
	_md5			 = "";
	_destroying		 = false;	//VOXOX - JRT - 2009.09.21 

	_imAccountManager		= new IMAccountManager();
	_imContactListHandler	= new IMContactListHandler(*this);
	_connectHandler			= new ConnectHandler(this);
	_presenceHandler		= new PresenceHandler(*this);
	_chatHandler			= new ChatHandler(*this);
	_contactList			= new ContactList(*this);
	_coIpManager			= new CoIpManager(*this);

	_sipAccountMustConnectAfterInit = false;
	_sipAccountReadyToConnect		= false;
	_wenboxPlugin					= NULL;
	_historyLoaded					= false;

	// Settings Avatar to default
	Profile::setIcon( AvatarList::getInstance().getDefaultAvatarPicture() );	//VOXOX - JRT - 2009.08.17 - do NOT use UserProfile::setIcon().  It updates the server!  Bad side-effect.

	_history     = new History(*this);
	_fileStorage = new UserProfileFileStorage( *this );

	_apiTimer = NULL;
}

UserProfile::~UserProfile() 
{
//	LOG_INFO("Destroying UserProfile");
	RecursiveMutex::ScopedLock lock(_mutex);
	_destroying = true;	//VOXOX - JRT - 2009.09.21 

	// Disconnect events to prevent some crashes (call on deleted objects)
	if (_sipAccount) 
	{
		_sipAccount->loginStateChangedEvent			   -= boost::bind(&UserProfile::loginStateChangedEventHandler,			  this, _1, _2, _3);//VOXOX CHANGE by Rolando - 2009.09.10 
		_sipAccount->networkDiscoveryStateChangedEvent -= boost::bind(&UserProfile::networkDiscoveryStateChangedEventHandler, this, _1, _2);
	}

	// Prevent some synchronization crashes by unregistering to these Events.
	_connectHandler->unregisterAllEvents();
	_imContactListHandler->unregisterAllEvents();
	_chatHandler->unregisterAllEvents();
	_presenceHandler->unregisterAllEvents();

	OWSAFE_DELETE(_activePhoneCall);
	OWSAFE_DELETE(_wsSms);
	OWSAFE_DELETE(_wsSoftUpdate);
	OWSAFE_DELETE(_wsInfo);
	OWSAFE_DELETE(_wsTranslation);//VOXOX - CJC - 2010.01.17 
	OWSAFE_DELETE(_wsTranslationLanguages);//VOXOX - CJC - 2010.01.17 
	OWSAFE_DELETE(_wsContactNumber);
	OWSAFE_DELETE(_wsCallForward);
	OWSAFE_DELETE(_wsAccount);
	OWSAFE_DELETE(_wsContactProfile);

	OWSAFE_DELETE(_history);
	OWSAFE_DELETE(_fileStorage);
	OWSAFE_DELETE(_wenboxPlugin);

	OWSAFE_DELETE(_coIpManager);
	OWSAFE_DELETE(_contactList);
	OWSAFE_DELETE(_imContactListHandler);
	OWSAFE_DELETE(_chatHandler);
	OWSAFE_DELETE(_presenceHandler);

	disconnect();

	if (_activePhoneLine) 
	{
		Thread::sleep(5); // Sleep for PhApi: let's time to it to unregister
		_activePhoneLine->getSipWrapper().terminate();
	}

//	LOG_INFO("Clearing _phoneList");
	_phoneLineList.clear();

	OWSAFE_DELETE(_activePhoneLine);
	OWSAFE_DELETE(_connectHandler);
	OWSAFE_DELETE(_imAccountManager);

	OWSAFE_DELETE( _apiTimer );

	// HACK haaaack if _sipAccount is detroy when user try to
	//	connect with a bad password the softphone freeze....
	if (_sipAccount && (_sipAccount->getLastLoginState() != EnumSipLoginState::SipLoginStatePasswordError)) 
{
		OWSAFE_DELETE(_sipAccount);
	}
	////
}

void UserProfile::init() {
	initApiTimer();

	_wenboxPlugin = new WenboxPlugin(*this);

	// Emits IMAccountManager::imAccountAddedEvent
	_imAccountManager->emitIMAccountAddedEvent();

	// Connect sip account
	_sipAccountMustConnectAfterInit = true;
	sipAccountInit();

	// IMAccounts are now connected later, when SIPAccount is OK.
}

std::string UserProfile::getProfileDirectory() const {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	return File::convertPathSeparators(config.getConfigDir() + "profiles/" + _name + "/");
}

void UserProfile::connect() 
{
	connectIMAccounts();
}

void UserProfile::connectIMAccounts() 
{
	setInitialPresence();	//VOXOX - JRT - 2009.09.15 

	// Connects all IMAccounts
	// This will also connect the Wengo IMAccount. This will actually
	// call UserProfile::connectSipAccounts after being through the Connect class.
	IMAccountList imAccountList = _imAccountManager->getIMAccountListCopy();
	for (IMAccountList::const_iterator it = imAccountList.begin(); it != imAccountList.end(); ++it) 
	{
		if ( (it->second).shouldConnect() )		//VOXOX - JRT - 2009.09.08 
		{
			_connectHandler->connect((it->second).getKey());
		}
	}
}

//VOXOX - JRT - 2009.09.15 - This can be expanded to set ANY initial presence state.
void UserProfile::setInitialPresence()
{
	static bool alreadySet = false;

//	if ( ! alreadySet )
	{
		if ( mustLoginInvisible() )
		{
			_imAccountManager->setInitialPresence( EnumPresenceState::PresenceStateInvisible );
		}

		alreadySet = true;
	}
}

void UserProfile::addVoxOxIMAccount()
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string jabberServer = config.getJabberVoxoxServer();

	IMAccount imAccountVoxoxJabber( _sipAccount->getDisplayName()+"@"+jabberServer, _sipAccount->getImPassword(), EnumIMProtocol::IMProtocolJabber,true);	//VOXOX - JRT - 2009.10.22 - SIP/Jabber password separation.

	//VOXOX - JRT - 2009.10.22 - In case user change Jabber password on another PC, we need to correct this Voxox/Jabber account.
	if(_imAccountManager->containsVoxOxAccount())
	{
		IMAccount* imAccount = _imAccountManager->getIMAccountNoClone( imAccountVoxoxJabber );
		
		if ( imAccount )
		{
			imAccount->setPassword( _sipAccount->getImPassword() );
		}
	}
	else
	{
		// Init _imAccount parameters
		initImAccountParameters( &imAccountVoxoxJabber );

		IMAccountParameters& params = imAccountVoxoxJabber.getIMAccountParameters();

		params.set(IMAccountParameters::JABBER_USE_TLS_KEY,				false);
		params.set(IMAccountParameters::JABBER_REQUIRE_TLS_KEY,			false);
		params.set(IMAccountParameters::JABBER_USE_OLD_SSL_KEY,			false);
		params.set(IMAccountParameters::JABBER_AUTH_PLAIN_IN_CLEAR_KEY, false);
		params.set(IMAccountParameters::JABBER_CONNECTION_SERVER_KEY,	jabberServer);
		params.set(IMAccountParameters::JABBER_PORT_KEY,				5222);

		addIMAccount( imAccountVoxoxJabber );
	}
}


void UserProfile::initImAccountParameters( IMAccount* imAccount )
{
	std::string imAccountParametersData = UserProfileFileStorage::loadSystemIMAccountParametersData();
	IMAccountParametersXMLSerializer serializer(imAccount->getIMAccountParameters());
	serializer.unserializeSystemSettings(imAccountParametersData);
}

bool UserProfile::connectSipAccounts() {
	RecursiveMutex::ScopedLock lock(_mutex);

	// This method is called by the Connect class.

	// Connect the SipAccount if not connected
        LOG_DEBUG ("SIP_ In connectSipAccounts");
	if (_activePhoneLine && !_sipAccount->isConnected()) 
	{
		loginStateChangedEvent(*_sipAccount,EnumSipLoginState::SipLoginStateProgress );

		if (!_activePhoneLine->connect()) {
			LOG_ERROR("Couldn't connect phoneline");
			return false;
		}
		
		LOG_DEBUG("SIP connection OK");
		return true;
	}
	return false;
	////
}

void UserProfile::disconnect() {
	disconnectIMAccounts();
}

void UserProfile::disconnectIMAccounts() {
	IMAccountList imAccountList = _imAccountManager->getIMAccountListCopy();
	for (IMAccountList::const_iterator it = imAccountList.begin();
		it != imAccountList.end(); ++it) {
		_connectHandler->disconnect((it->second).getKey());
	}
}

void UserProfile::disconnectSipAccounts(bool force) {
	RecursiveMutex::ScopedLock lock(_mutex);

	// This method is called by the Connect class.
	if (_activePhoneLine && _sipAccount->isConnected()) {
		_activePhoneLine->disconnect(force);
	}
}

EnumMakeCallError::MakeCallError UserProfile::makeCall(Contact & contact) {
	
	//VOXOX CHANGE CJC - If Skype contact, call it through chat handler.
	IMContactSet imContactSet;
	IMContact * imContact = contact.getPreferredIMContact();

	if (imContact) {
		if(imContact->getProtocol() == EnumIMProtocol::IMProtocolSkype){	
			imContactSet.insert(*imContact);
			_chatHandler->callSkypeContact(imContact->getIMAccountId(), imContactSet);
			return EnumMakeCallError::NoError;
		}
	}

	if (_activePhoneLine) {
		return _activePhoneLine->makeCall(contact.getPreferredNumber());
	} else {
		return EnumMakeCallError::NotConnected;
	}
}

EnumMakeCallError::MakeCallError UserProfile::makeCall(const std::string & phoneNumber) {
	if (_activePhoneLine) {
		return _activePhoneLine->makeCall(phoneNumber);
	} else {
		return EnumMakeCallError::NotConnected;
	}
}

//VOXOX - CJC - 2009.06.03 
void UserProfile::initChat() {
	_chatHandler->initChat();
}


void UserProfile::startIM(Contact & contact) {
	IMContactSet imContactSet;
	IMContact * imContact = contact.getPreferredIMContact();

	if (imContact) 
	{
		std::string temp = "";
		imContactSet.insert(*imContact);
		_chatHandler->createSession(imContact->getIMAccountId(), imContactSet, IMChat::Chat, temp );
	} 
	else 
	{
		LOG_ERROR("There is no IMContact available");
	}
}
//VOXOX - CJC - 2009.05.07 Start IM To Email
void UserProfile::startIMToEmail(std::string & email) {
	IMContactSet imContactSet;
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	IMContact * askContact =  new IMContact(*getVoxOxAccount(),config.getAskIMVoxOxComAddress());
	//VOXOX - CJC - 2009.06.11 
	
	imContactSet.insert(*askContact);
	_chatHandler->createChatToEmailSession(askContact->getIMAccountId(), imContactSet,email);
	
}
//VOXOX - CJC - 2009.05.19 StartIM to sms
void UserProfile::startIMToSMS(std::string & number) {
	IMContactSet imContactSet;
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	IMContact * askContact =  new IMContact(*getVoxOxAccount(),config.getAskIMVoxOxComAddress());

	imContactSet.insert(*askContact);
	_chatHandler->createChatToSMSSession(askContact->getIMAccountId(), imContactSet,number);
	
}

//VOXOX CHANGE CJ TO BE ABLE TO START A IM AND SEND A MESSAGE AT THE SAME TIME
void UserProfile::startIMAndSendMessage(Contact & contact,const std::string & message) {
	IMContactSet imContactSet;
	IMContact * imContact = contact.getPreferredIMContact();

	if (imContact) {
		imContactSet.insert(*imContact);
		_chatHandler->createSessionAndSendMessage(imContact->getIMAccountId(), imContactSet, message);
	} else {
		LOG_ERROR("There is no IMContact available");
	}
}

//VOXOX CHANGE CJC SEND FILE TO CONTACT
void UserProfile::sendFile(Contact & contact) {
	IMContact * imContact = contact.getPreferredIMContact();

	if (imContact) {
		_chatHandler->createSendFile(imContact->getIMAccountId(), *imContact);
	} else {
		LOG_ERROR("There is no IMContact available");
	}
}

//VOXOX CHANGE CJC SEND FILE TO CONTACT
void UserProfile::sendFax(std::string & faxNumber) {
	
	_chatHandler->createSendFax(faxNumber);
}
//VOXOX CHANGE CJC OPEN CONTACT PROFILE ONCHAT
void UserProfile::openContactProfile(Contact & contact) {
	IMContact * imContact = contact.getPreferredIMContact();

	if (imContact) {
		_chatHandler->createContactProfile(imContact->getIMAccountId(), *imContact);
	} else {
		LOG_ERROR("There is no IMContact available");
	}
}

//-----------------------------------------------------------------------------

void UserProfile::inviteToGroupChat ( const std::string& userChatRoomName, const std::string& imAccountIdIn, const IMContactSet& imContactSet )
{
	std::string imAccountId = imAccountIdIn;

	if ( imAccountId.empty() )
	{
		imAccountId = imContactSet.getFirst()->getIMAccount()->getKey();
	}

	_chatHandler->createSession( imAccountId, imContactSet, IMChat::ChatGroup, userChatRoomName );
}

void UserProfile::setSipAccount(const SipAccount & sipAccount, bool needInitialization) {
	if (_sipAccount) {
		IPhoneLine * phoneLine = findWengoPhoneLine();
		if (phoneLine) {
			phoneLine->disconnect();
			//TODO remove the PhoneLine from _phoneLines & destroy it
		}

		std::vector<EnumIMProtocol::IMProtocol> protocols;
		protocols.push_back(EnumIMProtocol::IMProtocolWengo);
		protocols.push_back(EnumIMProtocol::IMProtocolSIP);
		protocols.push_back(EnumIMProtocol::IMProtocolSIPSIMPLE);
		IMAccountList imAccountList = _imAccountManager->getIMAccountsOfProtocolVector(protocols);
		if (imAccountList.size() > 0) 
		{
			_removeIMAccount( (imAccountList.begin()->second) );
		}
		OWSAFE_DELETE(_sipAccount);
	}

	if (sipAccount.getType() == SipAccount::SipAccountTypeWengo) {
		const WengoAccount & wengoAccount = dynamic_cast<const WengoAccount &>(sipAccount);
		//Empty login or password
		if (wengoAccount.getWengoLogin().empty() || wengoAccount.getWengoPassword().empty()) {
			loginStateChangedEvent(*_sipAccount, EnumSipLoginState::SipLoginStatePasswordError);
			return;
		}

		_sipAccount = new WengoAccount(wengoAccount);
	} else {
		if (sipAccount.getIdentity().empty()) {
			loginStateChangedEvent(*_sipAccount, EnumSipLoginState::SipLoginStatePasswordError);
			return;
		}

		_sipAccount = new SipAccount(sipAccount);
	}

	computeName();

	_sipAccount->networkDiscoveryStateChangedEvent	+= boost::bind(&UserProfile::networkDiscoveryStateChangedEventHandler, this, _1, _2);
	_sipAccount->loginStateChangedEvent				+= boost::bind(&UserProfile::loginStateChangedEventHandler,			   this, _1, _2, _3);//VOXOX CHANGE by Rolando - 2009.09.10 

	if (needInitialization) {
		// Discovers network.
		// Sends the HTTP request to the SSO if _sipAccount is a Wengo account.
		_sipAccountMustConnectAfterInit = false;
		_sipAccountReadyToConnect = false;
		_sipAccount->init();
	}
}

void UserProfile::addIMAccount(const IMAccount & imAccount) {
	/*
	This code is not used anymore because of ConfigImporter update.
	if (imAccount.getProtocol() == EnumIMProtocol::IMProtocolWengo) {
		LOG_FATAL("cannot add directly a Wengo IMAccount. Must use setWengoAccount");
		return;
	}
	*/

	_addIMAccount(imAccount);
}

void UserProfile::_addIMAccount(const IMAccount & imAccount) {
	LOG_DEBUG("adding an IMAccount");

	_imAccountManager->addIMAccount(imAccount);

	if ( _wsAccount )
	{
		if ( imAccount.getUserNetworkId() == 0 )	//VOXOX - JRT - 2009.06.09 
		{		
			if ( imAccount.needsSyncing() )			//VOXOX - JRT - 2009.07.14 
			{
				_wsAccount->setUserNetwork( imAccount );
				const_cast<IMAccount&>(imAccount).setNeedsSyncing( false );
			}
		}
		//TODO: what about serverNetworkId?
	}
}

//VOXOX CHANGE by Rolando - 2009.08.28 
void UserProfile::_updateIMAccountLogin(std::string newLogin, IMAccount * imAccount){
	LOG_DEBUG("updating login in IMAccount");

	if (_imAccountManager->contains(*imAccount ))
	{
		_imAccountManager->updateIMAccountLogin(newLogin, imAccount);
	} 
	else 
	{
		LOG_ERROR("IMAccount not in IMAccountManager");
	}
}

void UserProfile::removeIMAccount(const IMAccount & imAccount) {
	if (imAccount.getProtocol() == EnumIMProtocol::IMProtocolWengo) {
		LOG_FATAL("cannot remove directly a Wengo IMAccount. Must use setWengoAccount");
		return;
	}

	_removeIMAccount(imAccount);
}

void UserProfile::_removeIMAccount(const IMAccount & imAccount) {
	LOG_DEBUG("removing an IMAccount");

	if (_imAccountManager->contains(imAccount )) 	//VOXOX - JRT - 2009.04.24 - Let acctMgr determine best method of locating account to remove.
	{
		_wsAccount->removeUserNetwork( imAccount.getUserNetworkId() );	//VOXOX - JRT - 2009.06.09 - TODO: event handler?

		_imAccountManager->removeIMAccount(imAccount);
	} 
	else 
	{
		LOG_ERROR("IMAccount not in IMAccountManager");
	}
}

void UserProfile::addFriendsFromMySpace(const IMAccount & imAccount) {
	
	_addFriendsFromMySpace(imAccount);
}

void UserProfile::_addFriendsFromMySpace(const IMAccount & imAccount) {
	LOG_DEBUG("adding friend from myspace server");

	if (_imAccountManager->contains( imAccount )) 
	{
		_imAccountManager->addFriendsFromMySpace(imAccount);
	} 
	else 
	{
		LOG_ERROR("IMAccount not in IMAccountManager");
	}
}

void UserProfile::updateIMAccount(const IMAccount& imAccount) 
{
	LOG_DEBUG("updating IMAccount");

	//VOXOX - JRT - 2009.09.14 - We want to avoid calling updateIMAccount twice so...
	bool force = imAccount.needsSyncing();
	const_cast<IMAccount&>(imAccount).setNeedsSyncing( false );

	_imAccountManager->updateIMAccount(imAccount);

	syncIMAccount( imAccount, force );
}

//VOXOX - JRT - 2009.08.03 - Create separate method for better control.
void UserProfile::requestSyncData( const IMAccount& imAccount ) 
{
	if ( imAccount.isVoxOxAccount() )
	{
		if ( _wsAccount )
		{
			if ( needUserNetworks() )
			{
				_needUserNetworks = false;	//VOXOX - JRT - 2009.07.22 - Do this early to prevent duplicate requests.

				_wsAccount->getUserNetworks();
			}
		}

		if ( _wsContactProfile )
		{
			_wsContactProfile->getUserProfile( getTimestamp(), getIcon().getMd5() );
		}
	}
	else		//VOXOX - JRT - 2009.07.14 - Sync with server.
	{
		syncIMAccount( imAccount, false );
	}
}

void UserProfile::syncIMAccount( const IMAccount& imAccount, bool force )
{
	if ( imAccount.shouldSync() || force )
	{
		saveAccountList();							//Local cache
		_wsAccount->setUserNetwork( imAccount );	//Server

		const_cast<IMAccount&>(imAccount).setNeedsSyncing( false );
	}
}

void UserProfile::syncWithServer() 
{
	if ( _wsContactProfile )
	{
		_wsContactProfile->setUserProfile( *this );
	}
}

//VOXOX CHANGE by Rolando - 2009.05.22 - sets if user checked login with a invisible presence
void UserProfile::setLoginInvisible(bool loginInvisible){
	_loginInvisible = loginInvisible;
}

bool UserProfile::mustLoginInvisible(){
	return _loginInvisible;
}

EnumPresenceState::PresenceState UserProfile::getPresenceState() const 
{
	return _imAccountManager->getIMAccountListConst().getPresenceState();
}

EnumPresenceState::PresenceState UserProfile::getIMPresenceState() const {
	return _imAccountManager->getIMAccountListConst().getIMPresenceState();
}

EnumPresenceState::PresenceState UserProfile::getVoxOxIMPresenceState() const {
	return _imAccountManager->getIMAccountListConst().getVoxOxIMPresenceState();
}

EnumPresenceState::PresenceState UserProfile::getSIPPresenceState() const {
	return _imAccountManager->getIMAccountListConst().getSIPPresenceState();
}

IMAccount* UserProfile::getVoxOxAccount() const {
	return _imAccountManager->getIMAccountListConst().getVoxOxAccount();
}

//VOXOX - JRT - 2009.08.27 
void UserProfile::setPresenceState(EnumPresenceState::PresenceState presenceState, std::string imAccountId, bool save, std::string statusMessage) 
{
	//VOXOX - JRT - 2009.09.21 - We have COE thru here, so let's avoid it.
	if ( !isDestroying() )	//VOXOX - JRT - 2009.09.21 
	{
		typedef ThreadEvent4<void (EnumPresenceState::PresenceState, std::string, bool, std::string ), EnumPresenceState::PresenceState, std::string, bool, std::string > MyThreadEvent;
		MyThreadEvent* event = new MyThreadEvent(boost::bind(&UserProfile::setPresenceStateThreadSafe, this, _1, _2, _3, _4), presenceState, imAccountId, save, statusMessage );
		WengoPhone::getInstance().postEvent(event);
	}
}

//VOXOX - JRT - 2009.08.27 
void UserProfile::setPresenceStateThreadSafe( EnumPresenceState::PresenceState presenceState, std::string imAccountId, bool save, std::string statusMessage) 
{
	IMAccount*  realIMAccount = _imAccountManager->getIMAccount(imAccountId);

	if (realIMAccount) 
	{
		if ( presenceState == EnumPresenceState::PresenceStateOffline )	//VOXOX - JRT - 2009.09.14 
		{
			_connectHandler->disconnect(imAccountId);

			if ( save )
			{
				realIMAccount->updateInitialPresenceState( presenceState );	//VOXOX - JRT - 2009.09.14 
				updateIMAccount(*realIMAccount);
			}
		}
		else
		{
			//VOXOX - JRT - 2009.09.10 - Moved logic to IMAccount/QtEnumIMProtocol.
			std::string realStatusMessage = realIMAccount->getUseableStatusMsg( statusMessage );

			//VOXOX - JRT - 2009.09.10 - Moved logic to IMAccount
			if ( realIMAccount->isOkToChangePresence( presenceState ) )
			{
				if (!realIMAccount->isConnected()) 
				{
					_connectHandler->connect(imAccountId);
				}

				_presenceHandler->changeMyPresenceState(presenceState, realStatusMessage, realIMAccount, save);
			}
		}

		OWSAFE_DELETE(realIMAccount);
	}
}

void UserProfile::setAllPresenceState(EnumPresenceState::PresenceState presenceState) {
	if (!isConnected()) {
		connect();
	}

	_presenceHandler->changeMyPresenceState(presenceState, String::null, NULL, false);	//VOXOX - JRT - 2009.09.14 TODO?
}

void UserProfile::setAlias(const string & alias, IMAccount * imAccount) {
//	_alias = alias;
//	_presenceHandler->changeMyAlias(_alias, imAccount);
	Profile::setAlias( alias );	//VOXOX - JRT - 2009.05.22 
	_presenceHandler->changeMyAlias( getAlias(), imAccount);
}

//VOXOX CHANGE CJC STATUS MESSAGE SUPPOT
void UserProfile::setStatusMessage(const string & statusMessage, IMAccount * imAccount) {
//	_statusMessage = statusMessage;
//	_presenceHandler->changeMyStatusMessage(_statusMessage, imAccount);
	
	//Profile::setStatusMessage( statusMessage );	//VOXOX - JRT - 2009.05.22 //VOXOX CHANGE by Rolando - 2009.06.28 
	//_presenceHandler->changeMyStatusMessage( getStatusMessage(), imAccount);//VOXOX CHANGE by Rolando - 2009.06.28 

	if(imAccount == NULL){//VOXOX CHANGE by Rolando - 2009.06.29 
		Profile::setStatusMessage( statusMessage );//VOXOX CHANGE by Rolando - 2009.06.29 
	}
	_presenceHandler->changeMyStatusMessage( statusMessage, imAccount);//VOXOX CHANGE by Rolando - 2009.06.28 
}

bool UserProfile::addPhoneLine(SipAccount & account) {
	//Creates new a PhoneLine associated with the account just added
	PhoneLine * phoneLine = new PhoneLine(account, *this);
	if (!phoneLine->init()) {
		delete phoneLine;		//VOXOX - JRT - 2009.07.11 - Fix memory leak.
		LOG_ERROR("PhoneLine initialisation failed");
		return false;
	}

	//Adds the PhoneLine to the list of PhoneLine
	_phoneLineList += phoneLine;

	//Sets the active PhoneLine
	//FIXME should the last added PhoneLine be the active one all the time?
	if (!_activePhoneLine) {
		_activePhoneLine = phoneLine;
	}

	return true;
}

void UserProfile::sipAccountInit() {
	if (_sipAccount) {
		_sipAccount->networkDiscoveryStateChangedEvent += boost::bind(&UserProfile::networkDiscoveryStateChangedEventHandler, this, _1, _2);
		_sipAccount->loginStateChangedEvent			   += boost::bind(&UserProfile::loginStateChangedEventHandler, this, _1, _2, _3);//VOXOX CHANGE by Rolando - 2009.09.10 

		// Detect network and launch SSO request if needed WengoAccount
		_sipAccountReadyToConnect = false;
		_sipAccount->init();

		generateMd5();		//VOXOX - JRT - 2009.06.05 
	}
}

//VOXOX CHANGE by Rolando - 2009.09.10 
void UserProfile::loginStateChangedEventHandler(SipAccount & sender, EnumSipLoginState::SipLoginState state, std::string loginErrorMessage) {
	// FIXME: due to a threads synchronization problem, it's better to comment this part of code temporarily. //VOXOX - JRT - 2009.09.16 Fixed.

	//VOXOX - JRT - 2009.09.16 - Log how long it took to detect network.
	if ( state == EnumSipLoginState::SipLoginStateReady )
	{
		logApiTimer( "Network detection took %7.5f seconds." );
	}

//	typedef ThreadEvent2<void (SipAccount sender, EnumSipLoginState::SipLoginState state), SipAccount, EnumSipLoginState::SipLoginState> MyThreadEvent;
//	MyThreadEvent * event = new MyThreadEvent(boost::bind(&UserProfile::loginStateChangedEventHandlerThreadSafe, this, _1, _2),
//								dynamic_cast<SipAccount&>(sender), state);

	typedef ThreadEvent3<void (SipAccount sender, EnumSipLoginState::SipLoginState state, std::string loginErrorMessage), SipAccount, EnumSipLoginState::SipLoginState, std::string> MyThreadEvent;
	MyThreadEvent * event = new MyThreadEvent(boost::bind(&UserProfile::loginStateChangedEventHandlerThreadSafe, this, _1, _2, _3),
								dynamic_cast<SipAccount&>(sender), state, loginErrorMessage);

	WengoPhone::getInstance().postEvent(event);

	//loginStateChangedEvent(sender, state);
	//loginStateChangedEventHandlerThreadSafe(sender, state);
}

//VOXOX CHANGE by Rolando - 2009.09.10 
void UserProfile::loginStateChangedEventHandlerThreadSafe(SipAccount & sender, EnumSipLoginState::SipLoginState state, std::string loginErrorMessage) 
{
	RecursiveMutex::ScopedLock lock(_mutex);

	LOG_DEBUG("SIP_ " + _name);

	switch (state) 
	{
	case EnumSipLoginState::SipLoginStateReady: 
		handleLoginStateReady( loginErrorMessage );	//VOXOX - JRT - 2009.09.16 - Move logic to separate method for readibility.
		break;

	case EnumSipLoginState::SipLoginStatePasswordError: {
		LOG_DEBUG("SIP_ PASSWORD ERROR");
		if (!_sipAccountReadyToConnect) {
			sipAccountConnectionFailedEvent(EnumSipLoginState::SipLoginStatePasswordError, loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
		}
		break;
	}

	case EnumSipLoginState::SipLoginStateConnected: {
		loadCachedData( getProfileDirectory() );	//VOXOX - JRT - 2009.04.23 - We need to load contacts and history.
		LOG_DEBUG("SIP_ CONNECTED");
		break;
	}

	case EnumSipLoginState::SipLoginStateDisconnected: {
		LOG_DEBUG("SIP_ DISCONNECTED");
		break;
	}

   case EnumSipLoginState::SipLoginStateNetworkError: {
		LOG_DEBUG("SIP_ NetworkError");
		sipAccountConnectionFailedEvent(EnumSipLoginState::SipLoginStateNetworkError, loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
 		break;
 	}

   case EnumSipLoginState::SipLoginStateUnActivatedError:
		LOG_DEBUG("SIP_ PASSWORD ERROR");
		if (!_sipAccountReadyToConnect) {
			sipAccountConnectionFailedEvent(EnumSipLoginState::SipLoginStateUnActivatedError, loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
		}
		break;

   case EnumSipLoginState::SipLoginStateOtherError:
		LOG_DEBUG("SIP_ SERVER CUSTOM ERROR");
		if (!_sipAccountReadyToConnect) {
			sipAccountConnectionFailedEvent(EnumSipLoginState::SipLoginStateOtherError, loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
		}
		break;
		
	default:
		;
	}
}

//VOXOX - JRT - 2009.09.16 - Factor out for readability.
void UserProfile::handleLoginStateReady( const std::string& loginErrorMessage )
{
	LOG_DEBUG("SIP_ READY");

	// if no sip account (generic or wengo) is set : create one !!
	IMAccountList imAccountList;
	if (_sipAccount->getType() == SipAccount::SipAccountTypeWengo) 
	{
		imAccountList = _imAccountManager->getIMAccountsOfProtocol(EnumIMProtocol::IMProtocolWengo);

		if (imAccountList.size() == 0) 
		{
			IMAccount imAccount(_sipAccount->getIdentity(), _sipAccount->getPassword(), EnumIMProtocol::IMProtocolWengo);
			imAccount.setConnected(false);
			_addIMAccount(imAccount);
		}
		else	//VOXOX CHANGE by Rolando - 2009.08.28 
		{
			IMAccount * imAccount = imAccountList.getFirstIMAccount();//VOXOX CHANGE by Rolando - 2009.08.28 

			if(imAccount->getProtocol() == EnumIMProtocol::IMProtocolWengo)	//VOXOX CHANGE by Rolando - 2009.08.28 
			{
				if(_sipAccount->getIdentity() != imAccount->getLogin())	//VOXOX CHANGE by Rolando - 2009.08.28 
				{
					_updateIMAccountLogin(_sipAccount->getIdentity(), imAccount);//VOXOX CHANGE by Rolando - 2009.08.28 
				}
			}
		}
	} 
	else 
	{
		std::vector<EnumIMProtocol::IMProtocol> protocols;

		protocols.push_back(EnumIMProtocol::IMProtocolSIP);
		protocols.push_back(EnumIMProtocol::IMProtocolSIPSIMPLE);

		imAccountList = _imAccountManager->getIMAccountsOfProtocolVector(protocols);

		if (imAccountList.size() == 0) 
		{
			IMAccount imAccount(_sipAccount->getIdentity(), _sipAccount->getPassword(), EnumIMProtocol::IMProtocolSIP);
			imAccount.setConnected(false);
			_addIMAccount(imAccount);
		}
	}
	
	createWsAccount();			//VOXOX - JRT - 2009.05.27 - This needs to be created before we add the VoxOx account.
	createWsContactProfile();	//VOXOX - JRT - 2009.06.10 
	generateMd5();

	//Create VoxOx Account
	addVoxOxIMAccount();
	
	if (_sipAccountMustConnectAfterInit) 
	{
		_sipAccountMustConnectAfterInit = false;

		if (!addPhoneLine(*_sipAccount)) 
		{
			LOG_ERROR("Couldn't add phoneline");
			sipAccountConnectionFailedEvent(EnumSipLoginState::SipLoginStateNetworkError, loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
			return;
		}

		if (_sipAccount->getType() == SipAccount::SipAccountTypeWengo) 
		{
			WengoAccount * wengoAccount = dynamic_cast<WengoAccount*>(_sipAccount);

			////Creates SMS, SMS needs a WengoAccount
			//_wsSms = new WsSms(wengoAccount, *this);
			//wsSmsCreatedEvent(*this, *_wsSms);
			//LOG_DEBUG("SMS created");

			/*
			//callforward
			/*_wsCallForward = new WsCallForward(wengoAccount);
			wsCallForwardCreatedEvent(*this, *_wsCallForward);
			_wsCallForward->wsCallForwardEvent += boost::bind(&UserProfile::wsCallForwardEventHandler, this, _1, _2, _3);*/

			//Creates SoftUpdate, SoftUpdate needs a WengoAccount
			_wsSoftUpdate = new WsSoftUpdate(wengoAccount, *this);	//VOXOX - JRT - 2009.10.27 
			wsSoftUpdateCreatedEvent(*this, *_wsSoftUpdate, false);	//VOXOX - JRT - 2009.10.27 
			LOG_DEBUG("SoftUpdate created");

			_wsInfo = new WsInfo(wengoAccount);
			wsInfoCreatedEvent(*this, *_wsInfo);
			LOG_DEBUG("WsInfo created");

			_wsTranslation = new WsTranslation(wengoAccount);//VOXOX - CJC - 2010.01.17 
			LOG_DEBUG("WsTranslation Created");
			_wsTranslationLanguages = new WsTranslationLanguages(wengoAccount, getSecurityKey() );//VOXOX - CJC - 2010.01.17 
			LOG_DEBUG("WsTranslationLanguages Created");
			_wsTranslationLanguages->getLanguages();//VOXOX - CJC - 2010.01.28 
			_wsContactNumber = new WsContactNumber();
			wsContactNumberCreatedEvent(*this, *_wsContactNumber);	//VOXOX - JRT - 2009.10.05 - TODO: No handler for this.  Did we lose it?
			LOG_DEBUG("ContactNumber created");
	
			//createWsAccount();	//VOXOX - JRT - 2009.06.02 - Moved up.
		}
	
		phoneLineCreatedEvent(*this, *_activePhoneLine);

//		loadHistory(getProfileDirectory());			//VOXOX - JRT - 2009.04.23 - Let's not load until we have Authentication.
		loadCachedData( getProfileDirectory() );	//  BUT!  If we do load here, load all the cached data.

		userProfileInitializedEvent(*this);		//JRT-XXX - New
	
		//Connect SipAccounts and IMAccounts;
		connect();
	}

	sipAccountConnectedEvent();

	_sipAccountReadyToConnect = true;
}


void UserProfile::networkDiscoveryStateChangedEventHandler(SipAccount & sender, SipAccount::NetworkDiscoveryState state) {
	networkDiscoveryStateChangedEvent(sender, state);
}

IPhoneLine * UserProfile::findWengoPhoneLine() {
	for (unsigned i = 0; i != _phoneLineList.size() ; i++) {
		const SipAccount & sipAccount = _phoneLineList[i]->getSipAccount();

		if (sipAccount.getType() == SipAccount::SipAccountTypeWengo) {
			return _phoneLineList[i];
		} else {
			LOG_DEBUG("this SipAccount is not a WengoAccount");
		}
	}
	return NULL;
}

void UserProfile::updateIcon(const OWPicture & icon, IMAccount * imAccount) {	//VOXOX - JRT - 2009.08.17 
	
	Profile::setIcon(icon);

	if ( icon.isValid() )	//VOXOX - JRT - 2009.08.17 
	{
		_presenceHandler->changeMyIcon(icon, imAccount);
	}
}

//VOXOX - JRT - 2009.04.23 - Internally, replace all references to _history with _fileStorage.
//	We retain load/saveHistory because UserProfileFileStorage uses it.  (I think we should move it there). TODO
bool UserProfile::loadCachedData(const std::string & path) 
{
	bool toReturn = true;

	if ( !isHistoryLoaded() )
	{
		VoxApiTimer apiTimer( "UserProfile::loadCachedData", "fileStorage::loadCachedData", false);	//VOXOX - JRT - 2009.09.07 
		apiTimer.Start();
		toReturn = _fileStorage->loadCachedData( path );
		apiTimer.Stop();

		_historyLoaded = true;
		historyLoadedEvent(*this, *_history);
	}

	return toReturn;
}

bool UserProfile::saveCachedData(const std::string & path) {
	return _fileStorage->save( path );
}

bool UserProfile::loadHistory(const std::string & path) {
	bool toReturn = _history->load(path + "history.xml");
	_historyLoaded = true;
	historyLoadedEvent(*this, *_history);
	return toReturn;
}


bool UserProfile::saveHistory(const std::string & path) {
	return _history->save(path + "history.xml");
}

bool UserProfile::saveAccountList()	//VOXOX - JRT - 2009.09.14 
{
	return _fileStorage->saveIMAccountList( getProfileDirectory() );
}

bool UserProfile::hasWengoAccount() const {
	return (hasSipAccount() && (_sipAccount->getType() == SipAccount::SipAccountTypeWengo));
}

bool UserProfile::hasSipAccount() const {
	return _sipAccount != NULL;
}

//VOXOX - JRT - 2009.05.21 - TODO: Is this used?
void UserProfile::wsCallForwardEventHandler(WsCallForward & sender,
	int id, WsCallForward::WsCallForwardStatus status) {

	if (status == WsCallForward::WsCallForwardStatusOk) {
		_wsInfo->getWengosCount(false);
		_wsInfo->getSmsCount(false);
		_wsInfo->getActiveMail(false);
		_wsInfo->getUnreadVoiceMail(false);
		_wsInfo->getLandlineNumber(false);
		_wsInfo->getCallForwardInfo(true);
		_wsInfo->execute();
	}
}

void UserProfile::computeName() {
	_name = _sipAccount->getFullIdentity();
}

bool UserProfile::isConnected() const {
	return _imAccountManager->getIMAccountListConst().isConnected();
}

bool UserProfile::isIMConnected() const {
	return _imAccountManager->getIMAccountListConst().isIMConnected();
}

bool UserProfile::isSIPConnected() const 
{
	return _imAccountManager->getIMAccountListConst().isSIPConnected();
}

bool UserProfile::isVoxOxConnected() const 
{
	return _imAccountManager->getIMAccountListConst().isVoxOxConnected();
}

void UserProfile::createWsAccount()
{
	if ( _wsAccount == NULL )
	{
		_wsAccount = new WsAccount( *this );

		_wsAccount->wsAccountFetchedEvent += boost::bind( &UserProfile::wsAccountFetchedEventHandler, this, _1, _2 );
		_wsAccount->wsAccountAddedEvent   += boost::bind( &UserProfile::wsAccountAddedEventHandler,   this, _1, _2 );
		_wsAccount->wsAccountRemovedEvent += boost::bind( &UserProfile::wsAccountRemovedEventHandler, this, _1, _2 );

//		wsAccountCreatedEvent(*this, *_wsAccount);		//Not needed just yet.
		LOG_DEBUG("WsAccount created");
	}
}

void UserProfile::wsAccountFetchedEventHandler( const WsAccount& sender, const IMAccountList& imAccountList )
{
	typedef ThreadEvent2<void (WsAccount sender, IMAccountList imAccountList), WsAccount, IMAccountList> MyThreadEvent;
	MyThreadEvent* event = new MyThreadEvent(boost::bind(&UserProfile::wsAccountFetchedEventHandlerThreadSafe, this, _1, _2), sender, imAccountList );
	WengoPhone::getInstance().postEvent(event);
}

void UserProfile::wsAccountFetchedEventHandlerThreadSafe( WsAccount sender, IMAccountList imAccountList )
{
	if ( imAccountList.GetCount() > 0 )
	{
		//VOXOX - JRT - 2009.07.22 - Do this in two passes to assure newly added IMContacts are not missed.
		IMAccountList::iterator it;

		for ( it = imAccountList.begin(); it != imAccountList.end(); it++ )
		{
			IMAccount& imAccount = (*it).second;

			if ( ! getIMAccountManager().contains( imAccount ) )
			{
				initImAccountParameters( &imAccount );
				imAccount.getIMAccountParameters().setByProtocol( imAccount.getProtocol(), imAccount.isGtalkAccount(), imAccount.getServer() );

				addIMAccount(imAccount);
			}
			else
			{
				wsAccountAddedEventHandler( sender, imAccount );	//Treat as new add so serverNetworkId is updated.
			}
		}

		//VOXOX - JRT - 2009.07.22 - Now update ContactList.
		for ( it = imAccountList.begin(); it != imAccountList.end(); it++ )
		{
			IMAccount& imAccount = (*it).second;

			getContactList().updateUserNetworkIds( imAccount );	//VOXOX - JRT - 2009.06.05 
		}

		connectIMAccounts();
	}
}

void UserProfile::wsAccountAddedEventHandler( const WsAccount& sender, const IMAccount& imAccount )
{
	typedef ThreadEvent2<void (WsAccount sender, IMAccount imAccount), WsAccount, IMAccount> MyThreadEvent;
	MyThreadEvent* event = new MyThreadEvent(boost::bind(&UserProfile::wsAccountAddedEventHandlerThreadSafe, this, _1, _2), sender, imAccount );
	WengoPhone::getInstance().postEvent(event);
}

void UserProfile::wsAccountAddedEventHandlerThreadSafe( const WsAccount sender, const IMAccount imAccount )
{
	//VOXOX - JRT - 2009.09.10 - TODO: Not yet from Server, so let's wait.
//	if ( !getIMAccountManager().setImAccountUserSelectedPresenceState( imAccount ) )
//	{
//		assert(false);
//	}

	if ( !getIMAccountManager().setImAccountUserNetworkId( imAccount ) )
	{
		assert(false);
	}

	//TODO: may not need once server-side is updated to return UserNetworkId.
	if ( !getIMAccountManager().setImAccountServerProtocolId( imAccount ) )
	{
		assert(false);
	}
}

void UserProfile::wsAccountRemovedEventHandler( const WsAccount& sender, int userNetworkId )
{
	typedef ThreadEvent2<void (WsAccount sender, int userNetworkId), WsAccount, int> MyThreadEvent;
	MyThreadEvent* event = new MyThreadEvent(boost::bind(&UserProfile::wsAccountRemovedEventHandlerThreadSafe, this, _1, _2), sender, userNetworkId );
	WengoPhone::getInstance().postEvent(event);
}

void UserProfile::wsAccountRemovedEventHandlerThreadSafe( const WsAccount sender, int userNetworkId )
{
	//TODO: we should only get here if it failed.  Most likely we have already removed it from local list.
}


void UserProfile::createWsContactProfile()
{
	if ( _wsContactProfile == NULL )
	{
		_wsContactProfile = new WsContactProfile( *this );

		_wsContactProfile->wsUserProfileSetEvent += boost::bind(&UserProfile::wsContactProfileSetEventHandler, this, _1, _2, _3 );
		_wsContactProfile->wsUserProfileGetEvent += boost::bind(&UserProfile::wsContactProfileGetEventHandler, this, _1, _2, _3 );

//		wsAccountCreatedEvent(*this, *_wsContactProfrile);		//Not needed just yet.
		LOG_DEBUG("WsContactProfile created");
	}
}


void UserProfile::wsContactProfileSetEventHandler( const WsContactProfile& sender, int id, int timestamp )
{
	setTimestamp( timestamp );
}

void UserProfile::wsContactProfileGetEventHandler( const WsContactProfile& sender, int id, const ProfileData& profile )
{
	RecursiveMutex::ScopedLock lock(_mutex);		//VOXOX - JRT - 2009.09.11 

	LOG_INFO( "Entered" );
	//ThreadSafe not needed since this is just updating profile
	//Let's be sure we have proper info from server.
	if ( *(this->getVoxOxAccount()) == const_cast<ProfileData&>(profile).getIMAccount() )
	{
		//We very likely have VoxOx number in UserProfile, but not in ProfileData, so merge the two.
		Telephones& phones = const_cast<ProfileData&>(profile).getTelephones();

		LOG_INFO( "Existing Phones: " + String::fromNumber( this->getTelephones().GetValidCount() ) );
		LOG_INFO( "New      Phones: " + String::fromNumber( const_cast<ProfileData&>(profile).getTelephones().GetValidCount() ) );
		phones.Merge( this->getTelephonesConst() );

		LOG_INFO( "Merged Phones: " + String::fromNumber( phones.GetValidCount() ) );

		dynamic_cast<Profile&>(*this) = profile;
		LOG_INFO( "UserProfile Phones: " + String::fromNumber( this->getTelephones().GetValidCount() ) );

		LOG_INFO( "Update icon" );
		updateIcon( profile.getIcon() );	//VOXOX - JRT - 2009.08.17 - Call directly so it gets synced with server and other IM networks.
//#ifdef _WIN32		//JRT-XXX
//	size_t   size = getIcon().getData().length();
//
//	if ( size < 12000 )
//		int xxx = 1;
//
//	char msg[200];
//	sprintf_s( msg, 200, "UserProfile::wsContactProfileGetEventHandler: data len = %d, account: %s\n\n", size, getIMAccount().getLogin().c_str() );
//	OutputDebugString( msg );
//#endif
	}

	LOG_INFO( "Exiting" );
}

std::string UserProfile::getMd5()
{
	if ( _md5.empty() )
	{
		generateMd5();
	}

	return _md5;
}

void UserProfile::generateMd5()
{
	const WengoAccount* wengoAccount = dynamic_cast<const WengoAccount*>(_sipAccount);

	if ( wengoAccount )
	{
		String input = wengoAccount->getWengoLogin() + wengoAccount->getWengoPassword();
		input = input.toLowerCase();

		VoxMd5 md5;

		_md5 = md5.toString( input.c_str(), input.size() );
	}
}

std::string UserProfile::contactListToJSON()
{
	ContactListJSONSerializer serializer( getContactList() );
	std::string result = serializer.serialize();

	return result;
}

std::string UserProfile::contactGroupSetToJSON()
{
	ContactGroupSetJSONSerializer serializer( getContactList().getContactGroupSetRef() );
	std::string result = serializer.serialize();

	return result;
}

std::string UserProfile::accountListToJSON()
{
	IMAccountList imAccountList = _imAccountManager->getIMAccountListCopy();

	IMAccountListJSONSerializer serializer( imAccountList );
	std::string result = serializer.serialize();
	
	return result;
}

std::string UserProfile::contactProfileToJSON( const int qtId )
{
	std::string result = "";
	Contact* contact = getContactList().getContactByJSONId( qtId );

	if ( contact )
	{
		ContactProfileJSONSerializer serializer( getContactList(), *contact );
		result = serializer.serialize();
	}

	return result;
}

std::string UserProfile::contactProfileToJSON( const std::string& contactId )
{
	std::string result = "";
	Contact* contact = getContactList().getContactById( contactId );

	if ( contact )
	{
		ContactProfileJSONSerializer serializer( getContactList(), *contact );
		result = serializer.serialize();
	}

	return result;
}

//VOXOX - CJC - 2009.07.30 To re-request a user autorization
void UserProfile::contactReRequestAuthorization( const std::string& contactId )
{
	Contact* contact = getContactList().getContactById( contactId );

	if ( contact )
	{
		if(contact->getPreferredIMContact()){

			_presenceHandler->reRequestAuthorization(*contact->getPreferredIMContact());
		}
	}
}

void UserProfile::toProfileData( ProfileData& profileData )
{
	profileData.copy( dynamic_cast<Profile&>(*this) );

	profileData.getIMAccount() = *getVoxOxAccount();
}

void UserProfile::initApiTimer()
{
	OWSAFE_DELETE( _apiTimer );

	_apiTimer = new VoxApiTimer( "UserProfile::init()", "<generic>", false );

	_apiTimer->Start();
}

void UserProfile::logApiTimer( const std::string& fmt )
{
	if ( _apiTimer )
	{
		char msg[500];

		sprintf( msg, fmt.c_str(), _apiTimer->GetCurrentSeconds() );
		LOG_INFO( msg );
	}
}

//-----------------------------------------------------------------------------
	
bool UserProfile::shouldSoftUpdate( SoftUpdateInfo& suInfo )
{
	bool update = false;
	
	if ( suInfo.isMandatory() )			//Always to mandatory updates.
	{
		update = true;
	}
	else if ( suInfo.isManualCheck() )	//If user did a manual check (Check For Updates menu), then do update
	{
		update = true;
	}
	else if ( suInfo.isRecommended() )	//Check preferences to determine if update should be done.
	{
		//If server indicates it is OK to download, then continue.  Otherwise, don't bother doing the download
		//	because server is too busy to handle it.
		if ( suInfo.downloadNow() )
		{
			//Check if user has chosen to ignore this version.
			SoftUpdatePrefs& prefs = getSoftUpdatePrefs();

			//User may choose to not do autoupdate in preferences
			if ( prefs.autoCheck() )
			{
				//Ignore this version?
				if ( prefs.getVersionToIgnore() == "" )
				{
					update = true;
				}
				else
				{
					//We don't want to ignore version forever, so we check the date last ignored and reprompt if necessary.
					if ( prefs.getVersionToIgnore() == suInfo.getVersion() )
					{
						Date today;
						int  daysDiff = Date::daysDiff( today, prefs.getDateLastIgnored() );

						//Less than zero indicates user may have manually changed XML, so force the update to discourage this.
						//Allow a little tolerance with -1.
						if ( daysDiff > 15 || daysDiff < -1 )
						{
							update = true;
						}
					}
					else
					{
						update = true;	//Different version.
					}
				}
			}
		}
	}

	return update;
}

//-----------------------------------------------------------------------------

void UserProfile::ignoreVersion( const SoftUpdateInfo& suInfo )			//VOXOX - JRT - 2009.10.30 
{
	SoftUpdatePrefs& prefs = getSoftUpdatePrefs();

	//Ignore this version
	prefs.setVersionToIgnore( suInfo.getVersion() );
	prefs.setDateLastIgnored( Date() );		//Today.
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2010.01.25 
std::string UserProfile::getTranslationLanguagesXML()
{
	std::string result = "";
	if(_wsTranslationLanguages){

		result = _wsTranslationLanguages->getLanguagesXML();

	}
	return result;
}
