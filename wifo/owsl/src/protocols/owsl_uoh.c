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

#include <owlibc.h>
#include <owlist.h>
#include <curl/curl.h>

#include "owsl_uoh_common.h"

typedef struct	OWSLSocketInfo_UoH
{
	OWSL_SOCKET_INFO_WITH_CONNECTED_SYSTEM_SOCKET
	OWSLSocketType socket_type;
	CURL *curl;
	int awaited_bytes;
	char awaited_buffer[OWSL_UOH_BUFFER_SIZE];
	int read_bytes;
	int latest_read_bytes;
	int sent_bytes;
	OWList *params_list;
	OWSLConnectionState_UoH state;
}	OWSLSocketInfo_UoH;


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

static int owsl_uoh_global_parameter_set(const char * name, const void * value)
{
	return owsl_uoh_common_parameter_set(OWSLParamKeyList_UoH, name, value, gl_params_list);
}

static int owsl_uoh_parameter_set(OWSLSocketInfo * socket_info,
								  const char * name,
								  const void * value)
{
	OWSLSocketInfo_UoH *sock = (OWSLSocketInfo_UoH *)socket_info;

	return owsl_uoh_common_parameter_set(OWSLParamKeyList_UoH, name, value, sock->params_list);
}

/* ******************************************************** */

static void
owsl_uoh_system_socket_recv(OWSLSocketInfo_UoH *sock)
{
	int received = 0;
	char buff_tmp[2048];
	int ret = 0;

	// Get the first 4 bytes to know the packet size
	if (sock->awaited_bytes == 0)
	{
		int size = 0;

		// TODO: add 4 bytes in UOH header to know which host this packet is from
		if ((ret = recv(sock->system_socket, (char *) &size, OWSL_UOH_INT_SIZE, MSG_PEEK)) == OWSL_UOH_INT_SIZE)
		{
			if (recv(sock->system_socket, (char *) &size, OWSL_UOH_INT_SIZE, 0) != OWSL_UOH_INT_SIZE)
			{
				// TODO: log : something strange happens
				return;
			}
			sock->awaited_bytes = size;
		}
		else if (ret == -1 && ERR_SOCK(OW_GET_ERROR))
		{
			sock->state = UOH_ST_NOT_CONNECTED;
			return;
		}
	}
	else if (sock->awaited_bytes && sock->awaited_bytes != sock->read_bytes)
	{
		int needed = sock->awaited_bytes - sock->read_bytes;

		if ((received = recv(sock->system_socket, 
			sock->awaited_buffer + sock->read_bytes, needed, MSG_PEEK)) > -1)
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
			sock->read_bytes = 0;
			sock->awaited_bytes = 0;
			memset(sock->awaited_buffer, 0, sizeof(sock->awaited_buffer));
			
			// Remove the latest packet from the socket buffer
			recv(sock->system_socket, buff_tmp, received ? received : sock->latest_read_bytes, 0);
			sock->latest_read_bytes = 0;
		}
		else
		{
			// Save the latest packet size to remove it from the socket buffer.
			// Let something in the socket for having this function called again
			// once there will be space available in queue.
			sock->latest_read_bytes = received;
		}
	}
	else 
	{
		// it still is not the latest 'packet' to reach the awaited size
		// so remove it from the socket buffer
		recv(sock->system_socket, buff_tmp, received, 0);
	}
}

static void owsl_uoh_system_socket_send(OWSLSocketInfo_UoH *sock)
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
		sent_bytes = send(sock->system_socket, buffer + sock->sent_bytes, needed_bytes, 0);

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
	OWSLSocketInfo_UoH *sock = _socket;

	if (event & OWSL_MONITOR_ERROR || sock->system_socket != system_socket)
	{
		owsl_notify ((OWSLSocketInfo *) sock, OWSL_EVENT_ERROR) ;
		return ;
	}

	if (event & OWSL_MONITOR_READ)
	{
		owsl_uoh_system_socket_recv(sock);
	}
	if (event & OWSL_MONITOR_WRITE)
	{
		owsl_uoh_system_socket_send(sock);
	}
}

static int _owsl_uoh_monitor_socket(OWSLSocketInfo * socket_info)
{
	if (socket_info != NULL)
	{
		OWSLSocketInfo_UoH * sock = (OWSLSocketInfo_UoH *) socket_info;

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

static OWSLSocketInfo *owsl_uoh_socket(OWSLSocketType type)
{
	OWSLSocketInfo *socket_info;
	OWSLSocketInfo_UoH *socket_uoh;

	if (type != OWSL_TYPE_IPV4_UOH && type != OWSL_TYPE_IPV6_UOH)
	{
		return NULL ;
	}

	socket_info = owsl_socket_info_new(
		type,                                               /* socket type */
		sizeof (OWSLSocketInfo_UoH),                        /* size of socket structure */
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
	socket_uoh = (OWSLSocketInfo_UoH *) socket_info;
	socket_uoh->awaited_bytes = 0;
	memset(socket_uoh->awaited_buffer, 0, sizeof(socket_uoh->awaited_buffer));
	socket_uoh->read_bytes = 0;
	socket_uoh->latest_read_bytes = 0;
	socket_uoh->sent_bytes = 0;
	socket_uoh->curl = NULL;
	socket_uoh->params_list = owlist_new();
	socket_uoh->state = UOH_ST_NOT_CONNECTED;
	socket_uoh->system_socket = 0;
	socket_uoh->socket_type = type;
	socket_uoh->remote_address_length = 0;				//VOXOX - JRT - 2010.01.25 
	socket_uoh->remotely_closed       = 0;				//VOXOX - JRT - 2010.01.25 
	// End of initialization

	return socket_info;
}

static int owsl_uoh_close(OWSLSocketInfo * socket_info)
{
	OWSLSocketInfo_UoH * sock = (OWSLSocketInfo_UoH *) socket_info;
	int return_code = 0 ;

	if (sock->system_socket <= 0)
		return -1;
	else
		return_code |= owsl_monitor_socket_remove(sock->system_socket);

	if (sock->curl)
		curl_easy_cleanup(sock->curl);
	else if (sock->system_socket)
		return_code |= owsl_system_socket_close(sock->system_socket);
	else
		return_code = -1;

	return_code |= owsl_socket_info_free(socket_info);

	return return_code;
}

static int _owsl_uoh_create_curl_connection(OWSLSocketInfo_UoH *sock,
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

static int _owsl_uoh_create_std_connection(OWSLSocketInfo_UoH *sock,
											const char *gate_http_addr, unsigned short gate_http_port,
											int timeout)
{
	struct sockaddr *remote_address;

	if (sock->system_socket == 0)
	{
		sock->system_socket = socket(sock->socket_type == OWSL_TYPE_IPV4_UOH ? PF_INET : PF_INET6, 
			SOCK_STREAM, IPPROTO_TCP);
		if (sock->system_socket == -1)
		{
			return -1;
		}
	}

#ifdef SO_NOSIGPIPE
	setsockopt(sock->system_socket, SOL_SOCKET, SO_NOSIGPIPE, NULL, 0);
#endif

	remote_address = owsl_address_port_new(OWSL_AF_UNDEFINED, gate_http_addr, gate_http_port);

	//VOXOX CHANGE by ASV 07-06-2009: Fixed Http Tunnel not working on Mac.
	// The address_len argument was the correct size for the address familly.
	if (connect(sock->system_socket, remote_address, OWSL_IPV4_MAX_SIZE) == -1)
	{
		return -1;
	}

	return 0;
}

static int _owsl_uoh_get_http_response(OWSLSocketInfo_UoH *sock, char *buff, int buffsize)
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
			ret = recv(sock->system_socket, buff + nbytes, 1, 0);
			
			if (ret < 0)
				return -1;
			else if (ret == 0)
				return nbytes;
			else
				nbytes += ret;

			if (nbytes == buffsize)
				return nbytes;
			
			if (nbytes > 3 && strncmp("\r\n\r\n", buff + nbytes - 4, 4) == 0)
				break;
		}
	}

	return nbytes;
}

static int _owsl_uoh_make_http_request(OWSLSocketInfo_UoH *sock, const char * ip_addr, unsigned short port)
{
	char query[512];
	char buff[2048];
	int nbytes;

	snprintf(query, sizeof(query), HTTP_REQUEST_STR, ip_addr, port);

	nbytes = send(sock->system_socket, query, (int) strlen(query), MSG_NOSIGNAL);
	if (nbytes <= 0)
	{
		owsl_system_socket_close(sock->system_socket);
		return -1;
	}

	nbytes = _owsl_uoh_get_http_response(sock, buff, sizeof(buff) - 1);
	if (nbytes <= 0)
		return -1;

	buff[nbytes] = 0;
	if (strncmp(buff, "HTTP/1.0 200 OK\r\n", 17) && strncmp(buff, "HTTP/1.1 200 OK\r\n", 17))
		return -1;

	return 0;
}

static int _owsl_uoh_create_connection(OWSLSocketInfo_UoH *sock)
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
		if (_owsl_uoh_create_curl_connection(sock, gate_http_addr, gate_http_port,
			proxy_addr, proxy_port, proxy_login, proxy_passwd, timeout) < 0)
		{
			return -1;
		}
	}
	else
	{
		if (_owsl_uoh_create_std_connection(sock, gate_http_addr, gate_http_port, timeout) < 0)
		{
			return -1;
		}
	}

	return 0;
}

static int owsl_uoh_connect(OWSLSocketInfo *socket_info,
							const struct sockaddr *address,
							socklen_t address_length)
{
	OWSLSocketInfo_UoH * sock = (OWSLSocketInfo_UoH *) socket_info;
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
	
	if (_owsl_uoh_create_connection(sock) < 0)
	{
		return -1;
	}

	if (_owsl_uoh_make_http_request(sock, ip_address, port) < 0)
	{
		return -1;
	}

	_owsl_uoh_monitor_socket(socket_info);
	sock->state = UOH_ST_CONNECTED;

	return 0;
}

static int owsl_uoh_bind(OWSLSocketInfo * _socket_info_with_system_socket,
						 struct sockaddr * address,
						 socklen_t address_length)
{
	return 0;
}

static int owsl_uoh_send(OWSLSocketInfo * socket_info,
						 const void * buffer,
						 int length,
						 int flags)
{
	char tmp_buff[OWSL_UOH_BUFFER_SIZE];
	int total_length = length + OWSL_UOH_INT_SIZE;
	int bytes_sent = 0;
	OWSLSocketInfo_UoH * sock = (OWSLSocketInfo_UoH *) socket_info;

	// check if the connection is still established, otherwise create it
	if (sock->state != UOH_ST_CONNECTED)
	{
		if (owsl_uoh_connect(socket_info, (struct sockaddr *) &sock->remote_address, 
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

static int owsl_uoh_sendto(OWSLSocketInfo * socket_info, const void * buffer, int length,
						   int flags, const struct sockaddr * address, socklen_t address_length)
{
	OWSLSocketInfo_UoH * sock = (OWSLSocketInfo_UoH *) socket_info;

	// check if the connection is still established, otherwise create it
	if (sock->state != UOH_ST_CONNECTED)
	{
		if (owsl_uoh_connect(socket_info, address, address_length) != 0)
			return -1;
	}

	return owsl_uoh_send(socket_info, buffer, length, flags);
}

int owsl_uoh_initialize(void)
{
	OWSLSocketTypeInfo ipv4_uoh;
	OWSLSocketTypeInfo ipv6_uoh;

	gl_params_list = owlist_new();

	ipv4_uoh.type = OWSL_TYPE_IPV4_UOH;
	ipv4_uoh.address_family = OWSL_AF_IPV4;
	ipv4_uoh.mode = OWSL_MODE_DATAGRAM;
	ipv4_uoh.ciphering = OWSL_CIPHERING_DISABLED;
	ipv4_uoh.global_parameter_set = owsl_uoh_global_parameter_set;
	ipv4_uoh.is_readable = NULL;
	ipv4_uoh.is_writable = NULL;
	ipv4_uoh.has_error = NULL;
	ipv4_uoh.blocking_mode_set = NULL;
	ipv4_uoh.parameter_set = owsl_uoh_parameter_set;
	ipv4_uoh.reuse_set = owsl_base_system_socket_reuse_set;
	ipv4_uoh.remote_address_get = NULL;
	ipv4_uoh.on_queue_event = owsl_base_in_out_queues_callback_with_monitor;
	ipv4_uoh.socket = owsl_uoh_socket;
	ipv4_uoh.accept = NULL;
	ipv4_uoh.close = owsl_uoh_close;
	ipv4_uoh.bind = owsl_uoh_bind;
	ipv4_uoh.connect = owsl_uoh_connect;
	ipv4_uoh.listen = NULL;
	ipv4_uoh.send = owsl_uoh_send;
	ipv4_uoh.recv = owsl_base_in_queue_recv;
	ipv4_uoh.sendto = owsl_uoh_sendto;
	ipv4_uoh.recvfrom = owsl_base_in_queue_recvfrom;

	memcpy (&ipv6_uoh, &ipv4_uoh, sizeof (OWSLSocketTypeInfo));
	ipv6_uoh.type = OWSL_TYPE_IPV6_UOH;
	ipv6_uoh.address_family = OWSL_AF_IPV6;

	return owsl_socket_type_initialize (& ipv4_uoh)
		|| owsl_socket_type_initialize (& ipv6_uoh);
}
