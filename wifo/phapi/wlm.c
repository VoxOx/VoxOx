/*
 * Open Wengo phone library
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

#include "wlm.h"

#include <eXosip/eXosip.h>
#include <ow_osip.h>
#include <owpl_adapter.h>

static int
wlmLineConfigurationHook
(
	OWPL_LINE line,
	void * user_data
)
{
	OWPL_RESULT return_code ;

	return_code = owplLineSetBasicAuthenticationAtFirstMessage (line, 1) ;
	if (return_code != OWPL_RESULT_SUCCESS)
	{
		return -1 ;
	}

	return_code = owplLineSetAutoKeepAlive (line, 0, 0) ;
	if (return_code != OWPL_RESULT_SUCCESS)
	{
		return -1 ;
	}

	return 0 ;
}
/*
static int
wlmSipContactHostChange
(
	osip_message_t * sip_message,
	const char * host
)
{
	int return_code ;
	char * allocated_host ;
	osip_contact_t * contact = osip_list_get (& sip_message->contacts, 0) ;
	if (contact == NULL)
	{
		return 0 ;
	}
	if (contact->url == NULL)
	{
		return -1 ;
	}
	if (contact->url->host != NULL)
	{
		return_code = wlmSipContactUriParameterAdd (sip_message, "maddr", contact->url->host) ;
		if (return_code != 0)
		{
			return -1 ;
		}
		osip_free (contact->url->host) ;
		contact->url->host = NULL ;
	}
	allocated_host = osip_strdup (host) ;
	if (allocated_host == NULL)
	{
		return -1 ;
	}
	contact->url->host = allocated_host ;
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}
*/
static int
wlmSipCallIdHostRemove
(
	osip_message_t * sip_message
)
{
	osip_free (sip_message->call_id->host) ;
	sip_message->call_id->host = NULL ;
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}

static int
wlmSdpPayloadCompatibleCheck
(
	int payload,
	const sdp_media_t * media,
	int * compatible_codec_found
)
{
	int return_code ;
	if
	(
		! * compatible_codec_found
		&&
		(payload == 0 || payload == 8)
	)
	{
		* compatible_codec_found = 1 ;
		return 0 ;
	}
	if (payload > 95 && payload < 128)
	{
		char payload_name [20] ;
		return_code = owsip_sdp_payload_name_get (payload, media, payload_name, sizeof (payload_name)) ;
		if (return_code == 0 && strcmp (payload_name, "telephone-event") == 0)
		{
			return 0 ;
		}
	}
	return 1 ;
}

static int
wlmSdpMediaCodecsReduce
(
	sdp_media_t * media,
	int * compatible_codec_found
)
{
	int return_code ;
	char * payload ;
	int payload_index = 0 ;
	while (! osip_list_eol (& media->m_payloads, payload_index))
	{
		payload = osip_list_get (& media->m_payloads, payload_index) ;
		if (payload == NULL)
		{
			return -1 ;
		}
		return_code = wlmSdpPayloadCompatibleCheck
		(
			atoi (payload),
			media,
			compatible_codec_found
		) ;
		if (return_code < 0)
		{
			return -1 ;
		}
		if (return_code > 0)
		{
			return_code = osip_list_remove (& media->m_payloads, payload_index) ;
			if (return_code < 0)
			{
				return -1 ;
			}
			return_code = owsip_sdp_media_codec_attributes_remove (payload, media) ;
			osip_free (payload) ;
			if (return_code < 0)
			{
				return -1 ;
			}
		}
		else
		{
			payload_index ++ ;
		}
	}
	return 0 ;
}

static int
wlmSdpCodecsReduce
(
	sdp_message_t * sdp_message
)
{
	int return_code ;
	int compatible_codec_found = 0 ;
	sdp_media_t * media ;
	int media_index = 0 ;
	while (! osip_list_eol (& sdp_message->m_medias, media_index))
	{
		media = osip_list_get (& sdp_message->m_medias, media_index) ;
		if (media == NULL)
		{
			return -1 ;
		}
		return_code = wlmSdpMediaCodecsReduce
		(
			media,
			& compatible_codec_found
		) ;
		if (return_code != 0)
		{
			return -1 ;
		}
		if (osip_list_size (& media->m_payloads) > 0)
		{
			media_index ++ ;
		}
		else
		{
			return_code = osip_list_remove (& sdp_message->m_medias, media_index) ;
			if (return_code < 0)
			{
				return -1 ;
			}
			sdp_media_free (media) ;
		}
	}
	return 0 ;
}

static int
wlmSipCodecsReduce
(
	osip_message_t * sip_message
)
{
	int return_code ;
	sdp_message_t * sdp_message = owsip_sdp_get_first (sip_message) ;
	if (sdp_message == NULL)
	{
		return -1 ;
	}
	return_code = wlmSdpCodecsReduce (sdp_message) ;
	if (return_code != 0)
	{
		sdp_message_free (sdp_message) ;
		return -1 ;
	}
	return_code = owsip_sdp_replace_first (sip_message, sdp_message) ;
	if (return_code != 0)
	{
		sdp_message_free (sdp_message) ;
		return -1 ;
	}
	sdp_message_free (sdp_message) ;
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}

static int
wlmSipMessageFilter
(
	osip_message_t * sip_message,
	OWSIPDirection direction,
	OWSIPAccount account,
	osip_transaction_t * transaction,
	void * user_data
)
{
	int return_code = 0 ;
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
					return -1 ;
				}

				/* specific modifications */
				if (MSG_IS_REGISTER (sip_message))
				{
					const char * expire = owsip_header_value_get (sip_message, "Expires") ;
					/* TODO: replace owsip_basic_authentication_set by an osip implementation */
					if (expire != NULL && strcmp (expire, "0") != 0)
					{
						return_code |= owsip_basic_authentication_set (sip_message) ;
					}
					/*return_code |= osip_message_set_header (sip_message, "ms-keep-alive", "UAC;hop-hop=yes") ;*/
					/*return_code |= osip_message_set_header (sip_message, "o", "registration") ;*/
				}
				else if (MSG_IS_INVITE (sip_message))
				{
					return_code |= wlmSipCallIdHostRemove (sip_message) ;
					return_code |= owsip_from_parameter_add (sip_message, "epid", "0123456789") ;
					return_code |= osip_message_set_header (sip_message, "Ms-Conversation-ID", "f=0") ;
					return_code |= wlmSipCodecsReduce (sip_message) ;
					/*return_code |= osip_message_set_header (sip_message, "Supported", "norefersub") ;*/
					/*return_code |= osip_contact_host_change (sip_message, sip_message->from->url->host) ;*/
					/*return_code |= osip_request_uri_parameter_add (sip_message, "proxy", "replace") ;*/
					/*return_code |= osip_header_remove (sip_message, "Expires") ;*/
					/*return_code |= owsip_allow_change (sip_message, "INVITE, ACK, BYE, CANCEL, SUBSCRIBE, NOTIFY, PUBLISH, REFER, MESSAGE, OPTIONS") ;*/
					/*return_code |= owsip_sdp_username_set (sip_message, "-") ;*/
				}

				/* common modifications */
				return_code |= owsip_routes_remove (sip_message) ;
				return_code |= owsip_contact_parameter_add (sip_message, "proxy", "replace") ;
				/*return_code |= owsip_contact_uri_parameter_add (sip_message, "transport", "tls") ;*/
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
owplAdapterWlmInitialize
(
	const char * adapter_name
)
{
	return owplAdapterRegister
	(
		adapter_name,
		NULL,
		"207.46.96.151:443",
		TRANSPORT_TLS,
		wlmLineConfigurationHook,
		wlmSipMessageFilter
	) ;
}
