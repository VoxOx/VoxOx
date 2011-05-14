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

#include <owlibc.h>

#include <stdlib.h>
#include <string.h>

static pthread_mutex_t owsl_initialization_mutex = PTHREAD_MUTEX_INITIALIZER ;
static size_t owsl_initialization_count = 0 ;

int
owsl_initialize
(void)
{
	int is_first ;

	/* only the first call is effective */
	if (pthread_mutex_lock (& owsl_initialization_mutex))
	{
		return -1 ;
	}
	owsl_initialization_count ++ ;
	is_first = (owsl_initialization_count == 1 ? 1 : 0) ;
	if (pthread_mutex_unlock (& owsl_initialization_mutex))
	{
		return -1 ;
	}
	if (! is_first)
	{
		return 0 ;
	}

	/* init system socket library */
	if (owsl_system_socket_initialize ())
	{
		return -1 ;
	}

	if (owsl_openssl_wrapper_initialize ())
	{
		owsl_system_socket_terminate () ;
		return -1 ;
	}

	/* init socket types */
	if (owsl_socket_type_initialize_all ())
	{
		owsl_openssl_wrapper_terminate () ;
		owsl_system_socket_terminate () ;
		return -1 ;
	}

	/* init socket array */
	if (owsl_socket_initialize ())
	{
		owsl_socket_type_terminate_all () ;
		owsl_openssl_wrapper_terminate () ;
		owsl_system_socket_terminate () ;
		return -1 ;
	}

	/* init asynchronous mechanism */
	if (owsl_asynchronous_initialize ())
	{
		owsl_socket_terminate () ;
		owsl_socket_type_terminate_all () ;
		owsl_openssl_wrapper_terminate () ;
		owsl_system_socket_terminate () ;
		return -1 ;
	}

	/* init socket monitor */
	if (owsl_monitor_start ())
	{
		owsl_asynchronous_terminate () ;
		owsl_socket_terminate () ;
		owsl_socket_type_terminate_all () ;
		owsl_openssl_wrapper_terminate () ;
		owsl_system_socket_terminate () ;
		return -1 ;
	}

	return 0 ;
}

int
owsl_terminate
(void)
{
	int return_code = 0 ;
	int is_last ;

	/* only the last call is effective */
	return_code |= pthread_mutex_lock (& owsl_initialization_mutex) ;
	if (owsl_initialization_count > 0)
	{
		owsl_initialization_count -- ;
	}
	else
	{
		/* too many terminations */
		return -1 ;
	}
	is_last = (owsl_initialization_count == 0 ? 1 : 0) ;
	return_code |= pthread_mutex_unlock (& owsl_initialization_mutex) ;
	if (! is_last)
	{
		return return_code ;
	}

	/* free socket monitor */
	return_code |= owsl_monitor_stop () ;

	/* free asynchronous mechanism */
	return_code |= owsl_asynchronous_terminate () ;

	/* free socket array */
	return_code |= owsl_socket_terminate () ;

	/* free socket types */
	return_code |= owsl_socket_type_terminate_all () ;

	/* free openssl wrapper */
	return_code |= owsl_openssl_wrapper_terminate () ;

	/* free system socket library */
	return_code |= owsl_system_socket_terminate () ;

	return return_code ;
}


OWSLSocketType
owsl_type_get
(
	OWSLSocket socket
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL)
	{
		return OWSL_TYPE_UNKNOWN ;
	}
	return socket_info->type_info->type ;
}

OWSLAddressFamily
owsl_address_family_get
(
	OWSLSocket socket
)
{
	return owsl_socket_type_address_family_get (owsl_type_get (socket)) ;
}

OWSLSocketMode
owsl_mode_get
(
	OWSLSocket socket
)
{
	return owsl_socket_type_mode_get (owsl_type_get (socket)) ;
}

OWSLCiphering
owsl_ciphering_get
(
	OWSLSocket socket
)
{
	return owsl_socket_type_ciphering_get (owsl_type_get (socket)) ;
}

int
owsl_blocking_mode_set
(
	OWSLSocket socket,
	OWSLBlockingMode mode
)
{
	OWQueueBlockingMode queue_mode ;
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL)
	{
		return -1 ;
	}

	if (mode == OWSL_BLOCKING && socket_info->callback_function != NULL)
	{
		return -1 ;
	}

	switch (mode)
	{
		case OWSL_BLOCKING :
		{
			queue_mode = OWQUEUE_BLOCKING ;
			break ;
		}
		case OWSL_NON_BLOCKING :
		{
			queue_mode = OWQUEUE_NON_BLOCKING ;
			break ;
		}
		default :
		{
			return -1 ;
		}
	}

	if (socket_info->in_queue != NULL)
	{
		owqueue_blocking_mode_set (socket_info->in_queue, queue_mode) ;
	}
	if (socket_info->out_queue != NULL)
	{
		owqueue_blocking_mode_set (socket_info->out_queue, queue_mode) ;
	}

	if (socket_info->type_info->blocking_mode_set != NULL)
	{
		if (socket_info->type_info->blocking_mode_set (socket_info, mode))
		{
			return -1 ;
		}
	}

	socket_info->blocking_mode = mode ;
	return 0 ;
}

OWSLBlockingMode
owsl_blocking_mode_get
(
	OWSLSocket socket
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL)
	{
		return OWSL_BLOCKING_UNKNOWN ;
	}
	return socket_info->blocking_mode ;
}

int
owsl_parameter_set
(
	OWSLSocket socket,
	const char * name,
	const void * value
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->parameter_set == NULL)
	{
		return -1 ;
	}

	return socket_info->type_info->parameter_set (socket_info, name, value) ;
}

int
owsl_reuse_set
(
	OWSLSocket socket
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->reuse_set == NULL)
	{
		return -1 ;
	}

	return socket_info->type_info->reuse_set (socket_info) ;
}

OWSLSocket
owsl_socket
(
	OWSLAddressFamily address_family,
	OWSLSocketMode mode,
	OWSLCiphering ciphering
)
{
	return owsl_socket_by_type
	(
		owsl_socket_type_get
		(
			address_family,
			mode,
			ciphering
		)
	) ;
}

OWSLSocket
owsl_socket_by_type
(
	OWSLSocketType type
)
{
	OWSLSocket socket ;
	OWSLSocketInfo * socket_info ;
	pthread_mutex_t mutex ;

	if (type >= OWSL_TYPE_MAX)
	{
		return -1 ;
	}

	/* 2 sockets must not be created with the same handle */
	/* begin of critical section */
	if (pthread_mutex_init (& mutex, NULL))
	{
		return -1 ;
	}
	if (pthread_mutex_lock (& mutex))
	{
		pthread_mutex_destroy (& mutex) ;
		return -1 ;
	}

	socket = owsl_socket_handle_get_new () ;
	if (socket < 0)
	{
		pthread_mutex_unlock (& mutex) ;
		pthread_mutex_destroy (& mutex) ;
		return -1 ;
	}

	socket_info = owsl_socket_type_info_get (type)->socket (type) ;
	if (socket_info == NULL)
	{
		pthread_mutex_unlock (& mutex) ;
		pthread_mutex_destroy (& mutex) ;
		return -1 ;
	}
	socket_info->socket = socket ;

	owsl_socket_handle_set (socket, socket_info) ;

	/* end of critical section */
	pthread_mutex_unlock (& mutex) ;
	pthread_mutex_destroy (& mutex) ;

	return socket ;
}

OWSLSocket
owsl_accept
(
	OWSLSocket listening_socket,
	struct sockaddr * address,
	socklen_t * address_length
)
{
	OWSLSocket socket ;
	OWSLSocketInfo * socket_info ;
	OWSLSocketTypeInfo * type_info ;
	pthread_mutex_t mutex ;
	OWSLSocketInfo * listening_socket_info = owsl_socket_info_get (listening_socket) ;
	if (listening_socket_info == NULL || listening_socket_info->type_info->accept == NULL || listening_socket_info->listening == 0)
	{
		return -1 ;
	}

	type_info = listening_socket_info->type_info ;

	/* 2 sockets must not be created with the same handle */
	/* begin of critical section */
	if (pthread_mutex_init (& mutex, NULL))
	{
		return -1 ;
	}
	if (pthread_mutex_lock (& mutex))
	{
		pthread_mutex_destroy (& mutex) ;
		return -1 ;
	}

	socket = owsl_socket_handle_get_new () ;
	if (socket < 0)
	{
		pthread_mutex_unlock (& mutex) ;
		pthread_mutex_destroy (& mutex) ;
		return -1 ;
	}

	socket_info = type_info->accept (listening_socket_info, address, address_length) ;
	if (socket_info == NULL)
	{
		pthread_mutex_unlock (& mutex) ;
		pthread_mutex_destroy (& mutex) ;
		return -1 ;
	}
	socket_info->socket = socket ;

	owsl_socket_handle_set (socket, socket_info) ;

	/* end of critical section */
	pthread_mutex_unlock (& mutex) ;
	pthread_mutex_destroy (& mutex) ;

	socket_info->connected = 1 ;
	socket_info->listening = -1 ;

	return socket ;
}

int
owsl_close
(
	OWSLSocket socket
)
{
	int return_code ;
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->close == NULL)
	{
		return -1 ;
	}

	/* set socket as free */
	owsl_socket_handle_free (socket) ;

	/* call type-specific close function */
	return_code = socket_info->type_info->close (socket_info) ;

	return return_code ;
}

int
owsl_bind
(
	OWSLSocket socket,
	const struct sockaddr * address,
	socklen_t address_length
)
{
	int return_code ;
	int minimum_address_length ;
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->bind == NULL)
	{
		return -1 ;
	}

	return_code = socket_info->type_info->bind (socket_info, address, address_length) ;
	if (return_code == 0)
	{
		if (((struct sockaddr *) (& socket_info->bound_address))->sa_family == AF_MAX)
		{
			minimum_address_length = OW_MIN (OW_MAX (address_length, 0), (int) sizeof (socket_info->bound_address)) ;
			memcpy (& socket_info->bound_address, address, minimum_address_length) ;
		}
	}

	return return_code ;
}

int
owsl_connect
(
	OWSLSocket socket,
	const struct sockaddr * address,
	socklen_t address_length
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->connect == NULL || socket_info->connected != 0)
	{
		return -1 ;
	}

	if (owsl_socket_listen_disable (socket_info))
	{
		return -1 ;
	}

	return socket_info->type_info->connect (socket_info, address, address_length) ;
}

int
owsl_listen
(
	OWSLSocket socket,
	int pending_max
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->listen == NULL || socket_info->listening != 0)
	{
		return -1 ;
	}

	if (owsl_socket_listen_activate (socket_info))
	{
		return -1 ;
	}

	return socket_info->type_info->listen (socket_info, pending_max) ;
}

int
owsl_send
(
	OWSLSocket socket,
	const void * buffer,
	int length,
	int flags
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->send == NULL)
	{
		return -1 ;
	}

	if (owsl_socket_listen_disable (socket_info))
	{
		return -1 ;
	}

	return socket_info->type_info->send (socket_info, buffer, length, flags) ;
}

int
owsl_recv
(
	OWSLSocket socket,
	void * buffer,
	int size,
	int flags
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->recv == NULL)
	{
		return -1 ;
	}

	if (owsl_socket_listen_disable (socket_info))
	{
		return -1 ;
	}

	return socket_info->type_info->recv (socket_info, buffer, size, flags) ;
}

int
owsl_sendto
(
	OWSLSocket socket,
	const void * buffer,
	int length,
	int flags,
	const struct sockaddr * address,
	socklen_t address_length
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->sendto == NULL)
	{
		return -1 ;
	}

	if (owsl_socket_listen_disable (socket_info))
	{
		return -1 ;
	}

	return socket_info->type_info->sendto (socket_info, buffer, length, flags, address, address_length) ;
}

int
owsl_recvfrom
(
	OWSLSocket socket,
	void * buffer,
	int size,
	int flags,
	struct sockaddr * address,
	socklen_t * address_length
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->recvfrom == NULL)
	{
		return -1 ;
	}

	if (owsl_socket_listen_disable (socket_info))
	{
		return -1 ;
	}

	return socket_info->type_info->recvfrom (socket_info, buffer, size, flags, address, address_length) ;
}

int
owsl_name_set
(
	OWSLSocket socket,
	const char * name
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL)
	{
		return -1 ;
	}

	if (socket_info->name != NULL)
	{
		free (socket_info->name) ;
	}
	socket_info->name = strdup (name) ;
	if (socket_info->name == NULL)
	{
		return -1 ;
	}

	return 0 ;
}

char *
owsl_name_get
(
	OWSLSocket socket
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL)
	{
		return NULL ;
	}

	return socket_info->name ;
}

struct sockaddr *
owsl_bound_address_get
(
	OWSLSocket socket
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL)
	{
		return NULL ;
	}

	return (struct sockaddr *) & socket_info->bound_address ;
}

struct sockaddr *
owsl_remote_address_get
(
	OWSLSocket socket
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL || socket_info->type_info->remote_address_get == NULL)
	{
		return NULL ;
	}
	return socket_info->type_info->remote_address_get (socket_info) ;
}
