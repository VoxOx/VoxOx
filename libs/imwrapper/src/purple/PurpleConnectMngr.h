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

#ifndef PURPLECONNECTMNGR_H
#define PURPLECONNECTMNGR_H

extern "C" {
#include "libpurple/connection.h"
}

#include <thread/RecursiveMutex.h>

#include "PurpleIMConnect.h"
#include "PurpleAccountMngr.h"

#include <list>

class IMAccount;

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class PurpleConnectMngr
{
public:

	static PurpleConnectMngr *getInstance();

	void Init();

	void reset();

	PurpleIMConnect *AddIMConnect(IMAccount &account);
	void RemoveIMConnect(IMAccount &account);
	void UpdateIMConnect(IMAccount &account);
	static PurpleIMConnect *FindIMConnect(IMAccount &account);

	/* ********** PurpleConnectionsCallback *********** */
	static void ConnProgressCbk(PurpleConnection *gc, const char *text,
								size_t step, size_t step_count);
	static void ConnConnectedCbk(PurpleConnection *gc);
	static void ConnDisconnectedCbk(PurpleConnection *gc);
	static void ConnNoticeCbk(PurpleConnection *gc, const char *text);
	static void ConnReportDisconnectCbk(PurpleConnection *gc, const char *text);
	static void NetworkConnected();
	static void NetworkDisconnected();
	/* ********************************************** */

private:

	PurpleConnectMngr();
	static PurpleConnectMngr *_staticInstance;

	PurpleAccountMngr *_accountMngr;

	/* ********** CONNECT_LIST *********** */
	static std::list<PurpleIMConnect *> _purpleIMConnectList;
	typedef std::list<PurpleIMConnect *>::iterator PurpleIMConnectIterator;
	/* *********************************** */

	static RecursiveMutex _mutex;

};

#endif	//PURPLECONNECTMNGR_H
