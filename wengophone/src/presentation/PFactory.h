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

#ifndef OWPFACTORY_H
#define OWPFACTORY_H

class CWengoPhone;
class PWengoPhone;
class CPhoneLine;
class PPhoneLine;
class CPhoneCall;
class PPhoneCall;
class CContactList;
class PContactList;
class CContactGroup;
class PContactGroup;
class CContact;
class PContact;
class CWenboxPlugin;
class PWenboxPlugin;
class CConnectHandler;
class PConnectHandler;
class CPresenceHandler;
class PPresenceHandler;
class CChatHandler;
class PChatHandler;
class PSms;
class CSms;
class PHistory;
class CHistory;
class PSoftUpdate;
class CSoftUpdate;
class PUserProfile;
class CUserProfile;
class PUserProfileHandler;
class CUserProfileHandler;
class PConferenceCall;
class CConferenceCall;
class CDtmfThemeManager;

class IThreadEvent;

/**
 * Factories for the presentation component.
 *
 * Depending on the factory instanciated, the presentation component will use Qt, GTK+
 * or what ever you prefer.
 *
 * @ingroup presentation
 * @author Tanguy Krotoff
 */
class PFactory {
public:

	/**
	 * Sets the PresentationFactory to use (Qt or GTK+ in this case).
	 *
	 * @param factory factory to create the presentation component
	 */
	static void setFactory(PFactory * factory) {
		_factory = factory;
	}

	static PFactory & getFactory() {
		return * _factory;
	}

	virtual ~PFactory() {
		delete _factory;
	}

	virtual PWengoPhone * createPresentationWengoPhone(CWengoPhone & cWengoPhone, bool runInBackground) = 0;

	virtual PUserProfile * createPresentationUserProfile(CUserProfile & cUserProfile) = 0;

	virtual PUserProfileHandler * createPresentationUserProfileHandler(CUserProfileHandler & cUserProfileHandler) = 0;

	virtual PPhoneLine * createPresentationPhoneLine(CPhoneLine & cPhoneLine) = 0;

	virtual PPhoneCall * createPresentationPhoneCall(CPhoneCall & cPhoneCall) = 0;

	virtual PContactList * createPresentationContactList(CContactList & cContactList) = 0;

	virtual PWenboxPlugin * createPresentationWenboxPlugin(CWenboxPlugin & cWenboxPlugin) = 0;

	virtual PChatHandler * createPresentationChatHandler(CChatHandler & cChatHandler) = 0;

	virtual PSms * createPresentationSms(CSms & cSms) = 0;

	virtual PSoftUpdate * createPresentationSoftUpdate(CSoftUpdate & cSoftUpdate) = 0;

	virtual PHistory * createPresentationHistory(CHistory & cHistory) = 0;

	virtual PConferenceCall * createPresentationConferenceCall(CConferenceCall & cConferenceCall) = 0;

	//DtmfThemeManager
	virtual void createPresentationDtmfThemeManager(CDtmfThemeManager & _dtmfThemeManager) = 0;

	/**
	 * Because of the presentation event loop.
	 *
	 * @see QApplication::exec()
	 */
	virtual int exec() = 0;

	/** Processes all pending events of the presentation. */
	virtual void processEvents() = 0;

	/** Reset the factory. */
	virtual void reset() = 0;

	/** @see QApplication::postEvent() */
	static void postEvent(IThreadEvent * event) {
		_factory->postEventImpl(event);
	}

	virtual	void cleanup() = 0;				//VOXOX - JRT - 2009.04.13 


protected:

	virtual void postEventImpl(IThreadEvent * event) = 0;

private:

	static PFactory * _factory;
};

#endif	//OWPFACTORY_H
