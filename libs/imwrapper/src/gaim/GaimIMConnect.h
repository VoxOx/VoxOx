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

#ifndef GAIMIMCONNECT_H
#define GAIMIMCONNECT_H

#include <imwrapper/IMConnect.h>
#include <thread/Mutex.h>

/**
 * Gaim IM connection.
 *
 * @ingroup model
 * @author Julien Bossart
 */
class GaimIMConnect : public IMConnect {
	friend class GaimIMFactory;
public:

	GaimIMConnect(IMAccount account);
	~GaimIMConnect();

	void connect();

	void disconnect(bool force = false);

	bool isConnected() {return _connected;};
	void setConnected(bool connected) {_connected = connected;};

	const IMAccount & getIMAccount() const { return _imAccount; }

	static bool connectCbk(void * data);
	static bool disconnectCbk(void * data);

	static void * CreateAccount(const IMAccount & account);
	static void AddAccountParams(void *gaimAccount, const IMAccount & account);
	static void AddMSNAccountParams(void *gaimAccount, const IMAccountParameters & mParams);
	static void AddYahooAccountParams(void *gaimAccount, const IMAccountParameters & mParams);
	static void AddOscarAccountParams(void *gaimAccount, const IMAccountParameters & mParams);
	static void AddJabberAccountParams(void *gaimAccount, const IMAccountParameters & mParams);

	static Mutex _mutex;

private:

	bool _connected;
};

#endif	//GAIMIMCONNECT_H
