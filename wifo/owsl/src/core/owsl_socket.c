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

#include "owsl_internal.h"

#include <stdlib.h>
#include <memory.h>

/* mapping between a socket handle and its info structure */
static OWSLSocketInfo * owsl_socket_array [OWSL_SOCKET_MAX] ;
/* last allocated handle (for allocation increment) */
static int owsl_last_socket ;

int
owsl_socket_initialize
(void)
{
	memset (owsl_socket_array, 0, sizeof (owsl_socket_array)) ;
	owsl_last_socket = OWSL_SOCKET_MAX ;
	return 0 ;
}

int
owsl_socket_terminate
(void)
{
	OWSLSocket socket ;
	int return_code = 0 ;

	for (socket = 1 ; socket <= OWSL_SOCKET_MAX ; socket ++)
	{
		if (owsl_socket_info_get (socket) != NULL)
		{
			return_code |= owsl_close (socket) ;
		}
	}

	return return_code ;
}

OWSLSocket
owsl_socket_handle_get_new
(void)
{
	OWSLSocket socket = owsl_last_socket ;
	while (1)
	{
		/* socket must be in [1..OWSL_SOCKET_MAX] */
		socket = socket % OWSL_SOCKET_MAX + 1 ;
		if (owsl_socket_array [socket - 1] == NULL)
		{
			return socket ;  /* a free handle is found */
		}
		if (socket == owsl_last_socket)
		{
			return -1 ;   /* full */
		}
	}
}

void
owsl_socket_handle_set
(
	OWSLSocket socket,
	OWSLSocketInfo * socket_info
)
{
	owsl_socket_array [socket - 1] = socket_info ;
	owsl_last_socket = socket ;
	return ;
}

void
owsl_socket_handle_free
(
	OWSLSocket socket
)
{
	owsl_socket_array [socket - 1] = NULL ;
	return ;
}

OWSLSocketInfo *
owsl_socket_info_new
(
	OWSLSocketType type,
	size_t size,
	int in_queue_usable_size,
	OWQueuePacketMode in_queue_packet_mode,
	int in_queue_packet_max,
	int in_queue_packet_info_size,
	int out_queue_usable_size,
	OWQueuePacketMode out_queue_packet_mode,
	int out_queue_packet_max,
	int out_queue_packet_info_size
)
{
	OWSLSocketInfo * socket_info = malloc (size) ;
	if (socket_info != NULL)
	{
		socket_info->type_info = owsl_socket_type_info_get (type) ;
		socket_info->blocking_mode = OWSL_BLOCKING ;
		if (in_queue_usable_size > 0)
		{
			socket_info->in_queue = owqueue_new
			(
				in_queue_usable_size,
				in_queue_packet_mode,
				in_queue_packet_max,
				in_queue_packet_info_size
			) ;
			if (socket_info->in_queue == NULL)
			{
				free (socket_info) ;
				return NULL ;
			}
			owqueue_callback_set (socket_info->in_queue, (owsl_socket_type_info_get (type))->on_queue_event, socket_info) ;
		}
		else
		{
			socket_info->in_queue = NULL ;
		}
		if (out_queue_usable_size > 0)
		{
			socket_info->out_queue = owqueue_new
			(
				out_queue_usable_size,
				out_queue_packet_mode,
				out_queue_packet_max,
				out_queue_packet_info_size
			) ;
			if (socket_info->out_queue == NULL)
			{
				if (in_queue_usable_size > 0)
				{
					owqueue_free (socket_info->in_queue) ;
				}
				free (socket_info) ;
				return NULL ;
			}
			owqueue_callback_set (socket_info->out_queue, (owsl_socket_type_info_get (type))->on_queue_event, socket_info) ;
		}
		else
		{
			socket_info->out_queue = NULL ;
		}
		if (pthread_mutex_init (& socket_info->listening_mutex, NULL))
		{
			if (out_queue_usable_size > 0)
			{
				owqueue_free (socket_info->out_queue) ;
			}
			if (in_queue_usable_size > 0)
			{
				owqueue_free (socket_info->in_queue) ;
			}
			free (socket_info) ;
			return NULL ;
		}
		socket_info->listening = 0 ;
		socket_info->connected = 0 ;
		socket_info->error = 0 ;
		socket_info->callback_function = NULL ;
		socket_info->callback_user_data = NULL ;
		memset (& socket_info->bound_address, 0, sizeof (socket_info->bound_address)) ;
		((struct sockaddr *) (& socket_info->bound_address))->sa_family = AF_MAX ;
		socket_info->name = NULL ;
	}
	return socket_info ;
}

int
owsl_socket_info_free
(
	OWSLSocketInfo * socket
)
{
	int return_code = 0 ;

	return_code |= pthread_mutex_destroy (& socket->listening_mutex) ;
	if (socket->out_queue != NULL)
	{
		return_code |= owqueue_free (socket->out_queue) ;
	}
	if (socket->in_queue != NULL)
	{
		return_code |= owqueue_free (socket->in_queue) ;
	}
	if (socket->name != NULL)
	{
		free (socket->name) ;
	}

	free (socket) ;

	return return_code ;
}

OWSLSocketInfo *
owsl_socket_info_get
(
	OWSLSocket socket
)
{
	if (socket <= 0 || socket > OWSL_SOCKET_MAX)
	{
		return NULL ;
	}
	return owsl_socket_array [socket - 1] ;
}

int
owsl_socket_listen_disable
(
	OWSLSocketInfo * socket
)
{
	if (pthread_mutex_lock (& socket->listening_mutex))
	{
		return -1 ;
	}

	if (socket->listening == 0)
	{
		socket->listening = -1 ;
	}
	else if (socket->listening == 1)
	{
		pthread_mutex_unlock (& socket->listening_mutex) ;
		return -1 ;
	}

	if (pthread_mutex_unlock (& socket->listening_mutex))
	{
		return -1 ;
	}

	return 0 ;
}

int
owsl_socket_listen_activate
(
	OWSLSocketInfo * socket
)
{
	if (pthread_mutex_lock (& socket->listening_mutex))
	{
		return -1 ;
	}

	if (socket->listening == 0)
	{
		socket->listening = 1 ;
	}
	else if (socket->listening == -1)
	{
		pthread_mutex_unlock (& socket->listening_mutex) ;
		return -1 ;
	}

	if (pthread_mutex_unlock (& socket->listening_mutex))
	{
		return -1 ;
	}

	return 0 ;
}

int
owsl_socket_is_readable
(
	OWSLSocketInfo * socket
)
{
	return
	(
		(
			socket->type_info->mode == OWSL_MODE_DATAGRAM
			||
			socket->connected > 0
			||
			socket->listening > 0
		)
		&&
		(
			socket->in_queue == NULL
			||
			! owqueue_is_empty (socket->in_queue)
		)
		&&
		(
			socket->type_info->is_readable == NULL
			||
			socket->type_info->is_readable (socket)
		)
	) ;
}

int
owsl_socket_is_writable
(
	OWSLSocketInfo * socket
)
{
	return
	(
		(
			socket->type_info->mode == OWSL_MODE_DATAGRAM
			||
			socket->connected > 0
		)
		&&
		(
			socket->out_queue == NULL
			||
			! owqueue_is_full (socket->out_queue)
		)
		&&
		(
			socket->type_info->is_writable == NULL
			||
			socket->type_info->is_writable (socket)
		)
	) ;
}

int
owsl_socket_has_error
(
	OWSLSocketInfo * socket
)
{
	return
	(
		socket->error != 0
		||
		(
			socket->type_info->has_error != NULL
			&&
			socket->type_info->has_error (socket)
		)
	) ;
}
