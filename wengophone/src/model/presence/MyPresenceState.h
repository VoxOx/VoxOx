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

#ifndef OWMYPRESENCESTATE_H
#define OWMYPRESENCESTATE_H

#include <util/Interface.h>

#include <string>

/**
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class MyPresenceState : Interface {
public:

	virtual ~MyPresenceState() {
	}

	/**
	 * Switchs the presence status to online.
	 *
	 * @exception MyPresenceStateException if the switch cannot be done
	 * @return the new MyPresenceState that corresponds to the online presence status
	 */
	virtual MyPresenceState * online() = 0;

	/**
	 * Switchs the presence status to offline.
	 *
	 * @exception MyPresenceStateException if the switch cannot be done
	 * @return the new MyPresenceState that corresponds to the offline presence status
	 */
	virtual MyPresenceState * offline() = 0;

	/**
	 * Switchs the presence status to do not disturb.
	 *
	 * @exception MyPresenceStateException if the switch cannot be done
	 * @return the new MyPresenceState that corresponds to the do not disturb presence status
	 */
	virtual MyPresenceState * doNotDisturb() = 0;

	/**
	 * Switchs the presence status to away.
	 *
	 * @exception MyPresenceStateException if the switch cannot be done
	 * @return the new MyPresenceState that corresponds to the away presence status
	 */
	virtual MyPresenceState * away() = 0;

	/**
	 * Switchs the presence status to away.
	 *
	 * @exception MyPresenceStateException if the switch cannot be done
	 * @return the new MyPresenceState that corresponds to the away presence status
	 */
	virtual MyPresenceState * invisible() = 0;

	/**
	 * @return the string identifying the state.
	 */
	virtual std::string toString() const = 0;
};

#endif	//OWMYPRESENCESTATE_H
