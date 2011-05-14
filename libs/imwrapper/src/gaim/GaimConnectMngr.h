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

#ifndef GAIMCONNECTMNGR_H
#define GAIMCONNECTMNGR_H

extern "C" {
#include "gaim/connection.h"
}

#include <thread/RecursiveMutex.h>

#include "GaimIMConnect.h"
#include "GaimAccountMngr.h"

#include <list>

class IMAccount;

/**
 *
 * @ingroup model
 * @author Julien Bossart
 */
class GaimConnectMngr
{
public:

	static GaimConnectMngr *getInstance();

	void Init();

	void reset();

	GaimIMConnect *AddIMConnect(IMAccount &account);
	void RemoveIMConnect(IMAccount &account);
	void UpdateIMConnect(IMAccount &account);
	static GaimIMConnect *FindIMConnect(IMAccount &account);

	/* ********** GaimConnectionsCallback *********** */
	static void ConnProgressCbk(GaimConnection *gc, const char *text,
								size_t step, size_t step_count);
	static void ConnConnectedCbk(GaimConnection *gc);
	static void ConnDisconnectedCbk(GaimConnection *gc);
	static void ConnNoticeCbk(GaimConnection *gc, const char *text);
	static void ConnReportDisconnectCbk(GaimConnection *gc, const char *text);
	static void NetworkConnected();
	static void NetworkDisconnected();
	/* ********************************************** */

private:

	GaimConnectMngr();
	static GaimConnectMngr *_staticInstance;

	GaimAccountMngr *_accountMngr;

	/* ********** CONNECT_LIST *********** */
	static std::list<GaimIMConnect *> _gaimIMConnectList;
	typedef std::list<GaimIMConnect *>::iterator GaimIMConnectIterator;
	/* *********************************** */

	static RecursiveMutex _mutex;

};

#endif	//GAIMCONNECTMNGR_H
