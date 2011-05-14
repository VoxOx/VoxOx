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

#ifndef _OWSL_HEADER_
#define _OWSL_HEADER_

#include "owsl_common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**************************************
*         CONSTANTS AND TYPES         *
**************************************/

/** maximum number of sockets opened at the same time */
#define OWSL_SOCKET_MAX       64
/** maximum number of packets queued in a socket in a direction (in/out) at the same time */
#define OWSL_QUEUE_PACKET_MAX 10

typedef struct sockaddr_storage OWSLAddress ;
#define OWSL_ADDRESS_SIZE sizeof (OWSLAddress)
#define OWSL_LOCAL_ADDRESS NULL

#define OWSL_IPV4_MAX_SIZE sizeof ("255.255.255.255")
#define OWSL_IPV6_MAX_SIZE sizeof ("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")
#define OWSL_IP_MAX_SIZE OWSL_IPV6_MAX_SIZE

#define OWSL_PORT_MAX_SIZE sizeof ("65535")

#define OWSL_IP_PORT_MAX_SIZE OWSL_IPV6_MAX_SIZE + OWSL_PORT_MAX_SIZE + 2 /* "[]:" - '\0' */

typedef int OWSLSocket ;

typedef enum OWSLAddressFamily
{
	OWSL_AF_UNDEFINED = 0,
	OWSL_AF_IPV4      = AF_INET,
	OWSL_AF_IPV6      = AF_INET6
} OWSLAddressFamily ;

typedef enum OWSLSocketMode
{
	OWSL_MODE_UNDEFINED = 0,
	OWSL_MODE_STREAM    = SOCK_STREAM,
	OWSL_MODE_DATAGRAM  = SOCK_DGRAM
} OWSLSocketMode ;

typedef enum OWSLCiphering
{
	OWSL_CIPHERING_UNDEFINED = 0,
	OWSL_CIPHERING_DISABLED,
	OWSL_CIPHERING_ENABLED
} OWSLCiphering ;

typedef enum OWSLSocketType
{
	OWSL_TYPE_UNKNOWN = -1, /* insert types after this one */
	OWSL_TYPE_IPV4_TCP,
	OWSL_TYPE_IPV4_UDP,
	OWSL_TYPE_IPV4_TLS,
	OWSL_TYPE_IPV4_UOH,
	OWSL_TYPE_IPV4_UOHS,
	OWSL_TYPE_IPV6_TCP,
	OWSL_TYPE_IPV6_UDP,
	OWSL_TYPE_IPV6_TLS,
	OWSL_TYPE_IPV6_UOH,
	OWSL_TYPE_IPV6_UOHS,
	OWSL_TYPE_MAX /* insert types before this one */
} OWSLSocketType ;

typedef enum OWSLBlockingMode
{
	OWSL_BLOCKING_UNKNOWN = -1,
	OWSL_BLOCKING,
	OWSL_NON_BLOCKING
} OWSLBlockingMode ;

typedef enum OWSLEvent
{
	OWSL_EVENT_READ  = 1 << 0,
	OWSL_EVENT_WRITE = 1 << 1,
	OWSL_EVENT_ERROR = 1 << 2
} OWSLEvent ;
typedef void (* OWSLCallback) (OWSLSocket socket, OWSLEvent event, void * user_data) ;


/*************************************
*         LIBRARY MANAGEMENT         *
*************************************/

OWSL_FUNC_DEF int
owsl_initialize
(void) ;

OWSL_FUNC_DEF int
owsl_terminate
(void) ;

OWSL_FUNC_DEF int
owsl_global_parameter_set
(
	const char * name,
	const void * value
) ;


/*************************************************
*         DYNAMIC SOCKET TYPE MANAGEMENT         *
*************************************************/

OWSL_FUNC_DEF int
owsl_socket_type_add
(
	OWSLSocketType type
) ;

OWSL_FUNC_DEF int
owsl_socket_type_remove
(
	OWSLSocketType type
) ;

OWSL_FUNC_DEF int
owsl_socket_type_contains
(
	OWSLSocketType type
) ;

/**
 * Search a matching type in the list of registered types.
 *
 * @return a matching type on success
 * @return OWSL_TYPE_UNKNOWN if there is no or more than one matching type
 */
OWSL_FUNC_DEF OWSLSocketType
owsl_socket_type_get
(
	OWSLAddressFamily address_family,
	OWSLSocketMode mode,
	OWSLCiphering ciphering
) ;

OWSL_FUNC_DEF OWSLAddressFamily
owsl_socket_type_address_family_get
(
	OWSLSocketType type
) ;

OWSL_FUNC_DEF OWSLSocketMode
owsl_socket_type_mode_get
(
	OWSLSocketType type
) ;

OWSL_FUNC_DEF OWSLCiphering
owsl_socket_type_ciphering_get
(
	OWSLSocketType type
) ;


/***************************************************
*         SOCKET FUNCTIONS (OWSL specific)         *
***************************************************/

/* TODO: variable errno is not set if these functions return an error */

/**
 * Create a socket of a matching type in the list of registered types.
 * By default, socket is in blocking mode (synchronous).
 *
 * @return the newly created socket
 * @return a negative value in case of failure
 */
OWSL_FUNC_DEF OWSLSocket
owsl_socket
(
	OWSLAddressFamily address_family,
	OWSLSocketMode mode,
	OWSLCiphering ciphering
) ;

/**
 * Create a socket of the specified type.
 * By default, socket is in blocking mode (synchronous).
 *
 * @return the newly created socket
 * @return a negative value in case of failure
 */
OWSL_FUNC_DEF OWSLSocket
owsl_socket_by_type
(
	OWSLSocketType type
) ;

OWSL_FUNC_DEF OWSLSocketType
owsl_type_get
(
	OWSLSocket socket
) ;

OWSL_FUNC_DEF OWSLAddressFamily
owsl_address_family_get
(
	OWSLSocket socket
) ;

OWSL_FUNC_DEF OWSLSocketMode
owsl_mode_get
(
	OWSLSocket socket
) ;

OWSL_FUNC_DEF OWSLCiphering
owsl_ciphering_get
(
	OWSLSocket socket
) ;

OWSL_FUNC_DEF int
owsl_blocking_mode_set
(
	OWSLSocket socket,
	OWSLBlockingMode mode
) ;

OWSL_FUNC_DEF OWSLBlockingMode
owsl_blocking_mode_get
(
	OWSLSocket socket
) ;

OWSL_FUNC_DEF int
owsl_callback_set
(
	OWSLSocket socket,
	OWSLCallback callback_function,
	void * callback_user_data
) ;

OWSL_FUNC_DEF int
owsl_parameter_set
(
	OWSLSocket socket,
	const char * name,
	const void * value
) ;

OWSL_FUNC_DEF int
owsl_reuse_set
(
	OWSLSocket socket
) ;

OWSL_FUNC_DEF int
owsl_name_set
(
	OWSLSocket socket,
	const char * name
) ;

OWSL_FUNC_DEF char *
owsl_name_get
(
	OWSLSocket socket
) ;

OWSL_FUNC_DEF struct sockaddr *
owsl_bound_address_get
(
	OWSLSocket socket
) ;

/* In non connected mode, it returns NULL */
OWSL_FUNC_DEF struct sockaddr *
owsl_remote_address_get
(
	OWSLSocket socket
) ;


/*********************************************
*         SOCKET FUNCTIONS (BSD API)         *
*********************************************/

/* TODO: variable errno is not set if these functions return an error */

OWSL_FUNC_DEF OWSLSocket
owsl_accept
(
	OWSLSocket socket,
	struct sockaddr * address,
	socklen_t * address_length
) ;

OWSL_FUNC_DEF int
owsl_close
(
	OWSLSocket socket
) ;

OWSL_FUNC_DEF int
owsl_bind
(
	OWSLSocket socket,
	const struct sockaddr * address,
	socklen_t address_length
) ;

OWSL_FUNC_DEF int
owsl_connect
(
	OWSLSocket socket,
	const struct sockaddr * address,
	socklen_t address_length
) ;

OWSL_FUNC_DEF int
owsl_listen
(
	OWSLSocket socket,
	int pending_max
) ;

OWSL_FUNC_DEF int
owsl_send
(
	OWSLSocket socket,
	const void * buffer,
	int length,
	int flags
) ;

OWSL_FUNC_DEF int
owsl_recv
(
	OWSLSocket socket,
	void * buffer,
	int size,
	int flags
) ;

OWSL_FUNC_DEF int
owsl_sendto
(
	OWSLSocket socket,
	const void * buffer,
	int length,
	int flags,
	const struct sockaddr * address,
	socklen_t address_length
) ;

OWSL_FUNC_DEF int
owsl_recvfrom
(
	OWSLSocket socket,
	void * buffer,
	int size,
	int flags,
	struct sockaddr * address,
	socklen_t * address_length
) ;

OWSL_FUNC_DEF int
owsl_select
(
	OWSLSocket socket_max,
	fd_set * read_sockets,
	fd_set * write_sockets,
	fd_set * error_sockets,
	struct timeval * timeout
) ;


/********************************
*         ADDRESS TOOLS         *
********************************/

OWSL_FUNC_DEF OWSLAddressFamily
owsl_address_family_get_from_ip
(
	const char * ip
) ;

OWSL_FUNC_DEF struct sockaddr *
owsl_address_ip_port_new
(
	OWSLAddressFamily family,
	const char * ip,
	unsigned short port
) ;

OWSL_FUNC_DEF struct sockaddr *
owsl_address_port_new
(
	OWSLAddressFamily family,
	const char * address,
	unsigned short port
) ;

OWSL_FUNC_DEF struct sockaddr *
owsl_address_new
(
	OWSLAddressFamily family,
	const char * address
) ;

OWSL_FUNC_DEF void
owsl_address_free
(
	struct sockaddr * address
) ;

OWSL_FUNC_DEF int
owsl_address_parse
(
	const struct sockaddr * address,
	OWSLAddressFamily * family,   /* OUT */
	char * ip,                    /* OUT */
	size_t ip_size,
	unsigned short * port         /* OUT */
) ;

OWSL_FUNC_DEF int
owsl_address_parse_from_string
(
	const char * address_port,
	OWSLAddressFamily * family,   /* OUT */
	char * address,               /* OUT */
	size_t address_size,
	unsigned short * port         /* OUT */
) ;

OWSL_FUNC_DEF int
owsl_address_port_set_from_string
(
	const char * address,
	unsigned short port,
	char * address_port,   /* OUT */
	size_t address_port_size
) ;

OWSL_FUNC_DEF int
owsl_address_ip_port_set
(
	const struct sockaddr * address_struct,
	char * ip_port,   /* OUT */
	size_t ip_port_size
) ;

OWSL_FUNC_DEF int
owsl_address_compare
(
	const struct sockaddr * address1,
	const struct sockaddr * address2
) ;

OWSL_FUNC_DEF int
owsl_address_public_ip_get
(
	OWSLAddressFamily family,
	char * ip,   /* OUT */
	size_t ip_size
) ;


/***********************************
*         NETWORK EXPLORER         *
***********************************/

OWSL_FUNC_DEF int
owsl_network_check
(void) ;

typedef enum OWSLProxyAuthenticationType
{
	OWSL_PROXY_AUTH_UNKNOWN = 0,
	OWSL_PROXY_AUTH_BASIC,
	OWSL_PROXY_AUTH_DIGEST,
	OWSL_PROXY_AUTH_NTLM
} OWSLProxyAuthenticationType ;

OWSL_FUNC_DEF int
owsl_proxy_check
(
	const char * address,
	unsigned short port,
	const char * login,
	const char * password,
	OWSLProxyAuthenticationType * authentication_type /* TODO: remove this parameter */
) ;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OWSL_HEADER_ */
