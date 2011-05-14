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

#ifndef _OWSL_INTERNAL_HEADER_
#define _OWSL_INTERNAL_HEADER_

#include "owsl.h"

#include <owqueue.h>
#include <pthread.h>


/********************************
*         SYSTEM SOCKET         *
********************************/

#ifdef OS_WINDOWS
	typedef SOCKET OWSLSystemSocket ;
#else /* OS_POSIX */
	typedef int OWSLSystemSocket ;
#endif

int
owsl_system_socket_initialize
(void) ;

int
owsl_system_socket_terminate
(void) ;

/* the return value should be checked with owsl_system_socket_is_valid */
OWSLSystemSocket
owsl_system_socket_open
(
	OWSLAddressFamily address_family,
	OWSLSocketMode mode
) ;

int
owsl_system_socket_close
(
	OWSLSystemSocket socket
) ;

int
owsl_system_socket_is_valid
(
	OWSLSystemSocket socket
) ;

int
owsl_system_socket_blocking_mode_set
(
	OWSLSystemSocket socket,
	OWSLBlockingMode mode
) ;

int
owsl_system_socket_reuse_set
(
	OWSLSystemSocket socket
) ;


/*************************
*         SOCKET         *
*************************/

int
owsl_socket_initialize
(void) ;

int
owsl_socket_terminate
(void) ;

typedef struct OWSLSocketTypeInfo OWSLSocketTypeInfo ;

#define OWSL_SOCKET_INFO                                                                             \
	OWSLSocket socket ;                 /** handle for higher level layer */                         \
	OWSLSocketTypeInfo * type_info ;    /** type of the socket */                                    \
	OWSLBlockingMode blocking_mode ;    /** synchonous or asynchronous */                            \
	OWQueue * in_queue ;                /** queue that contains data received from network */        \
	OWQueue * out_queue ;               /** queue that contains data to send over network */         \
	pthread_mutex_t listening_mutex ;   /** listen is exclusive with connect, send, recv... */       \
	int listening ;                     /** 1 if listen was called, -1 if listen cannot be called */ \
	int connected ;                     /** 1 when really connected after connect or accept */       \
	int error ;                         /** should be 0 */                                           \
	OWSLCallback callback_function ;    /** called at each event */                                  \
	void * callback_user_data ;         /** parameter for callback function */                       \
	OWSLAddress bound_address ;         /** bound address */                                         \
	char * name ;                       /** name of the socket, used for debugging purposes */

typedef struct OWSLSocketInfo
{
	OWSL_SOCKET_INFO
} OWSLSocketInfo ;

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
) ;

int
owsl_socket_info_free
(
	OWSLSocketInfo * socket
) ;

OWSLSocketInfo *
owsl_socket_info_get
(
	OWSLSocket socket
) ;

OWSLSocket
owsl_socket_handle_get_new
(void) ;

void
owsl_socket_handle_set
(
	OWSLSocket socket,
	OWSLSocketInfo * socket_info
) ;

void
owsl_socket_handle_free
(
	OWSLSocket socket
) ;

int
owsl_socket_listen_disable
(
	OWSLSocketInfo * socket
) ;

int
owsl_socket_listen_activate
(
	OWSLSocketInfo * socket
) ;

int
owsl_socket_is_readable
(
	OWSLSocketInfo * socket
) ;

int
owsl_socket_is_writable
(
	OWSLSocketInfo * socket
) ;

int
owsl_socket_has_error
(
	OWSLSocketInfo * socket
) ;


/******************************
*         SOCKET TYPE         *
******************************/

struct OWSLSocketTypeInfo
{
	OWSLSocketType type ;                /** unique identifier of socket type */
	OWSLAddressFamily address_family ;   /** parameter of socket type */
	OWSLSocketMode mode ;                /** parameter of socket type */
	OWSLCiphering ciphering ;            /** parameter of socket type */
	int (* global_parameter_set) (const char * name, const void * value) ;
	int (* is_readable) (OWSLSocketInfo * socket) ;
	int (* is_writable) (OWSLSocketInfo * socket) ;
	int (* has_error) (OWSLSocketInfo * socket) ;
	int (* blocking_mode_set) (OWSLSocketInfo * socket, OWSLBlockingMode mode) ;
	int (* parameter_set) (OWSLSocketInfo * socket, const char * name, const void * value) ;
	int (* reuse_set) (OWSLSocketInfo * socket) ;
	struct sockaddr * (* remote_address_get) (OWSLSocketInfo * socket) ;
	OWQueueCallback on_queue_event ;
	OWSLSocketInfo * (* socket)   (OWSLSocketType type) ;
	OWSLSocketInfo * (* accept)   (OWSLSocketInfo * socket,       struct sockaddr * address, socklen_t * address_length) ;
	int              (* close)    (OWSLSocketInfo * socket) ;
	int              (* bind)     (OWSLSocketInfo * socket, struct sockaddr * address, socklen_t   address_length) ;
	int              (* connect)  (OWSLSocketInfo * socket, const struct sockaddr * address, socklen_t   address_length) ;
	int              (* listen)   (OWSLSocketInfo * socket, int pending_max) ;
	int              (* send)     (OWSLSocketInfo * socket, const void * buffer, int length, int flags) ;
	int              (* recv)     (OWSLSocketInfo * socket,       void * buffer, int size,   int flags) ;
	int              (* sendto)   (OWSLSocketInfo * socket, const void * buffer, int length, int flags, const struct sockaddr * address, socklen_t   address_length) ;
	int              (* recvfrom) (OWSLSocketInfo * socket,       void * buffer, int size,   int flags,       struct sockaddr * address, socklen_t * address_length) ;
} ;

int
owsl_socket_type_initialize_all
(void) ;

int
owsl_socket_type_terminate_all
(void) ;

int
owsl_socket_type_initialize
(
	OWSLSocketTypeInfo * info
) ;

OWSLSocketTypeInfo *
owsl_socket_type_info_get
(
	OWSLSocketType type
) ;


/*****************************************
*         ASYNCHRONOUS MECHANISM         *
*****************************************/

int
owsl_asynchronous_initialize
(void) ;

int
owsl_asynchronous_terminate
(void) ;

/** Call owsl_signal + owsl_callback. */
int
owsl_notify
(
	OWSLSocketInfo * socket,
	OWSLEvent event
) ;

int
owsl_signal
(
	OWSLSocketInfo * socket,
	OWSLEvent event
) ;

int
owsl_callback
(
	OWSLSocketInfo * socket,
	OWSLEvent event
) ;


/**************************
*         MONITOR         *
**************************/

typedef enum OWSLMonitorEvent
{
	OWSL_MONITOR_READ  = 1 << 0,
	OWSL_MONITOR_WRITE = 1 << 1,
	OWSL_MONITOR_ERROR = 1 << 2,
	OWSL_MONITOR_ONCE  = 1 << 3   /** monitor special events (+ ERROR), once trigerred then come back to default behaviour */
} OWSLMonitorEvent ;
typedef void (* OWSLMonitorCallback) (OWSLSystemSocket system_socket, OWSLMonitorEvent event, void * user_data) ;

/**
 * Create and start monitor.
 */
int
owsl_monitor_start
(void) ;

/**
 * Stop and destroy monitor.
 */
int
owsl_monitor_stop
(void) ;

/**
 * Add a new socket to be monitored and add error event.
 *
 * @param socket:        socket to monitor
 * @param callback_func: callback function to be called when there is an event on the socket
 * @param user_data:     data that is passed back to the callback when an event occurs
 *
 * @return 0 on success
 * @return a negative value in case of failure
 */
int
owsl_monitor_socket_add
(
	OWSLSystemSocket system_socket,
	OWSLMonitorCallback callback_function,
	void * callback_user_data
) ;

/**
 * Remove a new socket from the monitor.
 *
 * @param socket: socket that is monitored
 *
 * @return 0 on success
 * @return a negative value in case of failure
 */
int
owsl_monitor_socket_remove
(
	OWSLSystemSocket system_socket
) ;

/**
 * Monitor an event of an already registered socket.
 *
 * @param socket: socket to monitor
 * @param event:  socket events to monitor (bitwise)
 *
 * @return 0 on success
 * @return a negative value in case of failure
 */
int
owsl_monitor_event_add
(
	OWSLSystemSocket system_socket,
	OWSLMonitorEvent event
) ;

/**
 * Remove the monitoring of an event for a socket.
 *
 * @param socket: socket that is monitored
 * @param event:  socket events to not monitor (bitwise)
 *
 * @return 0 on success
 * @return a negative value in case of failure
 */
int
owsl_monitor_event_remove
(
	OWSLSystemSocket system_socket,
	OWSLMonitorEvent event
) ;


/**********************************
*         OPENSSL WRAPPER         *
**********************************/

/** Must be called only once (in OWSL initialization). */
int
owsl_openssl_wrapper_initialize
(void) ;

/** Must be called only once (in OWSL termination). */
int
owsl_openssl_wrapper_terminate
(void) ;

/** Should be called in initialization of socket implementations which use OpenSSL. */
int
owsl_openssl_initialize
(void) ;

/** Should be called in termination of socket implementations which use OpenSSL. */
int
owsl_openssl_terminate
(void) ;


/**********************************************
*         SOCKET TYPE IMPLEMENTATIONS         *
**********************************************/

int
owsl_tcp_initialize
(void) ;

int
owsl_udp_initialize
(void) ;

int
owsl_uoh_initialize
(void) ;

int
owsl_uohs_initialize
(void) ;

int
owsl_tls_initialize
(void) ;

int
owsl_tls_terminate
(void) ;


#endif /* _OWSL_INTERNAL_HEADER_ */
