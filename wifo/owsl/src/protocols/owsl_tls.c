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

#include <openssl/ssl.h>

#include <fcntl.h>
#include <memory.h>
#include <semaphore.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

#define OWSL_TLS_PACKET_AVG_SIZE 1000
#define OWSL_TLS_BUFFER_SIZE 2000

static SSL_CTX * owsl_tls_context = NULL ;

typedef enum OWSLTlsAction
{
	OWSL_TLS_NONE,
	OWSL_TLS_ACCEPT,
	OWSL_TLS_CONNECT,
	OWSL_TLS_PEEK,
	OWSL_TLS_READ,
	OWSL_TLS_WRITE
} OWSLTlsAction ;

typedef struct OWSLSocketInfo_TLS
{
	OWSL_SOCKET_INFO_WITH_CONNECTED_SYSTEM_SOCKET   /* must be in first position for inherited behaviour */
	SSL * connection ;
	pthread_mutex_t handshake_mutex ;
	OWSLTlsAction interrupted_action ;
	int interrupted_read_write_bytes ;
	sem_t *accept_connect_semaphore ;
} OWSLSocketInfo_TLS ;

static sem_t * owsl_sem_open(const char *name, int oflag, mode_t mode, unsigned int value)
{
	sem_t *sem = NULL;
	
#ifdef __APPLE__
	if ((sem = sem_open (name, oflag, mode, value)) == SEM_FAILED)
	{
		sem = NULL;
	}
#else
	if ((sem = malloc(sizeof(sem_t))))
	{
		if (sem_init(sem, 0, value) == -1)
		{
			free(sem);
			sem = NULL;
		}
	}
#endif

	return sem;
}

static int owsl_sem_close(sem_t *sem)
{
	int ret = 0;

#ifdef __APPLE__
	ret = sem_close(sem);
#else
	ret = sem_destroy(sem);
	free(sem);
#endif

	return ret;
}

static void
owsl_tls_fatal_error
(
	OWSLSocketInfo_TLS * socket
)
{
	owsl_notify ((OWSLSocketInfo *) socket, OWSL_EVENT_ERROR) ;
	sem_post (socket->accept_connect_semaphore) ;
	return ;
}

static int
owsl_tls_connect_handshake
(
	OWSLSocketInfo_TLS * socket
)
{
	int return_code ;

	return_code = pthread_mutex_trylock (& socket->handshake_mutex) ;
	if (return_code == 0)
	{
		return_code = SSL_connect (socket->connection) ;
		if (return_code > 0)
		{
			pthread_mutex_unlock (& socket->handshake_mutex) ;
			socket->connected = 1 ;
			owsl_notify ((OWSLSocketInfo *) socket, OWSL_EVENT_WRITE) ;
			sem_post (socket->accept_connect_semaphore) ;
			return 0 ;
		}
		else
		{
			switch (SSL_get_error (socket->connection, return_code))
			{
				case SSL_ERROR_WANT_READ :
				{
					socket->interrupted_action = OWSL_TLS_CONNECT ;
					if (owsl_monitor_event_add (socket->system_socket, OWSL_MONITOR_READ | OWSL_MONITOR_ONCE))
					{
						pthread_mutex_unlock (& socket->handshake_mutex) ;
						return -1 ;
					}
					break ;
				}
				case SSL_ERROR_WANT_WRITE :
				{
					socket->interrupted_action = OWSL_TLS_CONNECT ;
					if (owsl_monitor_event_add (socket->system_socket, OWSL_MONITOR_WRITE | OWSL_MONITOR_ONCE))
					{
						pthread_mutex_unlock (& socket->handshake_mutex) ;
						return -1 ;
					}
					break ;
				}
				default :
				{
					pthread_mutex_unlock (& socket->handshake_mutex) ;
					owsl_tls_fatal_error (socket) ;
					return -1 ;
				}
			}
			pthread_mutex_unlock (& socket->handshake_mutex) ;
			OW_SET_ERROR (EINPROGRESS) ;
			return -1 ;
		}
	}
	else
	{
		if (return_code == EBUSY)
		{
			OW_SET_ERROR (EINPROGRESS) ;
		}
		else
		{
			owsl_tls_fatal_error (socket) ;
		}
		return -1 ;
	}
}

static void
owsl_tls_system_socket_accept
(
	OWSLSocketInfo_TLS * socket
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
owsl_tls_system_socket_recv
(
	OWSLSocketInfo_TLS * socket,
	OWSLTlsAction interrupted_action
)
{
	static char buffer [OWSL_TLS_BUFFER_SIZE] ; /* must be static for repeated SSL_peek/SSL_read */
	int received_bytes, written_bytes ;

	/* get packet and packet size */
	if (interrupted_action == OWSL_TLS_READ)
	{
		received_bytes = socket->interrupted_read_write_bytes ;
	}
	else
	{
		received_bytes = SSL_peek (socket->connection, buffer, OWSL_TLS_BUFFER_SIZE) ;
	}
	if (received_bytes < 0)
	{
		if (SSL_get_error (socket->connection, received_bytes) == SSL_ERROR_WANT_WRITE)
		{
			socket->interrupted_action = OWSL_TLS_PEEK ;
			owsl_monitor_event_add (socket->system_socket, OWSL_MONITOR_WRITE | OWSL_MONITOR_ONCE) ;
		}
	}
	else if (received_bytes == 0)
	{
		socket->remotely_closed = 1 ;
		owsl_monitor_event_remove (socket->system_socket, OWSL_MONITOR_READ) ;
	}
	else
	{
		/* try to push packet in the input queue */
		written_bytes = owqueue_write (socket->in_queue, buffer, received_bytes, NULL, OWQUEUE_NON_BLOCKING | OWQUEUE_TRANSACTION_OPEN) ;
		if (interrupted_action == OWSL_TLS_READ && written_bytes != received_bytes)
		{
			owqueue_write_transaction_close (socket->in_queue, 0) ;
			owsl_tls_fatal_error (socket) ;
		}
		else if (written_bytes > 0)
		{
			/* remove packet from system socket buffer */
			received_bytes = SSL_read (socket->connection, buffer, written_bytes) ;
			if (received_bytes <= 0)
			{
				switch (SSL_get_error (socket->connection, received_bytes))
				{
					case SSL_ERROR_WANT_READ :
					{
						socket->interrupted_action = OWSL_TLS_READ ;
						owsl_monitor_event_add (socket->system_socket, OWSL_MONITOR_READ | OWSL_MONITOR_ONCE) ;
						break ;
					}
					case SSL_ERROR_WANT_WRITE :
					{
						socket->interrupted_action = OWSL_TLS_READ ;
						owsl_monitor_event_add (socket->system_socket, OWSL_MONITOR_WRITE | OWSL_MONITOR_ONCE) ;
						break ;
					}
					default :
					{
						owsl_tls_fatal_error (socket) ;
					}
				}
				socket->interrupted_read_write_bytes = written_bytes ;
			}
			/* commit or rollback transaction */
			owqueue_write_transaction_close (socket->in_queue, received_bytes == written_bytes ? received_bytes : 0) ;
			if (received_bytes == written_bytes)
			{
				/* notify that the socket can be read */
				owsl_notify ((OWSLSocketInfo *) socket, OWSL_EVENT_READ) ;
				if (SSL_pending (socket->connection) > 0)
				{
					/* recursive recall */
					owsl_tls_system_socket_recv (socket, 0) ;
				}
			}
		}
	}

	return ;
}

static void
owsl_tls_system_socket_send
(
	OWSLSocketInfo_TLS * socket,
	OWSLTlsAction interrupted_action
)
{
	static char buffer [OWSL_TLS_BUFFER_SIZE] ; /* must be static for repeated SSL_write */
	int read_bytes, sent_bytes ;

	/* get packet and packet size */
	if (interrupted_action == OWSL_TLS_WRITE)
	{
		read_bytes = socket->interrupted_read_write_bytes ;
	}
	else
	{
		read_bytes = OWSL_TLS_BUFFER_SIZE ;
	}
	read_bytes = owqueue_read (socket->out_queue, buffer, read_bytes, NULL, OWQUEUE_NON_BLOCKING | OWQUEUE_TRANSACTION_OPEN) ;
	if (interrupted_action == OWSL_TLS_WRITE && read_bytes != socket->interrupted_read_write_bytes)
	{
		owqueue_read_transaction_close (socket->out_queue, 0) ;
		owsl_tls_fatal_error (socket) ;
	}
	else if (read_bytes > 0)
	{
		/* send packet */
		sent_bytes = SSL_write (socket->connection, buffer, read_bytes) ;
		if (sent_bytes <= 0)
		{
			switch (SSL_get_error (socket->connection, sent_bytes))
			{
				case SSL_ERROR_WANT_READ :
				{
					socket->interrupted_action = OWSL_TLS_WRITE ;
					owsl_monitor_event_add (socket->system_socket, OWSL_MONITOR_READ | OWSL_MONITOR_ONCE) ;
					break ;
				}
				case SSL_ERROR_WANT_WRITE :
				{
					socket->interrupted_action = OWSL_TLS_WRITE ;
					owsl_monitor_event_add (socket->system_socket, OWSL_MONITOR_WRITE | OWSL_MONITOR_ONCE) ;
					break ;
				}
				default :
				{
					owsl_tls_fatal_error (socket) ;
				}
			}
			socket->interrupted_read_write_bytes = read_bytes ;
		}
		/* commit or rollback transaction */
		owqueue_read_transaction_close (socket->out_queue, sent_bytes > 0 ? sent_bytes : 0) ;
	}

	return ;
}

static void
owsl_tls_monitor_callback
(
	OWSLSystemSocket system_socket,
	OWSLMonitorEvent event,
	void * _socket
)
{
	OWSLSocketInfo_TLS * socket = _socket ;
	OWSLTlsAction interrupted_action = OWSL_TLS_NONE ;

	if (event & OWSL_MONITOR_ERROR || socket->system_socket != system_socket)
	{
		owsl_tls_fatal_error (socket) ;
		return ;
	}

	/* handle once event matching an interrupted action */
	if (event & OWSL_MONITOR_ONCE)
	{
		interrupted_action = socket->interrupted_action ;
		socket->interrupted_action = 0 ;
		switch (interrupted_action)
		{
			case OWSL_TLS_NONE :
			{
				break ;
			}
			case OWSL_TLS_ACCEPT :
			{
				event = OWSL_MONITOR_READ ;
				break ;
			}
			case OWSL_TLS_CONNECT :
			{
				event = OWSL_MONITOR_WRITE ;
				break ;
			}
			case OWSL_TLS_PEEK :
			{
				event = OWSL_MONITOR_READ ;
				break ;
			}
			case OWSL_TLS_READ :
			{
				event = OWSL_MONITOR_READ ;
				break ;
			}
			case OWSL_TLS_WRITE :
			{
				event = OWSL_MONITOR_WRITE ;
				break ;
			}
		}
	}

	if (event & OWSL_MONITOR_READ)
	{
		if (socket->listening > 0)
		{
			owsl_tls_system_socket_accept (socket) ;
		}
		else if (socket->connected > 0)
		{
			owsl_tls_system_socket_recv (socket, interrupted_action) ;
		}
	}

	if (event & OWSL_MONITOR_WRITE)
	{
		if (socket->connected == 0)
		{
			owsl_tls_connect_handshake (socket) ;
		}
		if (socket->connected > 0)
		{
			owsl_tls_system_socket_send (socket, interrupted_action) ;
		}
	}

	return ;
}

static OWSLSocketInfo *
owsl_tls_socket
(
	OWSLSocketType type
)
{
	OWSLSocketInfo * socket ;
	char sem_name[8];

	OWSLAddressFamily address_family ;
	switch (type)
	{
		case OWSL_TYPE_IPV4_TLS :
		{
			address_family = OWSL_AF_IPV4 ;
			break ;
		}
		case OWSL_TYPE_IPV6_TLS :
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
		sizeof (OWSLSocketInfo_TLS),                        /* size of socket structure */
		OWSL_TLS_PACKET_AVG_SIZE * OWSL_QUEUE_PACKET_MAX,   /* input queue usable size */
		OWQUEUE_NO_PACKET,                                  /* input queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* input queue packet maximum */
		0,                                                  /* input queue packet info size */
		OWSL_TLS_PACKET_AVG_SIZE * OWSL_QUEUE_PACKET_MAX,   /* output queue usable size */
		OWQUEUE_NO_PACKET,                                  /* output queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* output queue packet maximum */
		0                                                   /* output queue packet info size */
	) ;
	if (socket != NULL)
	{
		OWSLSocketInfo_TLS * socket_tls = (OWSLSocketInfo_TLS *) socket ;
		OWSLSocketInfo_TCP * socket_tcp = (OWSLSocketInfo_TCP *) socket ;
		if (owsl_base_tcp_open (socket_tcp, address_family, owsl_tls_monitor_callback))
		{
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		owsl_system_socket_blocking_mode_set (socket_tls->system_socket, OWSL_NON_BLOCKING) ;
		socket_tls->connection = SSL_new (owsl_tls_context) ;
		if (socket_tls->connection == NULL)
		{
			owsl_base_tcp_close (socket_tcp) ;
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		if (SSL_set_fd (socket_tls->connection, (int) socket_tls->system_socket) != 1)
		{
			SSL_free (socket_tls->connection) ;
			owsl_base_tcp_close (socket_tcp) ;
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		if (pthread_mutex_init (& socket_tls->handshake_mutex, NULL))
		{
			SSL_free (socket_tls->connection) ;
			owsl_base_tcp_close (socket_tcp) ;
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		socket_tls->interrupted_action = OWSL_TLS_NONE ;
		socket_tls->interrupted_read_write_bytes = 0 ;
		snprintf(sem_name, sizeof(sem_name), "%d", ((int)time(NULL)) % 10000);
		if ((socket_tls->accept_connect_semaphore = owsl_sem_open (sem_name, O_CREAT | O_EXCL, 700, 0)) == NULL)
		{
			pthread_mutex_destroy (& socket_tls->handshake_mutex) ;
			SSL_free (socket_tls->connection) ;
			owsl_base_tcp_close (socket_tcp) ;
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
	}

	return socket ;
}

static OWSLSocketInfo *
owsl_tls_accept
(
	OWSLSocketInfo * listen_socket,
	struct sockaddr * address,
	socklen_t * address_length
)
{
	OWSLConnection connection ;
	OWSLSocketInfo * socket ;
	char sem_name[8];

	if (owqueue_read (listen_socket->in_queue, & connection, sizeof (connection), NULL, 0) != sizeof (connection))
	{
		return NULL ;
	}

	socket = owsl_socket_info_new
	(
		listen_socket->type_info->type,                     /* socket type */
		sizeof (OWSLSocketInfo_TLS),                        /* size of socket structure */
		OWSL_TLS_PACKET_AVG_SIZE * OWSL_QUEUE_PACKET_MAX,   /* input queue usable size */
		OWQUEUE_NO_PACKET,                                  /* input queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* input queue packet maximum */
		0,                                                  /* input queue packet info size */
		OWSL_TLS_PACKET_AVG_SIZE * OWSL_QUEUE_PACKET_MAX,   /* output queue usable size */
		OWQUEUE_NO_PACKET,                                  /* output queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* output queue packet maximum */
		0                                                   /* output queue packet info size */
	) ;
	if (socket != NULL)
	{
		OWSLSocketInfo_TLS * socket_tls = (OWSLSocketInfo_TLS *) socket ;
		OWSLSocketInfo_TCP * socket_tcp = (OWSLSocketInfo_TCP *) socket ;
		if (owsl_base_tcp_set (socket_tcp, & connection, owsl_tls_monitor_callback))
		{
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		owsl_system_socket_blocking_mode_set (socket_tls->system_socket, OWSL_NON_BLOCKING) ;
		socket_tls->connection = SSL_new (owsl_tls_context) ;
		if (socket_tls->connection == NULL)
		{
			owsl_base_tcp_close (socket_tcp) ;
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		if (SSL_set_fd (socket_tls->connection, (int) socket_tls->system_socket) != 1)
		{
			SSL_free (socket_tls->connection) ;
			owsl_base_tcp_close (socket_tcp) ;
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		if (SSL_accept (socket_tls->connection) != 1)
		{
			/* TODO: handle non blocking socket handshake */
			SSL_free (socket_tls->connection) ;
			owsl_base_tcp_close (socket_tcp) ;
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		if (pthread_mutex_init (& socket_tls->handshake_mutex, NULL))
		{
			SSL_free (socket_tls->connection) ;
			owsl_base_tcp_close (socket_tcp) ;
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		socket_tls->interrupted_action = OWSL_TLS_NONE ;
		socket_tls->interrupted_read_write_bytes = 0 ;
		snprintf(sem_name, sizeof(sem_name), "%d", ((int)time(NULL)) % 10000);
		if ((socket_tls->accept_connect_semaphore = owsl_sem_open (sem_name, O_CREAT | O_EXCL, 700, 0)) == NULL)
		{
			pthread_mutex_destroy (& socket_tls->handshake_mutex) ;
			SSL_free (socket_tls->connection) ;
			owsl_base_tcp_close (socket_tcp) ;
			owsl_socket_info_free (socket) ;
			return NULL ;
		}
		OW_MEMCPY (address, address_length, & connection.remote_address, connection.remote_address_length) ;
	}

	return socket ;
}

static int
owsl_tls_close
(
	OWSLSocketInfo * socket
)
{
	OWSLSocketInfo_TLS * socket_tls = (OWSLSocketInfo_TLS *) socket ;
	int return_code = 0 ;

	SSL_shutdown (socket_tls->connection) ;
	/* in non blocking mode, SSL_shutdown could need to wait and recall SSL_shutdown (for TLS conformance) */
	/* remove from the monitor before freeing SSL structure */
	return_code |= owsl_base_tcp_close ((OWSLSocketInfo_TCP *) socket) ;
	SSL_free (socket_tls->connection) ;
 	return_code |= owsl_sem_close (socket_tls->accept_connect_semaphore) ;
	return_code |= pthread_mutex_destroy (& socket_tls->handshake_mutex) ;
	return_code |= owsl_socket_info_free (socket) ;

	return return_code ;
}

static int
owsl_tls_connect
(
	OWSLSocketInfo * socket,
	const struct sockaddr * address,
	socklen_t address_length
)
{
	OWSLSocketInfo_TLS * socket_tls = (OWSLSocketInfo_TLS *) socket ;
	int return_code = connect (socket_tls->system_socket, address, address_length) ;
	if (return_code == 0 || OW_GET_ERROR == EINPROGRESS || OW_GET_ERROR == EWOULDBLOCK)
	{
		socket_tls->remote_address_length = OWSL_ADDRESS_SIZE ;
		OW_MEMCPY (& socket_tls->remote_address, & socket_tls->remote_address_length, address, address_length) ;
		if (return_code == 0)
		{
			if (owsl_tls_connect_handshake (socket_tls))
			{
				return_code = -1 ;
			}
		}
		else /* OW_GET_ERROR == EINPROGRESS || EWOULDBLOCK */
		{
			socket_tls->interrupted_action = OWSL_TLS_CONNECT ;
			owsl_monitor_event_add (socket_tls->system_socket, OWSL_MONITOR_WRITE | OWSL_MONITOR_ONCE) ;
			OW_SET_ERROR (EINPROGRESS) ;
		}
		if (socket->blocking_mode == OWSL_BLOCKING && return_code != 0)
		{
			if (sem_wait (socket_tls->accept_connect_semaphore))
			{
				return -1 ;
			}
			sem_post (socket_tls->accept_connect_semaphore) ;
			if (socket->connected > 0)
			{
				return 0 ;
			}
		}
	}
	return return_code ;
}

int
owsl_tls_initialize
(void)
{
	OWSLSocketTypeInfo type_ipv4 ;
	OWSLSocketTypeInfo type_ipv6 ;

	if (owsl_openssl_initialize ())
	{
		return -1 ;
	}
	owsl_tls_context = SSL_CTX_new (SSLv23_method ()) ;
	if (owsl_tls_context == NULL)
	{
		return -1 ;
	}
	SSL_CTX_set_options (owsl_tls_context, SSL_OP_NO_SSLv2) ;
	SSL_CTX_set_mode (owsl_tls_context, SSL_MODE_ENABLE_PARTIAL_WRITE) ;

	type_ipv4.type = OWSL_TYPE_IPV4_TLS ;
	type_ipv4.address_family = OWSL_AF_IPV4 ;
	type_ipv4.mode = OWSL_MODE_STREAM ;
	type_ipv4.ciphering = OWSL_CIPHERING_ENABLED ;
	type_ipv4.global_parameter_set = NULL ;
	type_ipv4.is_readable = NULL ;
	type_ipv4.is_writable = NULL ;
	type_ipv4.has_error = NULL ;
	type_ipv4.blocking_mode_set = NULL ;
	type_ipv4.parameter_set = NULL ;
	type_ipv4.reuse_set = owsl_base_system_socket_reuse_set ;
	type_ipv4.remote_address_get = owsl_base_remote_address_get ;
	type_ipv4.on_queue_event = owsl_base_in_out_queues_callback_with_monitor ;
	type_ipv4.socket = owsl_tls_socket ;
	type_ipv4.accept = owsl_tls_accept ;
	type_ipv4.close = owsl_tls_close ;
	type_ipv4.bind = owsl_base_bind ;
	type_ipv4.connect = owsl_tls_connect ;
	type_ipv4.listen = owsl_base_in_queue_listen ;
	type_ipv4.send = owsl_base_out_queue_send ;
	type_ipv4.recv = owsl_base_in_queue_connected_recv ;
	type_ipv4.sendto = owsl_base_out_queue_sendto ;
	type_ipv4.recvfrom = owsl_base_in_queue_connected_recvfrom ;

	memcpy (& type_ipv6, & type_ipv4, sizeof (OWSLSocketTypeInfo)) ;
	type_ipv6.type = OWSL_TYPE_IPV6_TLS ;
	type_ipv6.address_family = OWSL_AF_IPV6 ;

	return owsl_socket_type_initialize (& type_ipv4)
		|| owsl_socket_type_initialize (& type_ipv6) ;
}

int
owsl_tls_terminate
(void)
{
	int return_code = 0 ;

	SSL_CTX_free (owsl_tls_context) ;
	owsl_tls_context = NULL ;
	return_code |= owsl_openssl_terminate () ;

	return return_code ;
}
