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

#ifndef _TRANSPORT_MANAGER_HEADER_
#define _TRANSPORT_MANAGER_HEADER_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Transport manager do owsl_socket(), owsl_connect(), owsl_accept(), owsl_bind() and owsl_listen().
 * An address can be listened by calling transport_manager_listen().
 * Data can be sent by automatically creating or retrieving socket with transport_manager_socket(),
 * and then calling owsl_sendto(). This can be done with transport_sendto() also.
 * Incoming data can be received in the callback with owsl_recvfrom().
 */

#include <owsl.h>
#include <owlist.h>

typedef enum TransportProtocol
{
	TRANSPORT_UNKNOWN = -1,
	TRANSPORT_UDP,
	TRANSPORT_TCP,
	TRANSPORT_TLS
} TransportProtocol ;

/**
 * When data are available, owsl_recvfrom() should be called.
 */
typedef void (* TransportRecv) (OWSLSocket socket) ;

/**
 * When an error happens on a listening socket, it is closed. A new listen should be done.
 */
typedef void (* TransportListenError) (TransportProtocol protocol, struct sockaddr * bind_address) ;

/**
 * Initialize OWSL and transport manager.
 *
 * @param recv_callback can be NULL
 * @param listen_error_callback can be NULL
 */
int
transport_initialize
(
	TransportRecv recv_callback,
	TransportListenError listen_error_callback
) ;

/**
 * Terminate OWSL and transport manager.
 */
int
transport_terminate
(void) ;

/**
 * Get a string representation of protocol.
 */
const char *
transport_protocol_string_get
(
	TransportProtocol protocol
) ;

/**
 * Get a socket to perform owsl_sendto().
 */
OWSLSocket
transport_socket
(
	TransportProtocol protocol,
	const struct sockaddr * remote_address,
	int remote_address_length
) ;

/**
 * Get a socket and call owsl_sendto().
 */
int
transport_sendto
(
	TransportProtocol protocol,
	const void * buffer,
	int length,
	int flags,
	const struct sockaddr * address,
	int address_length
) ;

/**
 * Start listening.
 *
 * @warning The bound port can be different of the specified one
 * and ip address can be different of the local one.
 *
 * @param pending_max is ignored in datagram mode
 *
 * @return the bound address if listen is started
 * @return NULL otherwise
 */
struct sockaddr *
transport_listen
(
	TransportProtocol protocol,
	OWSLAddressFamily address_family,
	unsigned short bind_port,
	int pending_max
) ;

/**
 * @param iterator must point to a NULL pointer to get the first address.
 * After, the function returns the next addresses.
 * If not NULL, *iterator must be freed by calling owlist_iterator_free().
 */
struct sockaddr *
transport_listening_address_get
(
	TransportProtocol protocol,
	OWSLAddressFamily address_family,
	OWListIterator ** iterator
) ;

struct sockaddr *
transport_listening_address_get_first
(
	TransportProtocol protocol,
	OWSLAddressFamily address_family
) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TRANSPORT_MANAGER_HEADER_ */
