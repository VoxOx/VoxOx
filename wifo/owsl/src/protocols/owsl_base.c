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
#include <errno.h>

int
owsl_base_system_socket_blocking_mode_set
(
	OWSLSocketInfo * socket_info_with_system_socket,
	OWSLBlockingMode mode
)
{
	OWSLSocketInfo_with_SystemSocket * socket = (OWSLSocketInfo_with_SystemSocket *) socket_info_with_system_socket ;
	return owsl_system_socket_blocking_mode_set (socket->system_socket, mode) ;
}

int
owsl_base_system_socket_reuse_set
(
	OWSLSocketInfo * socket_info_with_system_socket
)
{
	OWSLSocketInfo_with_SystemSocket * socket = (OWSLSocketInfo_with_SystemSocket *) socket_info_with_system_socket ;
	return owsl_system_socket_reuse_set (socket->system_socket) ;
}

struct sockaddr *
owsl_base_remote_address_get
(
	OWSLSocketInfo * socket_info_with_connected_system_socket
)
{
	OWSLSocketInfo_with_ConnectedSystemSocket * socket = (OWSLSocketInfo_with_ConnectedSystemSocket *) socket_info_with_connected_system_socket ;
	if (socket->remote_address_length > 0 && ((struct sockaddr *) (& socket->remote_address))->sa_family != AF_MAX)
	{
		return (struct sockaddr *) & socket->remote_address ;
	}
	else
	{
		return NULL ;
	}
}

void
owsl_base_in_queue_callback_with_monitor
(
	OWQueue * queue,
	OWQueueEvent event,
	void * socket_info_with_system_socket
)
{
	OWSLSocketInfo_with_SystemSocket * socket = socket_info_with_system_socket ;

	if (queue != socket->in_queue)
	{
		return ;
	}

	if (event & OWQUEUE_SPACE_NEEDED)
	{
		owsl_monitor_event_remove (socket->system_socket, OWSL_MONITOR_READ) ;
	}
	if (event & OWQUEUE_SPACE_AVAILABLE)
	{
		owsl_monitor_event_add (socket->system_socket, OWSL_MONITOR_READ) ;
	}

	return ;
}

void
owsl_base_out_queue_callback_with_monitor
(
	OWQueue * queue,
	OWQueueEvent event,
	void * socket_info_with_system_socket
)
{
	OWSLSocketInfo_with_SystemSocket * socket = socket_info_with_system_socket ;

	if (queue != socket->out_queue)
	{
		return ;
	}

	if (event & OWQUEUE_SPACE_AVAILABLE)
	{
		owsl_notify ((OWSLSocketInfo *) socket, OWSL_EVENT_WRITE) ;
	}
	if (event & OWQUEUE_NOT_EMPTY)
	{
		owsl_monitor_event_add (socket->system_socket, OWSL_MONITOR_WRITE) ;
	}
	if (event & OWQUEUE_EMPTY)
	{
		owsl_monitor_event_remove (socket->system_socket, OWSL_MONITOR_WRITE) ;
	}

	return ;
}

void
owsl_base_in_out_queues_callback_with_monitor
(
	OWQueue * queue,
	OWQueueEvent event,
	void * _socket_info_with_system_socket
)
{
	owsl_base_in_queue_callback_with_monitor (queue, event, _socket_info_with_system_socket) ;
	owsl_base_out_queue_callback_with_monitor (queue, event, _socket_info_with_system_socket) ;
	return ;
}

static int
owsl_base_register_in_monitor_for_read
(
	OWSLSocketInfo_with_SystemSocket * socket,
	OWSLMonitorCallback callback_function
)
{
	void * callback_user_data = socket ;
	if (owsl_monitor_socket_add (socket->system_socket, callback_function, callback_user_data))
	{
		return -1 ;
	}
	if (owsl_monitor_event_add (socket->system_socket, OWSL_MONITOR_READ))
	{
		owsl_monitor_socket_remove (socket->system_socket) ;
		return -1 ;
	}
	return 0 ;
}

int
owsl_base_udp_open
(
	OWSLSocketInfo_with_SystemSocket * socket,
	OWSLAddressFamily address_family,
	OWSLMonitorCallback callback_function
)
{
	socket->system_socket = owsl_system_socket_open (address_family, OWSL_MODE_DATAGRAM) ;
	if (! owsl_system_socket_is_valid (socket->system_socket))
	{
		return -1 ;
	}

	if (owsl_base_register_in_monitor_for_read (socket, callback_function))
	{
		owsl_monitor_socket_remove (socket->system_socket) ;
		return -1 ;
	}

	return 0 ;
}

int
owsl_base_udp_close
(
	OWSLSocketInfo_with_SystemSocket * socket
)
{
	int return_code = 0 ;
	return_code |= owsl_monitor_socket_remove (socket->system_socket) ;
	return_code |= owsl_system_socket_close (socket->system_socket) ;
	return return_code ;
}

int
owsl_base_tcp_open
(
	OWSLSocketInfo_with_ConnectedSystemSocket * socket,
	OWSLAddressFamily address_family,
	OWSLMonitorCallback callback_function
)
{
	socket->system_socket = owsl_system_socket_open (address_family, OWSL_MODE_STREAM) ;
	if (! owsl_system_socket_is_valid (socket->system_socket))
	{
		return -1 ;
	}

	if (owsl_base_register_in_monitor_for_read ((OWSLSocketInfo_with_SystemSocket *) socket, callback_function))
	{
		owsl_monitor_socket_remove (socket->system_socket) ;
		return -1 ;
	}

	memset (& socket->remote_address, 0, sizeof (socket->remote_address)) ;
	((struct sockaddr *) (& socket->remote_address))->sa_family = AF_MAX ;
	socket->remote_address_length = 0 ;

	socket->remotely_closed = 0 ;

	return 0 ;
}

int
owsl_base_tcp_set
(
	OWSLSocketInfo_with_ConnectedSystemSocket * socket,
	OWSLConnection * connection,
	OWSLMonitorCallback callback_function
)
{
	socket->system_socket = connection->system_socket ;
	if (socket->system_socket < 0)
	{
		return -1 ;
	}

	if (owsl_base_register_in_monitor_for_read ((OWSLSocketInfo_with_SystemSocket *) socket, callback_function))
	{
		owsl_monitor_socket_remove (socket->system_socket) ;
		return -1 ;
	}

	memset (& socket->remote_address, 0, sizeof (socket->remote_address)) ;
	memcpy (& socket->remote_address, & connection->remote_address, connection->remote_address_length) ;
	socket->remote_address_length = connection->remote_address_length ;

	socket->remotely_closed = 0 ;

	return 0 ;
}

int
owsl_base_tcp_close
(
	OWSLSocketInfo_with_ConnectedSystemSocket * socket
)
{
	int return_code = 0 ;
	return_code |= owsl_monitor_socket_remove (socket->system_socket) ;
	return_code |= owsl_system_socket_close (socket->system_socket) ;
	return return_code ;
}

int
owsl_base_bind
(
	OWSLSocketInfo * socket_info_with_system_socket,
	struct sockaddr * address,
	socklen_t address_length
)
{
        int ret;
	OWSLSocketInfo_with_SystemSocket * socket = (OWSLSocketInfo_with_SystemSocket *) socket_info_with_system_socket ;
	ret = bind (socket->system_socket, address, address_length) ;
	if (!ret)
	{
	    socklen_t slen = address_length;
	    getsockname(socket->system_socket, address, &slen);
	}
	return ret;
}

int
owsl_base_in_queue_listen
(
	OWSLSocketInfo * socket_info_with_system_socket,
	int pending_max
)
{
	OWSLSocketInfo_with_SystemSocket * socket = (OWSLSocketInfo_with_SystemSocket *) socket_info_with_system_socket ;
	if (listen (socket->system_socket, pending_max))
	{
		return -1 ;
	}
	if (socket->out_queue != NULL)
	{
		if (owqueue_free (socket->out_queue))
		{
			return -1 ;
		}
		socket->out_queue = NULL ;
	}
	if (owqueue_free (socket->in_queue))
	{
		return -1 ;
	}
	/* queue size must be exactly a multiple of connection size to allow callback when it is just full */
	/* if full, callback will disable monitoring for incoming connections */
	socket->in_queue = owqueue_new
	(
		sizeof (OWSLConnection) * (pending_max / 2 + 1),   /* usable size */
		OWQUEUE_PACKET,                                    /* mode */
		pending_max / 2 + 1,                               /* packet maximum */
		0                                                  /* packet info size */
	) ;
	if (socket->in_queue == NULL)
	{
		return -1 ;
	}
	return 0 ;
}

int
owsl_base_in_queue_recv
(
	OWSLSocketInfo * socket,
	void * buffer,
	int size,
	int flags
)
{
	int read_bytes = owqueue_read (socket->in_queue, buffer, size, NULL, 0) ;
	if (read_bytes <= 0)
	{
		if (read_bytes == 0)
		{
			OW_SET_ERROR (EAGAIN) ;
		}
		read_bytes = -1 ;
	}
	return read_bytes ;
}

int
owsl_base_in_queue_connected_recv
(
	OWSLSocketInfo * socket_info_with_connected_system_socket,
	void * buffer,
	int size,
	int flags
)
{
	OWSLSocketInfo_with_ConnectedSystemSocket * socket = (OWSLSocketInfo_with_ConnectedSystemSocket *) socket_info_with_connected_system_socket ;

	int read_bytes = owqueue_read (socket->in_queue, buffer, size, NULL, socket->remotely_closed ? OWQUEUE_NON_BLOCKING : 0) ;
	if (read_bytes <= 0)
	{
		if (socket->remotely_closed)
		{
			read_bytes = 0 ;
		}
		else
		{
			read_bytes = -1 ;
		}
	}
	return read_bytes ;
}

int
owsl_base_in_queue_recvfrom
(
	OWSLSocketInfo * socket,
	void * buffer,
	int size,
	int flags,
	struct sockaddr * address,
	socklen_t * address_length
)
{
	OWSLRemoteAddress info ;

	int read_bytes = owqueue_read (socket->in_queue, buffer, size, & info, 0) ;
	if (read_bytes > 0)
	{
		OW_MEMCPY (address, address_length, & info.remote_address, info.remote_address_length) ;
	}
	else
	{
		read_bytes = -1 ;
	}

	return read_bytes ;
}

int
owsl_base_in_queue_connected_recvfrom
(
	OWSLSocketInfo * socket_info_with_connected_system_socket,
	void * buffer,
	int size,
	int flags,
	struct sockaddr * address,
	socklen_t * address_length
)
{
	OWSLSocketInfo_with_ConnectedSystemSocket * socket = (OWSLSocketInfo_with_ConnectedSystemSocket *) socket_info_with_connected_system_socket ;

	int read_bytes = owqueue_read (socket->in_queue, buffer, size, NULL, socket->remotely_closed ? OWQUEUE_NON_BLOCKING : 0) ;
	if (read_bytes > 0)
	{
		OW_MEMCPY (address, address_length, & socket->remote_address, socket->remote_address_length) ;
	}
	else
	{
		if (socket->remotely_closed)
		{
			read_bytes = 0 ;
		}
		else
		{
			read_bytes = -1 ;
		}
	}

	return read_bytes ;
}

int
owsl_base_out_queue_send
(
	OWSLSocketInfo * socket,
	const void * buffer,
	int length,
	int flags
)
{
	return owqueue_write (socket->out_queue, buffer, length, NULL, 0) ;
}

int
owsl_base_out_queue_sendto
(
	OWSLSocketInfo * socket,
	const void * buffer,
	int length,
	int flags,
	const struct sockaddr * address,
	socklen_t address_length
)
{
	/* address and address_length are ignored */
	return owqueue_write (socket->out_queue, buffer, length, NULL, 0) ;
}
