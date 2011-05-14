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

#include <owpl_plugin.h>

#include <eXosip/eXosip.h>

#include <sfp-plugin/sfp-plugin.h>
#include <sfp-plugin/sfp-error.h>
#include <sfp-plugin/sfp-parser.h>
#include <sfp-plugin/sfp-transfer.h>

#include <phapi-util/util.h>
#include <phapi-util/mappinglist.h>
#include <phapi-util/mystdio.h>
#include <phapi-util/mystring.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>


#define	IP_ADDR_LENGTH	16
#define	MAX_USERNAME_LENGTH	64
#define	MAX_MESSAGE_LENGTH	4096



/* ----- EVENTS ----- */

int handleEvent(OWPL_EVENT_CATEGORY category,
		void* pInfo,
		void* pUserData);


/* ----- COMMANDS ----- */

int sfp_send_file(const OWPL_LINE hLine,
		  char * uri,
		  char * filename,
		  char * short_filename,
		  char * file_type,
		  char * file_size);

int sfp_receive_file(int cid,
		     const char * filename);

int sfp_cancel_transfer(int call_id);

int sfp_pause_transfer(int call_id);

int sfp_resume_transfer(int call_id);


/* ----- CALLBACKS ----- */

inviteToTransferCb_T inviteToTransfer = 0;
newIncomingFileCb_T newIncomingFile = 0;
waitingForAnswerCb_T waitingForAnswer = 0;
transferCancelledCb_T transferCancelled = 0;
transferCancelledByPeerCb_T transferCancelledByPeer = 0;
sendingFileBeginCb_T sendingFileBegin = 0;
receivingFileBeginCb_T receivingFileBegin = 0;
transferClosedByPeerCb_T transferClosedByPeer = 0;
transferClosedCb_T transferClosed = 0;
transferFromPeerFinishedCb_T transferFromPeerFinished = 0;
transferToPeerFinishedCb_T transferToPeerFinished = 0;
transferFromPeerFailedCb_T transferFromPeerFailed = 0;
transferToPeerFailedCb_T transferToPeerFailed = 0;
transferFromPeerStoppedCb_T transferFromPeerStopped = 0;
transferToPeerStoppedCb_T transferToPeerStopped = 0;
transferProgressionCb_T transferProgression = 0;
transferPausedByPeerCb_T transferPausedByPeer = 0;
transferPausedCb_T transferPaused = 0;
transferResumedByPeerCb_T transferResumedByPeer = 0;
transferResumedCb_T transferResumed = 0;
peerNeedUpgradeCb_T peerNeedUpgrade = 0;
needUpgradeCb_T needUpgrade = 0;


/* ----- PARAMS ----- */

static char sfp_file_transfer_port[6] = "11000"; // Default SFP listening port

static char sfp_default_ip_protocol[4] = "tcp"; // Default SFP transport protocol


/* ----- PLUGIN ----- */

OWPL_PLUGIN_CMD cmds[] = {
	{"sfp_send_file",  sfp_send_file,    6, 0},
	{"sfp_receive_file",      sfp_receive_file,        2, 0},
	{"sfp_cancel_transfer",  sfp_cancel_transfer,    1, 0},
	{"sfp_pause_transfer",    sfp_pause_transfer,     1, 0},
	{"sfp_resume_transfer",    sfp_resume_transfer,     1, 0},
	{0, 0, 0, 0 }
};

OWPL_PLUGIN_CALLBACK callbacks[] = {
	{"inviteToTransfer", (void *)&inviteToTransfer},
	{"newIncomingFile", (void *)&newIncomingFile},
	{"waitingForAnswer", (void *)&waitingForAnswer},
	{"transferCancelled", (void *)&transferCancelled},
	{"transferCancelledByPeer", (void *)&transferCancelledByPeer},
	{"sendingFileBegin", (void *)&sendingFileBegin},
	{"receivingFileBegin", (void *)&receivingFileBegin},
	{"transferClosedByPeer", (void *)&transferClosedByPeer},
	{"transferClosed", (void *)&transferClosed},
	{"transferFromPeerFinished", (void *)&transferFromPeerFinished},
	{"transferToPeerFinished", (void *)&transferToPeerFinished},
	{"transferFromPeerFailed", (void *)&transferFromPeerFailed},
	{"transferToPeerFailed", (void *)&transferToPeerFailed},
	{"transferFromPeerStopped", (void *)&transferFromPeerStopped},
	{"transferToPeerStopped", (void *)&transferToPeerStopped},
	{"transferProgression", (void *)&transferProgression},
	{"transferPausedByPeer", (void *)&transferPausedByPeer},
	{"transferPaused", (void *)&transferPaused},
	{"transferResumedByPeer", (void *)&transferResumedByPeer},
	{"transferResumed", (void *)&transferResumed},
	{"peerNeedUpgrade", (void *)&peerNeedUpgrade},
	{"needUpgrade", (void *)&needUpgrade},
	{0, 0}
};

OWPL_PLUGIN_PARAM params[] = {
	{"sfp_file_transfer_port",      OWPL_PARAM_STR, &sfp_file_transfer_port, sizeof(sfp_file_transfer_port)},
	{"sfp_default_ip_protocol",       OWPL_PARAM_STR, &sfp_default_ip_protocol, sizeof(sfp_default_ip_protocol)},
	{0, 0, 0, 0}
};

OWPL_PLUGIN_CONTENT_TYPE content_type  = {"application/sfp", handleEvent };

OWPL_DECLARE_EXPORT OWPL_PLUGIN_EXPORT exports = {
	"SFPPlugin",
	OWPL_PLUGIN_VERSION,
	&content_type,
	cmds,
	callbacks,
	params,
	0,
	0, /* destroy function */
	0, /* param getter function*/
	0  /* param setter function*/
};



/* ----- IMPLEMENTATION ----- */

#define SFP_INCOMING_INFO		1
#define SFP_OUTGOING_INFO		2
#define SFP_SENDER			1
#define SFP_RECEIVER			2

/**
* A mapping list used to keep track of the file transfer sessions
*/
mappinglist_t * sfp_sessions_by_call_ids = NULL;

/**
 * Mutex used to prevent from pause / resume deadlocks
 */
pthread_mutex_t pause_mutex;
int pause_mutex_initialized = 0;

static void newIncomingFileTransferHandler(int hCall, const char * username, const char * local_ip_address, const char * message);
static void transferAcceptedHandler(int hCall, const char * message);
static void transferRejectedHandler(int hCall);
static void waitingForTransferHandler(int hCall, const char * remote_uri);
static void transferFailureHandler(int hCall);
static void transferClosedHandler(int hCall);
static void transferHoldHandler(int hCall);
static void transferResumedHandler(int hCall);

static void sfp_generate_tranfer_id(char * buffer, const size_t sizeof_buffer);
static sfp_session_info_t * create_sfp_session_info();
static void free_sfp_session_info(sfp_session_info_t ** session);
static unsigned int sfp_add_property(char ** dst, const char * src);
static unsigned int sfp_add_session_info(int call_id, sfp_session_info_t * info);
static unsigned int sfp_remove_session_info(int call_id);
static sfp_session_info_t * sfp_get_session_info(int call_id);
static sfp_session_info_t * sfp_make_session(const char * username, const char * local_ip_adress);
static sfp_session_info_t * sfp_make_session_for_invite(const char * username, const char * local_ip_address, char * filename, char * short_filename, char * file_type, char * file_size, char * bandwidth);
static sfp_session_info_t * sfp_make_session_info_from_body_info(int call_id, sfp_session_info_t * existing_session, sfp_info_t * info, const unsigned int in_or_out);
static sfp_info_t * sfp_make_body_info_from_session_info(sfp_session_info_t * session);
static void sfp_get_file_transfer_port(char buf[]);
static char * sfp_get_default_ip_protocol(void);
static char * sfp_get_default_packet_size(void);
static unsigned int sfp_can_do_tcp(void);
static unsigned int sfp_can_do_udp(void);
static void sfp_receive_terminaison(sfp_session_info_t * session, sfp_returncode_t code);
static void sfp_send_terminaison(sfp_session_info_t * session, sfp_returncode_t code);
static void sfp_progressionCallback(sfp_session_info_t * session, int percentage);
static void sfp_session_updateState(sfp_session_info_t * session, sfp_action_t action);
static unsigned int sfp_session_isInitiated(sfp_session_info_t * session);
static unsigned int sfp_session_isTrying(sfp_session_info_t * session);
static unsigned int sfp_session_isRunning(sfp_session_info_t * session);
static unsigned int sfp_session_isCancelled(sfp_session_info_t * session);
static unsigned int sfp_session_isCancelledByPeer(sfp_session_info_t * session);
static unsigned int sfp_session_isPaused(sfp_session_info_t * session);
static unsigned int sfp_session_isPausedByPeer(sfp_session_info_t * session);
static unsigned int sfp_session_isComplete(sfp_session_info_t * session);
static unsigned int sfp_session_isFinished(sfp_session_info_t * session);
static unsigned int sfp_session_hasFailed(sfp_session_info_t * session);


/**
* Sends a file (send an INVITE to the transfer)
*
* @param	[in]	vlid : a virtual line id
* @param	[in]	filename : the "full" file name (the whole path) to send
* @param	[in]	short_filename : the "short" file name (without the path) to send
* @param	[in]	file_type : the file type
* @param	[in]	file_size : the exact file size in bytes
* @return	the call id if the transfer initiation succeeded
*/
int sfp_send_file(const OWPL_LINE hLine, char * uri, char * filename, char * short_filename, char * file_type, char * file_size){
	char * body;
	int call_id;
	sfp_session_info_t * session;
	sfp_info_t * body_info;

	char szLocalUserName[IP_ADDR_LENGTH];
	int sizeofSzLocalUserName = (int)sizeof(szLocalUserName);
	char szLocalBoundAddr[MAX_USERNAME_LENGTH];

	memset(szLocalUserName, 0, sizeof(szLocalUserName));
	memset(szLocalBoundAddr, 0, sizeof(szLocalBoundAddr));

	if( owplLineGetLocalUserName(hLine, szLocalUserName, &sizeofSzLocalUserName) != OWPL_RESULT_SUCCESS
		|| owplConfigGetBoundLocalAddr(szLocalBoundAddr, sizeof(szLocalBoundAddr))  != OWPL_RESULT_SUCCESS ) {
		return FALSE;
	}

if(strcmp(file_size, "0") != 0) {

	if((session = sfp_make_session_for_invite(szLocalUserName, szLocalBoundAddr, filename, short_filename, file_type, file_size, SFP_REQUIRED_BANDWIDTH_DEFAULT)) == NULL){ // TODO bouchon pour la bandwidth a virer
		m_log_error("Could not create session","sfp_send_file");
		return FALSE;
	}

	if (strfilled(uri)) {
		sfp_add_property(&(session->remote_username), uri);
	}

	// get an available file transfer port
	if(!strfilled(session->http_proxy) && sfp_transfer_get_free_port(session) != SUCCESS){
		m_log_error("Could not find a free transfer port","sfp_send_file");
		free_sfp_session_info(&session);
		return FALSE;
	}

	if((body_info = sfp_make_body_info_from_session_info(session)) == NULL){
		m_log_error("Could not create sfp body info from session","sfp_send_file");
		// free the session
		free_sfp_session_info(&session);
		return FALSE;
	}

	body = sfp_make_message_body_from_sfp_info(body_info);
	if(!strfilled(body)){
		m_log_error("Could not create sfp body from sfp info","sfp_send_file");
		// free the body info
		sfp_free_sfp_info(&body_info);
		// free the session
		free_sfp_session_info(&session);
		return FALSE;
	}

	// we can now free the sent infos
	sfp_free_sfp_info(&body_info);

		// send the INVITE message
	if(owplCallCreate(hLine, (OWPL_CALL *)&call_id) != OWPL_RESULT_SUCCESS) {
		// TODO ERROR
		return FALSE;
	}
	if(call_id <= 0) {
		// TODO ERROR
		return FALSE;
	}
	if(owplCallConnectWithBody((OWPL_CALL)call_id, uri, "application/sfp", body, sizeof(body)) != OWPL_RESULT_SUCCESS) {
		// TODO ERROR
		return FALSE;
	}
	sfp_session_updateState(session, SFP_ACTION_INVITE);

	// we can now free the body
	free(body);

	// set the call_id
	session->call_id = call_id;

	sfp_add_session_info(call_id, session);

	// notify GUI
	if(inviteToTransfer) { inviteToTransfer(call_id, uri, short_filename, file_type, file_size); }

		return call_id;
	}

	return -1;
}

/**
* Receives a file (sends a 200OK and starts a thread to receive the file)
*
* @param	[in]	cid : the call id
* @param	[in]	filename : the "full" file name (the whole path) where to write what has been received
* @return	TRUE if succeeded; FALSE else
*/
int sfp_receive_file(int cid, const char * filename){
	sfp_info_t * to_send_info;
	char * to_send_body;
	sfp_session_info_t * session = NULL;
	pthread_t thread;

	if((session = sfp_get_session_info(cid)) == NULL){
		m_log_error("Could not retrieve the session","sfp_receive_file");
		return FALSE;
	}

	// make the sfp body info from the session
	if((to_send_info = sfp_make_body_info_from_session_info(session)) == NULL){
		m_log_error("Could not create infos to send from the session","sfp_receive_file");
		// free the session
		sfp_remove_session_info(cid);
		return FALSE; // TODO notify GUI
	}

	// make the sfp body to send
	if((to_send_body = sfp_make_message_body_from_sfp_info(to_send_info)) == NULL){
		m_log_error("Could not make the sfp body to send from the sfp body info","sfp_receive_file");
		// free the body info
		sfp_free_sfp_info(&to_send_info);
		// free the session
		sfp_remove_session_info(cid);
		return FALSE; // TODO notify GUI
	}

	// now that the body is done we can free the sfp body info
	sfp_free_sfp_info(&to_send_info);

	// add the place wher we want the file to be stored
	sfp_add_property(&(session->filename), filename);

	// install the terminaison callback
	session->terminaisonCallback = &sfp_receive_terminaison;

	// create the receive thread
	if(pthread_create(&thread, NULL, (void *)sfp_transfer_receive_file, (void *)session) != 0){
		// TODO free le thread
		m_log_error("Could not create receive thread", "sfp_receive_file");
		owplCallDisconnect((OWPL_CALL)cid);
		// TODO notify GUI
		return FALSE;
	}else{
		// notify GUI of the begining of the send
		if(receivingFileBegin) { receivingFileBegin(cid, session->remote_username, session->local_filename, session->file_type, session->file_size); }
	}

	// send the accept answer
	if(owplCallAnswerWithBody((OWPL_CALL)cid, "application/sfp", to_send_body, sizeof(to_send_body)) != OWPL_RESULT_SUCCESS) {
		// TODO ERROR
	}

	// we can now free the body
	free(to_send_body);

	// TODO notify GUI
	return FALSE;
}

/**
* Cancels a file transfer
* (sends a CANCEL if still in the INVITE phase and that we are the sender)
* (sends a DECLINE if still in the INVITE phase and that we are the receiver)
* (tells the transfer thread to stop if already transferring)
*
* @param	[in]	call_id : the call id
* @return	TRUE if succeeded; FALSE else
*/
int sfp_cancel_transfer(int call_id){
	sfp_session_info_t * session = NULL;

	// retrieve the session corresponding to the transfer to stop
	if((session = sfp_get_session_info(call_id)) == NULL){
		m_log_error("Could not retrieve the session","sfp_stop_transfer");
		return FALSE;
	}

	if(session->isInitiated(session)) {
		session->updateState(session, SFP_ACTION_CANCEL);
		if(session->isCancelled(session)) {
			// send a 486 Busy Here
			if(owplCallReject((OWPL_CALL)call_id, 486, "Transfer rejected") != OWPL_RESULT_SUCCESS) {
				return FALSE;
				// TODO ERROR
			}

			// remove the session
			sfp_remove_session_info(call_id);
			return TRUE;
		}
	}else if(session->isRunning(session) || session->isTrying(session)) {
		session->updateState(session, SFP_ACTION_CANCEL);
		if(session->isCancelled(session)) {
			// send a BYE or CANCEL
			if(owplCallDisconnect((OWPL_CALL)call_id) != OWPL_RESULT_SUCCESS) {
				return FALSE;
				// TODO ERROR
			}

			// notify the GUI
			if(transferCancelled) { transferCancelled(call_id, session->local_filename, session->file_type, session->file_size); }

			return TRUE;
		}
	}

	return FALSE;
}

/**
* Pauses a file transfer
* (sends an INVITE containing "holdon")
*
* @param	[in]	call_id : the call id
* @return	TRUE if succeeded; FALSE else
*/
int sfp_pause_transfer(int call_id){
	sfp_session_info_t * session = NULL;

	if((session = sfp_get_session_info(call_id)) == NULL){
		m_log_error("Could not retrieve the session","sfp_pause_transfer");
		return FALSE;
	}

	// FIXME replace with no text, but hold in a header
	// TODO ERROR
	pthread_mutex_lock(&pause_mutex);
	if(( !session->isPaused(session) || !session->isPausedByPeer(session) ) && owplCallHoldWithBody((OWPL_CALL)call_id, "application/sfp", "holdon", 6) == OWPL_RESULT_SUCCESS) {
		session->updateState(session, SFP_ACTION_PAUSE);
		if(session->isPaused(session)) {
			if(transferPaused) { transferPaused(call_id, session->remote_username, session->local_filename, session->file_type, session->file_size); }
				pthread_mutex_unlock(&pause_mutex);
				return TRUE;
		}
	}

	pthread_mutex_unlock(&pause_mutex);
	return FALSE;
}

/**
* Resumes a file transfer
* (sends an INVITE containing "holdoff"))
*
* @param	[in]	call_id : the call id
* @return	TRUE if succeeded; FALSE else
*/
int sfp_resume_transfer(int call_id){
	sfp_session_info_t * session = NULL;

	if((session = sfp_get_session_info(call_id)) == NULL){
		m_log_error("Could not retrieve the session","sfp_resume_transfer");
		return FALSE;
	}

	pthread_mutex_lock(&pause_mutex);
	// FIXME replace with unhold that use header instead of a body
	if(session->isPaused(session) && owplCallUnholdWithBody((OWPL_CALL)call_id, "application/sfp", "holdoff", 7) == OWPL_RESULT_SUCCESS) {
		session->updateState(session, SFP_ACTION_RESUME);
		if(session->isRunning(session)) {
			if(transferResumed) { transferResumed(call_id, session->remote_username, session->local_filename, session->file_type, session->file_size); }
			pthread_mutex_unlock(&pause_mutex);
			return TRUE;
		}
	}
	pthread_mutex_unlock(&pause_mutex);

	return FALSE;
}

/**************** SESSION INFO ****************/

/**
* Creates a new session info
*
* @return	a new allocated session info
*/
static sfp_session_info_t * create_sfp_session_info(){
	sfp_session_info_t * session = NULL;

	if( (session = (sfp_session_info_t *)malloc(sizeof(sfp_session_info_t))) == NULL) {
		m_log_error("Could not allocate memory for the sfp_session_info_t!", "create_sfp_session_info");
		return NULL;
	}

	memset(session, 0, sizeof(sfp_session_info_t));

	session->_state = SFP_SESSION_INITIATED;
	session->progressionCallback = &sfp_progressionCallback;
	session->sendBye = (void(*)(int)) &owplCallDisconnect;

	session->local_socket = -1;

	owplConfigLocalHttpProxyGetAddr(session->http_proxy, sizeof(session->http_proxy));
	owplConfigLocalHttpProxyGetPort(&(session->http_proxy_port));
	owplConfigLocalHttpProxyGetUserName(session->http_proxy_user, sizeof(session->http_proxy_user));
	owplConfigLocalHttpProxyGetPasswd(session->http_proxy_passwd, sizeof(session->http_proxy_passwd));

	session->updateState = &sfp_session_updateState;
	session->isInitiated = &sfp_session_isInitiated;
	session->isTrying = & sfp_session_isTrying;
	session->isRunning = &sfp_session_isRunning;
	session->isCancelled = &sfp_session_isCancelled;
	session->isCancelledByPeer = &sfp_session_isCancelledByPeer;
	session->isPaused = &sfp_session_isPaused;
	session->isPausedByPeer = &sfp_session_isPausedByPeer;
	session->isComplete = &sfp_session_isComplete;
	session->isFinished = &sfp_session_isFinished;
	session->hasFailed = &sfp_session_hasFailed;

	return session;
}

/**
* Frees a session info
* Example
* <pre>
*	sfp_session_info_t * session;
*	free_sfp_session_info(&session);
* </pre>
*
* @param	[in-out]	session : the session info to free
*/
static void free_sfp_session_info(sfp_session_info_t ** session){

	if(!session || !*session) return;

	free((*session)->session_id);
	free((*session)->local_username);
	free((*session)->local_mode);
	free((*session)->local_ip_address_type);
	free((*session)->local_ip);
	free((*session)->local_port);
	free((*session)->remote_username);
	free((*session)->remote_ip_address_type);
	free((*session)->remote_ip);
	free((*session)->remote_port);
	free((*session)->ip_protocol);
	free((*session)->required_bandwidth);
	free((*session)->packet_size);
	free((*session)->key_info);
	free((*session)->crypted_key);
	free((*session)->filename);
	free((*session)->short_filename);
	free((*session)->file_type);
	free((*session)->file_size);
	free(*session);
	*session = NULL;
}

/**
* Adds cleanly textual properties to a sfp sessiçon info
*
* @param	[in-out]	dst : the property receiving the new value
* @param	[in]	src : the new value to be set
* @return	TRUE if the new value could be added; FALSE else
*/
static unsigned int sfp_add_property(char ** dst, const char * src){
	if(strfilled(src)){
		if(strfilled(*dst)){
			free(*dst);
		}
		*dst = strdup(src);
		return TRUE;
	}
	return FALSE;
}

/************ SESSION INFO STORAGE ************/

/**
* Adds a session to the list of created sessions, and associates it to the corresponding call_id
*
* @param	[in]	call_id : the call id
* @param	[in]	info : the session to store
* @return	TRUE if the new session could be added; FALSE else
*/
static unsigned int sfp_add_session_info(int call_id, sfp_session_info_t * info){
	if(sfp_sessions_by_call_ids == NULL){
		sfp_sessions_by_call_ids = create_mappinglist();
	}
	return mappinglist_put_with_int_key(sfp_sessions_by_call_ids, call_id, info);
}

/**
* Removes the session associated to the given call_id
*
* @param	[in]	call_id : the call id
* @return	TRUE if the session could be removed; FALSE else
*/
static unsigned int sfp_remove_session_info(int call_id){
	sfp_session_info_t * session;
	if((session = mappinglist_remove_with_int_key(sfp_sessions_by_call_ids, call_id)) != NULL){
		free_sfp_session_info(&session);
		return TRUE;
	}
	return FALSE;
}

/**
* Retrieves the session associated to the given call_id
*
* @param	[in]	call_id : the call id
* @return	the session info associated to the given call_id; NULL else
*/
static sfp_session_info_t * sfp_get_session_info(int call_id){
	return (sfp_session_info_t *)mappinglist_get_with_int_key(sfp_sessions_by_call_ids, call_id);
}

/**
* Generates a "supposed unique" random id
*
* @param	[out]	buffer : a "supposed unique" random id
*/
static void sfp_generate_tranfer_id(char * buffer, const size_t sizeof_buffer){ // TODO rename to generate_session_id
	int radix = 10;
	//char * buffer = (char *)malloc(sizeof(int)*8+1); // for radix = 2

	srand((unsigned)time(NULL));

	itostr(rand(), buffer, sizeof_buffer, radix);

	//return buffer;
}

/**
* Makes a session with every info available locally
* Not for use on creation of a session to do an INVITE, use sfp_make_session_for_invite(...) instead.
*
* @param	[in]	vlid : a virtual line id
* @return	a new session initialized with every info available locally
*/
static sfp_session_info_t * sfp_make_session(const char * username, const char * local_ip_address){
	char port[16];

	sfp_session_info_t * session = NULL;

	sfp_get_file_transfer_port(port);

	if((session = create_sfp_session_info()) == NULL){
		m_log_error("Could not create sfp_session_info_t", "sfp_make_session");
		return NULL;
	}

	if(!pause_mutex_initialized) {
		pthread_mutex_init(&pause_mutex, NULL);
		pause_mutex_initialized = 1;
	}

	sfp_add_property(&(session->local_username), username);
	sfp_add_property(&(session->local_ip_address_type), SFP_ADDRESS_TYPE_IPV4); // TODO fixed for the moment
	sfp_add_property(&(session->local_ip), local_ip_address);
	sfp_add_property(&(session->local_port), port);
	//sfp_add_property(&(session->local_mode), SFP_MODE_PASSIVE); // mode defaults to passive
	sfp_add_property(&(session->local_mode), SFP_MODE_ACTIVE); // mode defaults to active
	sfp_add_property(&(session->ip_protocol), sfp_get_default_ip_protocol());
	sfp_add_property(&(session->packet_size), sfp_get_default_packet_size());

	return session;
}
/**
* Creates a file transfer session to use before an INVITE, adding all the file information
*
* @param	[in]	vlid : a virtual line id
* @param	[in]	filename : the "full" file name (the whole path)
* @param	[in]	short_filename : the "short" file name (without the path)
* @param	[in]	file_type : the file type
* @param	[in]	file_size : the file size
* @param	[in]	bandwidth : the required bandwidth for the transfer
* @return	a session info that can be used to make an INVITE
*/
static sfp_session_info_t * sfp_make_session_for_invite(const char * username, const char * local_ip_address, char * filename, char * short_filename, char * file_type, char * file_size, char * bandwidth){
	char file_sending_id[12];
	char connection_id[16];
	sfp_session_info_t * session = NULL;

	// args check
	if(!strfilled(filename)) return NULL;
	if(!strfilled(short_filename)) return NULL;
	if(!strfilled(file_type)) return NULL;
	if(!strfilled(file_size)) return NULL;

	sfp_generate_tranfer_id(file_sending_id, sizeof(file_sending_id));

	if((session = sfp_make_session(username, local_ip_address)) == NULL){
		m_log_error("Could not create sfp_session_info_t", "sfp_make_session");
		return NULL;
	}

	/* JULIEN */
	sfp_generate_tranfer_id(connection_id, sizeof(connection_id));
	sfp_add_property(&(session->connection_id), connection_id);
	/* ****** */

	sfp_add_property(&(session->session_id), file_sending_id);
	if(strfilled(bandwidth)){
		sfp_add_property(&(session->required_bandwidth), bandwidth);
	}else{
		sfp_add_property(&(session->required_bandwidth), SFP_REQUIRED_BANDWIDTH_DEFAULT);
	}
	sfp_add_property(&(session->filename), filename);
	sfp_add_property(&(session->local_filename), short_filename);
	sfp_add_property(&(session->remote_filename), short_filename);
	sfp_add_property(&(session->file_type), file_type);
	sfp_add_property(&(session->file_size), file_size);

	return session;
}
/**
* Extracs information useful for a session from a sfp body info.
* Can be used to get info from incoming messages
*
* @param	[in]	call_id : the call_id
* @param	[in-out]	existing_session : a session inf if yet existing
* @param	[in]	info : the info extracted from an SFP body
* @param	[in]	in_or_out : flag indicating wether we are in the case of an incoming or outgoing case, can be SFP_INCOMING_INFO or SFP_OUTGOING_INFO
* @return	a session info containing the proper extracted infos from and sfp body info
*/
static sfp_session_info_t * sfp_make_session_info_from_body_info(int call_id, sfp_session_info_t * existing_session, sfp_info_t * info, const unsigned int in_or_out){
	sfp_session_info_t * session = NULL;

	// try to retrieve an existing session (if exists)
	if(existing_session != NULL){
		session = existing_session;
	}else if( (call_id <= 0) || (call_id > 0 && (session = sfp_get_session_info(call_id)) == NULL) ){
		// else create one
		session = create_sfp_session_info();

		session->local_socket = -1;
	}
	// then if we have a session, fill it
	if(session != NULL && in_or_out == SFP_INCOMING_INFO){ // we are the receiver of the SFP message

		// SESSION ID // TODO necessary ?
		if(strfilled(info->file_sending_id) && !strfilled(session->session_id)){
			sfp_add_property(&(session->session_id), info->file_sending_id);
		}

		// PEER
		if(strfilled(info->username) && !strfilled(session->remote_username))
			sfp_add_property(&(session->remote_username), info->username);

		if (strfilled(info->ip_address) && !strfilled(session->remote_ip)){
			sfp_add_property(&(session->remote_ip), info->ip_address);
		}

		if(strfilled(info->ip_address_type) && !strfilled(session->remote_ip_address_type)){
			sfp_add_property(&(session->remote_ip_address_type), info->ip_address_type);
		}

		if(strfilled(info->address_port) && !strfilled(session->remote_port)){
			sfp_add_property(&(session->remote_port), info->address_port);
		}

		// NETWORK REQUIREMENTS
		// on receive of INVITE fill in what is possible considering the INVITE protocol request
		// on receive of 200OK must accept protocol or if can't, must send a CANCEL
		if(strfilled(info->ip_protocol) && strfilled(session->ip_protocol)){ // receive of 200OK case
			if(strequals(info->ip_protocol, SFP_IP_PROTOCOL_TCP)){
				if(sfp_can_do_tcp()){
					sfp_add_property(&(session->ip_protocol), SFP_IP_PROTOCOL_TCP);
				}else{
					if(owplCallDisconnect((OWPL_CALL)call_id) != OWPL_RESULT_SUCCESS) {
						// TODO ERROR
					}
				}
			}else if(strequals(info->ip_protocol, SFP_IP_PROTOCOL_UDP)){
				if(sfp_can_do_udp()){
					sfp_add_property(&(session->ip_protocol), SFP_IP_PROTOCOL_UDP);
				}else{
					if(owplCallDisconnect((OWPL_CALL)call_id) != OWPL_RESULT_SUCCESS) {
						// TODO ERROR
					}
				}
			}
		}else if(strfilled(info->ip_protocol) && !strfilled(session->ip_protocol)){ // receive of INVITE case
			if(strequals(info->ip_protocol, SFP_IP_PROTOCOL_TCP)){
				if(sfp_can_do_tcp()){
					sfp_add_property(&(session->ip_protocol), SFP_IP_PROTOCOL_TCP);
				}else{
					sfp_add_property(&(session->ip_protocol), SFP_IP_PROTOCOL_UDP);
				}
			}else if(strequals(info->ip_protocol, SFP_IP_PROTOCOL_UDP)){
				if(sfp_can_do_udp()){
					sfp_add_property(&(session->ip_protocol), SFP_IP_PROTOCOL_UDP);
				}else{
					sfp_add_property(&(session->ip_protocol), SFP_IP_PROTOCOL_TCP);
				}
			}
		}

		if(strfilled(info->required_bandwidth) && !strfilled(session->required_bandwidth)){
			sfp_add_property(&(session->required_bandwidth), info->required_bandwidth);
		}else if(strfilled(info->required_bandwidth) && strfilled(session->required_bandwidth)){
			// TODO change to min because required_bandwith will become max_bandwidth_use
			// take the max
			sfp_add_property(&(session->required_bandwidth), str_int_max(session->required_bandwidth, info->required_bandwidth));
		}

		if(strfilled(info->packet_size) && !strfilled(session->packet_size)){
			sfp_add_property(&(session->packet_size), info->packet_size);
		}else if(strfilled(info->packet_size) && strfilled(session->packet_size)){
			// take the min
			sfp_add_property(&(session->packet_size), str_int_min(session->packet_size, info->packet_size));
		}

		// FILE CONCERNS
		if(strfilled(info->filename) && !strfilled(session->remote_filename)) {
			sfp_add_property(&(session->remote_filename), info->filename);
		}
		if(strfilled(info->filename) && !strfilled(session->local_filename)) {
#ifdef WIN32
			clean_filename_for_windows(info->filename, sizeof(info->filename));
#endif /* WIN32 */
			sfp_add_property(&(session->local_filename), info->filename);
		}

		if(strfilled(info->file_type) && !strfilled(session->file_type))
			sfp_add_property(&(session->file_type), info->file_type);

		if(strfilled(info->file_size) && !strfilled(session->file_size))
			sfp_add_property(&(session->file_size), info->file_size);

		// MODE
		if(strfilled(info->mode)){
			if(strequals(info->mode, SFP_MODE_ACTIVE)){
				sfp_add_property(&(session->local_mode), SFP_MODE_PASSIVE);
			}else if(strequals(info->mode, SFP_MODE_PASSIVE)){
				sfp_add_property(&(session->local_mode), SFP_MODE_ACTIVE);
			}
		}

		// CONNECTION_ID
		if(strfilled(info->connection_id))
			sfp_add_property(&(session->connection_id), info->connection_id);


		// TODO key and uri
	}
	else if(session != NULL && in_or_out == SFP_OUTGOING_INFO){ // we are the sender of the SFP message

		// LOCAL INFO
		if(strfilled(info->username) && !strfilled(session->local_username))
			sfp_add_property(&(session->local_username), info->username);

		if(strfilled(info->ip_address_type) && !strfilled(session->local_ip_address_type))
			sfp_add_property(&(session->local_ip_address_type), info->ip_address_type);

		if(strfilled(info->ip_address) && !strfilled(session->local_ip))
			sfp_add_property(&(session->local_ip), info->ip_address);

		if(strfilled(info->address_port) && !strfilled(session->local_port))
			sfp_add_property(&(session->local_port), info->address_port);

		// NETWORK REQUIREMENTS
		// for sending INVITE set what the top level program gave
		// for sending 200OK it's been done when on receiving INVITE, non need to do anything more
		if(strfilled(info->ip_protocol) && !strfilled(session->ip_protocol)) // sending INVITE case (set by top level program)
			sfp_add_property(&(session->ip_protocol), info->ip_protocol);

		if(strfilled(info->required_bandwidth) && !strfilled(session->required_bandwidth)){ // sending INVITE case
			sfp_add_property(&(session->required_bandwidth), info->required_bandwidth);
		}else if(strfilled(info->required_bandwidth) && strfilled(session->required_bandwidth)){ // sending 200OK case
			// TODO change to min because required_bandwith will become max_bandwidth_use
			// take the max
			sfp_add_property(&(session->required_bandwidth), str_int_max(session->required_bandwidth, info->required_bandwidth));
		}

		if(strfilled(info->packet_size) && !strfilled(session->packet_size)){ // sending INVITE case
			sfp_add_property(&(session->packet_size), info->packet_size);
		}else if(strfilled(info->packet_size) && strfilled(session->packet_size)){ // sending 200OK case
			// take the min
			sfp_add_property(&(session->packet_size), str_int_min(session->packet_size, info->packet_size));
		}

		// FILE CONCERNS
		if(strfilled(info->filename) && !strfilled(session->remote_filename)) {
			sfp_add_property(&(session->remote_filename), info->filename);
		}
		if(strfilled(info->filename) && !strfilled(session->local_filename)) {
#ifdef WIN32
			clean_filename_for_windows(info->filename, sizeof(info->filename));
#endif /* WIN32 */
			sfp_add_property(&(session->local_filename), info->filename);
		}

		if(strfilled(info->file_type) && !strfilled(session->file_type))
			sfp_add_property(&(session->file_type), info->file_type);

		if(strfilled(info->file_size) && !strfilled(session->file_size))
			sfp_add_property(&(session->file_size), info->file_size);

		// CONNECTION_ID
		if(strfilled(info->connection_id))
			sfp_add_property(&(session->connection_id), info->connection_id);

		// TODO key and uri
	}

	return session;
}


/**
* Extracs information useful for a sfp body info from a session.
* Can be used to make outgoing messages.
*
* @param	[in]	session : a session info
* @return	a sfp body info ready to be used for an outgoing message
*/
static sfp_info_t * sfp_make_body_info_from_session_info(sfp_session_info_t * session){
	sfp_info_t * body_info = NULL;

	if((body_info = sfp_create_empty_sfp_info()) == NULL){
		m_log_error("Could not create sfp_info_t", "sfp_make_body_info_from_session_info");
		return NULL;
	}

	sfp_add_version_info(body_info, SFP_PROTOCOL_VERSION);
	if(strfilled(session->session_id) &&
		strfilled(session->local_username) &&
		strfilled(session->local_ip_address_type) &&
		strfilled(session->local_ip) &&
		strfilled(session->local_port) &&
		strfilled(session->connection_id))
		// TODO network type fixed for the moment
		sfp_add_origin_info(body_info, session->session_id, session->local_username, SFP_NETWORK_TYPE_INTERNET, session->local_ip_address_type, session->local_ip, session->local_port, session->connection_id);

	if(strfilled(session->local_mode))
		sfp_add_mode_info(body_info, session->local_mode);

	if(strfilled(session->ip_protocol) &&
		strfilled(session->required_bandwidth) &&
		strfilled(session->packet_size))
		sfp_add_transfer_info(body_info, session->ip_protocol, session->required_bandwidth, session->packet_size);

	if(strfilled(session->remote_filename) &&
		strfilled(session->file_type) &&
		strfilled(session->file_size))
		sfp_add_file_info(body_info, session->remote_filename, session->file_type, session->file_size);

	return body_info;
}
/**
* Gives an available port for the file transfer.
* WARNING always give the same port; function not doing its work yet
* TODO really try to find an available port by trying to bind, and store the socket for later use
*
* @param	[out]	buf : the buffer that will contain the port
*/
static void sfp_get_file_transfer_port(char buf[]){
	int port = atoi(sfp_file_transfer_port);

	buf[0] = 0;

	// TODO replace with some function that checks wether the port is already in use in some file transfer session ?
	sprintf(buf, "%d", port);
	return;
}

/**
* Gets the default protocol to use for file transfers
*
* @return	the default protocol to be used : either SFP_IP_PROTOCOL_TCP or SFP_IP_PROTOCOL_UDP
*/
static char * sfp_get_default_ip_protocol(void){
	if(strfilled(sfp_default_ip_protocol)){
		return sfp_default_ip_protocol;
	}
	// else default to TCP
	return SFP_IP_PROTOCOL_TCP;
}
/**
* Gets the default packet size for UDP transfers
*
* @return	the default packet size for UDP transfers
*/
static char * sfp_get_default_packet_size(void){ // TODO rename to sfp_get_default_udp_packet_size
	return "0";
}

/**
* Indicates wether the client can do TCP file transfers or not
* TODO for the moment, always says TRUE
*
* @return	TRUE if TCP file transfers can be done; FALSE else
*/
static unsigned int sfp_can_do_tcp(void){
	return TRUE; // TODO bouchon; set par le prog top level, a proposer en service du plugin
}

/**
* Indicates wether the client can do UDP file transfers or not
* TODO for the moment, always says FALSE
*
* @return	TRUE if UDP file transfers can be done; FALSE else
*/
static unsigned int sfp_can_do_udp(void){
	return FALSE; // TODO bouchon; set par le prog top level, a proposer en service du plugin
}
/**
* Callback that get called when the receive thread ends.
* Notifies the top level program, and does some cleanup.
*
* @param	[in-out]	session : the session info
* @param	[in]	code : the return code of the thread (TRUE or FALSE for the moment)
*/
static void sfp_receive_terminaison(sfp_session_info_t * session, sfp_returncode_t code) {
	int call_id = -1;

	if(session != NULL) {
		call_id = session->call_id;
	} else {
		m_log_error("No given session","sfp_receive_terminaison");
		return;
	}

	if(code != SUCCESS) {
		if(transferFromPeerFailed) { transferFromPeerFailed(call_id, session->remote_username, session->local_filename, session->file_type, session->file_size); }
		// MAYBE send a BYE?
		remove(session->filename);
	} else if(code == SUCCESS) {
		if(session->isCancelled(session)) {
			if(transferCancelled) { transferCancelled(call_id, session->local_filename, session->file_type, session->file_size); }
			remove(session->filename);
		} else if(session->isCancelledByPeer(session)) {
			if(transferCancelledByPeer) { transferCancelledByPeer(call_id, session->remote_username, session->local_filename, session->file_type, session->file_size); }
			remove(session->filename);
		} else if(session->isFinished(session)) {
			if(transferFromPeerFinished) { transferFromPeerFinished(call_id, session->remote_username, session->local_filename, session->file_type, session->file_size); }
		} else {
			if(transferFromPeerFailed) { transferFromPeerFailed(call_id, session->remote_username, session->local_filename, session->file_type, session->file_size); }
			// MAYBE send a BYE?
			remove(session->filename);
		}
	}

	sfp_remove_session_info(session->call_id);
}

/**
* Callback that get called when the send thread ends.
* Notifies the top level program, and does some cleanup.
*
* @param	[in-out]	session : the session info
* @param	[in]	code : the return code of the thread (TRUE or FALSE for the moment)
*/
static void sfp_send_terminaison(sfp_session_info_t * session, sfp_returncode_t code){
	int call_id = -1;

	if(session != NULL){
		call_id = session->call_id;
	}else{
		m_log_error("No given session","sfp_send_terminaison");
		return;
	}

	if(code != SUCCESS) {
		if(transferToPeerFailed) { transferToPeerFailed(call_id, session->remote_username, session->local_filename, session->file_type, session->file_size); }
		if(owplCallDisconnect((OWPL_CALL)call_id) != OWPL_RESULT_SUCCESS) {
			// TODO ERROR
		}
	} else if(code == SUCCESS) {
		if(session->isCancelled(session)) {
			if(transferCancelled) { transferCancelled(call_id, session->local_filename, session->file_type, session->file_size); }
		} else if(session->isCancelledByPeer(session)) {
			if(transferCancelledByPeer) { transferCancelledByPeer(call_id, session->remote_username, session->local_filename, session->file_type, session->file_size); }
		} else if(session->isFinished(session)) {
			if(transferToPeerFinished) { transferToPeerFinished(call_id, session->remote_username, session->local_filename, session->file_type, session->file_size); }
		} else {
			if(transferToPeerFailed) { transferToPeerFailed(call_id, session->remote_username, session->local_filename, session->file_type, session->file_size); }
			if(owplCallDisconnect((OWPL_CALL)call_id) != OWPL_RESULT_SUCCESS) {
				// TODO ERROR
			};
		}
	}

	// suppress the session
	sfp_remove_session_info(call_id);
}
/**
* Callback that get called by the transfer thread to notify top level program of the progression of the transfer
*
* @param	[in]	session : the session info
* @param	[in]	percentage : the percentage of transfer progress
*/
static void sfp_progressionCallback(sfp_session_info_t * session, int percentage){
	if(transferProgression) { transferProgression(session->call_id, percentage); }
}

/************* PLUGIN CALLBACKS **************/


int handleEvent(OWPL_EVENT_CATEGORY category, void* pInfo, void* pUserData){
	OWPL_CALLSTATE_INFO * info = NULL;

	char szLocalBoundAddr[IP_ADDR_LENGTH];
	char szLocalUserName[MAX_USERNAME_LENGTH];
	int sizeofSzLocalUserName = (int)sizeof(szLocalUserName);
	char szMessage[MAX_MESSAGE_LENGTH];


	if(category == EVENT_CATEGORY_CALLSTATE){
		if(pInfo == NULL) {
			// TODO ERROR
			return -1;
		}
		info = (OWPL_CALLSTATE_INFO *)pInfo;

		switch(info->event) {
			/*case CALLSTATE_NEWCALL : // a new invite to transfer a file has been received
				switch(info->cause) {
					case CALLSTATE_NEW_CALL_NORMAL :
						newIncomingFileTransferHandler();
						break;
					default :
						break;
				}
				break;*/
			case CALLSTATE_OFFERING : // a new invite to transfer a file has been received
				if(info->pData == NULL) {
					// TODO ERROR
					return -1;
				}
				memset(szMessage, 0, sizeof(szMessage));
				strncpy(szMessage, (const char *)info->pData, sizeof(szMessage)-1);
				memset(szLocalBoundAddr, 0, sizeof(szLocalBoundAddr));
				memset(szLocalUserName, 0, sizeof(szLocalUserName));
				if( owplLineGetLocalUserName(info->hLine, szLocalUserName, &sizeofSzLocalUserName) == OWPL_RESULT_SUCCESS
					&& owplConfigGetBoundLocalAddr(szLocalBoundAddr, sizeof(szLocalBoundAddr))  == OWPL_RESULT_SUCCESS ) {
						newIncomingFileTransferHandler(info->hCall, szLocalUserName, szLocalBoundAddr, szMessage);
				}
				break;
			case CALLSTATE_REMOTE_OFFERING : // invite sent, waiting for answer
				waitingForTransferHandler(info->hCall, info->szRemoteIdentity);
				break;
			case CALLSTATE_REMOTE_ALERTING : // waiting for peer to accept / reject the file transfer
				waitingForTransferHandler(info->hCall, info->szRemoteIdentity);
				break;
			case CALLSTATE_ALERTING :
				waitingForTransferHandler(info->hCall, info->szRemoteIdentity);
				break;
			case CALLSTATE_CONNECTED : // transfer accepted
				if(info->pData == NULL) {
					// TODO ERROR
					return -1;
				}
				memset(szMessage, 0, sizeof(szMessage));
				strncpy(szMessage, (const char *)info->pData, sizeof(szMessage)-1);
				transferAcceptedHandler(info->hCall, szMessage);
				break;
			case CALLSTATE_DISCONNECTED :
				switch(info->cause) {
					case CALLSTATE_DISCONNECTED_BUSY :
						transferRejectedHandler(info->hCall);
						break;
					case CALLSTATE_DISCONNECTED_NORMAL :
						transferClosedHandler(info->hCall);
						break;
					default :
						transferFailureHandler(info->hCall);
						break;
				}

			case CALLSTATE_HOLD :
				switch(info->cause) {
					case CALLSTATE_HOLD_STARTED :
						transferHoldHandler(info->hCall);
						break;
					case CALLSTATE_HOLD_RESUMED :
						transferResumedHandler(info->hCall);
						break;
					default :
						break;
				}
				break;
			default :
				break;

		}

	}
	// TODO RETURNCODE
	return 0;
}

/**
 * Handler called when a new invite to a file transfer has been received.
 * Parses the SFP message. Creates a transfer session. Notifies the top level program that an
 * invite to a file transfer arrived.
 *
 */
static void newIncomingFileTransferHandler(int hCall, const char * username, const char * local_ip_address, const char * message){
	sfp_info_t * received_info;
	sfp_session_info_t * session = NULL;

	// get the body and parse it
	if(!strfilled(message)){
		m_log_error("No body in the event","newIncomingFileTransferHandler");
		return; // TODO notify GUI
	}
	if((received_info = sfp_parse_message((char *)message)) == NULL){
		m_log_error("Could not parse sfp body","newIncomingFileTransferHandler");
		return; // TODO notify GUI
	}

	// create a new session
	if((session = sfp_make_session(username, local_ip_address)) == NULL){
		m_log_error("Could not create session","newIncomingFileTransferHandler");
		// free the received infos
		sfp_free_sfp_info(&received_info);
		return; // TODO notify GUI
	}

	// set the call_id
	session->call_id = hCall;

	// set the local usable port
	if(!strfilled(session->http_proxy) && sfp_transfer_get_free_port(session) != SUCCESS){
		m_log_error("Could not find a free port","newIncomingFileTransferHandler");
		// free the received infos
		sfp_free_sfp_info(&received_info);
		free_sfp_session_info(&session);
		return; // TODO notify GUI
	}

	// store the session
	sfp_add_session_info(hCall, session);

	// store incoming body infos into a session (which will normally be the one we created and stored upper)
	if((session = sfp_make_session_info_from_body_info(hCall, session, received_info, SFP_INCOMING_INFO)) == NULL){
		m_log_error("Could not add incoming infos to the session","newIncomingFileTransferHandler");
		return; // TODO notify GUI
	}

	// the sfp incoming infos are stored, we can free info
	sfp_free_sfp_info(&received_info);

	// TODO notify GUI
	if(newIncomingFile) { newIncomingFile(hCall, session->remote_username, session->local_filename, session->file_type, session->file_size); }
}
/**
 * Handler called when a file transfer has been accepted by peer
 * Parses the SFP message. Adds information to the session. Starts the receive thread. Notifies the top level program
 *
 */
static void transferAcceptedHandler(int hCall, const char * message){
	sfp_info_t * received_info;
	sfp_session_info_t * session = NULL;
	pthread_t thread;

	// retrieve the corresponding session
	if((hCall <= 0) || (hCall > 0 && (session = sfp_get_session_info(hCall)) == NULL)){
		m_log_error("Could not retrieve session","transferAcceptedHandler");
		return;
	}

	if(session->isTrying(session)){ // INVITE has been sent, waiting for a 200 OK to start the transfer

		// get the body and parse it
		if(!strfilled(message)){
			m_log_error("No body in the event","transferAcceptedHandler");
			return; // TODO notify GUI
		}
		if((received_info = sfp_parse_message((char *)message)) == NULL){
			m_log_error("Could not parse sfp body","transferAcceptedHandler");
			return; // TODO notify GUI
		}

		// store incoming body infos into the session we retrieved before
		if((session = sfp_make_session_info_from_body_info(hCall, session, received_info, SFP_INCOMING_INFO)) == NULL){
			m_log_error("Could not add incoming infos to the session","newIncomingFileTransferHandler");
			// free the received info
			sfp_free_sfp_info(&received_info);
			// remove the session
			sfp_remove_session_info(hCall);
			return; // TODO notify GUI
		}

		// the sfp incoming infos are stored, we can free info
		sfp_free_sfp_info(&received_info);

		// install the terminaison callback
		session->terminaisonCallback = &sfp_send_terminaison;

		// create the send thread
		if(pthread_create(&thread, NULL, (void *)sfp_transfer_send_file, (void *)session) != 0){
			// TODO free le thread
			m_log_error("Could not create send thread", "transferAcceptedHandler");
			if(owplCallDisconnect((OWPL_CALL)hCall) != OWPL_RESULT_SUCCESS) {
				// TODO ERROR
			}

			if(transferToPeerFailed) { transferToPeerFailed(hCall, session->remote_username, session->local_filename, session->file_type, session->file_size); }
			sfp_remove_session_info(hCall);

			return;
		}else{
			if(sendingFileBegin) { sendingFileBegin(hCall, session->remote_username, session->local_filename, session->file_type, session->file_size);	}
		}
	}
}

/**
 * Handler called if the peer refused the file transfer
 *
 */
static void transferRejectedHandler(int hCall) {
	// TODO IMPLEMENT
}
/**
 * Handler called after the invite to a new file transfer has been sent
 * and before it has been accepted or rejected by peer
 *
 */
static void waitingForTransferHandler(int hCall, const char * remote_uri){
	sfp_session_info_t * session = NULL;

	// retrieve the corresponding session
	if((hCall <= 0) || (hCall > 0 && (session = sfp_get_session_info(hCall)) == NULL)){
		m_log_error("Could not retrieve session","transferAcceptedHandler");
		return; // TODO notify GUI
	}

	if(session->isInitiated(session)){
		if(waitingForAnswer) { waitingForAnswer(hCall, (char *)remote_uri); }
	}
}

/**
 * Callback called if the transfer fails
 *
 */
static void transferFailureHandler(int hCall){
	sfp_session_info_t * session = NULL;

	// retrieve the corresponding session
	if((hCall <= 0) || (hCall > 0 && (session = sfp_get_session_info(hCall)) == NULL)){
		m_log_error("Could not retrieve session","transferAcceptedHandler");
		return; // TODO notify GUI
	}

	if(transferToPeerFailed) { transferToPeerFailed(hCall, session->remote_username, session->local_filename, session->file_type, session->file_size); }

	sfp_remove_session_info(hCall);
}

/**
 * Callback called on receive of a CANCEL or a BYE.
 * Notifies the thread if running. Makes some clean up.
 *
 */
static void transferClosedHandler(int hCall){ // BYE received
	sfp_session_info_t * session = NULL;

	// retrieve the corresponding session
	if((hCall <= 0) || (hCall > 0 && (session = sfp_get_session_info(hCall)) == NULL)){
		m_log_error("Could not retrieve session","transferAcceptedHandler");
		return; // TODO notify GUI
	}

	if(session->isInitiated(session)) {
		session->updateState(session, SFP_ACTION_BYE_OR_CANCEL_RECEIVED);
		if(session->isCancelledByPeer(session)) {
			if(transferCancelledByPeer) { transferCancelledByPeer(hCall, session->remote_username, session->local_filename, session->file_type, session->file_size); }
			sfp_remove_session_info(hCall);
			//owplCallDisconnect(hCall);
		}

	} else if(session->isRunning(session)) {
		session->updateState(session, SFP_ACTION_BYE_OR_CANCEL_RECEIVED);
		/*if(session->isCancelledByPeer(session)) {
			if(transferCancelledByPeer) { transferCancelledByPeer(hCall, session->remote_username, session->local_filename, session->file_type, session->file_size); }
			owplCallDisconnect(hCall);
		}*/
	} else {
		session->updateState(session, SFP_ACTION_BYE_OR_CANCEL_RECEIVED);
	}
}

/**
 * Handler called when a transfer is put on hold
 *
 */
static void transferHoldHandler(int hCall){
	sfp_session_info_t * session = NULL;

	// retrieve the corresponding session
	if((hCall <= 0) || (hCall > 0 && (session = sfp_get_session_info(hCall)) == NULL)){
		m_log_error("Could not retrieve session","transferAcceptedHandler");
		return; // TODO notify GUI
	}

	pthread_mutex_lock(&pause_mutex);

	if(!session->isPaused(session) || !session->isPausedByPeer(session)) {
		session->updateState(session, SFP_ACTION_HOLDON_RECEIVED);
		if(session->isPausedByPeer(session)) {
			if(transferPausedByPeer) { transferPausedByPeer(hCall, session->remote_username, session->local_filename, session->file_type, session->file_size); }
		}
	} else if(session->isPaused(session)) {
		session->updateState(session, SFP_ACTION_RESUME);
		if(session->isRunning(session)) {
			if(transferResumed) { transferResumed(hCall, session->remote_username, session->local_filename, session->file_type, session->file_size); }
		}
	}

	pthread_mutex_unlock(&pause_mutex);
}


/**
 * Handler callde when a transfer is resumed
 *
 */
static void transferResumedHandler(int hCall){
	sfp_session_info_t * session = NULL;

	// retrieve the corresponding session
	if((hCall <= 0) || (hCall > 0 && (session = sfp_get_session_info(hCall)) == NULL)){
		m_log_error("Could not retrieve session","transferAcceptedHandler");
		return; // TODO notify GUI
	}

	pthread_mutex_lock(&pause_mutex);
	if(session->isPausedByPeer(session)) {
		session->updateState(session, SFP_ACTION_HOLDOFF_RECEIVED);
		if(session->isRunning(session)) {
			if(transferResumedByPeer) { transferResumedByPeer(hCall, session->remote_username, session->local_filename, session->file_type, session->file_size); }
		}
	}
	pthread_mutex_unlock(&pause_mutex);

}


/* ----- FUNCTIONS TO MANIPULATE A SFP_SESSION_INFO_T ----- */

static void sfp_session_updateState(sfp_session_info_t * session, sfp_action_t action) {
	switch(action) {

		case SFP_ACTION_INVITE :
			switch(session->_state) {
				case SFP_SESSION_INITIATED :
					session->_state = SFP_SESSION_TRYING;
					break;
				default :
					break;
			}
			break;

		case SFP_ACTION_START :
			switch(session->_state) {
				case SFP_SESSION_INITIATED :
					session->_state = SFP_SESSION_RUNNING;
					break;
				case SFP_SESSION_TRYING :
					session->_state = SFP_SESSION_RUNNING;
					break;
				default :
					break;
			}
			break;

		case SFP_ACTION_CANCEL :
			switch(session->_state) {
				case SFP_SESSION_INITIATED :
					session->_state = SFP_SESSION_CANCELLED;
					break;
				case SFP_SESSION_TRYING :
					session->_state = SFP_SESSION_CANCELLED;
					break;
				case SFP_SESSION_RUNNING :
					session->_state = SFP_SESSION_CANCELLED;
					break;
				default :
					break;
			}
			break;

		case SFP_ACTION_BYE_OR_CANCEL_RECEIVED :
			switch(session->_state) {
				case SFP_SESSION_INITIATED :
					session->_state = SFP_SESSION_CANCELLED_BY_PEER;
					break;
				case SFP_SESSION_TRYING :
					session->_state = SFP_SESSION_CANCELLED_BY_PEER;
					break;
				case SFP_SESSION_RUNNING :
					session->_state = SFP_SESSION_CANCELLED_BY_PEER;
					break;
				case SFP_SESSION_COMPLETE :
					session->_state = SFP_SESSION_FINISHED;
					break;
				case SFP_SESSION_PAUSED :
					session->_state = SFP_SESSION_CANCELLED_BY_PEER;
					break;
				case SFP_SESSION_PAUSED_BY_PEER :
					session->_state = SFP_SESSION_CANCELLED_BY_PEER;
					break;
				default :
					session->_state = SFP_SESSION_FAILED;
					break;
			}
			break;

		case SFP_ACTION_PAUSE :
			switch(session->_state) {
				case SFP_SESSION_RUNNING :
					session->_state = SFP_SESSION_PAUSED;
					break;
				default :
					break;
			}
			break;

		case SFP_ACTION_HOLDON_RECEIVED :
			switch(session->_state) {
				case SFP_SESSION_RUNNING :
					session->_state = SFP_SESSION_PAUSED_BY_PEER;
					break;
				default :
					break;
			}
			break;

		case SFP_ACTION_RESUME :
			switch(session->_state) {
				case SFP_SESSION_PAUSED :
					session->_state = SFP_SESSION_RUNNING;
					break;
				default :
					break;
			}
			break;

		case SFP_ACTION_HOLDOFF_RECEIVED :
			switch(session->_state) {
				case SFP_SESSION_PAUSED_BY_PEER :
					session->_state = SFP_SESSION_RUNNING;
					break;
				default :
					break;
			}
			break;

		case SFP_ACTION_SOCKET_CLOSED :
			switch(session->_state) {
				case SFP_SESSION_COMPLETE :
					session->_state = SFP_SESSION_FINISHED;
					break;
				case SFP_SESSION_RUNNING :
					session->_state = SFP_SESSION_FAILED;
					break;
				case SFP_SESSION_PAUSED :
					session->_state = SFP_SESSION_FAILED;
					break;
				case SFP_SESSION_PAUSED_BY_PEER :
					session->_state = SFP_SESSION_FAILED;
					break;
				default :
					break;
			}
			break;

		case SFP_ACTION_TRANSFER_COMPLETED :
			switch(session->_state) {
				case SFP_SESSION_RUNNING :
					session->_state = SFP_SESSION_COMPLETE;
					break;
				default :
					break;
			}
			break;

		default :
			break;
	}
}

static unsigned int sfp_session_isInitiated(sfp_session_info_t * session) {
	if(session->_state == SFP_SESSION_INITIATED) {
		return TRUE;
	}
	return FALSE;
}

static unsigned int sfp_session_isTrying(sfp_session_info_t * session) {
	if(session->_state == SFP_SESSION_TRYING) {
		return TRUE;
	}
	return FALSE;
}

static unsigned int sfp_session_isRunning(sfp_session_info_t * session) {
	if(session->_state == SFP_SESSION_RUNNING) {
		return TRUE;
	}
	return FALSE;
}

static unsigned int sfp_session_isCancelled(sfp_session_info_t * session) {
	if(session->_state == SFP_SESSION_CANCELLED) {
		return TRUE;
	}
	return FALSE;
}

static unsigned int sfp_session_isCancelledByPeer(sfp_session_info_t * session) {
	if(session->_state == SFP_SESSION_CANCELLED_BY_PEER) {
		return TRUE;
	}
	return FALSE;
}

static unsigned int sfp_session_isPaused(sfp_session_info_t * session) {
	if(session->_state == SFP_SESSION_PAUSED) {
		return TRUE;
	}
	return FALSE;
}

static unsigned int sfp_session_isPausedByPeer(sfp_session_info_t * session) {
	if(session->_state == SFP_SESSION_PAUSED_BY_PEER) {
		return TRUE;
	}
	return FALSE;
}

static unsigned int sfp_session_isComplete(sfp_session_info_t * session) {
	if(session->_state == SFP_SESSION_COMPLETE) {
		return TRUE;
	}
	return FALSE;
}

static unsigned int sfp_session_isFinished(sfp_session_info_t * session) {
	if(session->_state == SFP_SESSION_FINISHED) {
		return TRUE;
	}
	return FALSE;
}

static unsigned int sfp_session_hasFailed(sfp_session_info_t * session) {
	if(session->_state == SFP_SESSION_FAILED) {
		return TRUE;
	}
	return FALSE;
}
