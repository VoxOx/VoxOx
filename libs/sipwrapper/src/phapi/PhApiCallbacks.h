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

#ifndef PHAPICALLBACKS_H
#define PHAPICALLBACKS_H

#include <phapi.h>
#include <phevents.h>

#include <util/Singleton.h>

#include <string>
#include <set>

/**
 * PhApi callbacks.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class PhApiCallbacks : public Singleton<PhApiCallbacks> {

	friend class Singleton<PhApiCallbacks>;

public:

	void startListeningPhApiEvents();

	void registerProgress(OWPL_LINESTATE_INFO * info);

	void callProgress(OWPL_CALLSTATE_INFO * info);

	void subscriptionProgress(OWPL_SUBSTATUS_INFO * info);

	void onNotify(OWPL_NOTIFICATION_INFO * info);

	void messageProgress(OWPL_MESSAGE_INFO * info);

	void errorNotify(OWPL_ERROR_INFO * info);

private:

	PhApiCallbacks();

	~PhApiCallbacks();


	/**
	 * Transforms a PhApi contacts (e.g: sip:joe@voip.wengo.fr)
	 * in an IMWrapper compatible form (e.g: joe)
	 */
	std::string computeContactId(const std::string & contactFromPhApi);
};

#endif	//PHAPICALLBACKS_H
