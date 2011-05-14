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

#ifndef OWENUMSIPLOGINSTATE_H
#define OWENUMSIPLOGINSTATE_H

#include <util/NonCopyable.h>

/**
 * SIP account login state.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class EnumSipLoginState : NonCopyable {
public:

	enum SipLoginState {
		/** Initial login state. */
		SipLoginStateUnknown,

		/** This SIP account is ready to login. */
		SipLoginStateReady,

		/** Registration has been successful, login ok connection done. */
		SipLoginStateConnected,

		/** This SIP account has been disconnected. */
		SipLoginStateDisconnected,

		/** Login/Password incorrect. */
		SipLoginStatePasswordError,

		/** A network error occured. */
		SipLoginStateNetworkError,

		/** Try to connect. */
		SipLoginStateProgress,

		/** The account has not been activated */
		SipLoginStateUnActivatedError,

		/** The server sent a custom error message*/
		SipLoginStateOtherError//VOXOX CHANGE by Rolando - 2009.09.10 

	};

	/**
	 * Converts a state into a string.
	 *
	 * @return the string
	 */
	//static std::string toString(SipLoginState state);
};

#endif	//OWENUMSIPLOGINSTATE_H
