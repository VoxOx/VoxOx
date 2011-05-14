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

#ifndef PPHONELINE_H
#define PPHONELINE_H

#include "Presentation.h"

#include <sipwrapper/EnumPhoneLineState.h>

#include <util/Event.h>

#include <string>

class CPhoneCall;

/**
 *
 * @ingroup presentation
 * @author Tanguy Krotoff
 */
class PPhoneLine : public Presentation {
public:

	/**
	 * @see PhoneLine::stateChangedEvent
	 */
	Event<void (EnumPhoneLineState::PhoneLineState state)> stateChangedEvent;

	/**
	 * @see PhoneLine::phoneCallCreatedEvent
	 */
	Event<void (CPhoneCall & cPhoneCall)> phoneCallCreatedEvent;

	/**
	 * @see PhoneLine::phoneCallClosedEvent
	 */
	Event<void (CPhoneCall & cPhoneCall)> phoneCallClosedEvent;
};

#endif	//PPHONELINE_H
