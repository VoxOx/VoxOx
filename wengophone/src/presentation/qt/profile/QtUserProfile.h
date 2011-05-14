/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
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

#ifndef OWQTUSERPROFILE_H
#define OWQTUSERPROFILE_H

#include <presentation/PUserProfile.h>

#include <imwrapper/IMContact.h>

#include <qtutil/QObjectThreadSafe.h>

#include <QtCore/QString>

class CUserProfile;
class QtWengoPhone;

/**
 * Qt implementation of UserProfile Presentation layer.
 *
 * @author Philippe Bernery
 */
class QtUserProfile : public QObjectThreadSafe, public PUserProfile {
	Q_OBJECT
public:

	/**
	 * @param parenWidget Parent widget. Used for Dialog created during events.
	 */
	QtUserProfile(CUserProfile & cUserProfile, QtWengoPhone & qtWengoPhone);

	virtual ~QtUserProfile();

	void loginStateChangedEventHandler(SipAccount & sender,
		EnumSipLoginState::SipLoginState state);

	void networkDiscoveryStateChangedEventHandler(SipAccount & sender,
		SipAccount::NetworkDiscoveryState state);

	void authorizationRequestEventHandler(PresenceHandler & sender,
		const IMContact & imContact, const std::string & message);

	void incomingSubscribeEventHandler(PresenceHandler & sender,
		const std::string & imaccountId,int sid,const std::string & from,const std::string & evtType);

Q_SIGNALS:

	//FIXME: here we should be very careful with pointer as
	// we are changing the Thread and pointer may not be valid when used.
	void loginStateChangedEventHandlerSignal(SipAccount * sender, int iState);

	void networkDiscoveryStateChangedEventHandlerSignal(SipAccount * sender, int iState);

	void authorizationRequestEventHandlerSignal(PresenceHandler * sender,
		IMContact imContact, QString message);

	void incomingSubscribeEventHandlerSignal(PresenceHandler * sender,
		const QString& imaccountId,int sid,const QString& from, const QString& evtType);

private Q_SLOTS:

	void loginStateChangedEventHandlerSlot(SipAccount * sender, int iState);

	void networkDiscoveryStateChangedEventHandlerSlot(SipAccount * sender, int iState);

	void authorizationRequestEventHandlerSlot(PresenceHandler * sender,
		IMContact imContact, QString message);

	void incomingSubscribeEventHandlerSlot(PresenceHandler * sender,
		const QString& imaccountId,int sid,const QString& from, const QString& evtType);

private:

	void connectionIsUpEventHandler();//VOXOX CHANGE by Rolando - 2009.07.27 

	void connectionIsDownEventHandler();//VOXOX CHANGE by Rolando - 2009.07.27 

	void initThreadSafe();

	void updatePresentation();

	void updatePresentationThreadSafe();

	QtWengoPhone & _qtWengoPhone;

	CUserProfile & _cUserProfile;
};

#endif //OWQTUSERPROFILE_H
