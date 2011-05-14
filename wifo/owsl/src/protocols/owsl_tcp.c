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

#define OWSL_TCP_PACKET_AVG_SIZE 1000
#define OWSL_TCP_BUFFER_SIZE 2000

static void
owsl_tcp_system_socket_accept
(
	OWSLSocketInfo_TCP * socket
)
{
	OWSLConnection connection ;
	int written_bytes ;

	/* if event is received, then monitoring is enabled */
	/* if monitoring is enabled, then queue is not full */
	/* if queue is not full, connection can be accepted and pushed */

	/* accept incoming connection */
	connection.remote_address_length = sizeof (connection.remote_address) ;
	connection.system_socket = accept (socket->system_socket, (struct sockaddr *) & connection.remote_address, & connection.remote_address_length) ;
	if (owsl_system_socket_is_valid (connection.system_socket))
	{
		/* push connection in the input queue */
		written_bytes = owqueue_write (socket->in_queue, & connection, sizeof (connection), NULL, OWQUEUE_NON_BLOCKING) ;
		if (written_bytes == sizeof (connection))
		{
			/* notify that the socket can accept a new connection */
			owsl_notify ((OWSLSocketInfo *) socket, OWSL_EVENT_READ) ;
		}
	}

	return ;
}

static void
owsl_tcp_system_socket_recv
(
	OWSLSocketInfo_TCP * socket
)
{
	int received_bytes, written_bytes ;
	char buffer [OWSL_TCP_BUFFER_SIZE] ;

	/* get packet and packet size */
	received_bytes = recv (socket->system_socket, buffer, OWSL_TCP_BUFFER_SIZE, MSG_PEEK) ;
	if (received_bytes > 0)
	{
		/* try to push packet in the input queue */
		written_bytes = owqueue_write (socket->in_queue, buffer, received_bytes, NULL, OWQUEUE_NON_BLOCKING | OWQUEUE_TRANSACTION_OPEN) ;
		if (written_bytes > 0)
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
	else if (received_bytes == 0)
	{
		socket->remotely_closed = 1 ;
		owsl_monitor_event_remove (socket->system_socket, OWSL_MONITOR_READ) ;
	}

	return ;
}

static void
owsl_tcp_system_socket_send
(
	OWSLSocketInfo_TCP * socket
)
{
	int read_bytes, sent_bytes ;
	char buffer [OWSL_TCP_BUFFER_SIZE] ;

	/* get packet and packet size */
	read_bytes = owqueue_read (socket->out_queue, buffer, OWSL_TCP_BUFFER_SIZE, NULL, OWQUEUE_NON_BLOCKING | OWQUEUE_TRANSACTION_OPEN) ;
	if (read_bytes > 0)
	{
		/* send packet */
		sent_bytes = send (socket->system_socket, buffer, read_bytes, 0) ;
		/* commit or rollback transaction */
		owqueue_read_transaction_close (socket->out_queue, sent_bytes > 0 ? sent_bytes : 0) ;
	}

	return ;
}

static void
owsl_tcp_monitor_callback
(
	OWSLSystemSocket system_socket,
	OWSLMonitorEvent event,
	void * _socket
)
{
	OWSLSocketInfo_TCP * socket = _socket ;

	if (event & OWSL_MONITOR_ERROR || socket->system_socket != system_socket)
	{
		owsl_notify ((OWSLSocketInfo *) socket, OWSL_EVENT_ERROR) ;
		return ;
	}

	if (event & OWSL_MONITOR_READ)
	{
		if (socket->listening > 0)
		{
			owsl_tcp_system_socket_accept (socket) ;
		}
		else
		{
			owsl_tcp_system_socket_recv (socket) ;
		}
	}

	if (event & OWSL_MONITOR_WRITE)
	{
		if (socket->connected == 0)
		{
			/* mark socket as connected */
			socket->connected = 1 ;
			owsl_notify ((OWSLSocketInfo *) socket, OWSL_EVENT_WRITE) ;
		}
		owsl_tcp_system_socket_send (socket) ;
	}

	return ;
}

static OWSLSocketInfo *
owsl_tcp_socket
(
	OWSLSocketType type
)
{
	OWSLSocketInfo * socket ;

	OWSLAddressFamily address_family ;
	switch (type)
	{
		case OWSL_TYPE_IPV4_TCP :
		{
			address_family = OWSL_AF_IPV4 ;
			break ;
		}
		case OWSL_TYPE_IPV6_TCP :
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
		sizeof (OWSLSocketInfo_TCP),                        /* size of socket structure */
		OWSL_TCP_PACKET_AVG_SIZE * OWSL_QUEUE_PACKET_MAX,   /* input queue usable size */
		OWQUEUE_NO_PACKET,                                  /* input queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* input queue packet maximum */
		0,                                                  /* input queue packet info size */
		OWSL_TCP_PACKET_AVG_SIZE * OWSL_QUEUE_PACKET_MAX,   /* output queue usable size */
		OWQUEUE_NO_PACKET,                                  /* output queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* output queue packet maximum */
		0                                                   /* output queue packet info size */
	) ;
	if (socket != NULL)
	{
		if (owsl_base_tcp_open ((OWSLSocketInfo_TCP *) socket, address_family, owsl_tcp_monitor_callback))
		{
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
	}

	return socket ;
}

static OWSLSocketInfo *
owsl_tcp_accept
(
	OWSLSocketInfo * listen_socket,
	struct sockaddr * address,
	socklen_t * address_length
)
{
	OWSLConnection connection ;
	OWSLSocketInfo * socket ;

	if (owqueue_read (listen_socket->in_queue, & connection, sizeof (connection), NULL, 0) != sizeof (connection))
	{
		return NULL ;
	}

	socket = owsl_socket_info_new
	(
		listen_socket->type_info->type,                     /* socket type */
		sizeof (OWSLSocketInfo_TCP),                        /* size of socket structure */
		OWSL_TCP_PACKET_AVG_SIZE * OWSL_QUEUE_PACKET_MAX,   /* input queue usable size */
		OWQUEUE_NO_PACKET,                                  /* input queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* input queue packet maximum */
		0,                                                  /* input queue packet info size */
		OWSL_TCP_PACKET_AVG_SIZE * OWSL_QUEUE_PACKET_MAX,   /* output queue usable size */
		OWQUEUE_NO_PACKET,                                  /* output queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* output queue packet maximum */
		0                                                   /* output queue packet info size */
	) ;
	if (socket != NULL)
	{
		if (owsl_base_tcp_set ((OWSLSocketInfo_TCP *) socket, & connection, owsl_tcp_monitor_callback))
		{
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		OW_MEMCPY (address, address_length, & connection.remote_address, connection.remote_address_length) ;
	}

	return socket ;
}

static int
owsl_tcp_close
(
	OWSLSocketInfo * socket
)
{
	int return_code = 0 ;

	return_code |= owsl_base_tcp_close ((OWSLSocketInfo_TCP *) socket) ;
	return_code |= owsl_socket_info_free (socket) ;

	return return_code ;
}

static int
owsl_tcp_connect
(
	OWSLSocketInfo * socket,
	const struct sockaddr * address,
	socklen_t address_length
)
{
	OWSLSocketInfo_TCP * socket_tcp = (OWSLSocketInfo_TCP *) socket ;
	int return_code = connect (socket_tcp->system_socket, address, address_length) ;
	if (return_code == 0 || OW_GET_ERROR == EINPROGRESS || OW_GET_ERROR == EWOULDBLOCK)
	{
		socket_tcp->remote_address_length = OWSL_ADDRESS_SIZE ;
		OW_MEMCPY (& socket_tcp->remote_address, & socket_tcp->remote_address_length, address, address_length) ;
		if (return_code == 0)
		{
			socket->connected = 1 ;
		}
		else /* OW_GET_ERROR == EINPROGRESS || EWOULDBLOCK */
		{
			owsl_monitor_event_add (socket_tcp->system_socket, OWSL_MONITOR_WRITE | OWSL_MONITOR_ONCE) ;
			OW_SET_ERROR (EINPROGRESS) ;
		}
	}
	return return_code ;
}

int
owsl_tcp_initialize
(void)
{
	OWSLSocketTypeInfo type_ipv4 ;
	OWSLSocketTypeInfo type_ipv6 ;

	type_ipv4.type = OWSL_TYPE_IPV4_TCP ;
	type_ipv4.address_family = OWSL_AF_IPV4 ;
	type_ipv4.mode = OWSL_MODE_STREAM ;
	type_ipv4.ciphering = OWSL_CIPHERING_DISABLED ;
	type_ipv4.global_parameter_set = NULL ;
	type_ipv4.is_readable = NULL ;
	type_ipv4.is_writable = NULL ;
	type_ipv4.has_error = NULL ;
	type_ipv4.blocking_mode_set = owsl_base_system_socket_blocking_mode_set ;
	type_ipv4.parameter_set = NULL ;
	type_ipv4.reuse_set = owsl_base_system_socket_reuse_set ;
	type_ipv4.remote_address_get = owsl_base_remote_address_get ;
	type_ipv4.on_queue_event = owsl_base_in_out_queues_callback_with_monitor ;
	type_ipv4.socket = owsl_tcp_socket ;
	type_ipv4.accept = owsl_tcp_accept ;
	type_ipv4.close = owsl_tcp_close ;
	type_ipv4.bind = owsl_base_bind ;
	type_ipv4.connect = owsl_tcp_connect ;
	type_ipv4.listen = owsl_base_in_queue_listen ;
	type_ipv4.send = owsl_base_out_queue_send ;
	type_ipv4.recv = owsl_base_in_queue_connected_recv ;
	type_ipv4.sendto = owsl_base_out_queue_sendto ;
	type_ipv4.recvfrom = owsl_base_in_queue_connected_recvfrom ;

	memcpy (& type_ipv6, & type_ipv4, sizeof (OWSLSocketTypeInfo)) ;
	type_ipv6.type = OWSL_TYPE_IPV6_TCP ;
	type_ipv6.address_family = OWSL_AF_IPV6 ;

	return owsl_socket_type_initialize (& type_ipv4)
		|| owsl_socket_type_initialize (& type_ipv6) ;
}
