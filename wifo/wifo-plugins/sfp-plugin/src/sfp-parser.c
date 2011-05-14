/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#include <sfp-plugin/sfp-commons.h>
#include <sfp-plugin/sfp-plugin.h>
#include <sfp-plugin/sfp-parser.h>

#include <phapi-util/mystdio.h>
#include <phapi-util/mystring.h>

#include <phapi-util/util.h>

#include <stdlib.h>



// ----- PUBLIC FUNCTIONS DEFINITION -----

/**
* Creates and initializes a new sfp_info_t with protocol version,
* ip protocol, required bandwidth, and packet size initialized.
*
* @return	a new allocated spf info struct pointer
*/
sfp_info_t * sfp_create_sfp_info(){
	sfp_info_t * info;

	if((info = (sfp_info_t *)malloc(sizeof(sfp_info_t))) == NULL){
		m_log_error("Not enough memory", "sfp_create_sfp_info");
		return NULL;
	}
	memset(info, 0, sizeof(sfp_info_t));

	sfp_add_version_info(info, SFP_PROTOCOL_VERSION);
	sfp_add_transfer_info(info, SFP_IP_PROTOCOL_TCP, SFP_REQUIRED_BANDWIDTH_DEFAULT, SFP_PACKET_SIZE_DEFAULT);

	return info;
}

/**
* Creates and initializes a new sfp_info_t
*
* @return	a new allocated spf info struct pointer
*/
sfp_info_t * sfp_create_empty_sfp_info(){
	sfp_info_t * info;

	if(!(info = (sfp_info_t *)malloc(sizeof(sfp_info_t)))){
		m_log_error("Not enough memory", "sfp_create_empty_sfp_info");
		return NULL;
	}

	memset(info, 0, sizeof(sfp_info_t));
	return info;
}

/**
* Frees all the memory used by a sfp_info_t
*
* Example
* <pre>
*	sfp_info_t * info;
*	sfp_free_sfp_info(&info);
* </pre>
*
* @param	[in-out]	info : a pointer to an sfp info struct pointer
*/
void sfp_free_sfp_info(sfp_info_t ** info){

	if(!info || !*info) return;

	free(*info);
	*info = NULL;
}

/**
* Parses an sfp body. If the protocol version is different from the one of the
* parser, the body isn't parsed
* 
* @param	[in] text : the text of the body to parse
* @return	a pointer to an sfp info struct concerning the file sending; NULL if parsing failed
*/
sfp_info_t * sfp_parse_message(char * text){
	sfp_info_t * info;

	info = sfp_create_sfp_info();

	if(text != NULL){
		while(*text != '\0'){
			while(*text == '\n'){
				text++;
			}
			switch(*text){
				case SFP_VERSION_LINE :
					//msscanf(&text, SFP_VERSION_LINE_FORMAT, &(info->protocol_version));
					sscanf3(&text, SFP_VERSION_LINE_FORMAT, info->protocol_version, sizeof(info->protocol_version));
					// if protocol version differs, don't try to parse the rest of the message
					if(strcmp(info->protocol_version, SFP_PROTOCOL_VERSION) < 0){
						if(peerNeedUpgrade) { peerNeedUpgrade(); }
						return NULL;
					}else if(strcmp(info->protocol_version, SFP_PROTOCOL_VERSION) > 0){
						if(needUpgrade) { needUpgrade(); }
						return NULL;
					}
					break;
				case SFP_ORIGIN_LINE :
					//msscanf(&text, SFP_ORIGIN_LINE_FORMAT, &(info->file_sending_id), &(info->username), &(info->network_type), &(info->ip_address_type), &(info->ip_address), &(info->address_port));
					sscanf3(&text, SFP_ORIGIN_LINE_FORMAT, info->file_sending_id, sizeof(info->file_sending_id), info->username, sizeof(info->username), info->network_type, sizeof(info->network_type), info->ip_address_type, sizeof(info->ip_address_type), info->ip_address, sizeof(info->ip_address), info->address_port, sizeof(info->address_port));
					break;
				case SFP_MODE_INFO_LINE :
					//msscanf(&text, SFP_MODE_INFO_LINE_FORMAT, &(info->mode));
					sscanf3(&text, SFP_MODE_INFO_LINE_FORMAT, info->mode, sizeof(info->mode));
					break;
				case SFP_TRANSFER_INFO_LINE :
					//msscanf(&text, SFP_TRANSFER_INFO_LINE_FORMAT, &(info->ip_protocol), &(info->required_bandwidth), &(info->packet_size));
					sscanf3(&text, SFP_TRANSFER_INFO_LINE_FORMAT, info->ip_protocol, sizeof(info->ip_protocol), info->required_bandwidth, sizeof(info->required_bandwidth), info->packet_size, sizeof(info->packet_size));
					break;
				case SFP_URI_LINE :
					//msscanf(&text, SFP_URI_LINE_FORMAT, &(info->uri));
					sscanf3(&text, SFP_URI_LINE_FORMAT, info->uri, sizeof(info->uri));
					break;
				case SFP_CRYPTED_KEY_LINE :
					//msscanf(&text, SFP_CRYPTED_KEY_LINE_FORMAT, &(info->key_info), &(info->crypted_key));
					sscanf3(&text, SFP_CRYPTED_KEY_LINE_FORMAT, info->key_info, sizeof(info->key_info), info->crypted_key, sizeof(info->crypted_key));
					break;
				case SFP_FILE_INFO_LINE :
					//msscanf(&text, SFP_FILE_INFO_LINE_FORMAT, &(info->filename), &(info->file_type), &(info->file_size));
					sscanf3(&text, SFP_FILE_INFO_LINE_FORMAT, info->filename, sizeof(info->filename), info->file_type, sizeof(info->file_type), info->file_size, sizeof(info->file_size));
					break;
				case SFP_CONNECTION_ID_LINE :
					//msscanf(&text, SFP_CONNECTION_ID_LINE_FORMAT, &(info->connection_id));
					sscanf3(&text, SFP_CONNECTION_ID_LINE_FORMAT, info->connection_id, sizeof(info->connection_id));
					break;
				default :
					// go to the next readable line
					while(*text != '\n'){
						text++;
					}
					text++;
					break;
			}
		}
	}

	return info;
}

/**
* Turns the sfp_info_t into a sfp text message
*
* @param	[in]	info : an sfp info struct pointer
* @return	a string containing the corresponding sfp body message
*/
char * sfp_make_message_body_from_sfp_info(sfp_info_t * info){
	char * body = NULL;
	unsigned int body_length = 0;

	char * version_line = NULL;
	char * origin_line = NULL;
	char * mode_line = NULL;
	char * transfer_line = NULL;
	char * uri_line = NULL;
	char * key_line = NULL;
	char * file_line = NULL;
	char * connection_id_line = NULL;
	unsigned int length = 0;

	// VERSION LINE
	if(strfilled(info->protocol_version)){
		length = 0;
		length += (int)strlen(info->protocol_version);
		length += format_length_without_tokens(SFP_VERSION_LINE_FORMAT);
		version_line = (char *)malloc((length+1) * sizeof(char));
		sprintf(version_line, SFP_VERSION_LINE_FORMAT, info->protocol_version);

		body_length += length;
	}

	// ORIGIN LINE
	if(strfilled(info->file_sending_id) &&
		strfilled(info->username) &&
		strfilled(info->network_type) &&
		strfilled(info->ip_address_type) &&
		strfilled(info->ip_address) &&
		strfilled(info->address_port)){

		length = 0;
		length += (int)strlen(info->file_sending_id);
		length += (int)strlen(info->username);
		length += (int)strlen(info->network_type);
		length += (int)strlen(info->ip_address_type);
		length += (int)strlen(info->ip_address);
		length += (int)strlen(info->address_port);
		length += format_length_without_tokens(SFP_ORIGIN_LINE_FORMAT);
		origin_line = (char *)malloc((length+1) * sizeof(char));
		sprintf(origin_line, SFP_ORIGIN_LINE_FORMAT, info->file_sending_id, info->username, info->network_type, info->ip_address_type, info->ip_address, info->address_port);

		body_length += length;
	}

	// MODE LINE
	if(strfilled(info->mode)){

		length = 0;
		length += (int)strlen(info->mode);
		length += format_length_without_tokens(SFP_MODE_INFO_LINE_FORMAT);
		mode_line = (char *)malloc((length+1) * sizeof(char));
		sprintf(mode_line, SFP_MODE_INFO_LINE_FORMAT, info->mode);

		body_length += length;
	}

	// TRANSFER LINE
	if(strfilled(info->ip_protocol) &&
		strfilled(info->required_bandwidth) &&
		strfilled(info->packet_size)){

		length = 0;
		length += (int)strlen(info->ip_protocol);
		length += (int)strlen(info->required_bandwidth);
		length += (int)strlen(info->packet_size);
		length += format_length_without_tokens(SFP_TRANSFER_INFO_LINE_FORMAT);
		transfer_line = (char *)malloc((length+1) * sizeof(char));
		sprintf(transfer_line, SFP_TRANSFER_INFO_LINE_FORMAT, info->ip_protocol, info->required_bandwidth, info->packet_size);

		body_length += length;
	}

	// URI LINE
	if(strfilled(info->uri)){
		length = 0;
		length += (int)strlen(info->uri);
		length += format_length_without_tokens(SFP_URI_LINE_FORMAT);
		uri_line = (char *)malloc((length+1) * sizeof(char));
		sprintf(uri_line, SFP_URI_LINE_FORMAT, info->uri);

		body_length += length;
	}

	// KEY LINE
	if(strfilled(info->key_info) &&
		strfilled(info->crypted_key)){

		length = 0;
		length += (int)strlen(info->key_info);
		length += (int)strlen(info->crypted_key);
		length += format_length_without_tokens(SFP_CRYPTED_KEY_LINE_FORMAT);
		key_line = (char *)malloc((length+1) * sizeof(char));
		sprintf(key_line, SFP_CRYPTED_KEY_LINE_FORMAT, info->key_info, info->crypted_key);

		body_length += length;
	}

	// FILE LINE
	if(strfilled(info->filename) &&
		strfilled(info->file_type) &&
		strfilled(info->file_size)){

		length = 0;
		length += (int)strlen(info->filename);
		length += (int)strlen(info->file_type);
		length += (int)strlen(info->file_size);
		length += format_length_without_tokens(SFP_FILE_INFO_LINE_FORMAT);
		file_line = (char *)malloc((length+1) * sizeof(char));
		sprintf(file_line, SFP_FILE_INFO_LINE_FORMAT, info->filename, info->file_type, info->file_size);

		body_length += length;
	}

	/* JULIEN */
	// CONNECTION ID LINE
	if(strfilled(info->connection_id)){

		length = 0;
		length += (int) strlen(info->connection_id);
		length += format_length_without_tokens(SFP_CONNECTION_ID_LINE_FORMAT);
		connection_id_line = (char *)malloc((length+1) * sizeof(char));
		sprintf(connection_id_line, SFP_CONNECTION_ID_LINE_FORMAT, info->connection_id);

		body_length += length;
	}

	if(body_length > 0){
		body_length = (body_length+1) * sizeof(char);
		body = (char *)malloc(body_length);
		body[0] = '\0';
		if(strfilled(version_line)){
			strcat(body, version_line);
		}
		if(strfilled(origin_line)){
			strcat(body, origin_line);
		}
		if(strfilled(mode_line)){
			strcat(body, mode_line);
		}
		if(strfilled(transfer_line)){
			strcat(body, transfer_line);
		}
		if(strfilled(uri_line)){
			strcat(body, uri_line);
		}
		if(strfilled(key_line)){
			strcat(body, key_line);
		}
		if(strfilled(file_line)){
			strcat(body, file_line);
		}
		/* JULIEN */
		if(strfilled(connection_id_line)){
			strcat(body, connection_id_line);
		}
	}

	free(version_line);
	free(origin_line);
	free(mode_line);
	free(transfer_line);
	free(uri_line);
	free(key_line);
	free(file_line);
	free(connection_id_line);

	return body;
}

/**
* Adds the info concerning versionning to an sfp info
*
* @param	[in-out]	info : an sfp info
* @param	[in]	protocol_version : the version of the sfp protocol
*/
void sfp_add_version_info(sfp_info_t * info, char * protocol_version){
	strncpy(info->protocol_version, protocol_version, sizeof(info->protocol_version));
}

/**
* Adds the info concerning the origin of the message to an sfp info
*
* @param	[in-out]	info : an sfp info
* @param	[in]	file_sending_id : a "supposed unique" identifier
* @param	[in]	username : the username
* @param	[in]	network_type : the network type (default to IN for Internet)
* @param	[in]	ip_address_type : the ip address type can be SFP_ADDRESS_TYPE_IPV4 or SFP_ADDRESS_TYPE_IPV6
* @param	[in]	ip_address : the ip address
* @param	[in]	address_port : the port number
* @param	[in]	connection_id : the connection id
*/
void sfp_add_origin_info(sfp_info_t * info, char * file_sending_id, char * username, char * network_type, char * ip_address_type, char * ip_address, char * address_port, char * connection_id){
	strncpy(info->file_sending_id, file_sending_id, sizeof(info->file_sending_id));
	strncpy(info->username, username, sizeof(info->username));
	strncpy(info->network_type, network_type, sizeof(info->network_type));
	strncpy(info->ip_address_type, ip_address_type, sizeof(info->ip_address_type));
	strncpy(info->ip_address, ip_address, sizeof(info->ip_address));
	strncpy(info->address_port, address_port, sizeof(info->address_port));

	/* JULIEN */
	strncpy(info->connection_id, connection_id, sizeof(info->connection_id));
	/* ****** */
}

/**
* Adds the info concerning the transfer mode to be used to an sfp info
*
* @param	[in-out]	info : an sfp info
* @param	[in]	mode : the transfer mode to be used, can be SFP_MODE_ACTIVE or SFP_MODE_PASSIVE
*/
void sfp_add_mode_info(sfp_info_t * info, char * mode){
	strncpy(info->mode, mode, sizeof(info->mode));
}

/**
* Adds the info concerning the transfer to an sfp info
*
* @param	[in-out]	info : an sfp info
* @param	[in]	ip_protocol : the protocol to be used, can be SFP_IP_PROTOCOL_TCP or SFP_IP_PROTOCOL_UDP
* @param	[in]	required_bandwidth : the minimum bandwidth necessited for the transfer, in bytes per second
* @param	[in]	packet_size : the maximum packet size that can be used during the transfer, in bytes
*/
void sfp_add_transfer_info(sfp_info_t * info, char * ip_protocol, char * required_bandwidth, char * packet_size){
	strncpy(info->ip_protocol, ip_protocol, sizeof(info->ip_protocol));
	strncpy(info->required_bandwidth, required_bandwidth, sizeof(info->required_bandwidth));
	strncpy(info->packet_size, packet_size, sizeof(info->packet_size));
}

/**
* Adds an uri leading to more info, into an sfp info
*
* @param	[in-out]	info : an sfp info
* @param	[in]	uri : the uri leading to more info
*/
void sfp_add_uri_info(sfp_info_t * info, char * uri){
	strncpy(info->uri, uri, sizeof(info->uri));
}

/**
* Adds info concerning the crypting, into an sfp info
*
* @param	[in-out]	info : an sfp info
* @param	[in]	key_info : determines the way the crypted key appears in the message, can be SFP_KEY_INFO_CLEAR, SFP_KEY_INFO_BASE64, SFP_KEY_INFO_URI or SFP_KEY_INFO_PROMPT					
* @param	[in]	crypted_key : the crypted key
*/
void sfp_add_key_info(sfp_info_t * info, char * key_info, char * crypted_key){
	strncpy(info->key_info, key_info, sizeof(info->key_info));
	strncpy(info->crypted_key, crypted_key, sizeof(info->crypted_key));
}

/**
* Adds info concerning the file to transfer, into an sfp info
*
* @param	[in-out]	info : an sfp info
* @param	[in]	filename : the "short" filename (not the whole path)
* @param	[in]	file_type : the type of the file
* @param	[in]	file_size : the exact size in bytes of the file
*/
void sfp_add_file_info(sfp_info_t * info, char * filename, char * file_type, char * file_size){
	strncpy(info->filename, filename, sizeof(info->filename));
	info->filename[sizeof(info->filename) - 1] = 0; // make sure it always ends with a 0
	strncpy(info->file_type, file_type, sizeof(info->file_type));
	info->file_type[sizeof(info->file_type) - 1] = 0; // make sure it always ends with a 0
	strncpy(info->file_size, file_size, sizeof(info->file_size));
	info->file_size[sizeof(info->file_size) - 1] = 0; // make sure it always ends with a 0
}

/**
* Adds the public ip infos into an sfp info
* (intends to be used by proxy servers)
*
* @param	[in-out]	info : an sfp info
* @param	[in]	ip_address_type : the public ip address type can be SFP_ADDRESS_TYPE_IPV4 or SFP_ADDRESS_TYPE_IPV6
* @param	[in]	ip_address : the public ip address
* @param	[in]	address_port : the public port number
*/
void sfp_add_public_adress_info(sfp_info_t * info, char * ip_address_type, char * ip_address, char * address_port){
	strncpy(info->ip_address_type, ip_address_type, sizeof(info->ip_address_type));
	strncpy(info->ip_address, ip_address, sizeof(info->ip_address));
	strncpy(info->address_port, address_port, sizeof(info->address_port));
}

