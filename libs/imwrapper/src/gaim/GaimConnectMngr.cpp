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

extern "C" {
#include "glib.h"
#include "gaim/core.h"
}

#include "GaimConnectMngr.h"
#include "GaimIMFactory.h"

#include "GaimEnumIMProtocol.h"

#include <util/Logger.h>

/* ***************** GAIM CALLBACK ***************** */
static void C_ConnProgressCbk(GaimConnection *gc, const char *text,
							  size_t step, size_t step_count)
{
	GaimConnectMngr::ConnProgressCbk(gc, text, step, step_count);
}

static void C_ConnConnectedCbk(GaimConnection *gc)
{
	GaimConnectMngr::ConnConnectedCbk(gc);
}

static void C_ConnDisconnectedCbk(GaimConnection *gc)
{
	GaimConnectMngr::ConnDisconnectedCbk(gc);
}

static void C_ConnNoticeCbk(GaimConnection *gc, const char *text)
{
	GaimConnectMngr::ConnNoticeCbk(gc, text);
}

static void C_ConnReportDisconnectCbk(GaimConnection *gc, const char *text)
{
	GaimConnectMngr::ConnReportDisconnectCbk(gc, text);
}

static void C_NetworkConnected()
{
	GaimConnectMngr::NetworkConnected();
}

static void C_NetworkDisconnected()
{
	GaimConnectMngr::NetworkDisconnected();
}

GaimConnectionUiOps conn_wg_ops =
{
	C_ConnProgressCbk,
	C_ConnConnectedCbk,
	C_ConnDisconnectedCbk,
	C_ConnNoticeCbk,
	C_ConnReportDisconnectCbk,
	C_NetworkConnected,
	C_NetworkDisconnected,
};

GaimConnectionUiOps null_conn_wg_ops =
{
	NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/* ************************************************** */

GaimConnectMngr *GaimConnectMngr::_staticInstance = NULL;
std::list<GaimIMConnect *> GaimConnectMngr::_gaimIMConnectList;
RecursiveMutex GaimConnectMngr::_mutex;

GaimConnectMngr::GaimConnectMngr()
{
}

void GaimConnectMngr::Init()
{
	_accountMngr = GaimAccountMngr::getInstance();
}

GaimConnectMngr *GaimConnectMngr::getInstance()
{
	if (!_staticInstance)
	{
		_staticInstance = new GaimConnectMngr();
	}
	return _staticInstance;
}

GaimIMConnect *FindIMConnnectByGaimConnection(GaimConnection *gc)
{
	GaimAccount *gAccount = NULL;
	IMAccount	*mAccount = NULL;
	EnumIMProtocol::IMProtocol protocol = EnumIMProtocol::IMProtocolAll;

	gAccount = gaim_connection_get_account(gc);
	if (gAccount == NULL)
		return NULL;

	protocol = GaimIMPrcl::GetEnumIMProtocol(gAccount->protocol_id);
	mAccount = GaimAccountMngr::FindIMAccount(gAccount->username, protocol);
	if (mAccount == NULL)
		return NULL;

	return GaimConnectMngr::FindIMConnect(*mAccount);
}

void GaimConnectMngr::ConnProgressCbk(GaimConnection *gc, const char *text,
									  size_t step, size_t step_count)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMConnect *mIMConnect = FindIMConnnectByGaimConnection(gc);

	if (mIMConnect)
	{
		mIMConnect->connectionProgressEvent(*mIMConnect, step, step_count, text == NULL ? String::null : text);
	}
}

void GaimConnectMngr::ConnConnectedCbk(GaimConnection *gc)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMConnect *mIMConnect = FindIMConnnectByGaimConnection(gc);

	if (mIMConnect)
	{
		mIMConnect->setConnected(true);
		mIMConnect->connectedEvent(*mIMConnect);
	}
}

void GaimConnectMngr::ConnDisconnectedCbk(GaimConnection *gc)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMConnect *mIMConnect = FindIMConnnectByGaimConnection(gc);

	if (mIMConnect && mIMConnect->isConnected())
	{
		mIMConnect->setConnected(false);
		mIMConnect->disconnectedEvent(*mIMConnect, false, String::null);

		if (gc->wants_to_die)
		{
			gaim_account_set_enabled(gc->account, "Wengo GAIM", FALSE);
		}
	}
}

void GaimConnectMngr::ConnNoticeCbk(GaimConnection *gc, const char *text)
{
}

void GaimConnectMngr::ConnReportDisconnectCbk(GaimConnection *gc, const char *text)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMConnect *mIMConnect = FindIMConnnectByGaimConnection(gc);

	if (mIMConnect)
	{
		mIMConnect->setConnected(false);
		mIMConnect->disconnectedEvent(*mIMConnect, true, text == NULL ? String::null : text);

		if (gc->wants_to_die)
		{
			gaim_account_set_enabled(gc->account, "Wengo GAIM", FALSE);
		}
	}
}

void GaimConnectMngr::NetworkConnected()
{
}

void GaimConnectMngr::NetworkDisconnected()
{
}

/* **************** MANAGE CONNECT_LIST ****************** */
GaimIMConnect *GaimConnectMngr::FindIMConnect(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMConnectIterator i;

	if (account.getLogin().empty())
		return NULL;

	for (i = _gaimIMConnectList.begin(); i != _gaimIMConnectList.end(); i++)
	{
		if (GaimIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			return (*i);
		}
	}

	return NULL;
}

GaimIMConnect *GaimConnectMngr::AddIMConnect(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMConnect *mIMConnect = FindIMConnect(account);

	if (!mIMConnect)
	{
		mIMConnect = new GaimIMConnect(account);
		_gaimIMConnectList.push_back(mIMConnect);

		_accountMngr->AddIMAccount(account);
	}

	return mIMConnect;
}

void GaimConnectMngr::RemoveIMConnect(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMConnectIterator i;

	if (account.getLogin().empty())
		return;

	for (i = _gaimIMConnectList.begin(); i != _gaimIMConnectList.end(); i++)
	{
		if (GaimIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			_gaimIMConnectList.erase(i);
			break;
		}
	}
}

void GaimConnectMngr::UpdateIMConnect(IMAccount &account)
{
	RecursiveMutex::ScopedLock lock(_mutex);

	GaimIMConnectIterator i;

	if (account.getLogin().empty())
		return;

	for (i = _gaimIMConnectList.begin(); i != _gaimIMConnectList.end(); i++)
	{
		if (GaimIMFactory::equals((*i)->getIMAccount(), account.getLogin(), account.getProtocol()))
		{
			(*i)->setIMAccount(account);
			break;
		}
	}
}

void GaimConnectMngr::reset() {
	RecursiveMutex::ScopedLock lock(_mutex);

	_gaimIMConnectList.clear();
}

/* ******************************************************** */

