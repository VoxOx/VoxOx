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

#include <stdlib.h>
#include <memory.h>

#include <openssl/ssl.h>
#include <pthread.h>

#include <owlibc.h>
#include <owlist.h>
#include <curl/curl.h>

#include "owsl_uoh_common.h"

static SSL_CTX * owsl_uohs_context = NULL;
static pthread_mutex_t *lock_tab;

typedef struct	OWSLSocketInfo_UoHs
{
	OWSL_SOCKET_INFO_WITH_CONNECTED_SYSTEM_SOCKET
	OWSLSocketType socket_type;
	CURL *curl;
	SSL *ssl_handle;
	int awaited_bytes;
	char awaited_buffer[OWSL_UOH_BUFFER_SIZE];
	int read_bytes;
	int sent_bytes;
	OWList *params_list;
	OWSLConnectionState_UoH state;
}	OWSLSocketInfo_UoHs;


/* ******************************************************** */
/*                   Configuration Parameters               */
/* ******************************************************** */

struct OWList *gl_params_list;

static const int OWSLParamKeyListSize = 8;

static const OWSLParamKey_UoH OWSLParamKeyList_UoH[] =
{
	{"proxy.local.enable", INT_PARAM},
	{"proxy.local.addr", STR_PARAM},
	{"proxy.local.port", USHORT_PARAM},
	{"proxy.local.login", STR_PARAM},
	{"proxy.local.passwd", STR_PARAM},
	{"gateway.http.addr", STR_PARAM},
	{"gateway.http.port", USHORT_PARAM},
	{"connection.timeout", INT_PARAM},
	{NULL, 0}
};

static int owsl_uohs_global_parameter_set(const char * name, const void * value)
{
	return owsl_uoh_common_parameter_set(OWSLParamKeyList_UoH, name, value, gl_params_list);
}

static int owsl_uohs_parameter_set(OWSLSocketInfo * socket_info,
								  const char * name,
								  const void * value)
{
	OWSLSocketInfo_UoHs *sock = (OWSLSocketInfo_UoHs *)socket_info;

	return owsl_uoh_common_parameter_set(OWSLParamKeyList_UoH, name, value, sock->params_list);
}

/* ******************************************************** */

static void
owsl_uohs_system_socket_recv(OWSLSocketInfo_UoHs *sock)
{
	int received = 0;
	int ret = 0;

	// loop while there is data on ssl pending queue
	do
	{
		// Get the first 4 bytes to know the packet size
		if (sock->awaited_bytes == 0)
		{
			int size = 0;

			// TODO: add 4 bytes in UOH header to know which host this packet is from
			if ((ret = SSL_read(sock->ssl_handle, (char *) &size, 0)) == -1
					&& ERR_SOCK(OW_GET_ERROR))
			{
				// TODO: log : what kind of error it is
				sock->state = UOH_ST_NOT_CONNECTED;
				return;
			}
			if ((ret = SSL_pending(sock->ssl_handle)) >= OWSL_UOH_INT_SIZE)
			{
				if ((ret = SSL_read(sock->ssl_handle, (char *) &size, OWSL_UOH_INT_SIZE)) != OWSL_UOH_INT_SIZE)
				{
					// TODO: log : what a strange behavior
					return;
				}
				else
				{
					sock->awaited_bytes = size;
				}
			}
			else
			{
				// Not enough data on pending queue
				return;
			}

			if (!SSL_pending(sock->ssl_handle))
				return;
		}
	
		if (sock->awaited_bytes && sock->awaited_bytes != sock->read_bytes)
		{
			int needed = sock->awaited_bytes - sock->read_bytes;
		
			if ((received = SSL_read(sock->ssl_handle, 
				sock->awaited_buffer + sock->read_bytes, needed)) > -1)
			{
				sock->read_bytes += received;
			}
			else if (received == -1 && ERR_SOCK(OW_GET_ERROR))
			{
				sock->state = UOH_ST_NOT_CONNECTED;
				return;
			}
		}

		if (sock->read_bytes == sock->awaited_bytes)
		{
			OWSLSocketInfo *sock_info = (OWSLSocketInfo *) sock;

			/* info packet is sock->remote_address + sock->remote_address_length */
			if (owqueue_write(sock_info->in_queue, sock->awaited_buffer,
				sock->awaited_bytes, &sock->remote_address, OWQUEUE_NON_BLOCKING))
			{
				owsl_notify((OWSLSocketInfo *) sock, OWSL_EVENT_READ);
			}
			/*
			else
				// Here, the packet is dropped because no space is available in queue
			*/
			sock->read_bytes = 0;
			sock->awaited_bytes = 0;
			memset(sock->awaited_buffer, 0, sizeof(sock->awaited_buffer));		
			
		}
	} while (SSL_pending(sock->ssl_handle));
}

static void owsl_uohs_system_socket_send(OWSLSocketInfo_UoHs *sock)
{
	int read_bytes, sent_bytes;
	char buffer[OWSL_UOH_BUFFER_SIZE];
	OWSLSocketInfo *sock_info = (OWSLSocketInfo *) sock;

	memset(buffer, 0, OWSL_UOH_BUFFER_SIZE);
	/* get packet and packet size */
	read_bytes = owqueue_read(sock_info->out_queue, buffer, 
		OWSL_UOH_BUFFER_SIZE, NULL, OWQUEUE_NON_BLOCKING | OWQUEUE_TRANSACTION_OPEN);
	if (read_bytes > 0)
	{
		int needed_bytes = read_bytes - sock->sent_bytes;
		/* send packet */
		sent_bytes = SSL_write(sock->ssl_handle, buffer + sock->sent_bytes, needed_bytes);

		if (sent_bytes == -1 && ERR_SOCK(OW_GET_ERROR))
		{
			owqueue_read_transaction_close(sock_info->out_queue, read_bytes);
			sock->state = UOH_ST_NOT_CONNECTED;
			return;
		}
		else
		{
			/* commit or rollback transaction */
			owqueue_read_transaction_close(sock_info->out_queue, needed_bytes == sent_bytes ? read_bytes : 0);
		}

		if (needed_bytes == sent_bytes)
		{
			//owsl_monitor_event_remove(sock->system_socket, OWSL_EVENT_WRITE);
			sock->sent_bytes = 0;
		}
		else if (sent_bytes > 0) // && logically needed_bytes < sent_bytes
		{
			sock->sent_bytes += sent_bytes;
		}
	}
}

static void owsl_uoh_monitor_callback(OWSLSystemSocket system_socket,
									  OWSLMonitorEvent event,
									  void * _socket)
{
	OWSLSocketInfo_UoHs *sock = _socket;

	if (event & OWSL_MONITOR_ERROR || sock->system_socket != system_socket)
	{
		owsl_notify ((OWSLSocketInfo *) sock, OWSL_EVENT_ERROR) ;
		return ;
	}

	if ((event & OWSL_MONITOR_READ) || SSL_pending(sock->ssl_handle))
	{
		owsl_uohs_system_socket_recv(sock);
	}
	if (event & OWSL_MONITOR_WRITE)
	{
		owsl_uohs_system_socket_send(sock);
	}
}

static int _owsl_uohs_monitor_socket(OWSLSocketInfo * socket_info)
{
	if (socket_info != NULL)
	{
		OWSLSocketInfo_UoHs * sock = (OWSLSocketInfo_UoHs *) socket_info;

		if (sock->system_socket <= 0)
		{
			owsl_socket_info_free(socket_info);
			return -1;
		}
		if (owsl_monitor_socket_add(sock->system_socket, owsl_uoh_monitor_callback, socket_info))
		{
			owsl_system_socket_close(sock->system_socket);
			return -1;
		}
		if (owsl_monitor_event_add(sock->system_socket, OWSL_EVENT_READ))
		{
			owsl_monitor_socket_remove(sock->system_socket);
			owsl_system_socket_close(sock->system_socket);
			return -1;
		}
	}

	return 0;
}

static OWSLSocketInfo *owsl_uohs_socket(OWSLSocketType type)
{
	OWSLSocketInfo *socket_info;
	OWSLSocketInfo_UoHs *socket_uohs;

	if (type != OWSL_TYPE_IPV4_UOHS && type != OWSL_TYPE_IPV6_UOHS)
	{
		return NULL ;
	}

	socket_info = owsl_socket_info_new(
		type,                                               /* socket type */
		sizeof (OWSLSocketInfo_UoHs),                        /* size of socket structure */
		OWSL_UOH_PACKET_SIZE * OWSL_QUEUE_PACKET_MAX,       /* input queue usable size */
		OWQUEUE_PACKET,                                     /* input queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* input queue packet maximum */
		sizeof (OWSLRemoteAddress),                         /* input queue packet info size */
		OWSL_UOH_PACKET_SIZE * OWSL_QUEUE_PACKET_MAX,       /* output queue usable size */
		OWQUEUE_PACKET,                                     /* output queue mode */
		OWSL_QUEUE_PACKET_MAX,                              /* output queue packet maximum */
		0                                                   /* output queue packet info size */
	);

	if (!socket_info)
		return NULL;

	// Initialization of private vars
	socket_uohs = (OWSLSocketInfo_UoHs *) socket_info;
	socket_uohs->awaited_bytes = 0;
	memset(socket_uohs->awaited_buffer, 0, sizeof(socket_uohs->awaited_buffer));
	socket_uohs->read_bytes = 0;
	socket_uohs->sent_bytes = 0;
	socket_uohs->curl = NULL;
	socket_uohs->params_list = owlist_new();
	socket_uohs->state = UOH_ST_NOT_CONNECTED;
	socket_uohs->system_socket = 0;
	socket_uohs->socket_type = type;

	socket_uohs->remote_address_length = 0;				//VOXOX - JRT - 2010.01.25 
	socket_uohs->remotely_closed       = 0;				//VOXOX - JRT - 2010.01.25 
	socket_uohs->ssl_handle			   = 0;				//VOXOX - JRT - 2010.01.25 - causing crash if HTTP Tunnel not available.

	// End of initialization

	return socket_info;
}

static int owsl_uohs_close(OWSLSocketInfo * socket_info)
{
	OWSLSocketInfo_UoHs * sock = (OWSLSocketInfo_UoHs *) socket_info;
	int return_code = 0 ;

	if (sock->system_socket <= 0)
		return -1;
	else
		return_code |= owsl_monitor_socket_remove(sock->system_socket);

	if (sock->ssl_handle)
	{
		SSL_free(sock->ssl_handle);
	}

	if (sock->curl)
		curl_easy_cleanup(sock->curl);
	else if (sock->system_socket)
		return_code |= owsl_system_socket_close(sock->system_socket);
	else
		return_code = -1;

	return_code |= owsl_socket_info_free(socket_info);

	return return_code;
}

static int _owsl_uohs_create_curl_connection(OWSLSocketInfo_UoHs *sock,
											 const char *gate_http_addr, unsigned short gate_http_port,
											 const char *proxy_addr, unsigned short proxy_port,
											 const char *proxy_login, const char *proxy_passwd,
											 int timeout)
{
	char url_buff[1024];
	char proxy_buff[1024];
	char login_buff[1024];
	int ret = 0;

	if (!(sock->curl = curl_easy_init()))
	{
		return -1;
	}

	curl_easy_setopt(sock->curl, CURLOPT_CONNECT_ONLY, 1);

	snprintf(url_buff, sizeof(url_buff), "http://%s:%d", gate_http_addr, gate_http_port);
	curl_easy_setopt(sock->curl, CURLOPT_URL, url_buff);

	snprintf(proxy_buff, sizeof(proxy_buff), "%s:%d", proxy_addr, proxy_port);
	curl_easy_setopt(sock->curl, CURLOPT_PROXY, proxy_buff);

	curl_easy_setopt(sock->curl, CURLOPT_CONNECTTIMEOUT, timeout);
	curl_easy_setopt(sock->curl, CURLOPT_HTTPPROXYTUNNEL, 1);

	if (proxy_login)
	{
		snprintf(login_buff, sizeof(login_buff), "%s:%s", proxy_login, proxy_passwd);
		curl_easy_setopt(sock->curl, CURLOPT_PROXYUSERPWD, login_buff);
		curl_easy_setopt(sock->curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
	}

	if ((ret = curl_easy_perform(sock->curl)) != 0)
	{
		return -1;
	}

	curl_easy_getinfo(sock->curl, CURLINFO_LASTSOCKET, &sock->system_socket);
	return 0;
}

static int _owsl_uohs_create_std_connection(OWSLSocketInfo_UoHs *sock,
											const char *gate_http_addr, unsigned short gate_http_port,
											int timeout)
{
	struct sockaddr *remote_address;

	if (sock->system_socket == 0)
	{
		sock->system_socket = socket(sock->socket_type == OWSL_TYPE_IPV4_UOHS ? PF_INET : PF_INET6, 
			SOCK_STREAM, IPPROTO_TCP);
		if (sock->system_socket == -1)
		{
			return -1;
		}
	}

#ifdef SO_NOSIGPIPE
	setsockopt(sock->system_socket, SOL_SOCKET, SO_NOSIGPIPE, NULL, 0);
#endif

	remote_address = owsl_address_port_new(OWSL_AF_IPV4, gate_http_addr, gate_http_port);

	if (connect(sock->system_socket, remote_address, OWSL_ADDRESS_SIZE) == -1)
	{
		return -1;
	}

	return 0;
}

static int _owsl_uohs_get_http_response(OWSLSocketInfo_UoHs *sock, char *buff, int buffsize)
{
	struct timeval timeout;
	fd_set rfds;
	int ret;
	int nbytes = 0;

	while (1)
	{
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(sock->system_socket, &rfds);

		ret = select(sock->system_socket + 1, &rfds, 0, 0, &timeout);
		
		if (ret <= 0)
		{
			return -1;
		}

		if (FD_ISSET(sock->system_socket, &rfds))
		{
			do
			{
				ret = SSL_read(sock->ssl_handle, buff + nbytes, 1);
			
				if (ret < 0)
					return -1;
				else if (ret == 0)
					return nbytes;
				else
					nbytes += ret;

				if (nbytes == buffsize)
					return nbytes;

				if (nbytes > 3 && strncmp("\r\n\r\n", buff + nbytes - 4, 4) == 0)
					return nbytes;
			}
			while (SSL_pending(sock->ssl_handle));
		}
	}

	return nbytes;
}

static int _owsl_uohs_make_http_request(OWSLSocketInfo_UoHs *sock, const char * ip_addr, unsigned short port)
{
	char query[512];
	char buff[2048];
	int nbytes;

	snprintf(query, sizeof(query), HTTP_REQUEST_STR, ip_addr, port);

	nbytes = SSL_write(sock->ssl_handle, query, (int) strlen(query));
	if (nbytes <= 0)
	{
		owsl_system_socket_close(sock->system_socket);
		return -1;
	}

	nbytes = _owsl_uohs_get_http_response(sock, buff, sizeof(buff) - 1);
	if (nbytes <= 0)
		return -1;

	buff[nbytes] = 0;
	if (strncmp(buff, "HTTP/1.0 200 OK\r\n", 17) && strncmp(buff, "HTTP/1.1 200 OK\r\n", 17))
		return -1;

	return 0;
}

static int _owsl_uohs_create_connection(OWSLSocketInfo_UoHs * sock)
{
	int timeout = owsl_uoh_parameter_value_get(sock->params_list, "connection.timeout")->i;
	const char *proxy_addr = owsl_uoh_parameter_value_get(sock->params_list, "proxy.local.addr")->str;
	unsigned short proxy_port = owsl_uoh_parameter_value_get(sock->params_list, "proxy.local.port")->i;
	const char *proxy_login = owsl_uoh_parameter_value_get(sock->params_list, "proxy.local.login")->str;
	const char *proxy_passwd = owsl_uoh_parameter_value_get(sock->params_list, "proxy.local.passwd")->str;
	const char *gate_http_addr = owsl_uoh_parameter_value_get(sock->params_list, "gateway.http.addr")->str;
	unsigned short gate_http_port = owsl_uoh_parameter_value_get(sock->params_list, "gateway.http.port")->i;

	if (!timeout)
		timeout = owsl_uoh_parameter_value_get(gl_params_list, "connection.timeout")->i;
	if (!proxy_addr)
		proxy_addr = owsl_uoh_parameter_value_get(gl_params_list, "proxy.local.addr")->str;
	if (!proxy_port)
		proxy_port = owsl_uoh_parameter_value_get(gl_params_list, "proxy.local.port")->i;
	if (!proxy_login)
		proxy_login = owsl_uoh_parameter_value_get(gl_params_list, "proxy.local.login")->str;
	if (!proxy_passwd)
		proxy_passwd = owsl_uoh_parameter_value_get(gl_params_list, "proxy.local.passwd")->str;
	if (!gate_http_addr)
		gate_http_addr = owsl_uoh_parameter_value_get(gl_params_list, "gateway.http.addr")->str;
	if (!gate_http_port)
		gate_http_port = owsl_uoh_parameter_value_get(gl_params_list, "gateway.http.port")->i;
	

	if (owsl_uoh_parameter_value_get(sock->params_list, "proxy.local.enable")->i)
	{
		if (_owsl_uohs_create_curl_connection(sock, gate_http_addr, gate_http_port,
			proxy_addr, proxy_port, proxy_login, proxy_passwd, timeout) < 0)
		{
			return -1;
		}
	}
	else
	{
		if (_owsl_uohs_create_std_connection(sock, gate_http_addr, gate_http_port, timeout) < 0)
		{
			return -1;
		}
	}

	return 0;
}

static int _owsl_uohs_create_ssl_connection(OWSLSocketInfo_UoHs * sock)
{
	if (!(sock->ssl_handle = SSL_new(owsl_uohs_context)))
		return -1;

	SSL_set_fd(sock->ssl_handle, sock->system_socket);

	if (SSL_connect(sock->ssl_handle) <= 0)
		return -1;

	return 0;
}

static int owsl_uohs_connect(OWSLSocketInfo *socket_info,
							 const struct sockaddr *address,
							 socklen_t address_length)
{
	OWSLSocketInfo_UoHs * sock = (OWSLSocketInfo_UoHs *) socket_info;
	char ip_address[OWSL_IP_MAX_SIZE];
	unsigned short port;

	if (address == NULL)
	{
		return -1;
	}
	
	if (sock->state == UOH_ST_CONNECTED)
	{
		return 0;
	}

	sock->remote_address_length = OWSL_ADDRESS_SIZE;
	OW_MEMCPY(&sock->remote_address, &sock->remote_address_length, address, address_length);

	owsl_address_parse(address, NULL, ip_address, sizeof(ip_address), &port);
	
	if (_owsl_uohs_create_connection(sock) < 0)
	{
		return -1;
	}

	if (_owsl_uohs_create_ssl_connection(sock) < 0)
	{
		return -1;
	}

	if (_owsl_uohs_make_http_request(sock, ip_address, port) < 0)
	{
		return -1;
	}

	_owsl_uohs_monitor_socket(socket_info);
	sock->state = UOH_ST_CONNECTED;

	return 0;
}

static int owsl_uohs_bind(OWSLSocketInfo * _socket_info_with_system_socket,
						  struct sockaddr * address,
						  socklen_t address_length)
{
	return 0;
}

static int owsl_uohs_send(OWSLSocketInfo * socket_info,
						  const void * buffer,
						  int length,
						  int flags)
{
	char tmp_buff[OWSL_UOH_BUFFER_SIZE];
	int total_length = length + OWSL_UOH_INT_SIZE;
	int bytes_sent = 0;
	OWSLSocketInfo_UoHs * sock = (OWSLSocketInfo_UoHs *) socket_info;

	// check if the connection is still established, otherwise create it
	if (sock->state != UOH_ST_CONNECTED)
	{
		if (owsl_uohs_connect(socket_info, (struct sockaddr *) &sock->remote_address, 
			sock->remote_address_length) != 0)
		{
			return -1;
		}
	}

	memset(tmp_buff, 0, sizeof(tmp_buff));
	memcpy(tmp_buff, &length, OWSL_UOH_INT_SIZE);
	memcpy(tmp_buff + OWSL_UOH_INT_SIZE, buffer, length);

	bytes_sent = owqueue_write(socket_info->out_queue, tmp_buff, total_length, NULL, 0);
	if (bytes_sent < total_length)
	{
		return -1 ;
	}

	return (bytes_sent - OWSL_UOH_INT_SIZE);
}

static int owsl_uohs_sendto(OWSLSocketInfo * socket_info, const void * buffer, int length,
							int flags, const struct sockaddr * address, socklen_t address_length)
{
	OWSLSocketInfo_UoHs * sock = (OWSLSocketInfo_UoHs *) socket_info;

	// check if the connection is still established, otherwise create it
	if (sock->state != UOH_ST_CONNECTED)
	{
		if (owsl_uohs_connect(socket_info, address, address_length) != 0)
			return -1;
	}

	return owsl_uohs_send(socket_info, buffer, length, flags);
}

static void owsl_uohs_mutex_lock_unlock(int mode, int type, const char *file, int line)
{
	if (mode & CRYPTO_LOCK)
		pthread_mutex_lock(&(lock_tab[type]));
	else
		pthread_mutex_unlock(&(lock_tab[type]));
}

static unsigned long owsl_uohs_thread_id_get(void)
{
	pthread_t thread_id;

	thread_id = pthread_self();
	return ((unsigned long) *((unsigned long *) &thread_id));
}

static int owsl_uohs_ssl_context_initialize()
{
	int i;

	owsl_uohs_context = SSL_CTX_new (SSLv23_method ()) ;
	if (owsl_uohs_context == NULL)
	{
		return -1 ;
	}

	SSL_CTX_set_options (owsl_uohs_context, SSL_OP_NO_SSLv2);
	SSL_CTX_set_verify(owsl_uohs_context, SSL_VERIFY_NONE, 0);

	lock_tab = OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	for (i = 0; i < CRYPTO_num_locks(); i++)
	{
		pthread_mutex_init(&(lock_tab[i]), NULL);
	}
	CRYPTO_set_id_callback((unsigned long (*)(void))owsl_uohs_thread_id_get);
	
	CRYPTO_set_locking_callback(owsl_uohs_mutex_lock_unlock);
	
	return 0;
}

int owsl_uohs_initialize(void)
{
	OWSLSocketTypeInfo ipv4_uohs;
	OWSLSocketTypeInfo ipv6_uohs;

	gl_params_list = owlist_new();
	
	if (owsl_openssl_initialize ())
	{
		return -1;
	}

	if (owsl_uohs_ssl_context_initialize() != 0)
	{
		return -1;
	}

	ipv4_uohs.type = OWSL_TYPE_IPV4_UOHS;
	ipv4_uohs.address_family = OWSL_AF_IPV4;
	ipv4_uohs.mode = OWSL_MODE_DATAGRAM;
	ipv4_uohs.ciphering = OWSL_CIPHERING_DISABLED;
	ipv4_uohs.global_parameter_set = owsl_uohs_global_parameter_set;
	ipv4_uohs.is_readable = NULL;
	ipv4_uohs.is_writable = NULL;
	ipv4_uohs.has_error = NULL;
	ipv4_uohs.blocking_mode_set = NULL;
	ipv4_uohs.parameter_set = owsl_uohs_parameter_set;
	ipv4_uohs.reuse_set = owsl_base_system_socket_reuse_set;
	ipv4_uohs.remote_address_get = NULL;
	ipv4_uohs.on_queue_event = owsl_base_in_out_queues_callback_with_monitor;
	ipv4_uohs.socket = owsl_uohs_socket;
	ipv4_uohs.accept = NULL;
	ipv4_uohs.close = owsl_uohs_close;
	ipv4_uohs.bind = owsl_uohs_bind;
	ipv4_uohs.connect = owsl_uohs_connect;
	ipv4_uohs.listen = NULL;
	ipv4_uohs.send = owsl_uohs_send;
	ipv4_uohs.recv = owsl_base_in_queue_recv;
	ipv4_uohs.sendto = owsl_uohs_sendto;
	ipv4_uohs.recvfrom = owsl_base_in_queue_recvfrom;

	memcpy (&ipv6_uohs, &ipv4_uohs, sizeof (OWSLSocketTypeInfo));
	ipv6_uohs.type = OWSL_TYPE_IPV6_UOHS;
	ipv6_uohs.address_family = OWSL_AF_IPV6;

	return owsl_socket_type_initialize (& ipv4_uohs)
		|| owsl_socket_type_initialize (& ipv6_uohs);
}
