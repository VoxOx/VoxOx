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

#ifndef OWLISTENER_H
#define OWLISTENER_H

#include <util/Interface.h>

#include <sipwrapper/EnumPhoneCallState.h>
#include <sipwrapper/EnumPhoneLineState.h>
#include <imwrapper/EnumPresenceState.h>

#include <model/account/EnumSipLoginState.h>
#include <model/webservices/sms/EnumSmsState.h>

#include <string>

/**
 * Events notification from WengoPhone.
 *
 * @author Tanguy Krotoff
 */
class Listener : Interface {
public:

	/**
	 * Login procedure is done, event with the procedure result.
	 *
	 * @param state login procedure result
	 * @param login Wengo login used
	 * @param password Wengo password used
	 */
	virtual void wengoLoginStateChangedEvent(EnumSipLoginState::SipLoginState state, const std::string & login, const std::string & password) = 0;

	/**
	 * The state of the phone line has changed.
	 *
	 * @param state new phone line state
	 * @param lineId phone line id
	 * @param param for future use
	 */
	virtual void phoneLineStateChangedEvent(EnumPhoneLineState::PhoneLineState state, int lineId, void * param) = 0;

	/**
	 * The state of the phone call has changed.
	 *
	 * @param state new phone call state
	 * @param lineId phone line id associated with this phone call
	 * @param callId phone call id
	 * @param sipAddress caller/callee SIP address (can be empty)
	 * @param userName caller/callee username (can be empty)
	 * @param displayName caller/callee display name (can be empty)
	 * @param param for future use
	 */
	virtual void phoneCallStateChangedEvent(EnumPhoneCallState::PhoneCallState state, int lineId, int callId,
				const std::string & sipAddress, const std::string & userName, const std::string & displayName,
				void * param) = 0;

	/**
	 * Presence status of a buddy has changed.
	 *
	 * @param state new buddy presence status
	 * @param note user presence defined (state = PresenceUserDefined)
	 * @param sipAddress buddy SIP address (can be empty)
	 * @param userName buddy username (can be empty)
	 * @param displayName buddy display name (can be empty)
	 * @param param for future use
	 */
	//VOXOX CHANGE CHC SUPPORT STATUS MESSAGE
	virtual void presenceStateChangedEvent(EnumPresenceState::PresenceState state, const std::string & note, const std::string & statusMessage,
				const std::string & sipAddress, const std::string & userName, const std::string & displayName,
				void * param) = 0;

	/**
	 * Chat message received callback.
	 *
	 * @see presenceStateChangedEvent
	 * @param message message received
	 */
	virtual void chatMessageReceivedEvent(const std::string & message,
				const std::string & sipAddress, const std::string & userName, const std::string & displayName,
				void * param) = 0;

	/**
	 * Callback to check if the SMS was received or not.
	 *
	 * @param smsId SMS id
	 * @param state SMS state (ok or error)
	 */
	virtual void smsStatusEvent(int smsId, EnumSmsState::SmsState state) = 0;

	/**
	 * A login/password is needed for the HTTP proxy.
	 *
	 * @param hostname HTTP proxy server hostname detected
	 * @param port HTTP proxy server port detected
	 * @see Command::setHttpProxySettings()
	 */
	//virtual void httpProxySettingsNeededEvent(const std::string & hostname, unsigned port)/* = 0*/ { }
};

#endif	//OWLISTENER_H
