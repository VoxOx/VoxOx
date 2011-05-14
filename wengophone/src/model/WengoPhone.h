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

#ifndef OWWENGOPHONE_H
#define OWWENGOPHONE_H

#include <thread/Thread.h>

#include <util/Event.h>
#include <util/Singleton.h>
#include <util/Trackable.h>

#include <string>

class CoIpManager;
class ConfigImporter;
class DtmfThemeManager;
class UserProfile;
class UserProfileHandler;

/**
 * @defgroup model Model Component
 *
 * The model component is the heart of WengoPhone, it implements all the basic functionnalities
 * above the SIP stack like the ContactList, the History, it handles SIP accounts and more.
 * It contains all the 'intelligence' of WengoPhone.
 * The model is developped in pure C++ inside libraries like LibUtil, Boost, TinyXML and Curl.
 *
 * The model component communicates only with the control component via the desgin
 * pattern observer implemented inside LibUtil with the class Event.
 */

/**
 * Entry point of the application.
 *
 * Design Pattern Mediator.
 *
 * This class starts the model thread.
 * 2 threads are running: the model (including the control) and the presentation.
 * Thus the presentation component can be loaded and showed to the user
 * even before everything is ready.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class WengoPhone : public Singleton<WengoPhone>, public Thread, public Trackable {
	friend class Singleton<WengoPhone>;
public:

	/**
	 * Initialization step is finished (i.e we exit the init() method.
	 *
	 * @param sender this class
	 */
	Event<void (WengoPhone & sender)> initFinishedEvent;

	/**
	 * Application should be closed.
	 *
	 * This event goes to the presentation layer and the presentation layer
	 * should exit the application.
	 *
	 * @param sender this class
	 */
	Event<void (WengoPhone & sender)> exitEvent;

	/**
	 * Dtmf Theme Manager has been created.
	 *
	 * @param sender this class
	 * @param dtfmThemeManager DtfmThemeManager created
	 */
	Event<void (WengoPhone & sender, DtmfThemeManager & dtmfThemeManager)> dtmfThemeManagerCreatedEvent;

	/**
	 * Gets the UserProfileHandler.
	 */
	UserProfileHandler & getUserProfileHandler() {
		return *_userProfileHandler;
	}

	/**
	 * Gets the CoIpManager.
	 */
	CoIpManager * getCoIpManager() const;

	/**
	 * Prepares the model to terminate.
	 *
	 * Starts a 3 seconds timer that will launch exitEvent.
	 *
	 * @see terminate()
	 */
	void prepareToTerminate();

	/**
	 * Terminates the model component thread i.e this thread.
	 *
	 * This method should be called when you exit definitly WengoPhone.
	 * You cannot call start() then terminate() several times.
	 */
	void terminate();

	/**
	 * Starts the thread of the model component.
	 */
	virtual void run();

	/**
	 * Enable/disable data serialization.
	 * @param enable if True serialization is activated.
	 */
	void enableSerialization(bool enable);

	/**
	 * Store the phone number/sip address for a call which was requested on
	 * startup (for example from the command line)
	 */
	void setStartupCall(const std::string& call);

	std::string getStartupCall() const;

private:

	WengoPhone();

	virtual ~WengoPhone();

	/**
	 * Entry point of the application, equivalent to main().
	 */
	void init();

	/**
	 * Saves the Configuration.
	 */
	void saveConfiguration();

	/**
	 * @see Settings::valueChangedEvent
	 */
	void valueChangedEventHandler(const std::string & key);

	/**
	 * Emergency timeout handler called when SIP unregistering
	 * is not quick enough or timed out.
	 */
	void exitAfterTimeout();

	UserProfileHandler * _userProfileHandler;

	/** DtmfThemeManager. */
	DtmfThemeManager * _dtmfThemeManager;

	/** configuration serialization and UserProfileHandler autosave. */
	bool _enableSerialization;

	std::string _startupCall;
};

#endif	//OWWENGOPHONE_H
