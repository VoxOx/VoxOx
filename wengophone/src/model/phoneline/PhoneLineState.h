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

#ifndef PHONELINESTATE_H
#define PHONELINESTATE_H

#include <util/NonCopyable.h>

#include <sipwrapper/EnumPhoneLineState.h>

#include <string>

class IPhoneLine;

/**
 * Represents the state of a PhoneLine.
 *
 * @author Tanguy Krotoff
 */
class PhoneLineState : NonCopyable {
public:

	virtual ~PhoneLineState() {
	}

	virtual void execute(IPhoneLine & phoneLine) = 0;

	/**
	 * Gets the status code corresponding to this PhoneLine state.
	 *
	 * @return status code of this state
	 */
	virtual EnumPhoneLineState::PhoneLineState getCode() const = 0;
};

#endif	//PHONELINESTATE_H
