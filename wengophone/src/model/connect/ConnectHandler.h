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

#ifndef OWCONNECTHANDLER_H
#define OWCONNECTHANDLER_H

#include <model/chat/ChatHandler.h>

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMAccountList.h>
#include <imwrapper/IMConnect.h>

#include <thread/RecursiveMutex.h>

#include <util/Event.h>
#include <util/NonCopyable.h>
#include <util/Trackable.h>

#include <list>
#include <map>

class Connect;
class IMAccountManager;
class NetworkObserver;
class UserProfile;
class WengoPhone;

/**
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class ConnectHandler : NonCopyable, public Trackable {
public:

	/**
	 * @see IMConnect::connectedEvent
	 */
	Event<void (ConnectHandler & sender, std::string imAccountId)> connectedEvent;

	/**
	 * @see IMConnect::disconnectedEvent
	 */
	Event<void (ConnectHandler & sender, std::string imAccountId,
		bool connectionError, std::string reason)> disconnectedEvent;

	/**
	 * @see IMConnect::connectionProgressEvent
	 */
	Event<void (ConnectHandler & sender, std::string imAccountId,
		int currentStep, int totalSteps, std::string infoMessage)> connectionProgressEvent;

	ConnectHandler(UserProfile * userProfile);

	~ConnectHandler();

	void connect(const std::string & imAccountId);

	void disconnect(const std::string & imAccountId);
	
	void userProfileIsLoggingOff();

	/**
	 * Tells ConnectHandler to unregister to all registered Events.
	 */
	void unregisterAllEvents();

private:

	/**
	 * @see IMAccountManager::imAccountRemovedEvent
	 */
	void imAccountRemovedEventHandler(IMAccountManager & sender, std::string imAccountId);

	/**
	 * @see IMConnect::connectedEvent
	 */
	void connectedEventHandler(IMConnect & sender);

	/**
	 * Here we must change thread because are sent from Gaim or PhApi Thread.
	 * @see connectedEventHandler
	 */
	void connectedEventHandlerThreadSafe(std::string imAccountId, EnumPresenceState::PresenceState initialPresenceState );	//VOXOX - JRT - 2009.09.10 

	/**
	 * @see IMConnect::disconnectedEvent
	 */
	void disconnectedEventHandler(IMConnect & sender, bool connectionError, std::string reason, bool fatal );	//VOXOX - JRT - 2009.07.13 

	/**
	 * Here we must change thread because are sent from Gaim or PhApi Thread.
	 * @see disconnectedEventHandler
	 */
	void disconnectedEventHandlerThreadSafe(std::string imAccountId, bool connectionError, std::string reason, bool fatal );	//VOXOX - JRT - 2009.07.13 

	/**
	 * @see IMConnect::connectionProgressEvent
	 */
	void connectionProgressEventHandler(IMConnect & sender,
		int currentStep, int totalSteps, std::string infoMessage);

	/**
	 * @see IMConnect::connectionProgressEvent
	 */
	void connectionProgressEventHandlerThreadSafe(std::string imAccountId,
		int currentStep, int totalSteps, std::string infoMessage);

	/**
	 * @see NetworkObserver::connectionIsUpEvent
	 */
	void connectionIsUpEventHandler(NetworkObserver & sender);
	void connectionIsUpEventHandlerThreadSafe();

	/**
	 * Handle NetworkObserver::connectionIsDownEvent
	 */
	void connectionIsDownEventHandler(NetworkObserver & sender);
	void connectionIsDownEventHandlerThreadSafe();

	/**
	 * Associative table between an IMAccount id and a Connect.
	 */
	typedef std::map<std::string, Connect *> ConnectMap;
	ConnectMap _connectMap;

	RecursiveMutex _mutex;

	UserProfile * _userProfile;
	
	bool _freeze;
};

#endif	//OWCONNECTHANDLER_H
