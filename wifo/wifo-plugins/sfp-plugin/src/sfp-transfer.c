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

/*
* TODO
* indicateurs de transfer
* timeouts de transfer
* code strengthen
* on est limite a la taille d'un unsigned long pour la taille maxi de transfer de fichier
* unregister doit nettoyer les structures allouees
*/

#include <sfp-plugin/sfp-commons.h>
#include <sfp-plugin/sfp-plugin.h>
#include <sfp-plugin/sfp-error.h>
#include <sfp-plugin/sfp-session.h>

#include <phapi-util/mystdio.h>
#include <phapi-util/util.h>
#include <phapi-util/phapi-globals.h>
#include <phapi-util/mystring.h>

#include <curl/curl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>


#define READ_WRITE_BUFFER_SIZE					1024

#define SFP_TRANSFER_UDP_ENDING_STRING			"OK"
#define SFP_TRANSFER_UDP_ENDING_STRING_LENGTH	2

#define SFP_REUSABLE_SOCKET						1
#define SFP_NOT_REUSABLE_SOCKET					0

#define SFP_MAX_RETRIES							5
#define SFP_WAIT_TIME_BASE						1
#define SFP_TIMEOUT_SEC							25
#define SFP_MAX_PORT							65535

#define SFP_TRANSFER_ACTIVE						1
#define SFP_TRANSFER_PASSIVE					2

#define SFP_TRANSFER_TCP						SOCK_STREAM
#define SFP_TRANSFER_UDP						SOCK_DGRAM

#define WAIT_PAUSE_DELAY						25000 // microseconds
#define WAIT_FOR_BYE_DELAY						5000 // microseconds


struct sfp_connection {
	SOCKET sckt;
	CURL * curl_data;
};
typedef struct sfp_connection sfp_connection_t;


// ----- PRIVATE FUNCTION DECLARATION -----
sfp_returncode_t sfp_transfer_get_free_port(sfp_session_info_t * session);
int sfp_transfer_send_connect_id(SOCKET sckt, char * connect_id, int size);
unsigned int sfp_transfer_receive_file(sfp_session_info_t * session);
unsigned int sfp_transfer_send_file(sfp_session_info_t * session);
static sfp_returncode_t sfp_transfer_send_file2(char * filename, unsigned int ip_protocol, unsigned int mode, const char * ip, unsigned short port, sfp_session_info_t * session);
static sfp_returncode_t sfp_transfer_receive_file2(char * filename, unsigned int ip_protocol, unsigned int mode, const char * ip, unsigned short port, sfp_session_info_t * session);
static sfp_returncode_t sfp_transfer_send_switch(FILE * stream, unsigned int ip_protocol, unsigned int mode, const char * ip, unsigned short port, sfp_session_info_t * session);
static sfp_returncode_t sfp_transfer_receive_switch(FILE * stream, unsigned int ip_protocol, unsigned int mode, const char * ip, unsigned short port, sfp_session_info_t * session);
static sfp_returncode_t init_connection(struct sockaddr_in * address, SOCKET * sckt, unsigned int ip_protocol, const char * ip, unsigned short port);
static void finalize_connection(sfp_connection_t * connection);
static sfp_returncode_t sfp_transfer_send_active(FILE * stream, SOCKET sckt, sfp_session_info_t * session);
static sfp_returncode_t sfp_transfer_send_passive(FILE * stream, SOCKET sckt, struct sockaddr_in address, sfp_session_info_t * session);
static sfp_returncode_t sfp_transfer_receive_active(FILE * stream, SOCKET sckt, sfp_session_info_t * session);
static sfp_returncode_t sfp_transfer_receive_passive(FILE * stream, SOCKET sckt, struct sockaddr_in address, sfp_session_info_t * session);
static void notify_progress(sfp_session_info_t * session, unsigned long actual, unsigned long final, unsigned int * increase);
static sfp_returncode_t sfp_connect(sfp_connection_t * connection, struct sockaddr_in * address, sfp_session_info_t * session, int * http_code);
static int sfp_get_http_req(int fd, char *buff, int size);
static int sfp_get_sid_from_http_req(char *query, int size, char * buff, int size_of_buff);
static void sfp_get_proxy_auth_type(sfp_session_info_t * session, long * type);
static int sfp_send_http_req_200ok(SOCKET sckt);
// -----

// ----- PRIVATE FUNCTION DEFINITION -----

/**
* Sends a file using the information provided in the session info
* TODO mutualize initialization part
*
* @param	[in-out]	session : a session info
* @return	TRUE if the sending succeeded; FALSE else
*/
unsigned int sfp_transfer_send_file(sfp_session_info_t * session){
	unsigned int mode, protocol;
	unsigned short port;
	sfp_returncode_t res = SUCCESS;

	if(session == NULL){
		m_log_error("session is NULL!!", "sfp_transfer_send_file");
		return FAILURE;
	}

	session->updateState(session, SFP_ACTION_START);

	if(session->local_mode != NULL && strequals(session->local_mode, SFP_MODE_ACTIVE)){
		mode = SFP_TRANSFER_ACTIVE;
	}else if(session->local_mode != NULL && strequals(session->local_mode, SFP_MODE_PASSIVE)){
		mode = SFP_TRANSFER_PASSIVE;
	}else{
		m_log_error("session->local_mode is NULL!!", "sfp_transfer_send_file");
		return FAILURE;
	}
	if(session->ip_protocol != NULL && strequals(session->ip_protocol, SFP_IP_PROTOCOL_TCP)){
		protocol = SFP_TRANSFER_TCP;
	}else if(session->ip_protocol != NULL && strequals(session->local_mode, SFP_IP_PROTOCOL_UDP)){
		protocol = SFP_TRANSFER_UDP;
	}else{
		m_log_error("session->ip_protocol is NULL!!", "sfp_transfer_send_file");
		return FAILURE;
	}

	// check the mode first
	if(strequals(session->local_mode, SFP_MODE_ACTIVE)){
		// TODO add checks
		port = (unsigned short)atoi(session->remote_port);

		// TODO add checks
		res = sfp_transfer_send_file2(session->filename, protocol, mode, session->remote_ip, port, session);
	}else{
		// TODO add checks
		port = (unsigned short)atoi(session->local_port);

		res = sfp_transfer_send_file2(session->filename, protocol, mode, session->local_ip, port, session);
	}

	// tell the main thread that the tranfser ended, via a callback
	if(session->terminaisonCallback != NULL) session->terminaisonCallback(session, res);

	return res;
}

/**
* Receives a file using the information provided in the session info
* TODO mutualize initialization part
*
* @param	[in-out]	session
* @return	TRUE if the receiving succeeded; FALSE else
*/
unsigned int sfp_transfer_receive_file(sfp_session_info_t * session){
	unsigned int mode, protocol;
	unsigned short port;
	sfp_returncode_t res = SUCCESS;

	if(session == NULL){
		m_log_error("session is NULL!!", "sfp_transfer_receive_file");
		return FAILURE;
	}

	// mark the session as running
	session->updateState(session, SFP_ACTION_START);

	if(session->local_mode != NULL && strequals(session->local_mode, SFP_MODE_ACTIVE)){
		mode = SFP_TRANSFER_ACTIVE;
	}else if(session->local_mode != NULL && strequals(session->local_mode, SFP_MODE_PASSIVE)){
		mode = SFP_TRANSFER_PASSIVE;
	}else{
		m_log_error("session->local_mode is NULL!!", "sfp_transfer_receive_file");
		return FAILURE;
	}
	if(session->ip_protocol != NULL && strequals(session->ip_protocol, SFP_IP_PROTOCOL_TCP)){
		protocol = SFP_TRANSFER_TCP;
	}else if(session->ip_protocol != NULL && strequals(session->local_mode, SFP_IP_PROTOCOL_UDP)){
		protocol = SFP_TRANSFER_UDP;
	}else{
		m_log_error("session->ip_protocol is NULL!!", "sfp_transfer_receive_file");
		return FAILURE;
	}

	// check the mode first
	if(strequals(session->local_mode, SFP_MODE_ACTIVE)){
		// TODO add checks
		port = (unsigned short)atoi(session->remote_port);

		// TODO add checks
		res = sfp_transfer_receive_file2(session->filename, protocol, mode, session->remote_ip, port, session);
	}else{
		// TODO add checks
		port = (unsigned short)atoi(session->local_port);

		res = sfp_transfer_receive_file2(session->filename, protocol, mode, session->local_ip, port, session);
	}

	// tell the main thread that the tranfser ended, via a callback
	if(session->terminaisonCallback != NULL) session->terminaisonCallback(session, res);

	return res;
}

/**
* Sends a file using the information provided in the session info
* TODO fusion this function with the "sfp_transfer_send_file(sfp_session_info_t * session)"
*
* @param	[in]	filename : the "full" file name (including the whole path)
* @param	[in]	ip_protocol : the IP protocol, can be SFP_TRANSFER_TCP or SFP_TRANSFER_UDP
* @param	[in]	mode : the transfer mode, can be SFP_TRANSFER_ACTIVE or SFP_TRANSFER_PASSIVE
* @param	[in]	ip : the public ip 
* @param	[in]	port : the public port
* @param	[in-out]	session : a session info
* @return	TRUE if the sending succeeded; FALSE else
*/
static sfp_returncode_t sfp_transfer_send_file2(char * filename, unsigned int ip_protocol, unsigned int mode, const char * ip, unsigned short port, sfp_session_info_t * session){
	FILE * stream = NULL;
	unsigned int success = SUCCESS;

#ifdef WIN32 /* to handle UTF-8 */
	wchar_t filename4win[1024];
	int sizeoffilename4win = sizeof(filename4win);
	int sizeoffilename = strlen(filename) * sizeof(char);
	char opts[6] = "r\0b\0\0\0";
	
	memset(&filename4win, 0, sizeof(filename4win));
	UTF8ToUTF16LE((unsigned char *)filename4win, &sizeoffilename4win, (const unsigned char *)filename, &sizeoffilename);
	if((stream = _wfopen(filename4win, (const wchar_t *)opts)) == NULL){
#else /* WIN32 */
	if((stream = fopen(filename, "rb")) == NULL){
#endif /* WIN32 */
		m_log_error("Could not open file in read mode", "sfp_transfer_send_file2");
		return CANT_READ_FILE; // fail
	}else{
		// success
	}

	if(sfp_transfer_send_switch(stream, ip_protocol, mode, ip, port, session) != SUCCESS){
		success = FILE_SEND_FAILED; // fail
	}else{
		// success
	}

	if(fclose(stream) != 0){
		success = CANT_CLOSE_FILE; // fail
	}else{
		// success
	}

	return success; // TODO
}

/**
* Receives a file using the information provided in the session info
* TODO fusion this function with the "sfp_transfer_receive_file(sfp_session_info_t * session)"
*
* @param	[in]	filename : the "full" file name (including the whole path)
* @param	[in]	ip_protocol : the IP protocol, can be SFP_TRANSFER_TCP or SFP_TRANSFER_UDP
* @param	[in]	mode : the transfer mode, can be SFP_TRANSFER_ACTIVE or SFP_TRANSFER_PASSIVE
* @param	[in]	ip : the public ip 
* @param	[in]	port : the public port
* @param	[in-out]	session : a session info
* @return	TRUE if the receiving succeeded; FALSE else
*/
static sfp_returncode_t sfp_transfer_receive_file2(char * filename, unsigned int ip_protocol, unsigned int mode, const char * ip, unsigned short port, sfp_session_info_t * session){
	FILE * stream = NULL;
	unsigned int success = SUCCESS;

#ifdef WIN32 /* to handle UTF-8 */
	wchar_t filename4win[1024];
	int sizeoffilename4win = sizeof(filename4win);
	int sizeoffilename = strlen(filename) * sizeof(char);
	char opts[6] = "w\0b\0\0\0";
	
	memset(&filename4win, 0, sizeof(filename4win));
	UTF8ToUTF16LE((unsigned char *)filename4win, &sizeoffilename4win, (const unsigned char *)filename, &sizeoffilename);
	if((stream = _wfopen(filename4win, (const wchar_t *)opts)) == NULL){
#else /* WIN32 */
	if((stream = fopen(filename, "wb")) == NULL){
#endif /* WIN32 */
		m_log_error("Could not open file in write mode", "sfp_transfer_receive_file2");
		return CANT_WRITE_FILE; // fail
	}else{
		// success
	}

	if(sfp_transfer_receive_switch(stream, ip_protocol, mode, ip, port, session) != SUCCESS){
		success = FILE_RECEIVE_FAILED; // fail
	}else{
		// success
	}

	if(fclose(stream) != 0){
		success = CANT_CLOSE_FILE; // fail
	}else{
		// success
	}

	return success; // TODO
}

/**
* Selector function that chooses the right send function, concerning the mode and ip protocol
*
* @param	[in]	stream : the file stream from which to read
* @param	[in]	ip_protocol : the IP protocol, can be SFP_TRANSFER_TCP or SFP_TRANSFER_UDP
* @param	[in]	mode : the transfer mode, can be SFP_TRANSFER_ACTIVE or SFP_TRANSFER_PASSIVE
* @param	[in]	ip : the public ip 
* @param	[in]	port : the public port
* @param	[in-out]	session : a session info
* @return	TRUE if the sending succeeded; FALSE else
*/
static sfp_returncode_t sfp_transfer_send_switch(FILE * stream, unsigned int ip_protocol, unsigned int mode, const char * ip, unsigned short port, sfp_session_info_t * session){
	struct sockaddr_in address;
	sfp_connection_t connection;
	sfp_returncode_t success = SUCCESS;
	int http_code;

	connection.sckt = -1;
	connection.curl_data = NULL;

	if(ip_protocol == SOCK_STREAM){ // TCP
		if(mode == SFP_TRANSFER_ACTIVE){
			if(session->local_socket >= 0){
				close(session->local_socket);
			}
			if(!strfilled(session->http_proxy)) {
				if(init_connection(&address, &(connection.sckt), ip_protocol, ip, port) != SUCCESS) {
					finalize_connection(&connection);
					return NETWORK_INITIALIZATION_FAILED;
				}
			}
			if(sfp_connect(&connection, &address, session, &http_code) != SUCCESS) {
				finalize_connection(&connection);
				return NETWORK_INITIALIZATION_FAILED;
			}
			success = sfp_transfer_send_active(stream, connection.sckt, session);
		}else if(mode == SFP_TRANSFER_PASSIVE){
			success = sfp_transfer_send_passive(stream, session->local_socket, session->local_address, session);
		}
		finalize_connection(&connection);
	}

	return success; // TODO
}

/**
* Selector function that chooses the right receive function, concerning the mode and ip protocol
*
* @param	[in]	stream : the file stream into which to write
* @param	[in]	ip_protocol : the IP protocol, can be SFP_TRANSFER_TCP or SFP_TRANSFER_UDP
* @param	[in]	mode : the transfer mode, can be SFP_TRANSFER_ACTIVE or SFP_TRANSFER_PASSIVE
* @param	[in]	ip : the public ip 
* @param	[in]	port : the public port
* @param	[in-out]	session : a session info
* @return	TRUE if the receiving succeeded; FALSE else
*/
static sfp_returncode_t sfp_transfer_receive_switch(FILE * stream, unsigned int ip_protocol, unsigned int mode, const char * ip, unsigned short port, sfp_session_info_t * session){
	struct sockaddr_in address;
	sfp_connection_t connection;
	sfp_returncode_t success = SUCCESS;
	int http_code;

	connection.sckt = -1;
	connection.curl_data = NULL;

	if(ip_protocol == SOCK_STREAM){ // TCP
		if(mode == SFP_TRANSFER_ACTIVE){
			if(session->local_socket >= 0){
				close(session->local_socket);
			}
			if(!strfilled(session->http_proxy)) {
				if(init_connection(&address, &(connection.sckt), ip_protocol, ip, port) != SUCCESS) {
					finalize_connection(&connection);
					return NETWORK_INITIALIZATION_FAILED;
				}
			}
			if(sfp_connect(&connection, &address, session, &http_code) != SUCCESS){
				finalize_connection(&connection);
				return NETWORK_INITIALIZATION_FAILED;
			}
			success = sfp_transfer_receive_active(stream, connection.sckt, session);			
		}else if(mode == SFP_TRANSFER_PASSIVE){
			success = sfp_transfer_receive_passive(stream, session->local_socket, session->local_address, session);
		}
		finalize_connection(&connection);
	}

	return success; // TODO
}

/* JULIEN */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

int sfp_transfer_send_connect_id(SOCKET sckt, char * connect_id, int size)
{
	char buff[24];
	int bsize;
	int retry = 3;
	int ret = 0;

	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%s\n", connect_id);
	for (ret = 0, bsize = (size + 1); bsize && retry; retry--){
		ret = send(sckt, buff, bsize, MSG_NOSIGNAL);
		if (ret <= 0){
			return -1;
		}

		bsize -= ret;
	}

	return (retry == 0 ? -1 : 0);
}
/* ****** */

/**
* Sends the file read from a stream, in TCP and in active mode (means that it does a "connect").
*
* @param	[in]	stream : the file stream from which to read
* @param	[in]	sckt : the socket used for transfer
* @param	[in]	sockaddr_in : the adress used for transfer
* @param	[in-out]	session : a session info
* @return	TRUE if the sending succeeded; FALSE else
*/
static sfp_returncode_t sfp_transfer_send_active(FILE * stream, SOCKET sckt, sfp_session_info_t * session){
	char buffer[READ_WRITE_BUFFER_SIZE];
	size_t read = 0;
	int sent = 0;
	int tmp_sent = 0;
	long total_sent = 0;
	long total_to_send = (unsigned long)atol(session->file_size);
	unsigned int increase = 0;
	fd_set sckts;
	struct timeval timeout = {SFP_TIMEOUT_SEC, 0};
	int max_sckt;
	int wait_time = SFP_WAIT_TIME_BASE;
	int retries = SFP_MAX_RETRIES;

	notify_progress(session, total_sent, total_to_send, &increase);
	memset(buffer, 0, sizeof(buffer));
	while((read = fread(buffer, sizeof(char), READ_WRITE_BUFFER_SIZE, stream)) > 0){
		// if the transfer has been paused, wait
		while(session->isPaused(session) || session->isPausedByPeer(session)){
			usleep(WAIT_PAUSE_DELAY);
		}
		// if the transfer has been cancelled stop sending
		if(session->isCancelledByPeer(session)){
			return SUCCESS;
		}
		if(session->isCancelled(session)) {
			break;
		}
		sent = 0;
		while(sent < (int)read){
			FD_ZERO(&sckts);
			FD_SET(sckt, &sckts);
			max_sckt = sckt+1;
			timeout.tv_sec = SFP_TIMEOUT_SEC;
			timeout.tv_usec = 0;
			if(select(max_sckt, NULL, &sckts, NULL, &timeout) > 0){
				if((tmp_sent = send(sckt, buffer, (int)read-sent, MSG_NOSIGNAL)) >= 0){
					sent += tmp_sent;
				}else{
					session->updateState(session, SFP_ACTION_SOCKET_CLOSED);
					m_log_error("Send failed", "sfp_transfer_send_active");
					return TRANSFER_CORRUPTION; // fail
				}
			}else{
				FD_CLR(sckt, &sckts);
				session->updateState(session, SFP_ACTION_SOCKET_CLOSED);
				m_log_error("Connection timed out", "sfp_transfer_send_active");
				return CONNECTION_TIMED_OUT; // fail
			}
		}

		total_sent += sent;

		if(total_sent > total_to_send){
			m_log_error("Sent more bytes than declared", "sfp_transfer_send_active");
			return TRANSFER_CORRUPTION; // fail
		}

		// notify the progession of the transfer
		notify_progress(session, total_sent, total_to_send, &increase);

		memset(buffer, 0, sizeof(buffer));
	}

	// check that we have sent it all
	if(total_sent < total_to_send){ // TODO verify
		if(session->isCancelled(session)) {
			// wait that the receiver disconnects (receives the BYE)
			FD_ZERO(&sckts);
			FD_SET(sckt, &sckts);
			max_sckt = sckt+1;
			timeout.tv_sec = SFP_TIMEOUT_SEC;
			timeout.tv_usec = 0;
			select(max_sckt, &sckts, NULL, NULL, &timeout); // wait on read
			FD_CLR(sckt, &sckts);
			return SUCCESS;
		} else {
			session->updateState(session, SFP_ACTION_SOCKET_CLOSED);
			return TRANSFER_CORRUPTION;
		}
	} else if(total_sent == total_to_send) {
		session->updateState(session, SFP_ACTION_TRANSFER_COMPLETED);
		// wait little time to receive the BYE from receiver or timeout
		while(!session->isFinished(session) && retries-- > 0) {
			sleep(wait_time);
			wait_time = wait_time * 2;
		}
		if(!session->isFinished(session)) { // BYE has not been received
			//return FAILURE; // TODO what should we do?
		}
	}

	return SUCCESS; // TODO
}

/**
* Sends the file read from a stream, in TCP and in passive mode (means that it does a "select and accept").
*
* @param	[in]	stream : the file stream from which to read
* @param	[in]	sckt : the socket used for transfer
* @param	[in]	sockaddr_in : the adress used for transfer
* @param	[in-out]	session : a session info
* @return	TRUE if the sending succeeded; FALSE else
*/
static sfp_returncode_t sfp_transfer_send_passive(FILE * stream, SOCKET sckt, struct sockaddr_in address, sfp_session_info_t * session){
	char buffer[READ_WRITE_BUFFER_SIZE];
	socklen_t addrlen;
	size_t read = 0;
	SOCKET tmp = -1;
	fd_set sckts;
	struct timeval timeout = {SFP_TIMEOUT_SEC, 0};
	int max_sckt;
	int sent = 0;
	int tmp_sent = 0;
	long total_sent = 0;
	long total_to_send = (unsigned long)atol(session->file_size);
	unsigned int increase = 0;
	char http_req[2048];
	char cid[32];
	int wait_time = SFP_WAIT_TIME_BASE;
	int retries = SFP_MAX_RETRIES;

	addrlen = (socklen_t)sizeof(address);

	if(listen(sckt, 5) < 0){
		return CANT_LISTEN_ON_SOCKET; // fail
	}

	// use select to do a timeout in order not to stay blocked if peer cannot receive file
	FD_ZERO(&sckts);
	FD_SET(sckt, &sckts);
	max_sckt = (int)sckt + 1;
	if(select(max_sckt, NULL, &sckts, NULL, &timeout) <= 0){
		// no connection received
		FD_CLR(sckt, &sckts);
		m_log_error("Connection timed out", "sfp_transfer_send_passive");
		return CONNECTION_TIMED_OUT; // fail
	}
	if(FD_ISSET(sckt, &sckts) == 0){
		FD_CLR(sckt, &sckts);
		m_log_error("Connection timed out", "sfp_transfer_send_passive");
		return CONNECTION_TIMED_OUT; // fail
	}
	FD_CLR(sckt, &sckts);

	tmp = accept(sckt, (struct sockaddr *)&address, &addrlen);
	if(tmp < 0){
		m_log_error("Accept failed", "sfp_transfer_send_passive");
		return CANT_ACCEPT_CONNECTION; // fail
	}

	// receive the HTTP GET request
	if(sfp_get_http_req(tmp, http_req, sizeof(http_req)-1) <= 0) {
		m_log_error("Couldn't get the HTTP GET request", "sfp_transfer_send_passive");
		return CANT_ACCEPT_CONNECTION; // fail
	}

	// extract the connection id
	if(sfp_get_sid_from_http_req(http_req, sizeof(http_req)-1, cid, sizeof(cid)-1) < 0) {
		m_log_error("Couldn't extract the connection id from the HTTP GET request", "sfp_transfer_send_passive");
		return CANT_ACCEPT_CONNECTION; // fail
	}

	// compare connection ids
	if (strcasecmp(cid, session->connection_id) != 0) {
		m_log_error("Connection ids do not match", "sfp_transfer_send_passive");
		return CANT_ACCEPT_CONNECTION; // fail
	}

	notify_progress(session, total_sent, total_to_send, &increase);
	memset(buffer, 0, sizeof(buffer));
	while((read = fread(buffer, sizeof(char), READ_WRITE_BUFFER_SIZE, stream)) > 0){
		// if the transfer has been paused, wait
		while(session->isPaused(session) || session->isPausedByPeer(session)){
			usleep(WAIT_PAUSE_DELAY);
		}
		// if the transfer has been cancelled stop sending
		if(session->isCancelledByPeer(session)){
			close(tmp);
			return SUCCESS;
		}
		if(session->isCancelled(session)){
			break;
		}
		sent = 0;
		while(sent < (int)read){
			FD_ZERO(&sckts);
			FD_SET(tmp, &sckts);
			max_sckt = tmp+1;
			timeout.tv_sec = SFP_TIMEOUT_SEC;
			timeout.tv_usec = 0;
			if(select(max_sckt, NULL, &sckts, NULL, &timeout) > 0){
				if((tmp_sent = send(tmp, buffer, (int)read-sent, MSG_NOSIGNAL)) >= 0){
					sent += tmp_sent;
				}else{
					session->updateState(session, SFP_ACTION_SOCKET_CLOSED);
					m_log_error("Send failed", "sfp_transfer_send_active");
					return TRANSFER_CORRUPTION; // fail
					
				}
			}else{
				FD_CLR(tmp, &sckts);
				session->updateState(session, SFP_ACTION_SOCKET_CLOSED);
				m_log_error("Connection timed out", "sfp_transfer_send_active");
				return CONNECTION_TIMED_OUT; // fail
			}
		}

		total_sent += sent;

		if(total_sent > total_to_send){
			m_log_error("Sent more bytes than declared", "sfp_transfer_send_passive");
			return TRANSFER_CORRUPTION; // TODO errorcode
		}

		// notify the progession of the transfer
		notify_progress(session, total_sent, total_to_send, &increase);

		memset(buffer, 0, sizeof(buffer));
	}	

	// check that we have sent it all
	if(total_sent < total_to_send){ // TODO verify
		if(session->isCancelled(session)) {
			// wait that the receiver disconnects (receives the BYE) or timeout
			FD_ZERO(&sckts);
			FD_SET(tmp, &sckts);
			max_sckt = (int)tmp+1;
			timeout.tv_sec = SFP_TIMEOUT_SEC;
			timeout.tv_usec = 0;
			select(max_sckt, &sckts, NULL, NULL, &timeout); // wait on read
			FD_CLR(tmp, &sckts);
			close(tmp);
			return SUCCESS;
		} else {
			session->updateState(session, SFP_ACTION_SOCKET_CLOSED);
			close(tmp);
			return TRANSFER_CORRUPTION;
		}
	}else if(total_sent == total_to_send){
		session->updateState(session, SFP_ACTION_TRANSFER_COMPLETED);
		// wait little time to receive the BYE from receiver or timeout
		while(!session->isFinished(session) && retries-- > 0) {
			sleep(wait_time);
			wait_time = wait_time * 2;
		}
		if(!session->isFinished(session)) { // BYE has not been received
			//return FAILURE; // TODO what should we do?
		}
	}

	close(tmp);

	return SUCCESS; // TODO
}


/**
* Receives the file read from network and writes it into a strean, in TCP and in active mode (means that it does a "connect").
*
* @param	[in]	stream : the file stream into which to write
* @param	[in]	sckt : the socket used for transfer
* @param	[in]	sockaddr_in : the adress used for transfer
* @param	[in-out]	session : a session info
* @return	TRUE if the receiving succeeded; FALSE else
*/
static sfp_returncode_t sfp_transfer_receive_active(FILE * stream, SOCKET sckt, sfp_session_info_t * session){
	char buffer[READ_WRITE_BUFFER_SIZE];
	int received = 0;
	long total_received = 0;
	long total_to_receive = (unsigned long)atol(session->file_size);
	unsigned int increase = 0;
	fd_set sckts;
	struct timeval timeout = {SFP_TIMEOUT_SEC, 0};
	int max_sckt = (int)sckt + 1;

	FD_ZERO(&sckts);
	FD_SET(sckt, &sckts);
	if(select(max_sckt, &sckts, NULL, NULL, &timeout) == 0) {
		FD_CLR(sckt, &sckts);
		m_log_error("Connection timed out", "sfp_transfer_receive_active");
		return CONNECTION_TIMED_OUT;
	}

	notify_progress(session, total_received, total_to_receive, &increase);
	memset(buffer, 0, sizeof(buffer));
	while((received = recv(sckt, buffer, READ_WRITE_BUFFER_SIZE, 0)) > 0){
		total_received += (unsigned long)received;

		// if the transfer has been paused, wait 
		// (wait loop because we could have received data while we paused)
		while(session->isPaused(session) || session->isPausedByPeer(session)){
			usleep(WAIT_PAUSE_DELAY);
		}

		// notify the progession of the transfer
		notify_progress(session, total_received, total_to_receive, &increase);

		// if the transfer has been cancelled stop sending
		if(session->isCancelledByPeer(session)){
			return SUCCESS;
		}
		// if session is cancelled, continue receiving but drop the data until the receiver receives the BYE and disconnects
		if(!session->isCancelled(session) && (int)fwrite(buffer, sizeof(char), received, stream) < received){
			m_log_error("Wrote less char than what's been received", "sfp_transfer_receive_active");
			return WRITE_ERROR; // fail
		}

		if(total_received > total_to_receive){
			m_log_error("Received more bytes than declared", "sfp_transfer_receive_active");
			return TRANSFER_CORRUPTION;
		}else if(total_received == total_to_receive) {
			session->updateState(session, SFP_ACTION_TRANSFER_COMPLETED);
			session->sendBye(session->call_id);
			// only wait for the recv to fail (BYE has been sent, waiting for disconection)
		}

		memset(buffer, 0, sizeof(buffer));
	}

	session->updateState(session, SFP_ACTION_SOCKET_CLOSED);

	// check that we have received it all
	if(total_received < total_to_receive){

		// if recv failed and we are in a cancel state, it could be because we cancelled
		// (sent a BYE) the transfer and peer therefore closed the socket
		if(session->isCancelled(session) || session->isCancelledByPeer(session)){
			return SUCCESS;
		}

		return TRANSFER_CORRUPTION;
	}

	return SUCCESS;
}

/**
* Receives the file read from network and writes it into a strean, in TCP and in active mode (means that it does a "select and connect").
*
* @param	[in]	stream : the file stream into which to write
* @param	[in]	sckt : the socket used for transfer
* @param	[in]	sockaddr_in : the adress used for transfer
* @param	[in-out]	session : a session info
* @return	TRUE if the receiving succeeded; FALSE else
*/
static sfp_returncode_t sfp_transfer_receive_passive(FILE * stream, SOCKET sckt, struct sockaddr_in address, sfp_session_info_t * session){
	char buffer[READ_WRITE_BUFFER_SIZE];
	socklen_t addrlen;
	unsigned int received = 0;
	SOCKET tmp = -1;
	fd_set sckts;
	struct timeval timeout = {SFP_TIMEOUT_SEC, 0};
	int max_sckt;
	long total_received = 0;
	long total_to_receive = (unsigned long)atol(session->file_size);
	unsigned int increase = 0;
	char http_req[2048];
	char cid[32];


	addrlen = (socklen_t)sizeof(address);

	if(listen(sckt, 5) < 0){
		return CANT_LISTEN_ON_SOCKET; // fail
	}

	// wait for a connection (wait on read to receive the HTTP GET request)
	FD_ZERO(&sckts);
	FD_SET(sckt, &sckts);
	max_sckt = (int)sckt + 1;
	if(select(max_sckt, &sckts, NULL, NULL, &timeout) <= 0){
		// no connection received
		FD_CLR(sckt, &sckts);
		m_log_error("Connection timed out", "sfp_transfer_receive_passive");
		return CONNECTION_TIMED_OUT; // fail
	}
	if(FD_ISSET(sckt, &sckts) == 0){
		FD_CLR(sckt, &sckts);
		m_log_error("Connection timed out", "sfp_transfer_receive_passive");
		return CONNECTION_TIMED_OUT; // fail
	}
	FD_CLR(sckt, &sckts);
	
	// accept the connection
	tmp = accept(sckt, (struct sockaddr *)&address, &addrlen);
	if(tmp < 0){
		m_log_error("Accept failed", "sfp_transfer_receive_passive");
		return CANT_ACCEPT_CONNECTION; // fail
	}

	// receive the HTTP GET request
	if(sfp_get_http_req(tmp, http_req, sizeof(http_req)-1) <= 0) {
		m_log_error("Couldn't get the HTTP GET request", "sfp_transfer_receive_passive");
		return CANT_ACCEPT_CONNECTION; // fail
	}

	// extract the connection id
	if(sfp_get_sid_from_http_req(http_req, sizeof(http_req)-1, cid, sizeof(cid)-1) < 0) {
		m_log_error("Couldn't extract the connection id from the HTTP GET request", "sfp_transfer_receive_passive");
		return CANT_ACCEPT_CONNECTION; // fail
	}

	// compare connection ids
	if (strcasecmp(cid, session->connection_id) != 0) {
		m_log_error("Connection ids do not match", "sfp_transfer_receive_passive");
		return CANT_ACCEPT_CONNECTION; // fail
	}

	// answer the HTTP GET request by a HTTP 200 OK
	if(sfp_send_http_req_200ok(tmp) != SUCCESS) {
		m_log_error("Cannot send HTTP 200 OK", "sfp_transfer_receive_passive");
		return CANT_ACCEPT_CONNECTION; // fail
	}

	notify_progress(session, total_received, total_to_receive, &increase);
	memset(buffer, 0, sizeof(buffer));
	while((received = recv(tmp, buffer, READ_WRITE_BUFFER_SIZE, 0)) > 0){
		total_received += (unsigned long)received;

		// if the transfer has been paused, wait 
		// (wait loop because we could have received data while we paused)
		while(session->isPaused(session) || session->isPausedByPeer(session)){
			usleep(WAIT_PAUSE_DELAY);
		}

		// notify the progession of the transfer
		notify_progress(session, total_received, total_to_receive, &increase);

		// if the transfer has been cancelled stop sending
		if(session->isCancelledByPeer(session)){
			close(tmp);
			return SUCCESS;
		}
		if(!session->isCancelled(session) && fwrite(buffer, sizeof(char), received, stream) < received){
			m_log_error("Wrote less char than what's been received", "sfp_transfer_receive_passive");
			return WRITE_ERROR; // fail
		}

		if(total_received > total_to_receive){
			m_log_error("Received more bytes than declared", "sfp_transfer_receive_passive");
			return TRANSFER_CORRUPTION;
		}else if(total_received == total_to_receive) {
			session->updateState(session, SFP_ACTION_TRANSFER_COMPLETED);
			session->sendBye(session->call_id);
			// only wait for the recv to fail (BYE has been sent, waiting for disconection)
		}

		memset(buffer, 0, sizeof(buffer));
	}

	session->updateState(session, SFP_ACTION_SOCKET_CLOSED);

	if(total_received < total_to_receive){ // TODO verify
		// if recv failed and we are in a cancel state, it could because we cancelled
		// (sent a BYE) the transfer and peer therefore closed the socket
		if(session->isCancelled(session) || session->isCancelledByPeer(session)){
			close(tmp);
			return SUCCESS;
		}

		close(tmp);
		return TRANSFER_CORRUPTION;
	}

	close(tmp);

	return SUCCESS;
}

/**
* Initializes a connection by filling in a sockaddr_in struct and opening a socket.
*
* @param	[out]	address : the address to initialize
* @param	[out]	socket : the socket to open
* @param	[in]	ip_protocol : the protocol to use, can be SFP_TRANSFER_TCP or SFP_TRANSFER_UDP
* @param	[in]	ip : the public ip
* @param	[in]	port : the public port
* @return	TRUE if the initialization succeeded; FALSE else
*/
static sfp_returncode_t init_connection(struct sockaddr_in * address, SOCKET * sckt, unsigned int ip_protocol, const char * ip, unsigned short port){
#ifdef WIN32
	int opt;
#endif /* WIN32 */

	memset(address, 0, sizeof(struct sockaddr_in));
	address->sin_family = AF_INET;
	address->sin_port = htons(port);
	address->sin_addr.s_addr = inet_addr(ip);

	*sckt = socket(AF_INET, ip_protocol, 0);
	if(*sckt < 0){
		m_log_error("Could not get a socket", "init_connection");
		return CANT_GET_SOCKET; // fail
	}

#ifdef WIN32
	opt = SFP_REUSABLE_SOCKET;
	if(setsockopt(*sckt, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0){
		m_log("Could not ste the socket in reusable mode","init_connection");
		return CANT_SET_SOCKET_REUSABLE;
	}
#endif /* WIN32 */

	return SUCCESS; // TODO
}

/**
* Closes a socket.
* TODO turn it into a void func
*
* @return	TRUE
*/
static void finalize_connection(sfp_connection_t * connection) {
	if(connection->curl_data != NULL) {
		curl_easy_cleanup(connection->curl_data);
	} else if(connection->sckt >= 0){
		close(connection->sckt);
	}
}

/**
* Notifies of the progress of a file transfer
*
* @param	[in]	session : a session info providing the callback to call in order to notify of the transfer progress
* @param	[in]	actual : the actual size, in bytes
* @param	[in]	final : the size to reach, in bytes
* @param	[in-out]	increase : the progress step / next step to reach
*/
static void notify_progress(sfp_session_info_t * session, unsigned long actual, unsigned long final, unsigned int * increase){
	double percentage = 0;
	double pg = 49000000/((double)final+1000000)+1;
	int progression = (int)ceil(pg);

	if(actual == final || final == 0) {
		if(session->progressionCallback != NULL) session->progressionCallback(session, 100);
		*increase = 100;
	} else {
		percentage = ((double)actual / (double)final) * 100;
		if(*increase == 0){
			*increase = progression;
		}
		if(percentage >= (double)(*increase)){
			if(session->progressionCallback != NULL) session->progressionCallback(session, *increase);
			while(percentage >= (double)(*increase)){
				*increase += progression;
			}
		}
	}
}

sfp_returncode_t sfp_transfer_get_free_port(sfp_session_info_t * session){
#ifdef WIN32
	int opt;
#endif /* WIN32 */
	int res_bind = -1;
	unsigned short port;
	char temp[33];
	int ip_protocol;


	if(strequals(session->ip_protocol, SFP_IP_PROTOCOL_TCP)){
		ip_protocol = SFP_TRANSFER_TCP;
	}else if(strequals(session->local_mode, SFP_IP_PROTOCOL_UDP)){
		ip_protocol = SFP_TRANSFER_UDP;
	}

	session->local_socket = socket(AF_INET, ip_protocol, 0);
	if(session->local_socket < 0){
		m_log_error("Could not get a socket", "sfp_transfer_get_free_port");
		return CANT_GET_SOCKET; // fail
	}

#ifdef WIN32
	opt = SFP_REUSABLE_SOCKET;
	if(setsockopt(session->local_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0){
		m_log("Could not ste the socket in reusable mode","init_connection");
		return FALSE;
	}
#endif /* WIN32 */

	port = (unsigned short)atoi(session->local_port);

	memset(&(session->local_address), 0, sizeof(struct sockaddr_in));
	session->local_address.sin_family = AF_INET;
	session->local_address.sin_port = htons(port);
	session->local_address.sin_addr.s_addr = inet_addr(session->local_ip);

	while((res_bind = bind(session->local_socket, (struct sockaddr *)&(session->local_address), sizeof(session->local_address))) < 0 && port < SFP_MAX_PORT){
		port += 1;
		session->local_address.sin_port = htons(port);
	}

	if(port >= SFP_MAX_PORT){
		m_log_error("Could not get a free transfer port", "sfp_transfer_get_free_port");
		close(session->local_socket);
		return NO_AVAILABLE_PORT; // fail
	}

	if(session->local_port != NULL){
		free(session->local_port);
	}
	itostr((int)port, temp, sizeof(temp), 10);
	session->local_port = strdup(temp);

	return SUCCESS;
}

static sfp_returncode_t sfp_connect(sfp_connection_t * connection, struct sockaddr_in * address, sfp_session_info_t * session, int * http_code) {
	char query[512] = "";

	int res_connect = -1;
	int wait_time = SFP_WAIT_TIME_BASE;
	char message[256];
	int retries = SFP_MAX_RETRIES;
	unsigned int sent = 0;
	int tmp_sent = 0;
	fd_set sckts;
	struct timeval timeout = {SFP_TIMEOUT_SEC, 0};
	int max_sckt;
	char buffer[1024] = "";

	char url_buff[1024];
	char proxy_buff[1024];
	char login_buff[1024];
	char header_buff[1024];
	int curl_ret = -1;
	struct curl_slist * slist = NULL;
	int sckt_flags;

	if(strfilled(session->http_proxy)) {
		*http_code = 404;

		connection->curl_data = curl_easy_init();

		if(connection->curl_data) {
			curl_easy_setopt(connection->curl_data, CURLOPT_VERBOSE, 1);

			snprintf(url_buff, sizeof(url_buff), "http://%s:%s", session->remote_ip, session->remote_port);
			curl_easy_setopt(connection->curl_data, CURLOPT_URL, url_buff);

			snprintf(proxy_buff, sizeof(proxy_buff), "%s:%d", session->http_proxy, session->http_proxy_port);
			curl_easy_setopt(connection->curl_data, CURLOPT_PROXY, proxy_buff);

			curl_easy_setopt(connection->curl_data, CURLOPT_CONNECTTIMEOUT, SFP_TIMEOUT_SEC);

			if(strfilled(session->http_proxy_user) && strfilled(session->http_proxy_passwd)){
				long auth_type = 0;

				snprintf(login_buff, sizeof(login_buff), "%s:%s", session->http_proxy_user, session->http_proxy_passwd);
				sfp_get_proxy_auth_type(session, &auth_type);
				curl_easy_setopt(connection->curl_data, CURLOPT_PROXYUSERPWD, login_buff);
				curl_easy_setopt(connection->curl_data, CURLOPT_PROXYAUTH, auth_type);
			}

			curl_easy_setopt(connection->curl_data, CURLOPT_HTTPPROXYTUNNEL, 1);

			snprintf(header_buff, sizeof(header_buff), "ConnectionId: %s", session->connection_id);
			slist = curl_slist_append(slist, header_buff);
			slist = curl_slist_append(slist, "Connection: Keep-Alive");  
			slist = curl_slist_append(slist, "Pragma: no-cache");
			slist = curl_slist_append(slist, "Cache-Control: no-cache");
			curl_easy_setopt(connection->curl_data, CURLOPT_HTTPHEADER, slist);

			curl_ret = curl_easy_perform(connection->curl_data);

			curl_easy_getinfo(connection->curl_data, CURLINFO_RESPONSE_CODE, http_code);

			if (curl_ret != 0) {
				finalize_connection(connection);
				return FAILURE;
			}

			curl_slist_free_all(slist);

			curl_easy_getinfo(connection->curl_data, CURLINFO_LASTSOCKET, &(connection->sckt));
			if(connection->sckt == -1) {
				return FAILURE;
			}

			sckt_flags = fcntl(connection->sckt, F_GETFL, 0);
			if(fcntl(connection->sckt, F_SETFL, sckt_flags & ~O_NONBLOCK) != 0) {
			//if(ioctlsocket(connection->sckt, FIONBIO, &nonblocking) != 0) {
				return FAILURE;
			}

			return SUCCESS;
		}

	} else {

		// connection
		while((res_connect = connect(connection->sckt, (struct sockaddr *)address, sizeof(*address))) < 0 && retries-- > 0){
			sprintf(message, "Waiting for %d ms", wait_time);
			m_log(message, "sfp_connect");
			if(session->isCancelled(session) || session->isCancelledByPeer(session)){
				break;
			}
			sleep(wait_time);
			wait_time = wait_time * 2;
		}
		if(res_connect < 0){
			m_log_error("Could not connect to peer", "sfp_connect");
			return CANT_CONNECT; // fail
		}

		if (!session->connection_id || session->connection_id[0] == 0) {
			return FAILURE;
		}

		snprintf(query, sizeof(query), "GET / HTTP/1.1\r\nConnectionId: %s\r\n\r\n", session->connection_id);

		// send the query with the connection id
		while(sent < strlen(query)){
			FD_ZERO(&sckts);
			FD_SET(connection->sckt, &sckts);
			max_sckt = connection->sckt+1;
			timeout.tv_sec = SFP_TIMEOUT_SEC;
			timeout.tv_usec = 0;
			if(select(max_sckt, NULL, &sckts, NULL, &timeout) > 0){
				if((tmp_sent = send(connection->sckt, query, strlen(query)-sent, MSG_NOSIGNAL)) >= 0){
					sent += tmp_sent;
				} else {
					return FAILURE;
				}
			}else{
				FD_CLR(connection->sckt, &sckts);
				m_log_error("Connection timed out", "sfp_connect");
				return CONNECTION_TIMED_OUT; // fail
			}
		}

		FD_ZERO(&sckts);
		FD_SET(connection->sckt, &sckts);
		max_sckt = connection->sckt+1;
		timeout.tv_sec = SFP_TIMEOUT_SEC;
		timeout.tv_usec = 0;
		// wait on read, till timeout or till a receiver disconnection
		if(select(max_sckt, &sckts, NULL, NULL, &timeout) <= 0) {
			FD_CLR(connection->sckt, &sckts);
			return FAILURE;
		}
		if(sfp_get_http_req(connection->sckt, buffer, sizeof(buffer)-1) <= 0) {
			FD_CLR(connection->sckt, &sckts);
			return FAILURE;
		}
		FD_CLR(connection->sckt, &sckts);

		if(strncasecmp(buffer, "HTTP/1.0 200 OK", 15) != 0) {
			return FAILURE;
		}

		return SUCCESS;
	}

	return FAILURE;
}


/* <julien> */
static int sfp_get_http_req(int fd, char *buff, int size)
{
	int ret = 0;
	int cread = 0;
	int total = 0;
	struct timeval timeout = {3, 0};
	fd_set sckts;
	while (1)
	{           FD_ZERO(&sckts);
	FD_SET(fd, &sckts);
	ret = select(fd + 1, &sckts, NULL, NULL, &timeout);
	if (ret <= 0)
		return -1;
	if (FD_ISSET(fd, &sckts))
	{
		if (total < size)
		{
			if ((cread = recv(fd, &buff[total], 1, 0)) > 0)
				total += cread;
			else
				return -1;
			if (total > 3 && !strncmp("\r\n\r\n", buff + total - 4, 4))
			{
				break;
			}
		}
		else
		{
			return -1;
		}
	}
	else
		return -1;
	}
	return total;
}

static int sfp_get_sid_from_http_req(char *query, int size, char * buff, int size_of_buff)
{
	char  *tmp;
	int    i, j;

	tmp = 0;
	if (size && strncmp(query, "GET", 3) == 0)
	{           
		for (i = 0; i < size; i++)
		{
			if (strncasecmp(query + i, "connectionid:", 13) == 0)
			{
				while (i < size && ((*(query + i) < '0') || (*(query + i) > '9')))
					i++;
				for (tmp = query + i, j = 0;
					((i+j) < size) && (*(tmp+j) >= '0') && (*(tmp+j) <= '9'); j++);
					if ((i+j) < size)
					{
						if (j >= size_of_buff)
						{
							fprintf(stderr, "Error parsing GET query : %s", query);
							return -1;
						}
						memcpy(buff, query + i, j);
						buff[j] = 0;
						break;
					}
			}
		}
	}

	if(strlen(buff) <= 0){
		fprintf(stderr, "Error parsing GET query : %s", query);
		return -1;
	}
	return 0;
}

static void sfp_get_proxy_auth_type(sfp_session_info_t * session, long * type)
{
		CURL *curl_tmp;
		char url_buff[1024];
		char proxy_buff[1024];
		int ret;

		ret = 0;
		curl_tmp = curl_easy_init();

		snprintf(url_buff, sizeof(url_buff), "http://%s:%s", session->remote_ip, session->remote_port);
		curl_easy_setopt(curl_tmp, CURLOPT_URL, url_buff);

		snprintf(proxy_buff, sizeof(proxy_buff), "%s:%d", session->http_proxy, session->http_proxy_port);
		curl_easy_setopt(curl_tmp, CURLOPT_PROXY, proxy_buff);

		curl_easy_setopt(curl_tmp, CURLOPT_HTTPPROXYTUNNEL, 1);
		ret = curl_easy_perform(curl_tmp);

		curl_easy_getinfo(curl_tmp, CURLINFO_PROXYAUTH_AVAIL, type);
		
		// free the Curl tmp handle
		curl_easy_cleanup(curl_tmp);
}

static int sfp_send_http_req_200ok(SOCKET sckt) {
	const char * query = "HTTP/1.0 200 OK\r\n\r\n";

	unsigned int sent = 0;
	int tmp_sent = 0;
	fd_set sckts;
	struct timeval timeout = {SFP_TIMEOUT_SEC, 0};
	int max_sckt = -1;

	FD_ZERO(&sckts);
	FD_SET(sckt, &sckts);
	max_sckt = sckt+1;
	timeout.tv_sec = SFP_TIMEOUT_SEC;
	timeout.tv_usec = 0;

	// send
	while(sent < strlen(query)){
		FD_ZERO(&sckts);
		FD_SET(sckt, &sckts);
		max_sckt = sckt+1;
		timeout.tv_sec = SFP_TIMEOUT_SEC;
		timeout.tv_usec = 0;
		if(select(max_sckt, NULL, &sckts, NULL, &timeout) > 0){
			if((tmp_sent = send(sckt, query, strlen(query)-sent, MSG_NOSIGNAL)) >= 0){
				sent += tmp_sent;
			} else {
				return FAILURE;
			}
		}else{
			FD_CLR(sckt, &sckts);
			m_log_error("Connection timed out", "sfp_send_http_req_200ok");
			return CONNECTION_TIMED_OUT; // fail
		}
	}

	return SUCCESS;
}
/* </julien> */
