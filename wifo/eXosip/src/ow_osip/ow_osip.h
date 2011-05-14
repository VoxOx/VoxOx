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

/**
 * @file ow_osip.h
 * 
 * This should help to get less differences between
 * the Wengo version and the original one, in order to
 * submit a patch to the project maintainer.
 */

#ifndef OW_OSIP_H
#define OW_OSIP_H

#include "osip_negotiation.h"
#include "../eXosip2.h"

/* This function is internal in osip */
int __osip_transaction_set_state(osip_transaction_t * transaction, state_t state);

/* This function is in osip-3.0.3 */
/* TODO: remove this function when osip will be updated */
int
osip_message_replace_header
(
	osip_message_t * sip,
	const char * hname,
	const char * hvalue
) ;

#define owsip_message_set_header osip_message_replace_header

#define owsip_message_add_header osip_message_set_header

#define owsip_message_set_expires(sip_message,value) \
	owsip_message_set_header (sip_message, "Expires", value)

void owsip_message_set_modified (osip_message_t * sip_message) ;

int owsip_list_get_first_index (const osip_list_t * li, const void *el);
int owsip_list_add_nodup(osip_list_t * li, void *el, int pos);

/**
 * Remove all elements with a value from a list.
 * @param li The element to work on.
 * @param el: the value of the element to remove.
 */
int owsip_list_remove_element(osip_list_t * li, const void *el);

int owsip_sdp_payload_mime_get(const sdp_message_t * sdp, int pos_media, int payloadnumber, char *result, const size_t result_size);

int owsip_dialog_build_replaces_value(const osip_dialog_t * dialog, char **str);

/**
 * Clean the Contact header.
 * @param sip The element to work on.
 * @param hvalue The string describing the element.
 */
int owsip_message_clean_contacts (osip_message_t * sip);

int
owsip_request_uri_parameter_add
(
	osip_message_t * sip_message,
	const char * parameter_name,
	const char * parameter_value
) ;

char *
owsip_contact_parameter_get
(
	const osip_message_t * sip_message,
	const char * parameter_name
) ;

int
owsip_contact_parameter_add
(
	osip_message_t * sip_message,
	const char * parameter_name,
	const char * parameter_value
) ;

int
owsip_contact_uri_parameter_add
(
	osip_message_t * sip_message,
	const char * parameter_name,
	const char * parameter_value
) ;

int
owsip_from_parameter_add
(
	osip_message_t * sip_message,
	const char * parameter_name,
	const char * parameter_value
) ;

const char *
owsip_header_value_get
(
	const osip_message_t * sip_message,
	const char * headerName
) ;

int
owsip_header_remove
(
	osip_message_t * sip_message,
	const char * headerName
) ;

int
owsip_routes_remove
(
	osip_message_t * sip_message
) ;

int
owsip_allow_change
(
	osip_message_t * sip_message,
	const char * allow_value
) ;

sdp_message_t *
owsip_sdp_get_first
(
	const osip_message_t * sip_message
) ;

int
owsip_sdp_remove_first
(
	osip_message_t * sip_message
) ;

int
owsip_sdp_replace_first
(
	osip_message_t * sip_message,
	sdp_message_t * sdp_message
) ;

int
owsip_sdp_replace_first_by_string
(
	osip_message_t * sip_message,
	const char * sdp_message
) ;

int
owsip_sdp_username_set
(
	osip_message_t * sip_message,
	const char * username
) ;

int
owsip_sdp_payload_name_get
(
	int payload,
	const sdp_media_t * media,
	char * payload_name,
	size_t payload_name_size
) ;

int
owsip_sdp_media_codec_attributes_remove
(
	char * payload,
	sdp_media_t * media
) ;

#endif	/* OW_OSIP_H */

