/*
 * PhApi, a voice over Internet library
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

#include "nortel.h"
#include "phglobal.h"

#include <eXosip/eXosip.h>
#include <ow_osip.h>
#include <phvline.h>
#include <owpl_adapter.h>

static const char * state_busy = "busy" ;
static const char * state_do_not_disturb = "do not disturb" ;
static const char * state_offline = "offline" ;
static const char * state_online = "online" ;
/*static const char * state_away = "away" ;*/

#ifdef OS_WINDOWS
#define strcasecmp stricmp
#endif

static int
nortelSipRegisterPresenceAdd
(
	osip_message_t * sip_message,
	OWPL_LINE line_id
)
{
	int return_code ;
	phVLine * line ;
	char * previous_status ;
	int status ;
	char * note ;

	line = ph_vlid2vline (line_id) ;
	if (line == NULL)
	{
		return -1 ;
	}

	previous_status = owsip_contact_parameter_get
	(
		sip_message,
		"description"
	) ;
	if (previous_status != NULL)
	{
		return 0 ;
	}

	status = line->publishInfo.onlineState ;
	note = line->publishInfo.szStatus ;

	/* replace status to be compatible with Nortel presence support */
	if (status == 0 && (note == NULL || * note == '\0'))
	{
		note = (char *) state_offline ;
	}
	else if (strcasecmp(note, state_online) == 0)
	{
		note = NULL ;
	}
	else if (strcasecmp(note, state_do_not_disturb) == 0)
	{
		status = 0 ;
		note = (char *) state_busy ;
	}

	return_code = owsip_contact_parameter_add
	(
		sip_message,
		"description",
		status != 0 ? "Connected" : "Unavailable"
	) ;
	if (return_code != 0)
	{
		return -1 ;
	}

	owsip_message_set_modified (sip_message) ;

	if (note != NULL)
	{
		return_code = owsip_contact_parameter_add
		(
			sip_message,
			"note",
			note
		) ;
		if (return_code != 0)
		{
			return -1 ;
		}
	}

	return 0 ;
}

#if 0
static int
nortelSipRequestUriClean
(
	osip_message_t * sip_message
)
{
	osip_uri_t * uri = osip_message_get_uri (sip_message) ;
	if (uri != NULL)
	{
		osip_uri_header_freelist (& uri->url_headers) ;
		osip_uri_param_freelist (& uri->url_params) ;
		owsip_message_set_modified (sip_message) ;
	}
	return 0 ;
}
#endif

static int
nortelSipMessageFilter
(
	osip_message_t * sip_message,
	OWSIPDirection direction,
	OWSIPAccount account,
	osip_transaction_t * transaction,
	void * user_data
)
{
	int return_code = 0 ;
	phVLine * line ;
	
	line = ph_vlid2vline (* (OWPL_LINE *) user_data) ;
	if (line == NULL)
	{
		return -1 ;
	}

	switch (direction)
	{
		case OWSIP_IN :
		{
			break ;
		}
		case OWSIP_OUT :
		{
			if (MSG_IS_REQUEST (sip_message))
			{
				/* skip if not supported */
				if (MSG_IS_PUBLISH (sip_message))
				{
					if (user_data != NULL)
					{
						/* presence should be notified in a register message */
						phvlRegisterNoLock (* (OWPL_LINE *) user_data) ;
					}
					return -1 ;
				}

				/* specific modifications */
				if (MSG_IS_REGISTER (sip_message))
				{
					if (line->LineState == LINESTATE_REGISTERED)
					{
						return_code |= nortelSipRegisterPresenceAdd (sip_message, * (OWPL_LINE *) user_data) ;
					}
				}
				if (MSG_IS_ACK (sip_message))
				{
					/*return_code |= nortelSipRequestUriClean (sip_message) ;*/
				}

				/* common modifications */
				{
					osip_header_t *h = NULL;
					int pos;

					pos = osip_message_header_get_byname(sip_message, "Proxy-Require", 0, &h);
					if (pos == -1)
					{
						return_code |= osip_message_set_header
						(
							sip_message,
							"Proxy-Require",
							"com.nortelnetworks.firewall,com.nortelnetworks.im.encryption"
						) ;
					}
				}
			}
			else if (MSG_IS_RESPONSE (sip_message))
			{
			}
			break ;
		}
	}
	return return_code ;
}

OWPL_RESULT
owplAdapterNortelInitialize
(
	const char * adapter_name
)
{
	return owplAdapterRegister
	(
		adapter_name,
		NULL,
		NULL,
		TRANSPORT_UNKNOWN,
		NULL,
		nortelSipMessageFilter
	) ;
}
