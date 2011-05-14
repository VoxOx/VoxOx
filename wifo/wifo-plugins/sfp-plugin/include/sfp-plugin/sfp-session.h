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

#include <sfp-plugin/sfp-error.h>
#include <sfp-plugin/sfp-parser.h>

#include <phapi-util/phapi-globals.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	/**
	* An enumeration of the states of a session
	*
	* TODO pause / resume
	*/
	typedef enum sfp_session_state {
		SFP_SESSION_INITIATED,
		SFP_SESSION_TRYING,
		SFP_SESSION_RUNNING,
		SFP_SESSION_CANCELLED,
		SFP_SESSION_CANCELLED_BY_PEER,
		SFP_SESSION_COMPLETE,
		SFP_SESSION_FINISHED,
		SFP_SESSION_PAUSED,
		SFP_SESSION_PAUSED_BY_PEER,
		SFP_SESSION_FAILED,

		SFP_SESSION_RESUMED,
		SFP_SESSION_RECEIVED_COMPLETE,
		SFP_SESSION_RECEIVED_INCOMPLETE,
		SFP_SESSION_SENT_INCOMPLETE,
		SFP_SESSION_SENT_COMPLETE,
		SFP_SESSION_CLOSED_BY_PEER,

	} sfp_session_state_t;

	typedef enum sfp_action {
		SFP_ACTION_START,
		SFP_ACTION_INVITE,
		SFP_ACTION_CANCEL,
		SFP_ACTION_BYE_OR_CANCEL_RECEIVED,
		SFP_ACTION_PAUSE,
		SFP_ACTION_HOLDON_RECEIVED,
		SFP_ACTION_RESUME,
		SFP_ACTION_HOLDOFF_RECEIVED,
		SFP_ACTION_SOCKET_CLOSED,
		SFP_ACTION_TRANSFER_COMPLETED
	} sfp_action_t;
	typedef struct sfp_session_info_s sfp_session_info_t;

	/**
	* Structure to store all the information about a file transfer
	*/
	struct sfp_session_info_s{
		char * session_id;
		char * local_username;
		char * local_mode; // if both are natted i.e. bridge info is filled, mode must be active for both
		char * local_ip_address_type;
		char * local_ip;
		char * local_port;
		char * remote_username;
		char * remote_ip_address_type;
		char * remote_ip;
		char * remote_port;
		char * ip_protocol;
		char * required_bandwidth; /** bytes per second */
		char * packet_size; /** bytes */ // TODO rename to udp_packet_size
		char * key_info;
		char * crypted_key;
		char * filename;
		char * remote_filename;
		char * local_filename;
		char * short_filename;
		char * file_type;
		char * file_size; /** bytes */
		// TODO uri?
		struct sockaddr_in local_address;
		SOCKET local_socket;
		int call_id;
		void (*terminaisonCallback)(sfp_session_info_t * session, sfp_returncode_t code);
		void (*progressionCallback)(sfp_session_info_t * session, int percentage);
		void (*sendBye)(int call_id);

		sfp_session_state_t _state;
		/* ----- functions to manipulate the states */
		void (*updateState)(sfp_session_info_t * session, sfp_action_t action);
		unsigned int (*isInitiated)(sfp_session_info_t * session);
		unsigned int (*isTrying)(sfp_session_info_t * session);
		unsigned int (*isRunning)(sfp_session_info_t * session);
		unsigned int (*isCancelled)(sfp_session_info_t * session);
		unsigned int (*isCancelledByPeer)(sfp_session_info_t * session);
		unsigned int (*isPaused)(sfp_session_info_t * session);
		unsigned int (*isPausedByPeer)(sfp_session_info_t * session);
		unsigned int (*isComplete)(sfp_session_info_t * session);
		unsigned int (*isFinished)(sfp_session_info_t * session);
		unsigned int (*hasFailed)(sfp_session_info_t * session);
		/* ----- */

		/* JULIEN */
		char * connection_id;

		/* Proxy infos */
		char http_proxy[128];
		unsigned short http_proxy_port;
		char http_proxy_user[128];
		char http_proxy_passwd[128];
	};

#ifdef __cplusplus
}
#endif /* __cplusplus */
