/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#include "../eXosip2.h"
#include "ow_osip.h"

#include <ctype.h>

/* This function is in osip-3.0.3 */
/* TODO: remove this function when osip will be updated */
/* Add a or replace exising header  to a SIP message.       */
/* INPUT :  char *hname | pointer to a header name.         */
/* INPUT :  char *hvalue | pointer to a header value.       */
/* OUTPUT: osip_message_t *sip | structure to save results. */
/* returns -1 on error. */
int
osip_message_replace_header (osip_message_t * sip, const char *hname,
			     const char *hvalue)
{
  osip_header_t *h, *oldh;
  int i, oldpos = -1;

  if (hname == NULL)
    return -1;

  oldpos = osip_message_header_get_byname(sip, hname, 0, &oldh);

  i = osip_header_init (&h);
  if (i != 0)
    return -1;

  h->hname = (char *) osip_malloc (strlen (hname) + 1);

  if (h->hname == NULL)
    {
      osip_header_free (h);
      return -1;
    }
  osip_clrncpy (h->hname, hname, strlen (hname));

  if (hvalue != NULL)
    {                           /* some headers can be null ("subject:") */
      h->hvalue = (char *) osip_malloc (strlen (hvalue) + 1);
      if (h->hvalue == NULL)
	{
	  osip_header_free (h);
	  return -1;
	}
      osip_clrncpy (h->hvalue, hvalue, strlen (hvalue));
    } else
      h->hvalue = NULL;

  if (oldpos != -1)
    {
      osip_list_remove(&sip->headers, oldpos);
      osip_header_free(oldh);
    }

  sip->message_property = 2;
  osip_list_add (&sip->headers, h, -1);
  return 0;                     /* ok */
}

void owsip_message_set_modified (osip_message_t * sip_message)
{
	if (sip_message != NULL)
	{
		sip_message->message_property = 2 ;
	}
}

int owsip_list_get_first_index (const osip_list_t * li, const void *el)
{
	__node_t *ntmp;
	int i = 0;
	
	if (!li) {
		return -1;
	}
	ntmp = li->node;
	while ((i < li->nb_elt) && ntmp) {
		if (ntmp->element == el) {
			return i;
		}
		i++;
		ntmp = (__node_t *) ntmp->next;
	}
	return -1;
}

int owsip_list_add_nodup(osip_list_t * li, void *el, int pos)
{
	if (owsip_list_get_first_index(li,el) >= 0)
		return -1;
	return osip_list_add(li,el,pos);
}

/* return -1 if failed */
int
owsip_list_remove_element (osip_list_t * li, const void *el)
{
	__node_t *ntmp;
	int i = 0;
	ntmp = li->node;
	while ((i < li->nb_elt) && ntmp)
	{
		if (ntmp->element == el)
		{
			ntmp = (__node_t *) ntmp->next;
			osip_list_remove(li,i);
			continue;
		}
		i++;
		ntmp = (__node_t *) ntmp->next;
	}
	return li->nb_elt;
}

/**
 * Get the mime payload string of a media in a SDP message
 * that corresponds to the specified payload number
 * 
 * @param sdp			The sdp message.
 * @param pos_media		The index of media in the SDP message
 * @param payloadnumber	The payload number
 * @param result(out)	The output buffer to contain result
 * @param result		The output buffer size 
 * @return				0 on success. -1 on failure
 */
int owsip_sdp_payload_mime_get(const sdp_message_t * sdp, int pos_media, int payloadnumber, char *result, const size_t result_size)
{
	sdp_media_t *med = osip_list_get (&sdp->m_medias, pos_media);
	sdp_attribute_t *attr;
	int i;
	
	if (med == NULL)
		return -1;
	
	for( i = 0; ; i++)
    {
		attr = sdp_message_attribute_get ((sdp_message_t *)sdp, pos_media, i);
		
		if (!attr) 
			return -1;
		
		if (!strcmp(attr->a_att_field, "rtpmap") && (atoi(attr->a_att_value) == payloadnumber))
		{
			char *p = attr->a_att_value;
			
			p = strchr(p, ' ');
			if (!p)
				return -1;
			
			p += strspn(p, " ");
			
			if (*p) {
				if (strlen(p) >= result_size) {
						return -1;
				}
				strcpy(result, p);
				return 0;
			}
												
			return -1;
		}
    }
	return -1;
}

int owsip_dialog_build_replaces_value(const osip_dialog_t * dialog, char **str)
{
	char *to, *from;
	char* val;
	
	if (dialog->type == CALLER)
    {
		to = dialog->remote_tag;
		from = dialog->local_tag;
    }
	else
    {
		to = dialog->local_tag;
		from = dialog->remote_tag;
    }
	
	val = osip_malloc(256);
	snprintf(val, 255, "%s;to-tag=%s;from-tag=%s", dialog->call_id, to, from);
	*str = val;
	return 0;
}

/* removes all conntacts from contact list */
/* INPUT : osip_message_t *sip | sip message.                       */
/* OUTPUT: osip_contact_t *contact | structure to save results. */
/* returns -1 on error. */
int owsip_message_clean_contacts(osip_message_t * sip)
{
	osip_contact_t *ct;
	
	if (sip == NULL)
		return -1;
	
	while(0 != (ct = (osip_contact_t *)osip_list_get (&sip->contacts, 0)))
    {
		osip_list_remove(&sip->contacts, 0);
		owsip_message_set_modified(sip);
		osip_contact_free(ct);
    }
	
	return 0;
}

int
owsip_request_uri_parameter_add
(
	osip_message_t * sip_message,
	const char * parameter_name,
	const char * parameter_value
)
{
	int return_code ;
	char * allocated_name, * allocated_value ;
	osip_uri_t * uri = osip_message_get_uri (sip_message) ;
	if (uri == NULL)
	{
		return 0 ;
	}
	allocated_name = osip_strdup (parameter_name) ;
	if (allocated_name == NULL)
	{
		return -1 ;
	}
	allocated_value = osip_strdup (parameter_value) ;
	if (allocated_value == NULL)
	{
		osip_free (allocated_name) ;
		return -1 ;
	}
	return_code = osip_uri_param_add (& uri->url_params, allocated_name, allocated_value) ;
	if (return_code != 0)
	{
		osip_free (allocated_name) ;
		osip_free (allocated_value) ;
		return -1 ;
	}
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}

char *
owsip_contact_parameter_get
(
	const osip_message_t * sip_message,
	const char * parameter_name
)
{
	int return_code ;
	osip_uri_param_t * parameter ;
	osip_contact_t * contact = osip_list_get (& sip_message->contacts, 0) ;
	if (contact == NULL)
	{
		return NULL ;
	}
	return_code = osip_contact_param_get_byname (contact, (char *) parameter_name, & parameter) ;
	if (return_code != 0)
	{
		return NULL ;
	}
	return parameter->gvalue ;
}

int
owsip_contact_parameter_add
(
	osip_message_t * sip_message,
	const char * parameter_name,
	const char * parameter_value
)
{
	int return_code ;
	char * allocated_name, * allocated_value ;
	osip_contact_t * contact = osip_list_get (& sip_message->contacts, 0) ;
	if (contact == NULL)
	{
		return 0 ;
	}
	allocated_name = osip_strdup (parameter_name) ;
	if (allocated_name == NULL)
	{
		return -1 ;
	}
	allocated_value = osip_strdup (parameter_value) ;
	if (allocated_value == NULL)
	{
		osip_free (allocated_name) ;
		return -1 ;
	}
	return_code = osip_contact_param_add (contact, allocated_name, allocated_value) ;
	if (return_code != 0)
	{
		osip_free (allocated_name) ;
		osip_free (allocated_value) ;
		return -1 ;
	}
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}

int
owsip_contact_uri_parameter_add
(
	osip_message_t * sip_message,
	const char * parameter_name,
	const char * parameter_value
)
{
	int return_code ;
	char * allocated_name, * allocated_value ;
	osip_contact_t * contact = osip_list_get (& sip_message->contacts, 0) ;
	if (contact == NULL)
	{
		return 0 ;
	}
	allocated_name = osip_strdup (parameter_name) ;
	if (allocated_name == NULL)
	{
		return -1 ;
	}
	allocated_value = osip_strdup (parameter_value) ;
	if (allocated_value == NULL)
	{
		osip_free (allocated_name) ;
		return -1 ;
	}
	return_code = osip_uri_param_add (& contact->url->url_params, allocated_name, allocated_value) ;
	if (return_code != 0)
	{
		osip_free (allocated_name) ;
		osip_free (allocated_value) ;
		return -1 ;
	}
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}

int
owsip_from_parameter_add
(
	osip_message_t * sip_message,
	const char * parameter_name,
	const char * parameter_value
)
{
	int return_code ;
	char * allocated_name, * allocated_value ;
	allocated_name = osip_strdup (parameter_name) ;
	if (allocated_name == NULL)
	{
		return -1 ;
	}
	allocated_value = osip_strdup (parameter_value) ;
	if (allocated_value == NULL)
	{
		osip_free (allocated_name) ;
		return -1 ;
	}
	return_code = osip_from_param_add (sip_message->from, allocated_name, allocated_value) ;
	if (return_code != 0)
	{
		osip_free (allocated_name) ;
		osip_free (allocated_value) ;
		return -1 ;
	}
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}

const char *
owsip_header_value_get
(
	const osip_message_t * sip_message,
	const char * headerName
)
{
	int index ;
	osip_header_t * header ;
	index = osip_message_header_get_byname (sip_message, headerName, 0, & header) ;
	if (index < 0)
	{
		return NULL ;
	}
	return header->hvalue ;
}

int
owsip_header_remove
(
	osip_message_t * sip_message,
	const char * headerName
)
{
	int return_code ;
	int index ;
	osip_header_t * header ;
	index = osip_message_header_get_byname (sip_message, headerName, 0, & header) ;
	if (index < 0)
	{
		return -1 ;
	}
	return_code = osip_list_remove (& sip_message->headers, index) ;
	if (return_code < 0)
	{
		return -1 ;
	}
	osip_header_free (header) ;
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}

int
owsip_routes_remove
(
	osip_message_t * sip_message
)
{
	int return_code ;
	osip_route_t * route ;
	while (! osip_list_eol (& sip_message->routes, 0))
	{
		route = osip_list_get (& sip_message->routes, 0) ;
		if (route == NULL)
		{
			return -1 ;
		}
		return_code = osip_list_remove (& sip_message->routes, 0) ;
		if (return_code < 0)
		{
			return -1 ;
		}
		osip_route_free (route) ;
		owsip_message_set_modified (sip_message) ;
	}
	return 0 ;
}

int
owsip_allow_change
(
	osip_message_t * sip_message,
	const char * allow_value
)
{
	int return_code ;
	osip_allow_t * allow ;
	while (! osip_list_eol (& sip_message->allows, 0))
	{
		allow = osip_list_get (& sip_message->allows, 0) ;
		if (allow == NULL)
		{
			return -1 ;
		}
		return_code = osip_list_remove (& sip_message->allows, 0) ;
		if (return_code < 0)
		{
			return -1 ;
		}
		osip_allow_free (allow) ;
		owsip_message_set_modified (sip_message) ;
	}
	return_code = osip_message_set_allow (sip_message, allow_value) ;
	if (return_code != 0)
	{
		return -1 ;
	}
	return 0 ;
}

sdp_message_t *
owsip_sdp_get_first
(
	const osip_message_t * sip_message
)
{
	int return_code ;
	osip_body_t * body ;
	sdp_message_t * sdp_message = NULL ;
	int index = 0 ;
	while (sdp_message == NULL)
	{
		body = osip_list_get (& sip_message->bodies, index) ;
		if (body == NULL)
		{
			return NULL ;
		}
		return_code = sdp_message_init (& sdp_message) ;
		if (return_code != 0)
		{
			return NULL ;
		}
		return_code = sdp_message_parse (sdp_message, body->body) ;
		if (return_code != 0)
		{
			sdp_message_free (sdp_message) ;
			sdp_message = NULL ;
			index ++ ;
		}
	}
	return sdp_message ;
}

int
owsip_sdp_remove_first
(
	osip_message_t * sip_message
)
{
	int return_code ;
	osip_body_t * body ;
	sdp_message_t * sdp_message = NULL ;
	int index = 0 ;
	while (sdp_message == NULL)
	{
		body = osip_list_get (& sip_message->bodies, index) ;
		if (body == NULL)
		{
			return -1 ;
		}
		return_code = sdp_message_init (& sdp_message) ;
		if (return_code != 0)
		{
			return -1 ;
		}
		return_code = sdp_message_parse (sdp_message, body->body) ;
		if (return_code != 0)
		{
			sdp_message_free (sdp_message) ;
			sdp_message = NULL ;
			index ++ ;
		}
	}
	return_code = osip_list_remove (& sip_message->bodies, index) ;
	if (return_code < 0)
	{
		sdp_message_free (sdp_message) ;
		return -1 ;
	}
	osip_body_free (body) ;
	sdp_message_free (sdp_message) ;
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}

int
owsip_sdp_replace_first
(
	osip_message_t * sip_message,
	sdp_message_t * sdp_message
)
{
	int return_code ;
	char * sdp_message_string = NULL ;
	return_code = sdp_message_to_str (sdp_message, & sdp_message_string) ;
	if (return_code != 0)
	{
		return -1 ;
	}
	return_code = osip_message_set_body (sip_message, sdp_message_string, strlen (sdp_message_string)) ;
	if (return_code != 0)
	{
		osip_free (sdp_message_string) ;
		return -1 ;
	}
	return_code = owsip_sdp_remove_first (sip_message) ;
	if (return_code != 0)
	{
		osip_body_t * body = osip_list_get (& sip_message->bodies, sip_message->bodies.nb_elt - 1) ;
		if (body != NULL)
		{
			return_code = osip_list_remove (& sip_message->bodies, sip_message->bodies.nb_elt - 1) ;
			if (return_code >= 0)
			{
				osip_body_free (body) ;
			}
		}
		return -1 ;
	}
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}

int
owsip_sdp_replace_first_by_string
(
	osip_message_t * sip_message,
	const char * sdp_message
)
{
	int return_code ;
	return_code = osip_message_set_body (sip_message, sdp_message, strlen (sdp_message)) ;
	if (return_code != 0)
	{
		return -1 ;
	}
	return_code = owsip_sdp_remove_first (sip_message) ;
	if (return_code != 0)
	{
		osip_body_t * body = osip_list_get (& sip_message->bodies, sip_message->bodies.nb_elt - 1) ;
		if (body != NULL)
		{
			return_code = osip_list_remove (& sip_message->bodies, sip_message->bodies.nb_elt - 1) ;
			if (return_code >= 0)
			{
				osip_body_free (body) ;
			}
		}
		return -1 ;
	}
	owsip_message_set_modified (sip_message) ;
	return 0 ;
}

int
owsip_sdp_username_set
(
	osip_message_t * sip_message,
	const char * username
)
{
	int return_code ;
	sdp_message_t * sdp_message = owsip_sdp_get_first (sip_message) ;
	if (sdp_message == NULL)
	{
		return -1 ;
	}
	if (sdp_message->o_username != NULL)
	{
		osip_free (sdp_message->o_username) ;
	}
	sdp_message->o_username = osip_strdup (username) ;
	if (sdp_message->o_username == NULL)
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

int
owsip_sdp_payload_name_get
(
	int payload,
	const sdp_media_t * media,
	char * payload_name,
	size_t payload_name_size
)
{
	sdp_attribute_t * attribute ;
	int attribute_index = 0 ;
	char * attribute_value ;
	char payload_string [4] ;
	int payload_length ;
	size_t payload_name_length ;
	if (media == NULL || payload_name == NULL || payload_name_size <= 1)
	{
		return -1 ;
	}
	payload_length = snprintf (payload_string, sizeof (payload_string), "%d", payload) ;
	if (payload_length < 1)
	{
		return -1 ;
	}
	while (! osip_list_eol (& media->a_attributes, attribute_index))
	{
		attribute = osip_list_get (& media->a_attributes, attribute_index) ;
		if (attribute == NULL)
		{
			return -1 ;
		}
		if (strcmp (attribute->a_att_field, "rtpmap") == 0)
		{
			attribute_value = attribute->a_att_value ;
			if (attribute_value != NULL)
			{
				if (strncmp (attribute_value, payload_string, payload_length) == 0)
				{
					attribute_value += payload_length ;
					while (* attribute_value != 0 && isspace (* attribute_value))
					{
						attribute_value ++ ;
					}
					for (payload_name_length = 0 ; * attribute_value != 0 ; attribute_value ++)
					{
						if (* attribute_value == '/')
						{
							break ;
						}
						if (payload_name_length + 1 >= payload_name_size)
						{
							return -1 ;
						}
						payload_name [payload_name_length] = * attribute_value ;
						payload_name_length ++ ;
					}
					payload_name [payload_name_length] = 0 ;
					return 0 ;
				}
			}
		}
		attribute_index ++ ;
	}
	return -1 ;
}

int
owsip_sdp_media_codec_attributes_remove
(
	char * payload,
	sdp_media_t * media
)
{
	int return_code ;
	size_t payload_length = strlen (payload) ;
	int matching_attribute_found ;
	sdp_attribute_t * attribute ;
	char * attribute_value ;
	int attribute_index = 0 ;
	while (! osip_list_eol (& media->a_attributes, attribute_index))
	{
		matching_attribute_found = 0 ;
		attribute = osip_list_get (& media->a_attributes, attribute_index) ;
		if (attribute == NULL)
		{
			return -1 ;
		}
		attribute_value = attribute->a_att_value ;
		if (attribute_value != NULL)
		{
			if (strncmp (attribute_value, payload, payload_length) == 0)
			{
				attribute_value += payload_length ;
				if (* attribute_value == 0 || isspace (* attribute_value))
				{
					matching_attribute_found = 1 ;
				}
			}
		}
		if (matching_attribute_found)
		{
			return_code = osip_list_remove (& media->a_attributes, attribute_index) ;
			if (return_code < 0)
			{
				return -1 ;
			}
			sdp_attribute_free (attribute) ;
		}
		else
		{
			attribute_index ++ ;
		}
	}
	return 0 ;
}
