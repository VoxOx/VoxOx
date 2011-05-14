/*
 * Open Wengo SIP stack
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

#include "eXosip2.h"

#include <stdlib.h>
#include <pthread.h>

/* maximum number of accounts opened at the same time */
#define OWSIP_ACCOUNT_MAX 16
/* mapping between an account handle and its info structure */
static OWSIPAccountInfo * owsip_account_array [OWSIP_ACCOUNT_MAX] ;
/* last allocated handle (for allocation increment) */
static int owsip_last_account ;

int
owsip_account_initialize
(void)
{
	memset (owsip_account_array, 0, sizeof (owsip_account_array)) ;
	owsip_last_account = OWSIP_ACCOUNT_MAX ;
	return 0 ;
}

int
owsip_account_terminate
(void)
{
	OWSIPAccount account ;
	int return_code = 0 ;

	for (account = 1 ; account <= OWSIP_ACCOUNT_MAX ; account ++)
	{
		if (owsip_account_info_get (account) != NULL)
		{
			/* return_code |= owsip_account_free (account) ; */
		}
	}

	return return_code ;
}

static OWSIPAccount
owsip_account_handle_get_new
(void)
{
	OWSIPAccount account = owsip_last_account ;
	while (1)
	{
		/* account must be in [1..OWSIP_ACCOUNT_MAX] */
		account = account % OWSIP_ACCOUNT_MAX + 1 ;
		if (owsip_account_array [account - 1] == NULL)
		{
			return account ;  /* a free handle is found */
		}
		if (account == owsip_last_account)
		{
			return -1 ;   /* full */
		}
	}
}

static void
owsip_account_handle_set
(
	OWSIPAccount account,
	OWSIPAccountInfo * account_info
)
{
	owsip_account_array [account - 1] = account_info ;
	owsip_last_account = account ;
	return ;
}

static void
owsip_account_handle_free
(
	OWSIPAccount account
)
{
	owsip_account_array [account - 1] = NULL ;
	return ;
}

static OWSIPAccountInfo *
owsip_account_info_new
(
	OWSIPAccount account,
	const char * name,
	const char * user,
	const char * domain,
	TransportProtocol transport,
	const char * proxy,
	unsigned int port
)
{
	int error = 0 ;
	OWSIPAccountInfo * account_info = malloc (sizeof (OWSIPAccountInfo)) ;
	if (account_info != NULL)
	{
		account_info->account = account ;
		account_info->name = strdup (name) ;
		if (name != NULL && account_info->name == NULL)
		{
			error = 1 ;
			goto end ;
		}
		account_info->user = strdup (user) ;
		if (user != NULL && account_info->user == NULL)
		{
			error = 1 ;
			goto end ;
		}
		account_info->domain = strdup (domain) ;
		account_info->transport = transport ;
		account_info->proxy = strdup (proxy) ;
		if (proxy != NULL && account_info->proxy == NULL)
		{
			error = 1 ;
			goto end ;
		}
		account_info->port = port ;
		account_info->basic_authentication_in_first_message = 0 ;
		account_info->last_received_message_time = 0 ;
		account_info->idle_time_max = 0 ;
		account_info->route_filter = 1 ;
		account_info->message_filter = NULL ;
		account_info->message_filter_user_data = NULL ;
end :
		if (error)
		{
			if (account_info->name != NULL)
			{
				free (account_info->name) ;
			}
			if (account_info->user != NULL)
			{
				free (account_info->user) ;
			}
			if (account_info->proxy != NULL)
			{
				free (account_info->proxy) ;
			}
			return NULL ;
		}
	}

	return account_info ;
}

static int
owsip_account_info_free
(
	OWSIPAccountInfo * account
)
{
	int return_code = 0 ;
	free (account->name) ;
	free (account->user) ;
	free (account->domain) ;
	free (account) ;
	return return_code ;
}

OWSIPAccountInfo *
owsip_account_info_get
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = NULL ;
	if (account > 0 && account <= OWSIP_ACCOUNT_MAX)
	{
		info = owsip_account_array [account - 1] ;
		if (info != NULL && info->account != account)
		{
			info = NULL ;
		}
	}
	return info ;
}

char *
owsip_account_name_get
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return NULL ;
	}
	return info->name ;
}

char *
owsip_account_user_get
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return NULL ;
	}
	return info->user ;
}

char *
owsip_account_domain_get
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return NULL ;
	}
	return info->domain ;
}

int
owsip_account_domain_set
(
	OWSIPAccount account,
	char * domain
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	if (info->domain != NULL)
	{
		free (info->domain) ;
	}
	info->domain = strdup (domain) ;
	if (info->domain == NULL)
	{
		return -1 ;
	}
	return 0 ;
}

TransportProtocol
owsip_account_transport_get
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return TRANSPORT_UNKNOWN ;
	}
	return info->transport ;
}

int
owsip_account_transport_set
(
	OWSIPAccount account,
	TransportProtocol transport
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	info->transport = transport ;
	return 0 ;
}

static const char *
owsip_account_transport_string_get
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return NULL ;
	}
	return transport_protocol_string_get (info->transport) ;
}

char *
owsip_account_proxy_get
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return NULL ;
	}
	return info->proxy ;
}

int
owsip_account_proxy_set
(
	OWSIPAccount account,
	char * proxy
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	if (info->proxy != NULL)
	{
		free (info->proxy) ;
	}
	info->proxy = strdup (proxy) ;
	if (info->proxy == NULL)
	{
		return -1 ;
	}
	return 0 ;
}

unsigned int
owsip_account_port_get
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return 0 ;
	}
	return info->port ;
}

char *
owsip_account_contact_get
(
	OWSIPAccount account,
	char * contact,
	size_t contact_size
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return NULL ;
	}

	if (contact != NULL)
	{
		int return_code ;
		int written = 0 ;
		char ip_port [OWSL_IP_PORT_MAX_SIZE] ;
		* ip_port = 0 ;

		if
		(
			owsl_address_ip_port_set
			(
				transport_listening_address_get_first
				(
					info->transport,
					/* TODO: use info->address_family */
					OWSL_AF_IPV4
				),
				ip_port,
				sizeof (ip_port)
			) < 0
			||
			* ip_port == 0
		)
		{
			return NULL ;
		}

		return_code = snprintf
		(
			contact + written,
			contact_size - written,
			"<sip:"
		) ;
		if (return_code < 0)
		{
			return NULL ;
		}
		written += return_code ;

		if (info->user != NULL)
		{
			return_code = snprintf
			(
				contact + written,
				contact_size - written,
				"%s@",
				info->user
			) ;
			if (return_code < 0)
			{
				return NULL ;
			}
			written += return_code ;
		}

		return_code = snprintf
		(
			contact + written,
			contact_size - written,
			"%s>",
			ip_port
		) ;
		if (return_code < 0)
		{
			return NULL ;
		}
		written += return_code ;
	}

	return contact ;
} 

char *
owsip_account_via_get
(
	OWSIPAccount account,
	char * via,
	size_t via_size
)
{
	if (via != NULL)
	{
		char ip_port [OWSL_IP_PORT_MAX_SIZE] ;
		int written ;

		written = owsl_address_ip_port_set
		(
			transport_listening_address_get_first
			(
				owsip_account_transport_get (account),
				/* TODO: use info->address_family */
				OWSL_AF_IPV4
			),
			ip_port,
			sizeof (ip_port)
		) ;
		if (written < 0)
		{
			return NULL ;
		}

		written = snprintf
		(
			via,
			via_size,
			"SIP/2.0/%s %s;rport;branch=z9hG4bK%u",
			owsip_account_transport_string_get (account),
			ip_port,
			via_branch_new_random ()
		) ;
		if (written < 0)
		{
			return NULL ;
		}
	}

  return via ;
}

int
owsip_account_basic_authentication_in_first_message_set
(
	OWSIPAccount account,
	int basic_authentication_in_first_message
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	info->basic_authentication_in_first_message = basic_authentication_in_first_message ;
	return 0 ;
}

time_t
owsip_account_idle_time_get
(
	OWSIPAccount account
)
{
	time_t now ;
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	if (info->last_received_message_time == 0)
	{
		return 0 ;
	}
	if (time (& now) < 0)
	{
		return -1 ;
	}
	return now - info->last_received_message_time ;
}

int
owsip_account_idle_time_reset
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	info->last_received_message_time = 0 ;
	return 0 ;
}

int
owsip_account_idle_time_refresh
(
	OWSIPAccount account
)
{
	time_t now ;
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	if (time (& now) > 0)
	{
		info->last_received_message_time = now ;
	}
	else
	{
		return -1 ;
	}
	return 0 ;
}

time_t
owsip_account_idle_time_max_get
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	return info->idle_time_max ;
}

int
owsip_account_idle_time_max_set
(
	OWSIPAccount account,
	time_t idle_time_max
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	info->idle_time_max = idle_time_max ;
	return 0 ;
}

int
owsip_account_route_filter_enable
(
	OWSIPAccount account,
	int enable
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	info->route_filter = enable ;
	return 0 ;
}

int
owsip_account_route_filter_is_enabled
(
	OWSIPAccount account
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	
	return info->route_filter ;
}

int
owsip_account_message_filter_set
(
	OWSIPAccount account,
	OWSIPAccountMessageFilter message_filter,
	void * user_data
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	info->message_filter = message_filter ;
	info->message_filter_user_data = user_data ;
	return 0 ;
}

int
owsip_account_message_filter
(
	OWSIPAccount account,
	osip_message_t * message,
	OWSIPDirection direction,
	osip_transaction_t * transaction
)
{
	OWSIPAccountInfo * info = owsip_account_info_get (account) ;
	if (info == NULL)
	{
		return -1 ;
	}
	if (info->message_filter == NULL)
	{
		return 0 ;
	}
	return info->message_filter
	(
		message,
		direction,
		account,
		transaction,
		info->message_filter_user_data
	) ;
}

OWSIPAccount
owsip_account_new
(
	const char * name,
	const char * user,
	const char * domain,
	TransportProtocol transport,
	const char * proxy,
	unsigned int port
)
{
	OWSIPAccount account ;
	OWSIPAccountInfo * account_info ;
	pthread_mutex_t mutex ;

	/* 2 accounts must not be created with the same handle */
	/* begin of critical section */
	if (pthread_mutex_init (& mutex, NULL))
	{
		return -1 ;
	}
	if (pthread_mutex_lock (& mutex))
	{
		pthread_mutex_destroy (& mutex) ;
		return -1 ;
	}

	account = owsip_account_handle_get_new () ;
	if (account < 0)
	{
		pthread_mutex_unlock (& mutex) ;
		pthread_mutex_destroy (& mutex) ;
		return -1 ;
	}

	account_info = owsip_account_info_new (account, name, user, domain, transport, proxy, port) ;
	if (account_info == NULL)
	{
		pthread_mutex_unlock (& mutex) ;
		pthread_mutex_destroy (& mutex) ;
		return -1 ;
	}

	owsip_account_handle_set (account, account_info) ;

	/* end of critical section */
	pthread_mutex_unlock (& mutex) ;
	pthread_mutex_destroy (& mutex) ;

	return account ;
}

int
owsip_account_free
(
	OWSIPAccount account
)
{
	int return_code = 0 ;
	OWSIPAccountInfo * account_info = owsip_account_info_get (account) ;
	if (account_info == NULL)
	{
		return -1 ;
	}

	owsip_account_handle_free (account) ;

	return_code = owsip_account_info_free (account_info) ;

	return return_code ;
}

OWSIPAccount
owsip_account_get
(
	const char * user,
	const char * domain
)
{
	OWSIPAccount account ;
	OWSIPAccountInfo * info ;
	for (account = 1 ; account <= OWSIP_ACCOUNT_MAX ; account ++)
	{
		info = owsip_account_info_get (account) ;
		if
		(
			info != NULL
			&&
			(user == NULL || strcmp (user, info->user) == 0)
			&&
			(domain == NULL || strcmp (domain, info->domain) == 0)
		)
		{
			return account ;
		}
	}
	return 0 ;
}

OWSIPAccount
owsip_account_get_from_uri
(
	const osip_uri_t * uri
)
{
	if (uri == NULL)
	{
		return 0 ;
	}
	return owsip_account_get (uri->username, uri->host) ;
}

OWSIPAccount
owsip_account_get_from_uri_string
(
	const char * uri
)
{
	OWSIPAccount account ;
	osip_from_t * uri_struct ;

	if (uri == NULL)
	{
		return 0 ;
	}

	if (osip_from_init (& uri_struct))
	{
		return 0 ;
	}

	if (osip_from_parse (uri_struct, uri))
	{
		osip_from_free (uri_struct) ;
		return 0 ;
	}

	account = owsip_account_get_from_uri (uri_struct->url) ;

	osip_from_free (uri_struct) ;
	return account ;
}

OWSIPAccount owsip_account_get_from_received_request (const osip_message_t * request)
{
	OWSIPAccount account ;

	account = owsip_account_get_from_uri (request->req_uri) ;
	if (account == 0)
	{
		if (strcmp (request->req_uri->host, request->to->url->host) != 0)
		{
			account = owsip_account_get (request->req_uri->username, request->to->url->host) ;
		}
		if (account == 0)
		{
			account = owsip_account_get (request->req_uri->username, NULL) ;
		}
	}

	return account ;
}

OWSIPAccount owsip_account_get_from_received_response (const osip_message_t * response)
{
	OWSIPAccount account ;

	account = owsip_account_get_from_uri (response->from->url) ;

	return account ;
}

OWSIPAccount owsip_account_get_from_received_message (const osip_message_t * message)
{
	OWSIPAccount account = -1 ;

	if (MSG_IS_REQUEST (message))
	{
		account = owsip_account_get_from_received_request (message) ;
	}
	else if (MSG_IS_RESPONSE (message))
	{
		account = owsip_account_get_from_received_response (message) ;
	}

	return account ;
}
