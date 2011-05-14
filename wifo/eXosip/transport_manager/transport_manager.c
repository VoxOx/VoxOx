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

#include "transport_manager.h"

#include <pthread.h>
#include <stdlib.h>

static TransportRecv transport_recv_callback = NULL ;
static TransportListenError transport_listen_error_callback = NULL ;

typedef enum TransportType
{
	TRANSPORT_DATA,       /* OWSL_DATAGRAM or connected OWSL_STREAM */
	TRANSPORT_LISTENING   /* listening OWSL_STREAM */
} TransportType ;

static OWList * transport_data_socket_list = NULL ;
static OWList * transport_listening_socket_list = NULL ;
static pthread_mutex_t transport_socket_lists_mutex ;

static void
transport_on_data_socket_event
(
	OWSLSocket socket,
	OWSLEvent event,
	void * user_data
) ;

static void
transport_on_listening_socket_event
(
	OWSLSocket socket,
	OWSLEvent event,
	void * user_data
) ;

const char *
transport_protocol_string_get
(
	TransportProtocol protocol
)
{
	/* must match with enum TransportProtocol */
	static const char * protocol_array [] = {"UDP", "TCP", "TLS"} ;
	return protocol_array [protocol] ;
}

static OWList *
transport_socket_list_get
(
	TransportType type,
	OWSLSocketMode mode
)
{
	switch (type)
	{
		case TRANSPORT_DATA :
		{
			return transport_data_socket_list ;
		}
		case TRANSPORT_LISTENING :
		{
			switch (mode)
			{
				case OWSL_MODE_DATAGRAM :
				{
					return transport_data_socket_list ;
				}
				case OWSL_MODE_STREAM :
				{
					return transport_listening_socket_list ;
				}
				default :
				{
					return NULL ;
				}
			}
		}
		default :
		{
			return NULL ;
		}
	}
}

static OWSLCallback
transport_callback_get
(
	TransportType type,
	OWSLSocketMode mode
)
{
	switch (type)
	{
		case TRANSPORT_DATA :
		{
			return transport_on_data_socket_event ;
		}
		case TRANSPORT_LISTENING :
		{
			switch (mode)
			{
				case OWSL_MODE_DATAGRAM :
				{
					return transport_on_data_socket_event ;
				}
				case OWSL_MODE_STREAM :
				{
					return transport_on_listening_socket_event ;
				}
				default :
				{
					return NULL ;
				}
			}
		}
		default :
		{
			return NULL ;
		}
	}
}

static int
transport_socket_compare
(
	const void * void1,
	const void * void2
)
{
	const OWSLSocket * socket1 = void1 ;
	const OWSLSocket * socket2 = void2 ;
	if (* socket1 < * socket2)
	{
		return -1 ;
	}
	else if (* socket1 == * socket2)
	{
		return 0 ;
	}
	else
	{
		return 1 ;
	}
}

static OWSLSocket *
transport_socket_add
(
	OWSLSocket socket,
	TransportType type
)
{
	OWSLSocketMode mode ;
	OWSLSocket * allocated_socket = NULL ;
	OWSLCallback callback ;
	OWList * list ;

	if (socket > 0)
	{
		allocated_socket = malloc (sizeof (OWSLSocket)) ;
		if (allocated_socket != NULL)
		{
			if (owsl_blocking_mode_set (socket, OWSL_NON_BLOCKING))
			{
				owsl_close (socket) ;
				free (allocated_socket) ;
				return NULL ;
			}

			mode = owsl_mode_get (socket) ;
			list = transport_socket_list_get (type, mode) ;
			callback = transport_callback_get (type, mode) ;

			if (owsl_callback_set (socket, callback, NULL))
			{
				owsl_close (socket) ;
				free (allocated_socket) ;
				return NULL ;
			}

			* allocated_socket = socket ;

			if (owlist_add (list, allocated_socket, transport_socket_compare))
			{
				owsl_close (socket) ;
				free (allocated_socket) ;
				return NULL ;
			}
		}
	}

	return allocated_socket ;
}

static int
transport_socket_free
(
	OWSLSocket * socket
)
{
	int return_code = 0 ;
	return_code |= owsl_close (* socket) ;
	free (socket) ;
	return return_code ;
}

static int
transport_socket_remove
(
	OWSLSocket socket,
	TransportType type
)
{
	OWList * list ;
	OWSLSocket * removed_socket ;

	list = transport_socket_list_get (type, owsl_mode_get (socket)) ;

	removed_socket = owlist_remove (list, & socket, transport_socket_compare) ;
	if (removed_socket == NULL)
	{
		return -1 ;
	}

	return transport_socket_free (removed_socket) ;
}

static OWSLSocket
transport_socket_get_next
(
	OWSLSocketType implementation,
	OWListIterator * list_iterator
)
{
	OWSLSocket found_socket = -1 ;
	OWSLSocket * socket ;
	while (owlist_iterator_next (list_iterator) == 0)
	{
		socket = owlist_iterator_get (list_iterator) ;
		if (owsl_type_get (* socket) == implementation)
		{
			found_socket = * socket ;
			break ;
		}
	}
	return found_socket ;
}

int
transport_initialize
(
	TransportRecv recv_callback,
	TransportListenError listen_error_callback
)
{
	transport_recv_callback = recv_callback ;
	transport_listen_error_callback = listen_error_callback ;

	if (owsl_initialize ())
	{
		return -1 ;
	}

	transport_data_socket_list = owlist_new () ;
	if (transport_data_socket_list == NULL)
	{
		owsl_terminate () ;
		return -1 ;
	}

	transport_listening_socket_list = owlist_new () ;
	if (transport_listening_socket_list == NULL)
	{
		owlist_free (transport_data_socket_list) ;
		transport_data_socket_list = NULL ;
		owsl_terminate () ;
		return -1 ;
	}

	if (pthread_mutex_init (& transport_socket_lists_mutex, NULL))
	{
		owlist_free (transport_listening_socket_list) ;
		transport_listening_socket_list = NULL ;
		owlist_free (transport_data_socket_list) ;
		transport_data_socket_list = NULL ;
		owsl_terminate () ;
		return -1 ;
	}

	return 0 ;
}

int
transport_terminate
(void)
{
	int return_code = 0 ;

	return_code |= pthread_mutex_destroy (& transport_socket_lists_mutex) ;

	return_code |= owlist_free_all (transport_listening_socket_list, (void (*) (void *)) transport_socket_free) ;
	transport_listening_socket_list = NULL ;

	return_code |= owlist_free_all (transport_data_socket_list, (void (*) (void *)) transport_socket_free) ;
	transport_data_socket_list = NULL ;

	return_code |= owsl_terminate () ;

	return return_code ;
}

static OWSLSocketType
transport_implementation_get
(
	TransportProtocol protocol,
	OWSLAddressFamily address_family
)
{
	switch (protocol)
	{
		case TRANSPORT_UDP :
		{
			return owsl_socket_type_get (address_family, OWSL_MODE_DATAGRAM, OWSL_CIPHERING_DISABLED) ;
		}
		case TRANSPORT_TCP :
		{
			return owsl_socket_type_get (address_family, OWSL_MODE_STREAM, OWSL_CIPHERING_DISABLED) ;
		}
		case TRANSPORT_TLS :
		{
			return owsl_socket_type_get (address_family, OWSL_MODE_STREAM, OWSL_CIPHERING_ENABLED) ;
		}
		default :
		{
			return OWSL_TYPE_UNKNOWN ;
		}
	}
}

static TransportProtocol
transport_protocol_get
(
	OWSLSocketType implementation
)
{
	OWSLSocketMode mode = owsl_socket_type_mode_get (implementation) ;
	OWSLCiphering ciphering = owsl_socket_type_ciphering_get (implementation) ;
	switch (mode)
	{
		case OWSL_MODE_DATAGRAM :
		{
			switch (ciphering)
			{
				case OWSL_CIPHERING_DISABLED :
				{
					return TRANSPORT_UDP ;
				}
				default :
				{
					return TRANSPORT_UNKNOWN ;
				}
			}
		}
		case OWSL_MODE_STREAM :
		{
			switch (ciphering)
			{
				case OWSL_CIPHERING_DISABLED :
				{
					return TRANSPORT_TCP ;
				}
				case OWSL_CIPHERING_ENABLED :
				{
					return TRANSPORT_TLS ;
				}
				default :
				{
					return TRANSPORT_UNKNOWN ;
				}
			}
		}
		default :
		{
			return TRANSPORT_UNKNOWN ;
		}
	}
}

static OWSLSocket
transport_data_socket_new
(
	OWSLSocketType implementation,
	const struct sockaddr * remote_address,
	int remote_address_length
)
{
	OWSLSocket * socket = transport_socket_add
	(
		owsl_socket_by_type (implementation),
		TRANSPORT_DATA
	) ;
	if (socket == NULL)
	{
			return -1 ;
	}

	if (owsl_socket_type_mode_get (implementation) == OWSL_MODE_STREAM)
	{
		if (owsl_connect (* socket, remote_address, remote_address_length))
		{
			if (OW_GET_ERROR != EINPROGRESS && OW_GET_ERROR != EWOULDBLOCK)
			{
				transport_socket_remove (* socket, TRANSPORT_DATA) ;
				return -1 ;
			}
		}
	}

	return * socket ;
}

static OWSLSocket
transport_listening_socket_new
(
	OWSLSocketType implementation,
	const struct sockaddr * bind_address,
	int bind_address_length,
	int strict_bind,
	int pending_max
)
{
	OWSLSocket * socket ;

	socket = transport_socket_add
	(
		owsl_socket_by_type (implementation),
		TRANSPORT_LISTENING
	) ;
	if (socket == NULL)
	{
			return -1 ;
	}

	if (owsl_bind (* socket, bind_address, bind_address_length))
	{
		transport_socket_remove (* socket, TRANSPORT_LISTENING) ;
		return -1 ;
	}
	if (strict_bind && owsl_address_compare (bind_address, owsl_bound_address_get (* socket)))
	{
		transport_socket_remove (* socket, TRANSPORT_LISTENING) ;
		return -1 ;
	}

	if (owsl_socket_type_mode_get (implementation) == OWSL_MODE_STREAM)
	{
		if (owsl_listen (* socket, pending_max))
		{
			transport_socket_remove (* socket, TRANSPORT_LISTENING) ;
			return -1 ;
		}
	}

	return * socket ;
}

static OWSLSocket
transport_socket_get
(
	TransportType type,
	OWSLSocketType implementation,
	const struct sockaddr * address,
	int strict_address
)
{
	OWSLSocketMode mode ;
	OWList * list ;
	OWListIterator * socket_list_iterator ;
	OWSLSocket socket, best_socket = 0 ;
	struct sockaddr * (* address_get) (OWSLSocket) = NULL ;
	struct sockaddr * socket_address ;
	int socket_found = 0 ;

	mode = owsl_socket_type_mode_get (implementation) ;

	list = transport_socket_list_get (type, mode) ;
	if (list == NULL)
	{
		return -1 ;
	}

	if (address != NULL)
	{
		switch (type)
		{
			case TRANSPORT_DATA :
			{
				if (mode == OWSL_MODE_STREAM)
				{
					address_get = owsl_remote_address_get ;
				}
				break ;
			}
			case TRANSPORT_LISTENING :
			{
				address_get = owsl_bound_address_get ;
				break ;
			}
		}
	}

	socket_list_iterator = owlist_iterator_new (list, OWLIST_READ) ;
	if (socket_list_iterator == NULL)
	{
		return -1 ;
	}

	do
	{
		socket = transport_socket_get_next (implementation, socket_list_iterator) ;
		if (socket > 0)
		{
			if (address_get != NULL)
			{
				socket_address = address_get (socket) ;
				if (owsl_address_compare ((const struct sockaddr *) address, socket_address) == 0)
				{
					socket_found = 1 ;
				}
				else if (! strict_address)
				{
					best_socket = socket ;
				}
			}
			else
			{
				socket_found = 1 ;
			}
		}
	} while (! socket_found && socket > 0) ;

	if (owlist_iterator_free (socket_list_iterator))
	{
		return -1 ;
	}

	return socket_found ? socket : (strict_address ? 0 : best_socket) ;
}

OWSLSocket
transport_socket
(
	TransportProtocol protocol,
	const struct sockaddr * remote_address,
	int remote_address_length
)
{
	OWSLSocket socket ;
	OWSLSocketType implementation = transport_implementation_get (protocol, remote_address->sa_family) ;

	if (pthread_mutex_lock (& transport_socket_lists_mutex))
	{
		return -1 ;
	}
	socket = transport_socket_get
	(
		TRANSPORT_DATA,
		implementation,
		remote_address,
		1
	) ;
	if (socket < 0)
	{
		pthread_mutex_unlock (& transport_socket_lists_mutex) ;
		return -1 ;
	}
	if (socket == 0)
	{
		socket = transport_data_socket_new
		(
			implementation,
			remote_address,
			remote_address_length
		) ;
		if (socket < 0)
		{
			pthread_mutex_unlock (& transport_socket_lists_mutex) ;
			return -1 ;
		}
	}
	if (pthread_mutex_unlock (& transport_socket_lists_mutex))
	{
		return -1 ;
	}

	return socket ;
}

int
transport_sendto
(
	TransportProtocol protocol,
	const void * buffer,
	int length,
	int flags,
	const struct sockaddr * address,
	int address_length
)
{
	return owsl_sendto
	(
		transport_socket
		(
			protocol,
			address,
			address_length
		),
		buffer,
		length,
		flags,
		address,
		address_length
	) ;
}

static struct sockaddr *
transport_address_listen
(
	TransportProtocol protocol,
	const struct sockaddr * bind_address,
	int bind_address_length,
	int strict_bind,   /* boolean to forbid binding on another address */
	int pending_max
)
{
	OWSLSocket socket ;
	OWSLSocketType implementation = transport_implementation_get (protocol, bind_address->sa_family) ;

	if (pthread_mutex_lock (& transport_socket_lists_mutex))
	{
		return NULL ;
	}
	socket = transport_socket_get
	(
		TRANSPORT_LISTENING,
		implementation,
		bind_address,
		strict_bind
	) ;
	if (socket != 0)
	{
		pthread_mutex_unlock (& transport_socket_lists_mutex) ;
		return NULL ;
	}
	socket = transport_listening_socket_new
	(
		implementation,
		bind_address,
		/*bind_address_length*/sizeof(struct sockaddr),
		strict_bind,
		pending_max
	) ;
	if (socket < 0)
	{
		pthread_mutex_unlock (& transport_socket_lists_mutex) ;
		return NULL ;
	}
	if (pthread_mutex_unlock (& transport_socket_lists_mutex))
	{
		return NULL ;
	}

	return owsl_bound_address_get (socket) ;
}

struct sockaddr *
transport_listen
(
	TransportProtocol protocol,
	OWSLAddressFamily address_family,
	unsigned short bind_port,
	int pending_max
)
{
	struct sockaddr * bound_address = NULL ;
	struct sockaddr * address ;
	char * ip = OWSL_LOCAL_ADDRESS ;
	char public_ip [OWSL_IP_MAX_SIZE] ;
	if (owsl_address_public_ip_get (address_family, public_ip, sizeof (public_ip)) == 0)
	{
		ip = public_ip ;
	}
	address = owsl_address_ip_port_new (address_family, ip, bind_port) ;
	if (address != NULL)
	{
		bound_address = transport_address_listen (protocol, address, /*OWSL_ADDRESS_SIZE*/sizeof(struct sockaddr), 0, pending_max) ;
		free (address) ;
	}
	return bound_address ;
}

struct sockaddr *
transport_listening_address_get
(
	TransportProtocol protocol,
	OWSLAddressFamily address_family,
	OWListIterator ** iterator
)
{
	OWList * list ;
	struct sockaddr * address = NULL ;
	OWSLSocketType implementation ;

	if (iterator == NULL)
	{
		return NULL ;
	}

	implementation = transport_implementation_get (protocol, address_family) ;
	if (implementation != OWSL_TYPE_UNKNOWN)
	{
		if (* iterator == NULL)
		{
			list = transport_socket_list_get (TRANSPORT_LISTENING, owsl_socket_type_mode_get (implementation)) ;
			* iterator = owlist_iterator_new (list, OWLIST_READ) ;
		}
		if (* iterator != NULL)
		{
			OWSLSocket socket = transport_socket_get_next
			(
				implementation,
				* iterator
			) ;
			if (socket > 0)
			{
				address = owsl_bound_address_get (socket) ;
			}
		}
	}

	return address ;
}

struct sockaddr *
transport_listening_address_get_first
(
	TransportProtocol protocol,
	OWSLAddressFamily address_family
)
{
	OWListIterator * socket_list_iterator = NULL ;
	struct sockaddr * address = NULL ;

	address = transport_listening_address_get (protocol, address_family, & socket_list_iterator) ;
	if (socket_list_iterator != NULL)
	{
		if (owlist_iterator_free (socket_list_iterator))
		{
			return NULL ;
		}
	}

	return address ;
}

static void
transport_on_data_socket_event
(
	OWSLSocket socket,
	OWSLEvent event,
	void * user_data
)
{
	if (event & OWSL_EVENT_ERROR)
	{
		transport_socket_remove (socket, TRANSPORT_DATA) ;
		return ;
	}

	if ((event & OWSL_EVENT_READ) && (transport_recv_callback != NULL))
	{
		transport_recv_callback (socket) ;
	}
	return ;
}

static void
transport_on_listening_socket_event
(
	OWSLSocket socket,
	OWSLEvent event,
	void * user_data
)
{
	if ((event & OWSL_EVENT_ERROR) != 0)
	{
		OWSLSocketType implementation = owsl_type_get (socket)  ;
		struct sockaddr * bind_address = owsl_bound_address_get (socket) ;
		transport_socket_remove (socket, TRANSPORT_DATA) ;
		if (transport_listen_error_callback != NULL)
		{
			transport_listen_error_callback (transport_protocol_get (implementation), bind_address) ;
		}
		return ;
	}

	if ((event & OWSL_EVENT_READ) != 0)
	{
		OWSLSocket new_socket = owsl_accept (socket, NULL, NULL) ;
		if (new_socket > 0)
		{
			transport_socket_add (new_socket, TRANSPORT_DATA) ;
		}
		return ;
	}
	return ;
}
