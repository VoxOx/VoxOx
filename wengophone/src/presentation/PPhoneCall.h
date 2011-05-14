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

#ifndef OWPPHONECALL_H
#define OWPPHONECALL_H

#include "Presentation.h"

#include <sipwrapper/EnumPhoneCallState.h>

#include <pixertool/pixertool.h>

class CPhoneCall;

/**
 * Graphical representation of a PhoneCall.
 *
 * @ingroup presentation
 * @author Tanguy Krotoff
 */
class PPhoneCall : public Presentation {
public:

	virtual CPhoneCall & getCPhoneCall() const = 0;

	/**
	 * @see PhoneCall::stateChangedEvent
	 */
	virtual void stateChangedEvent(EnumPhoneCallState::PhoneCallState state) = 0;

	/**
	 * @see PhoneCall::videoFrameReceivedEvent
	 */
	virtual void videoFrameReceivedEvent(piximage * remoteVideoFrame, piximage * localVideoFrame) = 0;

	/**
	 * The phone call has been closed, widget must be closed/destroyed.
	 */
	virtual void close() = 0;
};

#endif	//OWPPHONECALL_H
