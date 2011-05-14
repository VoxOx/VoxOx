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

#ifdef OS_WINDOWS
	#define OWSL_SETSOCKOPT(socket,level,option_name,option_value) \
		setsockopt (socket, level, option_name, (void *) & (option_value), sizeof (option_value)) ;
	#define OWSL_INVALID_SOCKET INVALID_SOCKET
#else /* OS_POSIX */
	#include <fcntl.h>
	#define OWSL_SETSOCKOPT(socket,level,option_name,option_value) \
		setsockopt (socket, level, option_name, (char *) & (option_value), sizeof (option_value)) ;
	#define OWSL_INVALID_SOCKET -1
#endif

int
owsl_system_socket_initialize
(void)
{
#ifdef OS_WINDOWS
	WSADATA wsa_data ;

	/* init windows socket library */
	if (WSAStartup (MAKEWORD (2, 0), & wsa_data))
	{
		return -1 ;
	}
#endif
	return 0 ;
}

int
owsl_system_socket_terminate
(void)
{
#ifdef OS_WINDOWS
	/* free windows socket library */
	if (WSACleanup ())
	{
		return -1 ;
	}
#endif
	return 0 ;
}

OWSLSystemSocket
owsl_system_socket_open
(
	OWSLAddressFamily address_family,
	OWSLSocketMode mode
)
{
	OWSLSystemSocket new_socket ;
	int pf, type ;
	int return_code = 0 ;

	switch (address_family)
	{
		case OWSL_AF_IPV4 :
		{
			pf = PF_INET ;
			break ;
		}
		case OWSL_AF_IPV6 :
		{
			pf = PF_INET6 ;
			break ;
		}
		default :
		{
			return -1 ;
		}
	}
	switch (mode)
	{
		case OWSL_MODE_STREAM :
		{
			type = SOCK_STREAM ;
			break ;
		}
		case OWSL_MODE_DATAGRAM :
		{
			type = SOCK_DGRAM ;
			break ;
		}
		default :
		{
			return -1 ;
		}
	}
	new_socket = socket (pf, type, 0) ;

	/* set socket options */
	if (owsl_system_socket_is_valid (new_socket))
	{
		/* always set REUSE option
		 * to make sure we can bind a socket shortly after having
		 * closed/crashed another one bound to the same address */
		return_code |= owsl_system_socket_reuse_set (new_socket) ;
		if (return_code != 0)
		{
			owsl_system_socket_close (new_socket) ;
			new_socket = OWSL_INVALID_SOCKET ;
		}
	}

	return new_socket ;
}

int
owsl_system_socket_close
(
	OWSLSystemSocket socket
)
{
#ifdef OS_WINDOWS
	return closesocket (socket) ;
#else /* OS_POSIX */
	return close (socket) ;
#endif
}

int
owsl_system_socket_is_valid
(
	OWSLSystemSocket socket
)
{
#ifdef OS_WINDOWS
	return socket != INVALID_SOCKET ;
#else /* OS_POSIX */
	return socket >= 0 ;
#endif
}

int
owsl_system_socket_blocking_mode_set
(
	OWSLSystemSocket socket,
	OWSLBlockingMode mode
)
{
	int flags ;
#ifdef OS_WINDOWS
	switch (mode)
	{
		case OWSL_BLOCKING :
		{
			flags = 0 ;
			break ;
		}
		case OWSL_NON_BLOCKING :
		{
			flags = 1 ;
			break ;
		}
		default :
		{
			return -1 ;
		}
	}
	if (ioctlsocket (socket, FIONBIO, & flags))
	{
		return -1 ;
	}
#else
	flags = fcntl (socket, F_GETFL) ;
	if (flags < 0)
	{
		return -1 ;
	}
	switch (mode)
	{
		case OWSL_BLOCKING :
		{
			flags &= (0xFFFF ^ O_NONBLOCK) ;
			break ;
		}
		case OWSL_NON_BLOCKING :
		{
			flags |= O_NONBLOCK ;
			break ;
		}
		default :
		{
			return -1 ;
		}
	}
	if (fcntl (socket, F_SETFL, flags))
	{
		return -1 ;
	}
#endif
	return 0 ;
}

int
owsl_system_socket_reuse_set
(
	OWSLSystemSocket socket
)
{
#ifdef SO_REUSEPORT
	int option_name = SO_REUSEPORT ;
#else
	int option_name = SO_REUSEADDR ;
#endif
	int option_value = 1 ;
	return OWSL_SETSOCKOPT (socket, SOL_SOCKET, option_name, option_value) ;
}
