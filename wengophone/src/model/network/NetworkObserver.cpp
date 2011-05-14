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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "NetworkObserver.h"

#include <util/Logger.h>
#include <thread/Thread.h>
//#include <netlib.h>
#include <QtNetwork/QtNetwork>

//#define RETRIES 8

#if defined(WIN32)
#include <Windows.h>
#include <Iphlpapi.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#endif

NetworkObserver::NetworkObserver() 
{
	//_prevstate = 0;
	_isConnected = (is_connection_available() ? true : false);
	//_timer.timeoutEvent += boost::bind(&NetworkObserver::timeoutEventHandler, this);

	//_timer.start(0, 500, 0);
	_idTimer = startTimer(15000);	//VOXOX - JRT - 2009.07.09 
}

NetworkObserver *NetworkObserver::_staticInstance = NULL;


//static Mutex mutex;

NetworkObserver & NetworkObserver::getInstance()
{
	if (_staticInstance)
		return *_staticInstance;

	//Mutex::ScopedLock scopedLock(mutex);

	if (!_staticInstance)
		_staticInstance = new NetworkObserver();


	return * _staticInstance;
}

NetworkObserver::~NetworkObserver() 
{
	killTimer(_idTimer);
	//_timer.stop();
}

bool NetworkObserver::isConnected() 
{
	//VOXOX CHANGE by Rolando - 2009.07.27 - modified to not let assign value to variable "_isConnected", this variable should change only on method "timerEvent()"
	//because otherwise it will never trigger the event "connectionIsDownEvent" when a connection is down.
	/*_isConnected = (is_connection_available() ? true : false);
	return _isConnected ; */

	return is_connection_available();//VOXOX CHANGE by Rolando - 2009.07.27 
}

void NetworkObserver::timerEvent(QTimerEvent*)
{
	if (is_connection_available())
	{
		if (!_isConnected)
		{
			//if(!_prevstate)
			//{
				LOG_DEBUG("Connection is up");
				_isConnected = true;
				connectionIsUpEvent(*this);
			//}
			//_prevstate--;
		}
	}
	else
	{
		if (_isConnected)
		{
			LOG_DEBUG("Connection is down");
			_isConnected = false;
			connectionIsDownEvent(*this);
			//_prevstate = RETRIES;
		}
	}
}

bool NetworkObserver::is_connection_available()
{
	QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();


	for(QList<QNetworkInterface>::iterator it = list.begin() ; it != list.end() ; it ++)
	{
		if((*it).isValid())
		{	
			if((*it).flags() & QNetworkInterface::IsLoopBack)
				continue;


			if( (*it).flags() & QNetworkInterface::IsUp && (*it).flags() & QNetworkInterface::IsRunning )
			{
				QList<QNetworkAddressEntry> list2 = (*it).addressEntries  ();
				for(QList<QNetworkAddressEntry>::iterator itt = list2.begin() ; itt != list2.end();itt++ )
				{
					if(!(*itt).ip().isNull() && (*itt).ip().protocol() == QAbstractSocket::IPv4Protocol)
					{
						return canConnect();						
					}
				}
			}
		}
	}

	return false;
}



bool NetworkObserver::canConnect()
{
#if defined(WIN32)
/* w2000 and W95/98 */
	unsigned long  best_interface_index;
	DWORD hr;

	/* NT4 (sp4 only?) */
	PMIB_IPFORWARDTABLE ipfwdt;
	DWORD siz_ipfwd_table = 0;
	unsigned int ipf_cnt;

	best_interface_index = -1;
	/* w2000 and W95/98 only */
	hr = GetBestInterface(inet_addr("216.151.151.59"),&best_interface_index); // VOXOX -ASV- 07-09-2009: we ping voxox.com to know if voxox has internet
	if (hr)
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR) &lpMsgBuf, 0, NULL);
		best_interface_index = -1;
	}

	if (best_interface_index != -1)
	{ /* probably W2000 or W95/W98 */
		char *servername;
		char *serverip;
		char *netmask;

		if (ppl_dns_get_local_fqdn(&servername, &serverip, &netmask,
						best_interface_index))
		{
			/*if(servername)
				delete servername;
			if(serverip)
				delete serverip;
			if(netmask)
				delete netmask;*/
			
			return true;
		}
		return false;
	}


	if (!GetIpForwardTable(NULL, &siz_ipfwd_table, FALSE) == ERROR_INSUFFICIENT_BUFFER
		|| !(ipfwdt = (PMIB_IPFORWARDTABLE) alloca (siz_ipfwd_table)))
	{
		return false;
	}


	/* NT4 (sp4 support only?) */
	if (!GetIpForwardTable(ipfwdt, &siz_ipfwd_table, FALSE))
	{
		for (ipf_cnt = 0; ipf_cnt < ipfwdt->dwNumEntries; ++ipf_cnt) 
		{
			if (ipfwdt->table[ipf_cnt].dwForwardDest == 0)
			{ /* default gateway found */
				char *servername;
				char *serverip;
				char *netmask;

				if (ppl_dns_get_local_fqdn(&servername,
								 &serverip,
								 &netmask,
								 ipfwdt->table[ipf_cnt].dwForwardIfIndex))
				{
					delete servername;
					delete serverip;
					delete netmask;
					return true;
				}
				return false;
			}
		}

	}
	/* no default gateway interface found */
	return true;

#else

	#if defined(OS_MACOSX)
		socklen_t len;
	#else
		unsigned int len;
	#endif
	int sock_rt, on=1;
	struct sockaddr_in iface_out;

	struct sockaddr_in remote;

	memset(&remote, 0, sizeof(struct sockaddr_in));

	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = inet_addr("216.151.151.59");// VOXOX -ASV- 07-09-2009: we ping voxox.com to know if voxox has internet
	remote.sin_port = htons(80);

	memset(&iface_out, 0, sizeof(iface_out));
	sock_rt = socket(AF_INET, SOCK_DGRAM, 0 );

	if (setsockopt(sock_rt, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) == -1) 
	{
		close(sock_rt);
		return false;
	}

	if (::connect(sock_rt, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) == -1 ) 
	{
		close(sock_rt);
		return false;
	}

	len = sizeof(iface_out);
	if (getsockname(sock_rt, (struct sockaddr *)&iface_out, &len) == -1 ) 
	{
		close(sock_rt);
		return false;
	}

	close(sock_rt);
	if (iface_out.sin_addr.s_addr == 0)
	{ /* what is this case?? */
		return false;
	}

	return true;

#endif
}

#if defined(WIN32)
bool NetworkObserver::ppl_dns_get_local_fqdn (char **servername, char **serverip,char **netmask, unsigned int WIN32_interface)
{
	unsigned int pos;

	*servername = NULL; /* no name on win32? */
	*serverip   = NULL;
	*netmask    = NULL;

	/* First, try to get the interface where we should listen */
	{
		DWORD size_of_iptable = 0;
		PMIB_IPADDRTABLE ipt;
		PMIB_IFROW ifrow;

		if (GetIpAddrTable(NULL, &size_of_iptable, TRUE) == ERROR_INSUFFICIENT_BUFFER)
		{
			ifrow = (PMIB_IFROW) _alloca (sizeof(MIB_IFROW));
			ipt = (PMIB_IPADDRTABLE) _alloca (size_of_iptable);
			if (ifrow==NULL || ipt==NULL)
			{
				/* not very usefull to continue */
				return true;
			}

			if (!GetIpAddrTable(ipt, &size_of_iptable, TRUE))
			{
				/* look for the best public interface */

				for (pos=0; pos < ipt->dwNumEntries && *netmask==NULL ; ++pos)
				{
					/* index is */
					struct in_addr addr;
					struct in_addr mask;
					ifrow->dwIndex = ipt->table[pos].dwIndex;
					if (GetIfEntry(ifrow) == NO_ERROR)
					{
						switch(ifrow->dwType)
						{
						case MIB_IF_TYPE_LOOPBACK:
						  /*	break; */
						case MIB_IF_TYPE_ETHERNET:
						default:
							addr.s_addr = ipt->table[pos].dwAddr;
							mask.s_addr = ipt->table[pos].dwMask;
							if (ipt->table[pos].dwIndex == WIN32_interface)
							{
								*servername = NULL; /* no name on win32? */
								*serverip   = inet_ntoa(addr);
								*netmask    = inet_ntoa(mask);
								break;
							}
						}
					}
				}
			}
		}
	}

	if (*serverip==NULL || *netmask==NULL)
	{
		return false;
	}

	return true;
}
#endif
