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

#ifndef PSOFTUPDATE_H
#define PSOFTUPDATE_H

#include "Presentation.h"

#include <util/Event.h>

#include <string>

class SoftUpdateInfo;	//Fwd declaration
/**
 *
 * @ingroup presentation
 * @author Tanguy Krotoff
 */
class PSoftUpdate : public Presentation 
{
public:

	/**
	 * @see SoftUpdate::updateWengoPhoneEvent
	 */
	//Event<void (const std::string & downloadUrl,
	//	unsigned long long buildId,
	//	const std::string & version,
	//	unsigned fileSize)> updateWengoPhoneEvent;

	Event<void ( const SoftUpdateInfo& suInfo )> updateWengoPhoneEvent;
	Event<void ( const SoftUpdateInfo& suInfo )> noAvailableUpdateEvent;	//VOXOX - JRT - 2009.10.27 
};

#endif	//PSOFTUPDATE_H
