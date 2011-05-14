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

#ifndef OWSAFECONNECT_H
#define OWSAFECONNECT_H

#include <qtutil/owqtutildll.h>

#include <util/NonCopyable.h>

#include <QtCore/Qt>

class QObject;


#define SAFE_CONNECT(sender, signal, method) \
SafeConnect::connect(sender, signal, this, method)

#define SAFE_CONNECT_TYPE(sender, signal, method, type) \
SafeConnect::connect(sender, signal, this, method, type)

#define SAFE_CONNECT_RECEIVER(sender, signal, receiver, method) \
SafeConnect::connect(sender, signal, receiver, method)

#define SAFE_CONNECT_RECEIVER_TYPE(sender, signal, receiver, method, type) \
SafeConnect::connect(sender, signal, receiver, method, type)


#define SAFE_DISCONNECT() \
SafeConnect::disconnect(NULL, NULL, NULL, NULL)

#define SAFE_DISCONNECT_SIGNAL(signal) \
SafeConnect::disconnect(signal, NULL, NULL)

#define SAFE_DISCONNECT_SIGNAL_RECEIVER(signal, receiver) \
SafeConnect::disconnect(this, signal, receiver, NULL)

#define SAFE_DISCONNECT_SIGNAL_RECEIVER_METHOD(signal, receiver, method) \
SafeConnect::disconnect(this, signal, receiver, method)

#define SAFE_DISCONNECT_SENDER_SIGNAL_RECEIVER_METHOD(sender, signal, receiver, method) \
SafeConnect::disconnect(sender, signal, receiver, method)


/**
 * Replace QObject::connect() and QObject::disconnect()
 *
 * Check if QObject::connect() and QObject::disconnect() succeed or not.
 * If not then an assertion will occur.
 *
 * This class was added because it's boring to check the console to see
 * a QObject::connect() failed or not: it's better to have an assertion failed.
 *
 * @see QObject::connect()
 * @see QObject::disconnect()
 * @author Tanguy Krotoff
 */
class SafeConnect : NonCopyable {
public:

	/**
	 * @see QObject::connect()
	 */
	OWQTUTIL_API static void connect(const QObject * sender, const char * signal, const QObject * receiver, const char * method,
			Qt::ConnectionType type = Qt::AutoCompatConnection);

	/**
	 * @see QObject::disconnect()
	 */
	OWQTUTIL_API static void disconnect(const QObject * sender, const char * signal, const QObject * receiver, const char * method);
};

#endif	//OWSAFECONNECT_H
