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

#ifndef OWPWENGOPHONE_H
#define OWPWENGOPHONE_H

#include "Presentation.h"

class ContactInfo;

/**
 *
 * @ingroup presentation
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class PWengoPhone : public Presentation {
public:

	virtual void showHistory() = 0;

	virtual void showAddContact(ContactInfo contactInfo) = 0;

	virtual void bringMainWindowToFront() = 0;

	/**
	 * @see WengoPhone::exitEvent
	 */
	virtual void exitEvent() = 0;
};

#endif	//OWPWENGOPHONE_H
