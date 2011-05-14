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

#ifndef OWUSERPROFILE_H
#define OWUSERPROFILE_H

#include "Profile.h"
#include "IMAccountManager.h"

#include <imwrapper/EnumPresenceState.h>

#include <model/account/wengo/WengoAccount.h>
#include <model/history/History.h>
#include <model/profile/UserProfileFileStorage.h>
#include <model/profile/SoftUpdatePrefs.h>			//VOXOX - JRT - 2009.10.26 
#include <model/phoneline/IPhoneLine.h>
#include <model/webservices/callforward/WsCallForward.h>

#include <thread/Condition.h>
#include <thread/RecursiveMutex.h>

#include <util/Event.h>
#include <util/String.h>
#include <util/Trackable.h>
#include <util/VoxApiTimer.h>	//VOXOX - JRT - 2009.09.16 

#include <list>

class ChatHandler;
class CoIpManager;
class ConnectHandler;
class Contact;
class ContactList;
class ContactProfile;			//VOXOX - JRT - 2009.05.25 
class History;
class UserProfileFileStorage;	//VOXOX - JRT - 2009.04.23 
class IMAccount;
class IMAccountManager;
class IMContactListHandler;
class IPhoneLine;
class NetworkObserver;
class PhoneCall;
class PresenceHandler;
class WenboxPlugin;
class WsInfo;
class WsTranslationLanguages;//VOXOX - CJC - 2010.01.17 
class WsTranslation;//VOXOX - CJC - 2010.01.17 
class WsSms;
class WsContactNumber;
class WsSoftUpdate;
class WsAccount;				//VOXOX - JRT - 2009.05.27 
class WsContactProfile;			//VOXOX - JRT - 2009.06.10 

class SoftUpdateInfo;			//VOXOX - JRT - 2009.10.27 

/**
 * Handle the profile of a User.
 *
 * @author Philippe Bernery
 */
class UserProfile : public Profile, public Trackable {
	friend class UserProfileFileStorage;
	friend class UserProfileFileStorage1;
	friend class UserProfileXMLSerializer;
	friend class Connect;
public:
	/**
	 * Login procedure is done, event with the procedure result.
	 *
	 * @see SipAccount::loginStateChangedEvent
	 */
	Event<void (SipAccount & sender, EnumSipLoginState::SipLoginState state)> loginStateChangedEvent;
	
	Event<void (UserProfile & sender)> userProfileInitializedEvent;	//VOXOX - JRT - 2009.09.07 

	/**
	 * Network event while trying to connect a SipAccount.
	 *
	 * @see SipAccount::networkDiscoveryStateChangedEvent
	 */
	Event< void (SipAccount & sender, SipAccount::NetworkDiscoveryState state) > networkDiscoveryStateChangedEvent;

	/**
	 * Emitted when SipAccount has been connected
	 */
	Event<void () > sipAccountConnectedEvent;

	/**
	 * Emitted when SipAccount failed to connect
	 */
	Event<void (EnumSipLoginState::SipLoginState, std::string loginErrorMessage) > sipAccountConnectionFailedEvent;//VOXOX CHANGE by Rolando - 2009.09.10 

	/**
	 * The history has been loaded.
	 *
	 * @param sender this class
	 * @param History History loaded
	 */
	Event<void (UserProfile & sender, History & history)> historyLoadedEvent;

	/**
	 * A PhoneLine has been created.
	 *
	 * @param sender this class
	 * @param phoneLine PhoneLine created
	 */
	Event<void (UserProfile & sender, IPhoneLine & phoneLine)> phoneLineCreatedEvent;

	/**
	 * Sms has been created.
	 *
	 * @param sender this class
	 * @param sms Sms created
	 */
	Event<void (UserProfile & sender, WsSms & sms)> wsSmsCreatedEvent;

	/**
	 * SoftUpdate has been created.
	 *
	 * @param sender this class
	 * @param sms Sms created
	 */
	Event<void (UserProfile & sender, WsSoftUpdate & softUpdate, bool manualCheck )> wsSoftUpdateCreatedEvent;	//VOXOX - JRT - 2009.10.27 

	/**
	 * WsInfo has been created.
	 *
	 * @param sender this class
	 * @param wsInfo WsInfo created
	 */
	Event<void (UserProfile & sender, WsInfo & wsInfo)> wsInfoCreatedEvent;

	
   /**
	 * WsContactNumber has been created.
	 *
	 * @param sender this class
	 * @param wsInfo WsInfo created
	 */
	Event<void (UserProfile & sender, WsContactNumber & wsContactNumber)> wsContactNumberCreatedEvent;

	/**
	 * WsCallForward has been created.
	 *
	 * @param sender this class
	 * @param wsCallForward WsCallForward created
	 */
	Event<void (UserProfile & sender, WsCallForward & wsCallForward)> wsCallForwardCreatedEvent;


	UserProfile();

	virtual ~UserProfile();

	/**
	 * Initializes the UserProfile.
	 */
	void init();

	/**
	 * Return the profile directory
	 * @return profile directory.
	 */
	std::string getProfileDirectory() const;

	/**
	 * Connects all this UserProfile accounts (SipAccount and IMAccount) set
	 * to be automatically connected.
	 */
	void connect();

	/**
	 * Disconnects all accounts.
	 */
	void disconnect();

	/** List of IPhoneLine. */
//	typedef List < IPhoneLine * > PhoneLines;
	typedef ListEx < IPhoneLine * > PhoneLines;	//VOXOX - JRT - 2009.07.12 - Fix crash.

	/**
	 * Gets the list of PhoneLine.
	 *
	 * Only one PhoneLine is active at a time.
	 *
	 * @return the list of PhoneLine
	 */
	const PhoneLines & getPhoneLineList() const {
		return _phoneLineList;
	}

	/**
	 * Gets the active/current PhoneLine.
	 *
	 * Only one PhoneLine is active at a time
	 *
	 * @return the active PhoneLine
	 */
	IPhoneLine * getActivePhoneLine() const {
		return _activePhoneLine;
	}

	/**
	 * Gets the active phone call.
	 *
	 * Used for playing DTMF.
	 *
	 * @return active phone call or NULL
	 */
	PhoneCall * getActivePhoneCall() const;

	/** Gets the ConnectHandler reference. */
	ConnectHandler & getConnectHandler() { return *_connectHandler; }

	/** Gets the PresenceHandler reference. */
	PresenceHandler & getPresenceHandler() { return *_presenceHandler; }

	/** Gets the ChatHandler reference. */
	ChatHandler & getChatHandler() { return *_chatHandler; }

	/**
	 * Gets the IMAccountManager reference. Here we can safely derefence
	 * the pointer as the IMAccountHandler is constructed in
	 * the UserProfile constructor.
	 */
	IMAccountManager & getIMAccountManager() { return *_imAccountManager; }

	/** Gets the IMContactListHandler reference. */
	IMContactListHandler & getIMContactListHandler() { return *_imContactListHandler; }

	/** Gets the ContactList reference. */
	ContactList & getContactList() { return *_contactList; }

	/**
	 * Gets the History reference. Here we can safely derefence the pointer
	 * as the History is constructed in the UserProfile constructor.
	 */
	History & getHistory() { return *_history; }

	/** Gets the WenboxPlugin pointer. */
	WenboxPlugin * getWenboxPlugin() { return _wenboxPlugin; }

	/** Gets the WsInfo pointer. */
	WsInfo * getWsInfo() { return _wsInfo; }

	WsTranslation * getWsTranslation() { return _wsTranslation; }//VOXOX - CJC - 2010.01.17 
	WsTranslationLanguages * getWsTranslationLanguages() { return _wsTranslationLanguages; }//VOXOX - CJC - 2010.01.17 

	std::string getTranslationLanguagesXML();//VOXOX - CJC - 2010.01.28 
	/** Gets the WsInfo pointer. */
	WsContactNumber * getWsContactNumber() { return _wsContactNumber; }

	/** Gets the WengoPhone update WebService pointer. */
	WsSoftUpdate*		getWsSoftUpdate()			{ return _wsSoftUpdate; }
	SoftUpdatePrefs&	getSoftUpdatePrefs()		{ return _softUpdatePrefs;	}	//VOXOX - JRT - 2009.10.26 
	bool				shouldSoftUpdate( SoftUpdateInfo& suInfo );					//VOXOX - JRT - 2009.10.27 
	void				ignoreVersion   ( const SoftUpdateInfo& suInfo );			//VOXOX - JRT - 2009.10.30 

	/** Gets the SMS WebService pointer. */
	WsSms * getWsSms() { return _wsSms; }

	/** Gets the WsCallForward WebService pointer. */
	WsCallForward * getWsCallForward()		{ return _wsCallForward; }

	WsAccount*		getWsAccount()			{ return _wsAccount;	}

	/** Gets the CoIpManager. */
	CoIpManager & getCoIpManager() { return *_coIpManager; }

	/** True if history has been loaded. */
	bool isHistoryLoaded() const 		//VOXOX - JRT - 2009.04.23 - now refers to ALL cached data.
	{
		return _historyLoaded;
	}

	/**
	 * Loads the history.
	 */
	bool loadHistory(const std::string & path);
	bool loadCachedData(const std::string & path);	//VOXOX - JRT - 2009.04.23 - Contacts and history.

	/**
	 * Saves the history.
	 */
	bool saveHistory    (const std::string & path);	
	bool saveCachedData (const std::string & path);		//VOXOX - JRT - 2009.04.23 - TODO: Ensure this is NOT redundant with save elsewhere in code
	bool saveAccountList();								//VOXOX - JRT - 2009.09.14 

	/**
	 * Sets the SipAccount of this UserProfile.
	 *
	 * The SipAccount os copied internally.
	 *
	 * @param SipAccount the SipAccount to set
	 */
	void setSipAccount(const SipAccount & sipAccount, bool needInitialization = true);

	/**
	 * Return true if a WengoAccount is active.
	 *
	 * @return true if a WengoAccount is active
	 */
	bool hasWengoAccount() const;

	/**
	 * Return true if a SipAccount is active.
	 *
	 * @return true if a SipAccount is active
	 */
	bool hasSipAccount() const;

	/**
	 * Gets the SipAccount.
	 *
	 * @return the SipAccount
	 */
	SipAccount * getSipAccount() const {
		return _sipAccount;
	}

	/**
	 * Gets the WengoAccount.
	 *
	 * @return the WengoAccount
	 */
	WengoAccount * getWengoAccount() const 
	{
		if (hasWengoAccount()) 
		{
			return dynamic_cast<WengoAccount *>(_sipAccount);
		} 
		else 
		{
			return NULL;
		}
	}

	/**
	 * Adds an IMAccount to this UserProfile.
	 * This method should currently not be called to add a Wengo
	 * IMAccount. A Wengo IMAccount is created internally when setWengoAccount
	 * is called. There is an assert to prevent that.
	 *
	 * The IMAccount is copied internally.
	 *
	 * @param imAccount the IMAccount to add
	 */
	void addIMAccount(const IMAccount & imAccount);

	void addVoxOxIMAccount();
	/**
	 * Removes an IMAccount from this UserProfile.
	 *
	 * An IMAccount of type Wengo must currently not be removed by this method.
	 * setWengoAccount must be used for this purpose.
	 * There is an assert to prevent that.
	 *
	 * @param imAccount the IMAccount to remove
	 */
	void removeIMAccount(const IMAccount & imAccount);

	void addFriendsFromMySpace(const IMAccount & imAccount);

	/**
	 * Updates an IMAccount from this UserProfile.
	 *
	 * An IMAccount of type Wengo must currently not be removed by this method.
	 * setWengoAccount must be used for this purpose.
	 * There is an assert to prevent that.
	 *
	 * @param imAccount the IMAccount to remove
	 */
	void updateIMAccount(const IMAccount & imAccount);
	void requestSyncData(const IMAccount & imAccount);	//VOXOX - JRT - 2009.08.03 - Separate method for better control.
	void syncIMAccount ( const IMAccount& imAccount, bool force );	//VOXOX - JRT - 2009.09.14 
	void syncWithServer();								//VOXOX - JRT - 2009.08.03 

	//VOXOX CHANGE by Rolando - 2009.05.22 - sets if user checked login with a invisible presence
	void setLoginInvisible(bool loginInvisible);

	bool mustLoginInvisible();

	/**
	 * @see IPhoneLine::makeCall()
	 */
	EnumMakeCallError::MakeCallError makeCall(Contact & contact);

	/**
	 * @see IPhoneLine::makeCall()
	 */
	EnumMakeCallError::MakeCallError makeCall(const std::string & phoneNumber);

	/**
	 * Start a instant messaging with a Contact.
	 *
	 * @param contact the Contact to talk to
	 */
	//VOXOX - CJC - 2009.06.03 
	void initChat();

	void startIM	   (Contact & contact);
	void startIMToEmail(std::string & email);
	void startIMToSMS  (std::string & number);	
	void startIMAndSendMessage(Contact & contact,const std::string & message);

	void sendFile(Contact & contact);
	void sendFax (std::string & faxNumber);

	//VOXOX CHANGE CJC SEND OPENCONTACT PROFILE
	void openContactProfile(Contact & contact);

	void inviteToGroupChat( const std::string& chatRoom, const std::string& imAccountId, const IMContactSet& imContactSet );	//VOXOX - JRT - 2009.06.14 


	/**
	 * Changes alias of this user.
	 *
	 * @param alias the alias to set
	 * @param imAccount the IMAccount to apply the alias to;
	 *        pass NULL to set the alias to all IMAccount
	 */
	void setAlias(const std::string & alias, IMAccount * imAccount);

	///VOXOX CHANGE CJC STATUS MESSAGE SUPPOT
	void setStatusMessage(const std::string & statusMessage, IMAccount * imAccount);

	/**
	 * Changes icon of this user.
	 *
	 * @param icon the icon to set
	 * @param imAccount the IMAccount to apply the icon to
	 *        pass NULL to set the icon to all IMAccount
	 */
	void updateIcon(const OWPicture & icon, IMAccount * account);								//VOXOX - JRT - 2009.08.17 
	void updateIcon(const OWPicture & icon)						{ updateIcon(icon, NULL); };	//VOXOX - JRT - 2009.08.17 

	/**
	 * Gets the PresenceState of this UserProfile.
	 */
	EnumPresenceState::PresenceState getIMPresenceState() const;//VOXOX CHANGE Rolando 03-20-09

	/**
	 * Gets the PresenceState of this UserProfile.
	 */
	EnumPresenceState::PresenceState getSIPPresenceState() const;//VOXOX CHANGE Rolando 03-20-09

	/**
	 * Gets the PresenceState of VoxOx account.
	 */
	EnumPresenceState::PresenceState getVoxOxIMPresenceState() const;//VOXOX CHANGE Rolando 03-20-09


	/**
	 * Gets the PresenceState of this UserProfile.
	 */
	EnumPresenceState::PresenceState getPresenceState() const;

	/**
	 * Checks if at least one IMAccount is in connected state.
	 *
	 * @return true if at least 1 IMAccount is connected; false otherwise
	 */
	bool isConnected() const;

	bool isIMConnected() const;

	bool isSIPConnected() const;

	bool isVoxOxConnected() const;

	/**
	 * Changes the PresenceState of an IMAccount.
	 *
	 * @param presenceState the PresenceState to set
	 * @param imAccountId the id of the IMAccount to apply the PresenceState.
	 */
	//VOXOX CHANGE CJC ADD STATUS MESSAGE SUPPORT
	//void setPresenceState(EnumPresenceState::PresenceState presenceState, std::string imAccountId);
	void setPresenceState(EnumPresenceState::PresenceState presenceState, std::string imAccountId, bool save, std::string statusMessage );	//=String::null);

	/**
	 * Changes the PresenceState of all IMAccount.
	 *
	 * @param presenceState the PresenceState to set
	 */
	void setAllPresenceState(EnumPresenceState::PresenceState presenceState);

	/**
	 * Gets the name of this UserProfile.
	 *
	 * The name is computed from the SipAccount. "Default" is returned if no
	 * SipAccount is set. There should be only one Default UserProfile at
	 * a time.
	 */
	std::string getName() const {
		return _name;
	}


	IMAccount* getVoxOxAccount() const;
	void initImAccountParameters( IMAccount* imAccount );

	static std::string	getSecurityKey()			{ return s_securityKey;	}
	std::string			getMd5();

	std::string contactListToJSON();
	std::string contactGroupSetToJSON();
	std::string accountListToJSON();				//VOXOX - JRT - 2009.09.24 
	std::string contactProfileToJSON( const std::string& contactId );
	std::string contactProfileToJSON( const int qtId );			//VOXOX - JRT - 2009.09.23 

	void contactReRequestAuthorization( const std::string& contactId );//VOXOX - CJC - 2009.07.30  Re-request contact autorization

	WsContactProfile*	getWsContactProfile	()			{ return _wsContactProfile;	}	//VOXOX - JRT - 2009.08.03  
	void				toProfileData( ProfileData& profileData );

	bool				isDestroying()					{ return _destroying;		}	//VOXOX - JRT - 2009.09.21 
	void				lock()							{ _mutex.lock();			}
	void				unlock()						{ _mutex.unlock();			}


private:
	//VOXOX - JRT - 2009.06.02 
	void createWsAccount();
	bool needUserNetworks() const						{ return _needUserNetworks;	}	//VOXOX - JRT - 2009.07.22 


	void wsAccountFetchedEventHandler( const WsAccount& sender, const IMAccountList& imAccountList );
	void wsAccountAddedEventHandler  ( const WsAccount& sender, const IMAccount& imAccount );
	void wsAccountRemovedEventHandler( const WsAccount& sender, int userNetworkId );

	void wsAccountFetchedEventHandlerThreadSafe( WsAccount sender, IMAccountList imAccountList );
	void wsAccountAddedEventHandlerThreadSafe  ( WsAccount sender, const IMAccount imAccount );
	void wsAccountRemovedEventHandlerThreadSafe( WsAccount sender, int userNetworkId );

	void createWsContactProfile();

	void wsContactProfileSetEventHandler( const WsContactProfile& sender, int id, int timestamp );
	void wsContactProfileGetEventHandler( const WsContactProfile& sender, int id, const ProfileData& profile );
	//End VoxOx

	void generateMd5();
	void setInitialPresence();					//VOXOX - JRT - 2009.09.15 

	void initApiTimer();						//VOXOX - JRT - 2009.09.16 
	void logApiTimer( const std::string& fmt );	//VOXOX - JRT - 2009.09.16 

	void handleLoginStateReady( const std::string& loginErrorMessage );		//VOXOX - JRT - 2009.09.16 - Factor out for readability.


	// Inherited from Profile
	virtual void setWengoPhoneId(const std::string & /*wengoPhoneId*/) {}
	virtual std::string getWengoPhoneId() const { return String::null; }
	////


	void setPresenceStateThreadSafe(EnumPresenceState::PresenceState presenceState, std::string imAccountId, bool save, std::string statusMessage);	//VOXOX - JRT - 2009.08.27 

	/**
	 * @see WsCallForward::wsCallForwardEvent
	 */
	void wsCallForwardEventHandler(WsCallForward & sender, int id, WsCallForward::WsCallForwardStatus status);


	/**
	 * Handles SipAccount::loginStateChangedEvent.
	 */
	void loginStateChangedEventHandler          (SipAccount & sender, EnumSipLoginState::SipLoginState state, std::string loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 
	void loginStateChangedEventHandlerThreadSafe(SipAccount & sender, EnumSipLoginState::SipLoginState state, std::string loginErrorMessage);//VOXOX CHANGE by Rolando - 2009.09.10 

	/** 
	 * Handles SipAccount::networkDiscoveryStateChangedEvent
	 */
	void networkDiscoveryStateChangedEventHandler(SipAccount & sender, SipAccount::NetworkDiscoveryState state);

	/**
	 * Compute the name of the UserProfile from the SipAccount
	 * and set the _name variable.
	 */
	void computeName();
	/**/

	/**
	 * Actually adds an IMAccount. Used internally and by addIMAccount after
	 * checking the given IMAccount.
	 */
	void _addIMAccount(const IMAccount & imAccount);

	/**
	 * Actually removes an IMAccount. Used internally and by removeIMAccount after
	 * checking the given IMAccount.
	 */
	void _removeIMAccount(const IMAccount & imAccount);

	void _addFriendsFromMySpace(const IMAccount & imAccount);

	void _updateIMAccountLogin(std::string newLogin, IMAccount * imAccount);//VOXOX CHANGE by Rolando - 2009.08.28 

	/**
	 * Connect all IMAccounts.
	 */
	void connectIMAccounts();

	/**
	 * Disconnect all IMAccounts.
	 */
	void disconnectIMAccounts();

	/**
	 * Connect all SipAccounts.
	 */
	bool connectSipAccounts();

	/**
	 * Disconnect all SipAccounts.
	 *
 	 * @param force if true, disconnect without doing any transactions
	 */
	void disconnectSipAccounts(bool force = false);

	/**
	 * Initializes the SipAccount.
	 */
	void sipAccountInit();

	/**
	 * Creates and adds a new PhoneLine given a SipAccount.
	 *
	 * This is a helper method.
	 *
	 * @param account SipAccount associated with the newly created PhoneLine
	 */
	bool addPhoneLine(SipAccount & account);

	/**
	 * find the wengo phone line
	 */
	IPhoneLine * findWengoPhoneLine();

	/** The active/current PhoneLine. */
	IPhoneLine * _activePhoneLine;

	/** The active PhoneCall. */
	PhoneCall * _activePhoneCall;

	/** List of PhoneLine. */
	PhoneLines _phoneLineList;

	/** History. */
	History * _history;
	UserProfileFileStorage* _fileStorage;	//VOXOX - JRT - 2009.04.23 

	/** WengoPhone update system. */
	WsSoftUpdate*	_wsSoftUpdate;
	SoftUpdatePrefs	_softUpdatePrefs;		//VOXOX - JRT - 2009.10.26 

	/** SMS. */
	WsSms * _wsSms;

	/** WsInfo. */
	WsInfo * _wsInfo;

	WsTranslation * _wsTranslation;//VOXOX - CJC - 2010.01.17 
	WsTranslationLanguages * _wsTranslationLanguages;//VOXOX - CJC - 2010.01.17 
	/** WsVoxOxContactNumber. */
	WsContactNumber * _wsContactNumber;

	/** WsCallForward. */
	WsCallForward *		_wsCallForward;
	WsAccount*			_wsAccount;			//VOXOX - JRT - 2009.05.27 
	WsContactProfile*	_wsContactProfile;	//VOXOX - JRT - 2009.06.10 

	//TODO: create a list of SipAccount
	SipAccount * _sipAccount;

	/** Wenbox. */
	WenboxPlugin * _wenboxPlugin;

	RecursiveMutex _mutex;

	/**
	 * True if the UserProfile must connect after intialization of the SipAccount
	 */
	bool _sipAccountMustConnectAfterInit;

	bool _sipAccountReadyToConnect;

	bool _historyLoaded;

	IMAccountManager * _imAccountManager;

	IMContactListHandler * _imContactListHandler;

	ConnectHandler * _connectHandler;

	PresenceHandler * _presenceHandler;

	ChatHandler * _chatHandler;

	ContactList * _contactList;

	CoIpManager * _coIpManager;

	/** Name of the UserProfile. */
	std::string _name;

	bool	_loginInvisible;
	bool	_needUserNetworks;	//VOXOX - JRT - 2009.07.22 

	std::string		_md5;			//VOXOX - JRT - 2009.06.05 
	bool			_destroying;	//VOXOX - JRT - 2009.09.21 - avoid COE.
	VoxApiTimer*	_apiTimer;	

	static const std::string s_securityKey;	//VOXOX - JRT - 2009.06.11 - Temp until MD5 is working.
};

#endif	//OWUSERPROFILE_H
