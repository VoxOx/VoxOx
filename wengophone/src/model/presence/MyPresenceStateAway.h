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

#ifndef MYPRESENCESTATEAWAY_H
#define MYPRESENCESTATEAWAY_H

#include "MyPresenceState.h"

/**
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class MyPresenceStateAway : public MyPresenceState {
public:

	MyPresenceState * online();

	MyPresenceState * offline();

	MyPresenceState * doNotDisturb();

	MyPresenceState * away();

	MyPresenceState * invisible();

	std::string toString() const {
		return toString_static();
	}

	static MyPresenceState * getInstance() {
		static MyPresenceStateAway presenceState;

		return &presenceState;
	}

private:

	MyPresenceStateAway() {
	}

	static std::string toString_static() {
		return "Away";
	}
};

#endif	//MYPRESENCESTATEAWAY_H
