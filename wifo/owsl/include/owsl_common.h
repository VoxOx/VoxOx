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

#ifndef _OWSL_COMMON_HEADER_
#define _OWSL_COMMON_HEADER_

#include <owcommon.h>

#ifdef OS_WINDOWS
	#if defined BUILD_OWSL_DLL
		#define OWSL_FUNC_DEF __declspec(dllexport)
	#elif ! defined BUILD_OWSL_STATIC
		#define OWSL_FUNC_DEF __declspec(dllimport)
	#endif
#endif
#ifndef OWSL_FUNC_DEF
	#define OWSL_FUNC_DEF
#endif /* OWSL_FUNC_DEF */

#include <sys/types.h>
#include <errno.h>

#ifdef OS_WINDOWS

	#include <winsock2.h>
	#include <ws2tcpip.h>

	/* matching between BSD socket errors and WinSock errors */
	#ifndef EWOULDBLOCK
	#define EWOULDBLOCK       WSAEWOULDBLOCK
	#endif
	#ifndef EINPROGRESS
	#define EINPROGRESS       WSAEINPROGRESS
	#endif
	#ifndef EALREADY
	#define EALREADY          WSAEALREADY
	#endif
	#ifndef ENOTSOCK
	#define ENOTSOCK          WSAENOTSOCK
	#endif
	#ifndef EDESTADDRREQ
	#define EDESTADDRREQ      WSAEDESTADDRREQ
	#endif
	#ifndef EMSGSIZE
	#define EMSGSIZE          WSAEMSGSIZE
	#endif
	#ifndef EPROTOTYPE
	#define EPROTOTYPE        WSAEPROTOTYPE
	#endif
	#ifndef ENOPROTOOPT
	#define ENOPROTOOPT       WSAENOPROTOOPT
	#endif
	#ifndef EPROTONOSUPPORT
	#define EPROTONOSUPPORT   WSAEPROTONOSUPPORT
	#endif
	#ifndef ESOCKTNOSUPPORT
	#define ESOCKTNOSUPPORT   WSAESOCKTNOSUPPORT
	#endif
	#ifndef EOPNOTSUPP
	#define EOPNOTSUPP        WSAEOPNOTSUPP
	#endif
	#ifndef EPFNOSUPPORT
	#define EPFNOSUPPORT      WSAEPFNOSUPPORT
	#endif
	#ifndef EAFNOSUPPORT
	#define EAFNOSUPPORT      WSAEAFNOSUPPORT
	#endif
	#ifndef EADDRINUSE
	#define EADDRINUSE        WSAEADDRINUSE
	#endif
	#ifndef EADDRNOTAVAIL
	#define EADDRNOTAVAIL     WSAEADDRNOTAVAIL
	#endif
	#ifndef ENETDOWN
	#define ENETDOWN          WSAENETDOWN
	#endif
	#ifndef ENETUNREACH
	#define ENETUNREACH       WSAENETUNREACH
	#endif
	#ifndef ENETRESET
	#define ENETRESET         WSAENETRESET
	#endif
	#ifndef ECONNABORTED
	#define ECONNABORTED      WSAECONNABORTED
	#endif
	#ifndef ECONNRESET
	#define ECONNRESET        WSAECONNRESET
	#endif
	#ifndef ENOBUFS
	#define ENOBUFS           WSAENOBUFS
	#endif
	#ifndef EISCONN
	#define EISCONN           WSAEISCONN
	#endif
	#ifndef ENOTCONN
	#define ENOTCONN          WSAENOTCONN
	#endif
	#ifndef ESHUTDOWN
	#define ESHUTDOWN         WSAESHUTDOWN
	#endif
	#ifndef ETOOMANYREFS
	#define ETOOMANYREFS      WSAETOOMANYREFS
	#endif
	#ifndef ETIMEDOUT
	#define ETIMEDOUT         WSAETIMEDOUT
	#endif
	#ifndef ECONNREFUSED
	#define ECONNREFUSED      WSAECONNREFUSED
	#endif
	#ifndef ELOOP
	#define ELOOP             WSAELOOP
	#endif
	#ifndef ENAMETOOLONG
	#define ENAMETOOLONG      WSAENAMETOOLONG
	#endif
	#ifndef EHOSTDOWN
	#define EHOSTDOWN         WSAEHOSTDOWN
	#endif
	#ifndef EHOSTUNREACH
	#define EHOSTUNREACH      WSAEHOSTUNREACH
	#endif
	#ifndef ENOTEMPTY
	#define ENOTEMPTY         WSAENOTEMPTY
	#endif
	#ifndef EPROCLIM
	#define EPROCLIM          WSAEPROCLIM
	#endif
	#ifndef EUSERS
	#define EUSERS            WSAEUSERS
	#endif
	#ifndef EDQUOT
	#define EDQUOT            WSAEDQUOT
	#endif
	#ifndef ESTALE
	#define ESTALE            WSAESTALE
	#endif
	#ifndef EREMOTE
	#define EREMOTE           WSAEREMOTE
	#endif

#else /* OS_POSIX */

	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <netdb.h>

#endif

#endif /* _OWSL_COMMON_HEADER_ */
