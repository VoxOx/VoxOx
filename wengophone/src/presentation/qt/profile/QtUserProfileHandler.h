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

#ifndef OWQTUSERPROFILEHANDLER_H
#define OWQTUSERPROFILEHANDLER_H

#include <presentation/PUserProfileHandler.h>

#include <model/account/wengo/WengoAccount.h>

#include <QtCore/QObject>

class CUserProfileHandler;
class QtLoginDialog;
class QtWengoPhone;

/**
 * Qt implementation of UserProfileHandler Presentation layer.
 *
 * @author Philippe Bernery
 */
class QtUserProfileHandler : public QObject, public PUserProfileHandler {
	Q_OBJECT
public:

	QtUserProfileHandler(CUserProfileHandler & cUserProfileHandler, QtWengoPhone & qtWengoPhone);

	virtual void setCurrentUserProfileEventHandler();

	virtual void noCurrentUserProfileSetEventHandler();

	virtual void currentUserProfileWillDieEventHandler();

	virtual void userProfileInitializedEventHandler();

	//VOXOX CHANGE by Rolando - 2009.09.10 
	virtual void sipAccountConnectionFailedEventHandler(const SipAccount & sipAccount, EnumSipLoginState::SipLoginState, std::string loginErrorMessage);

	virtual void profileLoadedFromBackupsEventHandler(std::string profileName);

	virtual void profileCannotBeLoadedEventHandler(std::string profileName);

	virtual void networkErrorEventHandler(const SipAccount & sipAccount);

	virtual void updatePresentation();

Q_SIGNALS:

	/**
	 * @see noCurrentUserProfileSetEventHandler
	 */
	void noCurrentUserProfileSetEventHandlerSignal();

	/**
	 * @see currentUserProfileWillDieEventHandler
	 */
	void currentUserProfileWillDieEventHandlerSignal();

	/**
	 * @see userProfileInitializedEventHandler
	 */
	void userProfileInitializedEventHandlerSignal();

	/**
	 * @see sipAccountConnectionFailedEventHandler
	 */
	void sipAccountConnectionFailedEventHandlerSignal(SipAccount* sipAccount, EnumSipLoginState::SipLoginState, std::string);//VOXOX CHANGE by Rolando - 2009.09.10 
	
	/**
	 * @see profileLoadedFromBackupsEventHandler
	 */
	void profileLoadedFromBackupsEventHandlerSignal(QString profileName);

	/**
	 * @see profileCannotBeLoadedEventHandler
	 */
	void profileCannotBeLoadedEventHandlerSignal(QString profileName);

	/**
	 * @see networkErrorEventHandler
	 */
	void sipNetworkErrorEventHandlerSignal(SipAccount* sipAccount);

	void setCurrentUserProfileEventHandlerSignal();
	
public Q_SLOTS:

	/**
	 * Show the Profile chooser window.
	 */
	void showLoginWindow();

	/**
	 * Show the Profile chooser window with the given WengoAccount.
	 *
	 */
	void showLoginWindowWithWengoAccount(const WengoAccount & wengoAccount);

private Q_SLOTS:

	/**
	 * @see noCurrentUserProfileSetEventHandler
	 */
	void noCurrentUserProfileSetEventHandlerSlot();

	/**
	 * @see currentUserProfileWillDieEventHandler
	 */
	void currentUserProfileWillDieEventHandlerSlot();

	/**
	 * @see userProfileInitializedEventHandler
	 */
	void userProfileInitializedEventHandlerSlot();

	/**
	 * @see sipAccountConnectionFailedEventHandler
	 */
	//VOXOX CHANGE by Rolando - 2009.09.11 void sipAccountConnectionFailedEventHandlerSlot(SipAccount*, EnumSipLoginState::SipLoginState, std::string);//VOXOX CHANGE by Rolando - 2009.09.10 
	void sipAccountConnectionFailedEventHandlerThreadSafe(SipAccount*, EnumSipLoginState::SipLoginState, std::string);//VOXOX CHANGE by Rolando - 2009.09.10 
	
	/**
	 * @see profileLoadedFromBackupsEventHandler
	 */
	void profileLoadedFromBackupsEventHandlerSlot(QString profileName);

	/**
	 * @see profileCannotBeLoadedEventHandler
	 */
	void profileCannotBeLoadedEventHandlerSlot(QString profileName);

	/**
	 * @see networkErrorEventHandler
	 */
	void sipNetworkErrorEventHandlerSlot(SipAccount* sipAccount);

	void setCurrentUserProfileEventHandlerSlot();

private:

	/** Link to control layer. */
	CUserProfileHandler & _cUserProfileHandler;

	/** Link to QtWengoPhone to create needed objects. */
	QtWengoPhone & _qtWengoPhone;

	QtLoginDialog * _qtLoginDialog;
};

#endif	//OWQTUSERPROFILEHANDLER_H
