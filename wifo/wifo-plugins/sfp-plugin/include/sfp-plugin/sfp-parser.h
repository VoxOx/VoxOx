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

#ifndef __SFP_PARSER_H__
#define __SFP_PARSER_H__


// ----- PARSING INFO -----
#define SFP_LINE_SEPARATOR					'\n'
#define SFP_PROPERTY_VALUE_SEPARATOR		'='
#define SFP_VALUES_LIST_SEPARATOR			' '
#define SFP_VERSION_LINE					'v'
#define SFP_VERSION_LINE_FORMAT				"v=%s\n" /* (protocol_version) */
#define	SFP_ORIGIN_LINE						'o'
#define	SFP_ORIGIN_LINE_FORMAT				"o=%s %s %s %s %s %s\n" /* (file_sending_id) (username) (network_type) (ip_address_type) (ip_address) (address_port) */
#define SFP_MODE_INFO_LINE					'm'
#define SFP_MODE_INFO_LINE_FORMAT			"m=%s\n" /* (mode) */
#define SFP_TRANSFER_INFO_LINE				't'
#define SFP_TRANSFER_INFO_LINE_FORMAT		"t=%s %s %s\n" /* (ip_protocol) (required_bandwidth) (packet_size) */
#define SFP_URI_LINE						'u'
#define SFP_URI_LINE_FORMAT					"u=%s\n" /* (uri) */
#define SFP_CRYPTED_KEY_LINE				'k'
#define SFP_CRYPTED_KEY_LINE_FORMAT			"k=%s %s\n" /* (key_info) (crypted_key) */
#define SFP_FILE_INFO_LINE					'f'
#define SFP_FILE_INFO_LINE_FORMAT			"f=\"%s\" \"%s\" %s\n" /* (filename) (file_type) (file_size) */
#define SFP_CONNECTION_ID_LINE				's'
#define SFP_CONNECTION_ID_LINE_FORMAT		"s=%s\n" /* (connection_id) */

// ----- PREDEFINED VALUES -----
#define SFP_PROTOCOL_VERSION				"0.1.4"
#define SFP_NETWORK_TYPE_INTERNET			"IN"
#define SFP_ADDRESS_TYPE_IPV4				"IPv4"
#define SFP_ADDRESS_TYPE_IPV6				"IPv6"
#define SFP_MODE_ACTIVE						"active"
#define SFP_MODE_PASSIVE					"passive"
#define SFP_IP_PROTOCOL_TCP					"tcp"
#define SFP_IP_PROTOCOL_UDP					"udp"
#define SFP_REQUIRED_BANDWIDTH_DEFAULT		"33000" /* bytes per second */
#define SFP_PACKET_SIZE_DEFAULT				"1024"
#define SFP_KEY_INFO_CLEAR					"clear"
#define SFP_KEY_INFO_BASE64					"base64"
#define SFP_KEY_INFO_URI					"uri"
#define SFP_KEY_INFO_PROMPT					"prompt"

/**
* The structure containing the sfp message infos
*/
typedef struct sfp_info{
	char protocol_version[10];
	char file_sending_id[12];
	char username[32];
	char network_type[3];
	char ip_address_type[5];
	char ip_address[40];
	char address_port[6];
	char mode[8];
	char ip_protocol[4];
	char required_bandwidth[13]; /* bytes per second */
	char packet_size[5]; /* bytes */ // TODO rename to sfp_default_udp_packet_size
	char uri[128];
	char key_info[7];
	char crypted_key[257];
	char filename[257];
	char file_type[32];
	char file_size[13]; /* bytes */

	/* JULIEN */
	char connection_id[16];
} sfp_info_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	/**
	* Creates and initializes a new sfp_info_t with protocol version,
	* ip protocol, required bandwidth, and packet size initialized.
	*
	* @return	a new allocated spf info struct pointer
	*/
	sfp_info_t * sfp_create_sfp_info();

	/**
	* Creates and initializes a new sfp_info_t
	*
	* @return	a new allocated spf info struct pointer
	*/
	sfp_info_t * sfp_create_empty_sfp_info();

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
	void sfp_free_sfp_info(sfp_info_t ** info);

	/**
	* Parses an sfp body. If the protocol version is different from the one of the
	* parser, the body isn't parsed
	* 
	* @param	[in] text : the text of the body to parse
	* @return	a pointer to an sfp info struct concerning the file sending; NULL if parsing failed
	*/
	sfp_info_t * sfp_parse_message(char * text);

	/**
	* Turns the sfp_info_t into a sfp text message
	*
	* @param	[in]	info : an sfp info struct pointer
	* @return	a string containing the corresponding sfp body message
	*/
	char * sfp_make_message_body_from_sfp_info(sfp_info_t * info);

	/**
	* Adds the info concerning versionning to an sfp info
	*
	* @param	[in-out]	info : an sfp info
	* @param	[in]	protocol_version : the version of the sfp protocol
	*/
	void sfp_add_version_info(sfp_info_t * info, char * protocol_version);

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
	void sfp_add_origin_info(sfp_info_t * info, char * file_sending_id, char * username, char * network_type, char * ip_address_type, char * ip_address, char * address_port, char * connection_id);

	/**
	* Adds the info concerning the transfer mode to be used to an sfp info
	*
	* @param	[in-out]	info : an sfp info
	* @param	[in]	mode : the transfer mode to be used, can be SFP_MODE_ACTIVE or SFP_MODE_PASSIVE
	*/
	void sfp_add_mode_info(sfp_info_t * info, char * mode);

	/**
	* Adds the info concerning the transfer to an sfp info
	*
	* @param	[in-out]	info : an sfp info
	* @param	[in]	ip_protocol : the protocol to be used, can be SFP_IP_PROTOCOL_TCP or SFP_IP_PROTOCOL_UDP
	* @param	[in]	required_bandwidth : the minimum bandwidth necessited for the transfer, in bytes per second
	* @param	[in]	packet_size : the maximum packet size that can be used during the transfer, in bytes
	*/
	void sfp_add_transfer_info(sfp_info_t * info, char * ip_protocol, char * required_bandwidth, char * packet_size);

	/**
	* Adds an uri leading to more info, into an sfp info
	*
	* @param	[in-out]	info : an sfp info
	* @param	[in]	uri : the uri leading to more info
	*/
	void sfp_add_uri_info(sfp_info_t * info, char * uri);

	/**
	* Adds info concerning the crypting, into an sfp info
	*
	* @param	[in-out]	info : an sfp info
	* @param	[in]	key_info : determines the way the crypted key appears in the message, can be SFP_KEY_INFO_CLEAR, SFP_KEY_INFO_BASE64, SFP_KEY_INFO_URI or SFP_KEY_INFO_PROMPT					
	* @param	[in]	crypted_key : the crypted key
	*/
	void sfp_add_key_info(sfp_info_t * info, char * key_info, char * crypted_key);

	/**
	* Adds info concerning the file to transfer, into an sfp info
	*
	* @param	[in-out]	info : an sfp info
	* @param	[in]	filename : the "short" filename (not the whole path)
	* @param	[in]	file_type : the type of the file
	* @param	[in]	file_size : the exact size in bytes of the file
	*/
	void sfp_add_file_info(sfp_info_t * info, char * filename, char * file_type, char * file_size);

	/**
	* Adds the public ip infos into an sfp info
	* (intends to be used by proxy servers)
	*
	* @param	[in-out]	info : an sfp info
	* @param	[in]	ip_address_type : the public ip address type can be SFP_ADDRESS_TYPE_IPV4 or SFP_ADDRESS_TYPE_IPV6
	* @param	[in]	ip_address : the public ip address
	* @param	[in]	address_port : the public port number
	*/
	void sfp_add_public_adress_info(sfp_info_t * info, char * ip_address_type, char * ip_address, char * address_port);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SFP_PARSER_H__ */

