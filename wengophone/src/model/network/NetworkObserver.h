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

#ifndef NETWORKOBSERVER_H
#define NETWORKOBSERVER_H

//#include <thread/Timer.h>
#include <util/Event.h>
#include <util/Trackable.h>
#include <QtCore/QObject>
/**
 * Watch the connection status.
 *
 * @ingroup model
 * @author Julien Bossart
 */
class NetworkObserver :  public QObject, public Trackable
{
	Q_OBJECT

public:

	/**
	 * Emitted when the connection is going up.
	 */
	Event< void (NetworkObserver & sender) > connectionIsUpEvent;

	/**
	 * Emitted when the connection is going down.
	 */
	Event< void (NetworkObserver & sender)> connectionIsDownEvent;

	bool isConnected()/* const { return _isConnected; }*/;

	static NetworkObserver & getInstance();
	
	//void init();

private:

	NetworkObserver();

	~NetworkObserver();

	bool is_connection_available();

	//void timeoutEventHandler();
	
	static NetworkObserver *_staticInstance;

	int _idTimer;
	//Timer _timer;

	bool _isConnected;

	//int _prevstate;

	bool canConnect();

#if defined(WIN32)
	bool ppl_dns_get_local_fqdn (char **servername, char **serverip,char **netmask, unsigned int WIN32_interface);
#endif

protected:
	void timerEvent(QTimerEvent*);
};

#endif /*NETWORKOBSERVER_H*/
