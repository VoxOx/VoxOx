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

#ifndef _OWSL_BASE_HEADER_
#define _OWSL_BASE_HEADER_

#include "owsl_internal.h"

#define OWSL_SYSTEM_SOCKET \
	OWSLSystemSocket system_socket ;

#define OWSL_REMOTE_ADDRESS      \
	OWSLAddress remote_address ; \
	socklen_t remote_address_length ;

#define OWSL_SOCKET_INFO_WITH_SYSTEM_SOCKET                                    \
	OWSL_SOCKET_INFO   /* must be in first position for inherited behaviour */ \
	OWSL_SYSTEM_SOCKET

#define OWSL_SOCKET_INFO_WITH_CONNECTED_SYSTEM_SOCKET                                             \
	OWSL_SOCKET_INFO_WITH_SYSTEM_SOCKET   /* must be in first position for inherited behaviour */ \
	OWSL_REMOTE_ADDRESS                                                                           \
	int remotely_closed ;

typedef struct OWSLSocketInfo_with_SystemSocket
{
	OWSL_SOCKET_INFO_WITH_SYSTEM_SOCKET
} OWSLSocketInfo_with_SystemSocket, OWSLSocketInfo_UDP ;

typedef struct OWSLSocketInfo_with_ConnectedSystemSocket
{
	OWSL_SOCKET_INFO_WITH_CONNECTED_SYSTEM_SOCKET
} OWSLSocketInfo_with_ConnectedSystemSocket, OWSLSocketInfo_TCP ;

typedef struct OWSLRemoteAddress
{
	OWSL_REMOTE_ADDRESS
} OWSLRemoteAddress ;

typedef struct OWSLConnection
{
	OWSL_SYSTEM_SOCKET
	OWSL_REMOTE_ADDRESS
} OWSLConnection ;

int
owsl_base_system_socket_blocking_mode_set
(
	OWSLSocketInfo * _socket_info_with_system_socket,
	OWSLBlockingMode mode
) ;

int
owsl_base_system_socket_reuse_set
(
	OWSLSocketInfo * _socket_info_with_system_socket
) ;

struct sockaddr *
owsl_base_remote_address_get
(
	OWSLSocketInfo * _socket_info_with_connected_system_socket
) ;

void
owsl_base_in_queue_callback_with_monitor
(
	OWQueue * queue,
	OWQueueEvent event,
	void * _socket_info_with_system_socket
) ;

void
owsl_base_out_queue_callback_with_monitor
(
	OWQueue * queue,
	OWQueueEvent event,
	void * _socket_info_with_system_socket
) ;

void
owsl_base_in_out_queues_callback_with_monitor
(
	OWQueue * queue,
	OWQueueEvent event,
	void * _socket_info_with_system_socket
) ;

int
owsl_base_udp_open
(
	OWSLSocketInfo_with_SystemSocket * socket,
	OWSLAddressFamily address_family,
	OWSLMonitorCallback callback_function
) ;

int
owsl_base_udp_close
(
	OWSLSocketInfo_with_SystemSocket * socket
) ;

int
owsl_base_tcp_open
(
	OWSLSocketInfo_with_ConnectedSystemSocket * socket,
	OWSLAddressFamily address_family,
	OWSLMonitorCallback callback_function
) ;

int
owsl_base_tcp_set
(
	OWSLSocketInfo_with_ConnectedSystemSocket * socket,
	OWSLConnection * connection,
	OWSLMonitorCallback callback_function
) ;

int
owsl_base_tcp_close
(
	OWSLSocketInfo_with_ConnectedSystemSocket * socket
) ;

int
owsl_base_bind
(
	OWSLSocketInfo * _socket_info_with_system_socket,
	struct sockaddr * address,
	socklen_t address_length
) ;

int
owsl_base_in_queue_listen
(
	OWSLSocketInfo * _socket_info_with_system_socket,
	int pending_max
) ;

int
owsl_base_in_queue_recv
(
	OWSLSocketInfo * socket,
	void * buffer,
	int size,
	int flags
) ;

int
owsl_base_in_queue_connected_recv
(
	OWSLSocketInfo * socket,
	void * buffer,
	int size,
	int flags
) ;

int
owsl_base_in_queue_recvfrom
(
	OWSLSocketInfo * socket,
	void * buffer,
	int size,
	int flags,
	struct sockaddr * address,
	socklen_t * address_length
) ;

int
owsl_base_in_queue_connected_recvfrom
(
	OWSLSocketInfo * _socket_info_with_connected_system_socket,
	void * buffer,
	int size,
	int flags,
	struct sockaddr * address,
	socklen_t * address_length
) ;

int
owsl_base_out_queue_send
(
	OWSLSocketInfo * socket,
	const void * buffer,
	int length,
	int flags
) ;

int
owsl_base_out_queue_sendto
(
	OWSLSocketInfo * socket,
	const void * buffer,
	int length,
	int flags,
	const struct sockaddr * address,
	socklen_t address_length
) ;

#endif /* _OWSL_BASE_HEADER_ */
