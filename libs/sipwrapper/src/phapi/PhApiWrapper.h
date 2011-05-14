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

#ifndef OWPHAPIWRAPPER_H
#define OWPHAPIWRAPPER_H

#include "PhApiCallbacks.h"

#include "PhApiIMChat.h"

#include <sipwrapper/SipWrapper.h>

#include <imwrapper/EnumPresenceState.h>
#include <imwrapper/IMChat.h>
#include <imwrapper/IMPresence.h>
#include <imwrapper/IMChatSession.h>

#include <sound/AudioDevice.h>
#include <thread/Mutex.h>
#include <thread/Timer.h>
#include <util/Event.h>

#include <phapi.h>

#include <string>

class PhApiSFPWrapper;

/**
 * SIP wrapper for PhApi.
 *
 * PhApi is a SIP implementation developed in C.
 * PhApi is based on eXosip, oSIP and oRTP.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 * @author Julien Bossart
 */
class PhApiWrapper : public SipWrapper {
public:

	Event<void (PhApiWrapper & sender, IMChatSession & imChatSession)> newIMChatSessionCreatedEvent;

	Event<void (PhApiWrapper & sender, IMChatSession & imChatSession, const std::string & contactId)> contactAddedEvent;

	Event<void (PhApiWrapper & sender, IMChatSession & imChatSession, const std::string & contactId)> contactRemovedEvent;

	Event<void (PhApiWrapper & sender, IMChatSession & chatSession, const std::string & from, const std::string & message)> messageReceivedEvent;

	Event<void (PhApiWrapper & sender, IMChatSession & chatSession, PhApiIMChat::StatusMessage status, const std::string & message)> statusMessageReceivedEvent;

	Event<void (PhApiWrapper & sender, IMChatSession & chatSession, const std::string & contactId, IMChat::TypingState state)> typingStateChangedEvent;

	Event<void (PhApiWrapper & sender)> connectedEvent;

	Event<void (PhApiWrapper & sender, bool connectionError, const std::string & reason)> disconnectedEvent;

	Event<void (PhApiWrapper & sender, int currentStep, int totalSteps, const std::string & infoMessage)> connectionProgressEvent;

	Event<void (PhApiWrapper & sender, EnumPresenceState::PresenceState state, const std::string & note, const std::string & from)> presenceStateChangedEvent;

	Event<void (PhApiWrapper & sender, EnumPresenceState::MyPresenceStatus status, const std::string & note)> myPresenceStatusEvent;

	Event<void (PhApiWrapper & sender, const std::string & contactId, IMPresence::SubscribeStatus status)> subscribeStatusEvent;

	Event<void (PhApiWrapper & sender, const std::string & contactId, const std::string filename)> contactIconChangedEvent;


	/** Value when phApi returns an error message. */
	static const int PhApiResultNoError = 0;

	static PhApiWrapper * getInstance() {
		static PhApiWrapper instance(PhApiCallbacks::getInstance());
		return &instance;
	}

	~PhApiWrapper();

	void terminate();

	int addVirtualLine(const std::string & displayName,
		const std::string & username, const std::string & identity,
		const std::string & password, const std::string & realm,
		const std::string & proxyServer, const std::string & registerServer);

	int registerVirtualLine(int lineId);

	void removeVirtualLine(int lineId, bool force = false);

	int makeCall(int lineId, const std::string & sipAddress, bool enableVideo);

	void sendRingingNotification(int callId, bool enableVideo);

	void acceptCall(int callId, bool enableVideo);

	void rejectCall(int callId);

	void closeCall(int callId);

	void holdCall(int callId);

	void resumeCall(int callId);

	void blindTransfer(int callId, const std::string & sipAddress);

	void playDtmf(int callId, char dtmf);

	void playSoundFile(int callId, const std::string & soundFile);

	void setAudioCodecList(const StringList & audioCodecList);

	CodecList::AudioCodec getAudioCodecUsed(int callId);

	CodecList::VideoCodec getVideoCodecUsed(int callId);

	void setVideoDevice(const std::string & deviceName);

	bool setCallInputAudioDevice(const AudioDevice & device);

	bool setRingerOutputAudioDevice(const AudioDevice & device);

	bool setCallOutputAudioDevice(const AudioDevice & device);

	void enableAEC(bool enable);

	void enableHalfDuplex(bool enable);

	void enablePIM(bool enable);

	void setPluginPath(const std::string & path);

	void setProxy(const std::string & address, unsigned port,
		const std::string & login, const std::string & password);

	void setTunnel(const std::string & address, unsigned port, bool ssl);

	void setNatType(EnumNatType::NatType natType);

	void setVideoQuality(EnumVideoQuality::VideoQuality videoQuality);

	void setSIP(const std::string & server, unsigned serverPort, unsigned localPort);

	/**
	 * @see SipWrapper::setCallsEncryption
	 */
	virtual void setCallsEncryption(bool enable);

	/**
	 * @see SipWrapper::areCallsEncrypted
	 */
	virtual bool isCallEncrypted(int callId);

	virtual	bool isInitialized(void);

	/**
	 * @name Conference
	 * @{
	 */

	int createConference();

	void joinConference(int confId, int callId);

	void splitConference(int confId, int callId);

	/** @} */

	/**
	 * @name PhApi callbacks
	 * @{
	 */

	void init();

	/** @} */

	/**
	 * @name IMConnect Implementation
	 * @{
	 */

	void connect();

	void disconnect(bool force = false);

	/** @} */

	/**
	 * @name IMChat Implementation
	 * @{
	 */

	void sendMessage(IMChatSession & chatSession, const std::string & message);
	void changeTypingState(IMChatSession & chatSession, IMChat::TypingState state);
	void createSession(IMChat & imChat, const IMContactSet & imContactSet, IMChat::IMChatType imChatType, const std::string& userChatRoomName );
	void closeSession(IMChatSession & chatSession);
	void addContact(IMChatSession & chatSession, const std::string & contactId);
	void removeContact(IMChatSession & chatSession, const std::string & contactId);

	/** @} */

	/**
	 * @name IMPresence Implementation
	 * @{
	 */

	void changeMyPresence(EnumPresenceState::PresenceState state, const std::string & note);

	//Look at changeMyIcon in IMPresence
	void sendMyIcon(const std::string & contactId, const std::string & iconFilename);

	void subscribeToPresenceOf(const std::string & contactId);

	void unsubscribeToPresenceOf(const std::string & contactId);

	void blockContact(const std::string & contactId);

	void unblockContact(const std::string & contactId);

	std::map<int, IMChatSession *> & getMessageIdChatSessionMap() {
		return _messageIdChatSessionMap;
	}

	std::map<const std::string, IMChatSession *> & getContactChatMap() {
		return _contactChatMap;
	}

	void allowWatcher(const std::string & watcher);

	void forbidWatcher(const std::string & watcher);

	/** @} */

	/**
	 * Helper function for the enum phCallStateEvent.
	 *
	 * @param event a phCallStateEvent to "convert" into a string
	 * @return a string representing the enum
	 */
	static std::string phapiCallStateToString(enum phCallStateEvent event);

	static const std::string PresenceStateOnline;
	static const std::string PresenceStateAway;
	static const std::string PresenceStateDoNotDisturb;
	static const std::string PresenceStateBusy;

	static PhApiWrapper * PhApiWrapperHack;

	// Fixme: temporary hack
	std::string _iconFilename;

	bool isRegistered() const { return _registered; };
	void setRegistered(bool registered) { _registered = registered; };

	int getActiveVline() const { return _wengoVline; };

	void flipVideoImage(bool flip);

	std::string getWengoRealm() {
		return _wengoRealm;
	};

	std::set<std::string> & getSubscribedContacts() { return _subscribedContacts; };

	/**
	 * Parse the From field from a SIP header.
	 * @param from the From from the SIP header.
	 * @return user@realm or just user if the realm is the same as the connection
	 */
	static std::string parseFromHeader(const std::string & from);

	/**
 	 * Looks for an IMChatSession associated with given contact.
	 *
	 * @return found IMChatSession, NULL otherwise
	 */
	IMChatSession *getIMChatSession(const std::string & from) const;


	virtual void setSipOptions(const std::string& optname, const std::string& optval);

	virtual void acceptSubscription(int sid);
	virtual void rejectSubscription(int sid);



private:

	friend class PhApiCallbacks;
	/**
	 * Set phApi with parameters previously set with setProxy, etc. methods.
	 */
	void setNetworkParameter();

	PhApiWrapper(PhApiCallbacks & callbacks);


	void handleIncomingSubscribe(int sid, const char *frm,  const char* evtType);
	void terminateIncomingSubscribe(int sid);


	/**
	 * Factorizes code.
	 *
	 * @see publishOnline()
	 * @see publishOffline()
	 */
	//void publishPresence(const std::string & pidf, const std::string & note);
	void publishPresence(bool online, const std::string & note);

	/** @see changeMyPresence() */
	void publishOnline(const std::string & note);

	/** @see changeMyPresence() */
	void publishOffline(const std::string & note);

	/** Changes audio devices inside PhApi. */
	bool setAudioDevices();

	/** Makes a SIP uri from a contact id. */
	/* (checks if a domain is set, otherwise adds the default one) */
	std::string makeSipAddress(const std::string & contactId);

	//FIXME ugly for call conference
	void phoneCallStateChangedEventHandler(SipWrapper & sender, int callId,
		EnumPhoneCallState::PhoneCallState state, const std::string & from);

	PhApiCallbacks * _callbacks;

	/** Checks if phApi has been initialized: if phInit() has been called. */
	bool _isInitialized;

	/** Virtual line for the Wengo SIP service. */
	int _wengoVline;

	/** SIP address for the Wengo SIP service. */
	std::string _wengoSipAddress;

	/** Realm for the Wengo SIP service. */
	std::string _wengoRealm;

	/** Playback audio device. */
	AudioDevice _outputAudioDevice;

	/** Record audio device. */
	AudioDevice _inputAudioDevice;

	std::map<int, IMChatSession *> _messageIdChatSessionMap;

	std::map<const std::string, IMChatSession *> _contactChatMap;

	std::string _proxyServer;

	unsigned _proxyPort;

	std::string _proxyLogin;

	std::string _proxyPassword;

	bool _tunnelNeeded;

	std::string _tunnelServer;

	unsigned _tunnelPort;

	bool _tunnelSSL;

	EnumNatType::NatType _natType;

	std::string _sipServer;

	unsigned _sipServerPort;

	unsigned _sipLocalPort;

	std::string _pluginPath;

	mutable Mutex _mutex;

	bool _registered;

	bool _mustReinitNetwork;

	std::set<std::string> _subscribedContacts;

	PhApiSFPWrapper & _phApiSFPWrapper;

	enum { SUBPENDING, SUBCONFIRMED };
	struct InSub { int state; std::string from; std::string evt; };
	std::map<int, InSub> _incomingSubscriptions;
	typedef std::map<int, InSub>::const_iterator InSubIter;



    struct  
    {
        bool sip_p2p_presence;
        bool sip_use_options_request;
        int   sip_register_timeout;
        int   sip_publish_timeout;
        bool   sip_use_typing_state;

    } sipOptions;

    bool isOnline;
    std::string presence_note;
    

};

#endif	//OWPHAPIWRAPPER_H
