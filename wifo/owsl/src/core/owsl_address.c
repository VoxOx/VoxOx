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

#include <owlibc.h>
#include "owsl.h"

#ifndef OS_WINDOWS
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include <ctype.h>
#endif /* ! OS_WINDOWS */

#ifdef OS_WINDOWS

/* inet_ntop functions
 * Copyright (C) 1996-2001  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
 * INTERNET SOFTWARE CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* author: Paul Vixie, 1996 */

static const char *
inet_ntop4
(
	const void * _src,
	char * dst,
	size_t size
)
{
	const unsigned char * src = _src ;
	char tmp [OWSL_IPV4_MAX_SIZE] ;

	if ((size_t) sprintf (tmp, "%u.%u.%u.%u", src [0], src [1], src [2], src [3]) + 1 > size)
	{
		OW_SET_ERROR (ENOSPC) ;
		return NULL ;
	}
	strcpy (dst, tmp) ;

	return dst ;
}

static const char *
inet_ntop6
(
	const void * _src,
	char * dst,
	size_t size
)
{
	const unsigned char * src = _src ;
	char tmp [OWSL_IPV6_MAX_SIZE] ;
	char * tp ;
	struct
	{
		int base ;
		int len ;
	} best, cur ;
	unsigned int words [8] ;
	int i ;

	/* copy the input (bytewise) array into a wordwise array */
	memset (words, 0, sizeof (words)) ;
	for (i = 0 ; i < 16 ; i ++)
	{
		words [i / 2] |= (src [i] << ((1 - (i % 2)) << 3)) ;
	}

	/* find the longest run of 0x00's in src for :: shorthanding */
	best.base = -1 ;
	cur.base = -1 ;
	for (i = 0 ; i < 8 ; i ++)
	{
		if (words [i] == 0)
		{
			if (cur.base == -1)
			{
				cur.base = i ;
				cur.len = 1 ;
			}
			else
			{
				cur.len ++ ;
			}
		}
		else
		{
			if (cur.base != -1)
			{
				if (best.base == -1 || cur.len > best.len)
				{
					best = cur ;
				}
				cur.base = -1 ;
			}
		}
	}
	if (cur.base != -1)
	{
		if (best.base == -1 || cur.len > best.len)
		{
			best = cur ;
		}
	}
	if (best.base != -1 && best.len < 2)
	{
		best.base = -1 ;
	}

	/* format the result */
	tp = tmp ;
	for (i = 0 ; i < 8 ; i ++)
	{
		/* are we inside the best run of 0x00's ? */
		if (best.base != -1 && i >= best.base && i < (best.base + best.len))
		{
			if (i == best.base)
			{
				* tp ++ = ':' ;
			}
			continue ;
		}
		/* are we following an initial run of 0x00s or any real hex ? */
		if (i != 0)
		{
			* tp ++ = ':' ;
		}
		/* is this address an encapsulated IPv4 ? */
		if (i == 6 && best.base == 0 && (best.len == 6 || (best.len == 5 && words [5] == 0xffff)))
		{
			if (! inet_ntop4 (src + 12, tp, (int) (sizeof (tmp) - (tp - tmp))))
			{
				return NULL ;
			}
			tp += strlen (tp) ;
			break ;
		}
		tp += sprintf (tp, "%x", words [i]) ;
	}
	/* was it a trailing run of 0x00's ? */
	if (best.base != -1 && (best.base + best.len) == 8)
	{
		* tp ++ = ':' ;
	}
	* tp ++ = '\0' ;

	/* check for overflow */
	if ((size_t) (tp - tmp) > size)
	{
		OW_SET_ERROR (ENOSPC) ;
		return NULL ;
	}

	strcpy (dst, tmp) ;
	return dst ;
}

static const char *
inet_ntop
(
	int af,
	const void * src,
	char * dst,
	size_t size
)
{
	switch (af)
	{
		case AF_INET :
		{
			return (inet_ntop4 (src, dst, size)) ;
		}
		case AF_INET6 :
		{
			return (inet_ntop6 (src, dst, size)) ;
		}
		default :
		{
			OW_SET_ERROR (EAFNOSUPPORT) ;
			return NULL ;
		}
	}
}

#endif /* OS_WINDOWS */

static size_t
owsl_address_separator_count
(
	const char * address,
	char separator
)
{
	char * c = (char *) address ;
	size_t count ;
	for (count = 0 ; * c != 0 ; c ++)
	{
		if (* c == separator)
		{
			count ++ ;
		}
	}
	return count ;
}

static size_t
owsl_address_decdigit_get
(
	char ** address
)
{
	size_t count = 0 ;
	while (isdigit (** address))
	{
		(* address) ++ ;
		count ++ ;
	}
	return count ;
}

static size_t
owsl_address_hexdigit_get
(
	char ** address
)
{
	size_t count = 0 ;
	while (isxdigit (** address))
	{
		(* address) ++ ;
		count ++ ;
	}
	return count ;
}

/**
 * @return a pointer after the IPv4 address
 * @return NULL if it doesn't begin with an IPv4 address
 */
static char *
owsl_address_is_ipv4
(
	const char * address
)
{
	char * c = (char *) address ;
	size_t i, count ;
	if (c == NULL)
	{
		return NULL ;
	}
	for (i = 0 ; i < 4 ; i ++)
	{
		count = owsl_address_decdigit_get (& c) ;
		if (count < 1 || count > 3)
		{
			return NULL ;
		}
		if (i == 3)
		{
			return c ;
		}
		if (* c == '.')
		{
			c ++ ;
		}
		else
		{
			return NULL ;
		}
	}
	return NULL ;
}

/**
 * @return a pointer after the IPv6 address
 * @return NULL if it doesn't begin with an IPv6 address
 */
static char *
owsl_address_is_ipv6
(
	const char * address
)
{
	char * c = (char *) address ;
	size_t digit_count, separator_count = 0 ;
	int has_shortcut = 0 ;
	if (c == NULL)
	{
		return NULL ;
	}
	while (* c != 0)
	{
		digit_count = owsl_address_hexdigit_get (& c) ;
		if (separator_count > 0 && (digit_count < 1 || digit_count > 4))
		{
			if (digit_count == 0 && * c == ':' && ! has_shortcut)
			{
				has_shortcut = 1 ;
			}
			else
			{
				return NULL ;
			}
		}
		if (* c == ':')
		{
			c ++ ;
			separator_count ++ ;
			if (separator_count > 7)
			{
				return NULL ;
			}
		}
		else if (* c != 0)
		{
			if
			(
				(has_shortcut && separator_count > 1 && separator_count < 7)
				||
				(! has_shortcut && separator_count == 6)
			)
			{
				return owsl_address_is_ipv4 (c) ;
			}
			return NULL ;
		}
	}
	if
	(
		(has_shortcut && separator_count > 1)
		||
		(! has_shortcut && separator_count == 7)
	)
	{
		return c ;
	}
	return NULL ;
}

OWSLAddressFamily
owsl_address_family_get_from_ip
(
	const char * ip
)
{
	char * next ;
	next = owsl_address_is_ipv4 (ip) ;
	if (next != NULL && * next == 0)
	{
		return OWSL_AF_IPV4 ;
	}
	else
	{
		next = owsl_address_is_ipv6 (ip) ;
		if (next != NULL && * next == 0)
		{
			return OWSL_AF_IPV6 ;
		}
		else
		{
			return OWSL_AF_UNDEFINED ;
		}
	}
}

static int
owsl_address_port_set
(
	struct sockaddr * address,
	unsigned short port
)
{
	if (address->sa_family == OWSL_AF_IPV4)
	{
		((struct sockaddr_in *) address)->sin_port = htons (port) ;
	}
	else if (address->sa_family == OWSL_AF_IPV6)
	{
		((struct sockaddr_in6 *) address)->sin6_port = htons (port) ;
	}
	else
	{
		return -1 ;
	}
	return 0 ;
}

static struct sockaddr *
owsl_address_dn_or_ip_port_new
(
	OWSLAddressFamily family,
	const char * address,
	int is_ip,
	unsigned short port
)
{
	OWSLAddress * sa = NULL ;
	if (address == NULL)
	{
		if (family == OWSL_AF_UNDEFINED)
		{
			return NULL ;
		}
		sa = malloc (OWSL_ADDRESS_SIZE) ;
		if (sa != NULL)
		{
			memset (sa, 0, OWSL_ADDRESS_SIZE) ;
			sa->ss_family = family ;
		}
	}
	else
	{
		struct addrinfo hints, * res ;
		memset (& hints, 0, sizeof (hints)) ;
		if (is_ip)
		{
			hints.ai_flags = AI_NUMERICHOST ;
		}
		hints.ai_family = family ;
		if (getaddrinfo (address, NULL, & hints, & res) == 0)
		{
			sa = malloc (OWSL_ADDRESS_SIZE) ;
			if (sa != NULL)
			{
				memcpy (sa, res->ai_addr, res->ai_addrlen) ;
				freeaddrinfo (res) ;
			}
		}
	}
	if (sa != NULL)
	{
		if (owsl_address_port_set ((struct sockaddr *) sa, port))
		{
			free (sa) ;
			return NULL ;
		}
	}
	return (struct sockaddr *) sa ;
}

struct sockaddr *
owsl_address_ip_port_new
(
	OWSLAddressFamily family,
	const char * ip,
	unsigned short port
)
{
	return owsl_address_dn_or_ip_port_new (family, ip, 1, port) ;
}

struct sockaddr *
owsl_address_port_new
(
	OWSLAddressFamily family,
	const char * address,
	unsigned short port
)
{
	return owsl_address_dn_or_ip_port_new (family, address, 0, port) ;
}

/**
 * @return a pointer after the address
 * @return NULL if it doesn't begin with a valid address or if malloc fails
 */
static char *
owsl_address_parse_from_string_and_allocate
(
	OWSLAddressFamily family,
	const char * address,
	char ** address_without_port,
	unsigned short * port
)
{
	char * return_value = NULL ;
	char * parsed_address = (char *) address ;
	char * c, * next ;
	char * allocated_address = NULL ;
	size_t address_length ;

	if (parsed_address != NULL)
	{
		/* test syntax [address]:port (port is optional) */
		if (* parsed_address == '[')
		{
			c = strrchr (parsed_address, ']') ;
			if (c != NULL)
			{
				c ++ ;
				if (address_without_port != NULL)
				{
					address_length = c - parsed_address - 2 ;
					allocated_address = malloc (address_length + 1) ;
					if (allocated_address == NULL)
					{
						return NULL ;
					}
					memcpy (allocated_address, parsed_address + 1, address_length) ;
					allocated_address [address_length] = 0 ;
					* address_without_port = allocated_address ;
					parsed_address = allocated_address ;
				}
				if (port != NULL && * c == ':')
				{
					c ++ ;
					if (isdigit (* c))
					{
						* port = (unsigned short) strtol (c, & next, 10) ;
						c = next ;
					}
				}
				return_value = c ;
			}
		}

		/* test syntax address:port (IPv4) */
		if
		(
			(family == OWSL_AF_UNDEFINED || family == OWSL_AF_IPV4)
			&&
			owsl_address_separator_count (parsed_address, ':') == 1
		)
		{
			c = strchr (parsed_address, ':') ;
			if (c != NULL)
			{
				if (address_without_port != NULL)
				{
					if (allocated_address == NULL)
					{
						address_length = c - parsed_address ;
						allocated_address = malloc (address_length + 1) ;
						if (allocated_address == NULL)
						{
							return NULL ;
						}
						memcpy (allocated_address, parsed_address, address_length) ;
						allocated_address [address_length] = 0 ;
						* address_without_port = allocated_address ;
					}
					else
					{
						* c = 0 ;
						* address_without_port = parsed_address ;
					}
				}
				if (port != NULL)
				{
					c ++ ;
					if (isdigit (* c))
					{
						* port = atoi (c) ;
					}
				}
			}
		}
	}

	return 0 ;
}

struct sockaddr *
owsl_address_new
(
	OWSLAddressFamily family,
	const char * address
)
{
	char * allocated_address = NULL ;
	char * address_without_port ;
	unsigned short port = 0 ;
	struct sockaddr * sa = NULL ;

	if (owsl_address_parse_from_string_and_allocate (family, address, & allocated_address, & port))
	{
		return NULL ;
	}
	if (allocated_address != NULL)
	{
		address_without_port = allocated_address ;
	}
	else
	{
		address_without_port = (char *) address ;
	}

	sa = owsl_address_port_new (family, address_without_port, port) ;

	if (allocated_address != NULL)
	{
		free (allocated_address) ;
	}

	return sa ;
}

void
owsl_address_free
(
	struct sockaddr * address
)
{
	if (address != NULL)
	{
		free (address) ;
	}
}

int
owsl_address_parse
(
	const struct sockaddr * address,
	OWSLAddressFamily * family,
	char * ip,
	size_t ip_size,
	unsigned short * port
)
{
	OWSLAddressFamily _family ;

	if (address == NULL)
	{
		return -1 ;
	}

	_family = ((OWSLAddress *) address)->ss_family ;
	if (family != NULL)
	{
		* family = _family ;
	}

	switch (_family)
	{
		case AF_INET :
		{
			if (port != NULL)
			{
				* port = ntohs (((struct sockaddr_in *) address)->sin_port) ;
			}
			if (ip != NULL)
			{
				if (inet_ntop (_family, & ((struct sockaddr_in *) address)->sin_addr, ip, ip_size) == NULL)
				{
					return -1 ;
				}
			}
			break ;
		}
		case AF_INET6 :
		{
			if (port != NULL)
			{
				* port = ntohs (((struct sockaddr_in6 *) address)->sin6_port) ;
			}
			if (ip != NULL)
			{
				if (inet_ntop (_family, & ((struct sockaddr_in6 *) address)->sin6_addr, ip, ip_size) == NULL)
				{
					return -1 ;
				}
			}
			break ;
		}
		default :
		{
			return -1 ;
		}
	}

	return 0 ;
}

int
owsl_address_parse_from_string
(
	const char * address_port,
	OWSLAddressFamily * family,
	char * address,
	size_t address_size,
	unsigned short * port
)
{
	char * allocated_address = NULL ;
	char * address_without_port ;

	if (owsl_address_parse_from_string_and_allocate (OWSL_AF_UNDEFINED, address_port, & allocated_address, port))
	{
		return -1 ;
	}
	if (allocated_address != NULL)
	{
		address_without_port = allocated_address ;
	}
	else
	{
		address_without_port = (char *) address_port ;
	}

	if (family != NULL)
	{
		char * next = owsl_address_is_ipv4 (address_without_port) ;
		if (next != NULL && * next == 0)
		{
			* family = OWSL_AF_IPV4 ;
		}
		else
		{
			next = owsl_address_is_ipv6 (address_without_port) ;
			if (next != NULL && * next == 0)
			{
				* family = OWSL_AF_IPV6 ;
			}
			else
			{
				* family = OWSL_AF_UNDEFINED ;
			}
		}
	}

	if (address != NULL)
	{
		strncpy (address, address_without_port, address_size) ;
	}

	if (allocated_address != NULL)
	{
		free (allocated_address) ;
	}

	return 0 ;
}

int
owsl_address_port_set_from_string
(
	const char * address,
	unsigned short port,
	char * address_port,
	size_t address_port_size
)
{
	if (strchr (address, ':') != NULL)
	{
		/* IPv6 */
		return snprintf (address_port, address_port_size, "[%s]:%u", address, port) ;
	}
	else
	{
		/* IPv4 */
		return snprintf (address_port, address_port_size, "%s:%u", address, port) ;
	}
}

int
owsl_address_ip_port_set
(
	const struct sockaddr * address_struct,
	char * ip_port,
	size_t ip_port_size
)
{
	char ip [OWSL_IP_MAX_SIZE] ;
	unsigned short port = 0 ;
	* ip = 0 ;

	if (owsl_address_parse (address_struct, NULL, ip, sizeof (ip), & port))
	{
		return -1 ;
	}

	if (port == 0)
	{
		strncpy (ip_port, ip, ip_port_size) ;
		return (int) OW_MIN (strlen (ip), ip_port_size) ;
	}
	else
	{
		return owsl_address_port_set_from_string (ip, port, ip_port, ip_port_size) ;
	}
}

int
owsl_address_compare
(
	const struct sockaddr * address1,
	const struct sockaddr * address2
)
{
	int compare_result ;
	unsigned short port1, port2 ;

	if (address1 == NULL || address2 == NULL)
	{
		if (address1 == NULL && address2 == NULL)
		{
			return 0 ;
		}
		else if (address1 == NULL)
		{
			return -1 ;
		}
		else
		{
			return 1 ;
		}
	}

	if (((OWSLAddress *) address1)->ss_family != ((OWSLAddress *) address2)->ss_family)
	{
		return -1 ;
	}

	switch (((OWSLAddress *) address1)->ss_family)
	{
		case AF_INET :
		{
			compare_result = memcmp
			(
				& ((struct sockaddr_in *) address1)->sin_addr,
				& ((struct sockaddr_in *) address2)->sin_addr,
				sizeof (((struct sockaddr_in *) address1)->sin_addr)
			) ;
			if (compare_result == 0)
			{
				port1 = ntohs (((struct sockaddr_in *) address1)->sin_port) ;
				port2 = ntohs (((struct sockaddr_in *) address2)->sin_port) ;
				if (port1 < port2)
				{
					return -1 ;
				}
				else if (port1 == port2)
				{
					return 0 ;
				}
				else
				{
					return 1 ;
				}
			}
			else
			{
				return compare_result ;
			}
			break ;
		}
		case AF_INET6 :
		{
			compare_result = memcmp
			(
				& ((struct sockaddr_in6 *) address1)->sin6_addr,
				& ((struct sockaddr_in6 *) address2)->sin6_addr,
				sizeof (((struct sockaddr_in6 *) address1)->sin6_addr)
			) ;
			if (compare_result == 0)
			{
				port1 = ntohs (((struct sockaddr_in6 *) address1)->sin6_port) ;
				port2 = ntohs (((struct sockaddr_in6 *) address2)->sin6_port) ;
				if (port1 < port2)
				{
					return -1 ;
				}
				else if (port1 == port2)
				{
					return 0 ;
				}
				else
				{
					return 1 ;
				}
			}
			else
			{
				return compare_result ;
			}
			break ;
		}
		default :
		{
			return -1 ;
		}
	}
}

#ifdef WIN32

#include <Windows.h>
#include <Iphlpapi.h>

static int
ppl_dns_get_local_fqdn (char **servername, char **serverip,
			char **netmask, unsigned int WIN32_interface)
{
	unsigned int pos;

	*servername = NULL; /* no name on win32? */
	*serverip   = NULL;
	*netmask    = NULL;

	/* First, try to get the interface where we should listen */
	{
		DWORD size_of_iptable = 0;
		PMIB_IPADDRTABLE ipt;
		PMIB_IFROW ifrow;

		if (GetIpAddrTable(NULL, &size_of_iptable, TRUE) == ERROR_INSUFFICIENT_BUFFER)
		{
			ifrow = (PMIB_IFROW) _alloca (sizeof(MIB_IFROW));
			ipt = (PMIB_IPADDRTABLE) _alloca (size_of_iptable);
			if (ifrow==NULL || ipt==NULL)
			{
				/* not very usefull to continue */
				return -1;
			}

			if (!GetIpAddrTable(ipt, &size_of_iptable, TRUE))
			{
				/* look for the best public interface */

				for (pos=0; pos < ipt->dwNumEntries && *netmask==NULL ; ++pos)
				{
					/* index is */
					struct in_addr addr;
					struct in_addr mask;
					ifrow->dwIndex = ipt->table[pos].dwIndex;
					if (GetIfEntry(ifrow) == NO_ERROR)
					{
						switch(ifrow->dwType)
						{
						case MIB_IF_TYPE_LOOPBACK:
						  /*	break; */
						case MIB_IF_TYPE_ETHERNET:
						default:
							addr.s_addr = ipt->table[pos].dwAddr;
							mask.s_addr = ipt->table[pos].dwMask;
							if (ipt->table[pos].dwIndex == WIN32_interface)
							{
								*servername = NULL; /* no name on win32? */
								*serverip   = strdup(inet_ntoa(addr));
								*netmask    = strdup(inet_ntoa(mask));
								break;
							}
						}
					}
				}
			}
		}
	}

	if (*serverip==NULL || *netmask==NULL)
	{
		return -1;
	}

	return 0;
}

/* TODO: refactor owsl_address_public_ip_get (temporary copied from eXosip) */
OWSL_FUNC_DEF int
owsl_address_public_ip_get
(
	OWSLAddressFamily family,
	char * ip,
	size_t ip_size
)
{
	/* w2000 and W95/98 */
	unsigned long  best_interface_index;
	DWORD hr;

	/* NT4 (sp4 only?) */
	PMIB_IPFORWARDTABLE ipfwdt;
	DWORD siz_ipfwd_table = 0;
	unsigned int ipf_cnt;

	ip[0] = '\0';
	best_interface_index = -1;
	/* w2000 and W95/98 only */
	hr = GetBestInterface(inet_addr("217.12.3.11"),&best_interface_index);
	if (hr)
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR) &lpMsgBuf, 0, NULL);
		best_interface_index = -1;
	}

	if (best_interface_index != -1)
	{ /* probably W2000 or W95/W98 */
		char *servername;
		char *serverip;
		char *netmask;
		if (0 == ppl_dns_get_local_fqdn(&servername, &serverip, &netmask,
						best_interface_index))
		{
			strncpy(ip, serverip, ip_size);
			free(servername);
			free(serverip);
			free(netmask);
			return 0;
		}
		return -1;
	}

	if (!GetIpForwardTable(NULL, &siz_ipfwd_table, FALSE) == ERROR_INSUFFICIENT_BUFFER
		|| !(ipfwdt = (PMIB_IPFORWARDTABLE) alloca (siz_ipfwd_table)))
	{
		return -1;
	}


	/* NT4 (sp4 support only?) */
	if (!GetIpForwardTable(ipfwdt, &siz_ipfwd_table, FALSE))
	{
		for (ipf_cnt = 0; ipf_cnt < ipfwdt->dwNumEntries; ++ipf_cnt) 
		{
			if (ipfwdt->table[ipf_cnt].dwForwardDest == 0)
			{ /* default gateway found */
				char *servername;
				char *serverip;
				char *netmask;
				if (0 ==  ppl_dns_get_local_fqdn(&servername,
								 &serverip,
								 &netmask,
								 ipfwdt->table[ipf_cnt].dwForwardIfIndex))
				{
					strncpy(ip, serverip, ip_size);
					free(servername);
					free(serverip);
					free(netmask);
					return 0;
				}
				return -1;
			}
		}
	}
	/* no default gateway interface found */
	return -1;
}

#else /* sun, *BSD, linux, and other? */


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/param.h>

#include <stdio.h>

static int ppl_dns_default_gateway_ipv4 (char *address, int size);
static int ppl_dns_default_gateway_ipv6 (char *address, int size);

OWSL_FUNC_DEF int
owsl_address_public_ip_get
(
	OWSLAddressFamily family,
	char * ip,
	size_t ip_size
)
{
  if (family==AF_INET6)
    {
      return ppl_dns_default_gateway_ipv6 (ip, ip_size);
    }
  else
    {
      int i = ppl_dns_default_gateway_ipv4 (ip, ip_size); 
      if (i)
	strncpy(ip, "127.0.0.1", ip_size);
      return i;
    }
}

/* This is a portable way to find the default gateway.
 * The ip of the default interface is returned.
 */
static int
ppl_dns_default_gateway_ipv4 (char *address, int size)
{
#ifdef __APPLE_CC__
  int len;
#else
  unsigned int len;
#endif
  int sock_rt, on=1;
  struct sockaddr_in iface_out;
  struct sockaddr_in remote;
  
  memset(&remote, 0, sizeof(struct sockaddr_in));

  remote.sin_family = AF_INET;
  remote.sin_addr.s_addr = inet_addr("217.12.3.11");
  remote.sin_port = htons(11111);
  
  memset(&iface_out, 0, sizeof(iface_out));
  sock_rt = socket(AF_INET, SOCK_DGRAM, 0 );
  
  if (setsockopt(sock_rt, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))
      == -1) {
    perror("DEBUG: [get_output_if] setsockopt(SOL_SOCKET, SO_BROADCAST");
    close(sock_rt);
    return -1;
  }
  
  if (connect(sock_rt, (struct sockaddr*)&remote, sizeof(struct sockaddr_in))
      == -1 ) {
    perror("DEBUG: [get_output_if] connect");
    close(sock_rt);
    return -1;
  }
  
  len = sizeof(iface_out);
  if (getsockname(sock_rt, (struct sockaddr *)&iface_out, &len) == -1 ) {
    perror("DEBUG: [get_output_if] getsockname");
    close(sock_rt);
    return -1;
  }

  close(sock_rt);
  if (iface_out.sin_addr.s_addr == 0)
    { /* what is this case?? */
      return -1;
    }
  strncpy(address, inet_ntoa(iface_out.sin_addr), size);
  return 0;
}


/* This is a portable way to find the default gateway.
 * The ip of the default interface is returned.
 */
static int
ppl_dns_default_gateway_ipv6 (char *address, int size)
{
#ifdef __APPLE_CC__
  int len;
#else
  unsigned int len;
#endif
  int sock_rt, on=1;
  struct sockaddr_in6 iface_out;
  struct sockaddr_in6 remote;
  
  memset(&remote, 0, sizeof(struct sockaddr_in));

  remote.sin6_family = AF_INET6;
  inet_pton(AF_INET6, "2001:638:500:101:2e0:81ff:fe24:37c6",
	    &remote.sin6_addr);
  remote.sin6_port = htons(11111);
  
  memset(&iface_out, 0, sizeof(iface_out));
  sock_rt = socket(AF_INET6, SOCK_DGRAM, 0 );
  
  if (setsockopt(sock_rt, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))
      == -1) {
    perror("DEBUG: [get_output_if] setsockopt(SOL_SOCKET, SO_BROADCAST");
    close(sock_rt);
    return -1;
  }
  
  if (connect(sock_rt, (struct sockaddr*)&remote, sizeof(struct sockaddr_in6))
      == -1 ) {
    perror("DEBUG: [get_output_if] connect");
    close(sock_rt);
    return -1;
  }
  
  len = sizeof(iface_out);
  if (getsockname(sock_rt, (struct sockaddr *)&iface_out, &len) == -1 ) {
    perror("DEBUG: [get_output_if] getsockname");
    close(sock_rt);
    return -1;
  }
  close(sock_rt);

  if (iface_out.sin6_addr.s6_addr == 0)
    { /* what is this case?? */
      return -1;
    }
  inet_ntop(AF_INET6, (const void*) &iface_out.sin6_addr, address, size-1);
  return 0;
}

#endif
