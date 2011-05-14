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

#ifndef PURPLEIMCONNECT_H
#define PURPLEIMCONNECT_H

#include <imwrapper/IMConnect.h>
#include <thread/Mutex.h>

extern "C" 
{
#include <libpurple/account.h>
}

class PurpleIMConnect;

//=============================================================================

class PurpleIMConnectCallbackData
{
public:
	PurpleIMConnectCallbackData()						{ initVars();	}
	

	PurpleIMConnect*	getPurpleIMConnect()  const				{ return _connect;			}
	int					getCallbackId()    const				{ return _callbackId;		}

	void setPurpleIMConnect( PurpleIMConnect*	val )			{ _connect		 = val;	}
	void setCallbackId     ( int				val )			{ _callbackId	 = val;	}

protected:
	void initVars()
	{
		setPurpleIMConnect( NULL );
		setCallbackId     ( 0 );
	}

private:
	PurpleIMConnect*	_connect;
	int					_callbackId;
};

//=============================================================================

/**
 * Purple IM connection.
 *
 * @ingroup model
 * @author Julien Bossart
 */
class PurpleIMConnect : public IMConnect {
	friend class PurpleIMFactory;
public:

	PurpleIMConnect(IMAccount account);
	~PurpleIMConnect();

	void connect();

	void disconnect(bool force = false);

	bool isConnected() {return _connected;};
	void setConnected(bool connected) {_connected = connected;};

	const IMAccount & getIMAccount() const { return _imAccount; }

	static bool connectCbk(void * data);
	static bool disconnectCbk(void * data);

	static void * CreateAccount(const IMAccount & account);
	static void AddAccountParams(void *purpleAccount, const IMAccount & account);
	static void AddMSNAccountParams(void *purpleAccount, const IMAccountParameters & mParams);
	static void AddMySpaceAccountParams(void *purpleAccount, const IMAccountParameters & mParams);
	static void AddFacebookAccountParams(void *purpleAccount, const IMAccountParameters & mParams);
	static void AddTwitterAccountParams(void *purpleAccount, const IMAccountParameters & mParams);
	static void AddSkypeAccountParams(void *purpleAccount, const IMAccountParameters & mParams);
	static void AddYahooAccountParams(void *purpleAccount, const IMAccountParameters & mParams);
	static void AddOscarAccountParams(void *purpleAccount, const IMAccountParameters & mParams);
	static void AddJabberAccountParams(void *purpleAccount, const IMAccountParameters & mParams);

	//VOXOX - JRT - 2009.08.21 - These should be in a base class
	static PurpleAccount* getPurpleAccount( const IMAccount& imAccount );
		   int			  timeoutAdd      ( void* cbData, GSourceFunc callbackFunc );
	static void			  timeoutRemove   ( PurpleIMConnectCallbackData* cbData );

	static Mutex _mutex;

private:

	bool _connected;
};

#endif	//PURPLEIMCONNECT_H
