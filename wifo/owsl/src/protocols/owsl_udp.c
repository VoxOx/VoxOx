/*
 * Open Wengo Socket Library
 * Copyright (C) 2007  Wengo
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

#include "owsl_base.h"

#include <memory.h>

#define OWSL_UDP_PACKET_AVG_SIZE 1000
#define OWSL_UDP_BUFFER_SIZE 4000

static void
owsl_udp_system_socket_recv
(
	OWSLSocketInfo_UDP * socket
)
{
	int received_bytes, written_bytes ;
	char buffer [OWSL_UDP_BUFFER_SIZE] ;
	OWSLRemoteAddress info ;

	/* get packet, address and packet size */
	info.remote_address_length = sizeof (info.remote_address) ;
	received_bytes = recvfrom (socket->system_socket, buffer, OWSL_UDP_BUFFER_SIZE, MSG_PEEK, (struct sockaddr *) & info.remote_address, & info.remote_address_length) ;
	if (received_bytes > 0)
	{
		/* try to push packet and address in the input queue */
		written_bytes = owqueue_write (socket->in_queue, buffer, received_bytes, & info, OWQUEUE_NON_BLOCKING | OWQUEUE_TRANSACTION_OPEN) ;
		if (written_bytes == received_bytes)
		{
			/* remove packet from system socket buffer */
			received_bytes = recv (socket->system_socket, buffer, written_bytes, 0) ;
			/* commit or rollback transaction */
			owqueue_write_transaction_close (socket->in_queue, received_bytes == written_bytes ? received_bytes : 0) ;
			if (received_bytes == written_bytes)
			{
				/* notify that the socket can be read */
				owsl_notify ((OWSLSocketInfo *) socket, OWSL_EVENT_READ) ;
			}
		}
	}

	return ;
}

static void
owsl_udp_monitor_callback
(
	OWSLSystemSocket system_socket,
	OWSLMonitorEvent event,
	void * _socket
)
{
	OWSLSocketInfo_UDP * socket = _socket ;

	if (event & OWSL_MONITOR_ERROR || socket->system_socket != system_socket)
	{
		owsl_notify ((OWSLSocketInfo *) socket, OWSL_EVENT_ERROR) ;
		return ;
	}

	if (event & OWSL_MONITOR_READ)
	{
		owsl_udp_system_socket_recv (socket) ;
	}

	return ;
}

static OWSLSocketInfo *
owsl_udp_socket
(
	OWSLSocketType type
)
{
	OWSLSocketInfo * socket ;

	OWSLAddressFamily address_family ;
	switch (type)
	{
		case OWSL_TYPE_IPV4_UDP :
		{
			address_family = OWSL_AF_IPV4 ;
			break ;
		}
		case OWSL_TYPE_IPV6_UDP :
		{
			address_family = OWSL_AF_IPV6 ;
			break ;
		}
		default :
		{
			return NULL ;
		}
	}

	socket = owsl_socket_info_new
	(
		type,                                               /* socket type */
		sizeof (OWSLSocketInfo_UDP),                        /* size of socket structure */
		OWSL_UDP_PACKET_AVG_SIZE * OWSL_QUEUE_PACKET_MAX,   /* input queue usable size */
		OWQUEUE_PACKET,                                     /* input queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* input queue packet maximum */
		sizeof (OWSLRemoteAddress),                         /* input queue packet info size */
		0, 0, 0, 0                                          /* no output queue */
	) ;
	if (socket != NULL)
	{
		if (owsl_base_udp_open ((OWSLSocketInfo_UDP *) socket, address_family, owsl_udp_monitor_callback))
		{
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
	}

	return socket ;
}

static int
owsl_udp_close
(
	OWSLSocketInfo * socket
)
{
	int return_code = 0 ;

	return_code |= owsl_base_udp_close ((OWSLSocketInfo_UDP *) socket) ;
	return_code |= owsl_socket_info_free (socket) ;

	return return_code ;
}

static int
owsl_udp_connect
(
	OWSLSocketInfo * socket,
	const struct sockaddr * address,
	socklen_t address_length
)
{
	OWSLSocketInfo_UDP * socket_udp = (OWSLSocketInfo_UDP *) socket ;
	return connect (socket_udp->system_socket, address, address_length) ;
}

static int
owsl_udp_send
(
	OWSLSocketInfo * socket,
	const void * buffer,
	int length,
	int flags
)
{
	OWSLSocketInfo_UDP * socket_udp = (OWSLSocketInfo_UDP *) socket ;
	int sent_bytes ;

	sent_bytes = send (socket_udp->system_socket, buffer, length, flags) ;
	if (sent_bytes < length)
	{
		return -1 ;
	}

	return sent_bytes ;
}

static int
owsl_udp_sendto
(
	OWSLSocketInfo * socket,
	const void * buffer,
	int length,
	int flags,
	const struct sockaddr * address,
	socklen_t address_length
)
{
	OWSLSocketInfo_UDP * socket_udp = (OWSLSocketInfo_UDP *) socket ;
	int sent_bytes ;

	sent_bytes = sendto (socket_udp->system_socket, buffer, length, flags, address, address_length) ;
	if (sent_bytes < length)
	{
		return -1 ;
	}

	return sent_bytes ;
}

int
owsl_udp_initialize
(void)
{
	OWSLSocketTypeInfo type_ipv4 ;
	OWSLSocketTypeInfo type_ipv6 ;

	type_ipv4.type = OWSL_TYPE_IPV4_UDP ;
	type_ipv4.address_family = OWSL_AF_IPV4 ;
	type_ipv4.mode = OWSL_MODE_DATAGRAM ;
	type_ipv4.ciphering = OWSL_CIPHERING_DISABLED ;
	type_ipv4.global_parameter_set = NULL ;
	type_ipv4.is_readable = NULL ;
	type_ipv4.is_writable = NULL ;
	type_ipv4.has_error = NULL ;
	type_ipv4.blocking_mode_set = NULL ;
	type_ipv4.parameter_set = NULL ;
	type_ipv4.reuse_set = owsl_base_system_socket_reuse_set ;
	type_ipv4.remote_address_get = NULL ;
	type_ipv4.on_queue_event = owsl_base_in_queue_callback_with_monitor ;
	type_ipv4.socket = owsl_udp_socket ;
	type_ipv4.accept = NULL ;
	type_ipv4.close = owsl_udp_close ;
	type_ipv4.bind = owsl_base_bind ;
	type_ipv4.connect = owsl_udp_connect ;
	type_ipv4.listen = NULL ;
	type_ipv4.send = owsl_udp_send ;
	type_ipv4.recv = owsl_base_in_queue_recv ;
	type_ipv4.sendto = owsl_udp_sendto ;
	type_ipv4.recvfrom = owsl_base_in_queue_recvfrom ;

	memcpy (& type_ipv6, & type_ipv4, sizeof (OWSLSocketTypeInfo)) ;
	type_ipv6.type = OWSL_TYPE_IPV6_UDP ;
	type_ipv6.address_family = OWSL_AF_IPV6 ;

	return owsl_socket_type_initialize (& type_ipv4)
		|| owsl_socket_type_initialize (& type_ipv6) ;
}
