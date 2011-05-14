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

#ifndef SIPWRAPPERFACTORY_H
#define SIPWRAPPERFACTORY_H

#include <util/Interface.h>

class SipWrapper;

/**
 * Factories for the SIP wrapper component.
 *
 * Depending on the factory instanciated, the SIP implementation component will use phApi, sipX
 * or what ever you prefer.
 *
 * @author Tanguy Krotoff
 */
class SipWrapperFactory : Interface {
public:

	virtual ~SipWrapperFactory() {
	}

	/**
	 * Sets the SIP implementation to use (phApi or sipX in this case).
	 *
	 * @param factory factory to create the SIP implementation component
	 */
	static void setFactory(SipWrapperFactory * factory) {
		_factory = factory;
	}

	/**
	 * Gets the factory that will instanciate a SIP implementation plugin
	 * (SIP stack + its callbacks handling).
	 *
	 * @return SIP implementation
	 */
	static SipWrapperFactory & getFactory() {
		return * _factory;
	}

	/**
	 * Instantiates a SIP stack implementation.
	 *
	 * @return the SIP stack implementation instance
	 */
	virtual SipWrapper * createSipWrapper() = 0;

private:

	static SipWrapperFactory * _factory;
};

#endif	//SIPWRAPPERFACTORY_H
