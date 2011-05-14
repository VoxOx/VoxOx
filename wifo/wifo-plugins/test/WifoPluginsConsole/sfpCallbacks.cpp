#include "sfpCallbacks.h"

#include <stdio.h>
#include <string.h>

sfp_callbacks_t sfp_cbks = {
	inviteToTransfer,
	newIncomingFile,
	waitingForAnswer,
	transferCancelled,
	transferCancelledByPeer,
	sendingFileBegin,
	transferClosedByPeer,
	transferClosed,
	transferFromPeerFinished,
	transferToPeerFinished,
	transferFromPeerFailed,
	transferToPeerFailed,
	transferFromPeerStopped,
	transferToPeerStopped,
	transferProgression,
	transferPausedByPeer,
	transferPaused,
	transferResumedByPeer,
	transferResumed
};

static void set_callid_for_wxwengouser(const char * username, int call_id){
	users_to_callids[username[9]] = call_id;
}

static void set_callid_from_wxwengouser_uri(const char * uri, int call_id){
	users_to_callids[uri[14]] = call_id;
}

/**
* TODO doc
*/
void inviteToTransfer(int cid, char * uri, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_from_wxwengouser_uri(uri, cid);

	printf(" * Inviting %s to get the file %s (TYPE, %s bytes)\n> ", uri, short_filename, file_size);
}

/**
* TODO doc
*/
void newIncomingFile(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	strncpy(step, "accept", sizeof(step));

	printf(" * User %s wants to send you the file %s (TYPE, %s bytes).\n> ", username, short_filename, file_size);
}

/**
* TODO doc
*/
void waitingForAnswer(int cid, char * uri){
	//g_callId = cid;
	set_callid_from_wxwengouser_uri(uri, cid);

	printf(" * Waiting for %s to accept the file\n> ", uri);
}

/**
* TODO doc
*/
void transferCancelled(int cid, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;

	strncpy(step, "send", sizeof(step));

	printf(" * You cancelled the transfer of the file %s (TYPE, %s)\n> ", short_filename, file_size);
}

/**
* TODO doc
*/
void transferCancelledByPeer(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	strncpy(step, "send", sizeof(step));

	printf(" * The user %s cancelled the transfer of the file %s (TYPE, %s)\n> ", username, short_filename, file_size);
}

/**
* TODO doc
*/
void sendingFileBegin(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	printf(" * The file %s (TYPE, %s) is being sent to %s\n> ", short_filename, file_size, username);
}

/**
* TODO doc
*/
void transferClosed(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	strncpy(step, "send", sizeof(step));

	printf(" * You closed the transfer of the file %s (TYPE, %s) to/from %s\n> ", username, short_filename, file_size);
}

/**
* TODO doc
*/
void transferClosedByPeer(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	strncpy(step, "send", sizeof(step));

	printf(" * The user %s closed the transfer of the file %s (TYPE, %s)\n> ", username, short_filename, file_size);
}

/**
* TODO doc
*/
void transferFromPeerFinished(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	strncpy(step, "send", sizeof(step));

	printf(" * Transfer of the file %s (TYPE, %s) from %s finished\n> ", short_filename, file_size, username);
}

/**
* TODO doc
*/
void transferToPeerFinished(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	strncpy(step, "send", sizeof(step));

	printf(" * Transfer of the file %s (TYPE, %s) to %s finished\n> ", short_filename, file_size, username);
}
/**
* TODO doc
*/
void transferFromPeerFailed(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	strncpy(step, "send", sizeof(step));

	printf(" * Transfer of the file %s (TYPE, %s) from %s failed\n> ", short_filename, file_size, username);
}
/**
* TODO doc
*/
void transferToPeerFailed(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	strncpy(step, "send", sizeof(step));

	printf(" * Transfer of the file %s (TYPE, %s) to %s failed\n> ", short_filename, file_size, username);
}

/**
* TODO doc
*/
void transferFromPeerStopped(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	strncpy(step, "send", sizeof(step));

	printf(" * Transfer of the file %s (TYPE, %s) from %s has been stopped\n> ", short_filename, file_size, username);
}

/**
* TODO doc
*/
void transferToPeerStopped(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	strncpy(step, "send", sizeof(step));

	printf(" * Transfer of the file %s (TYPE, %s) to %s has been stopped\n> ", short_filename, file_size, username);
}

/**
* TODO doc
*/
void transferProgression(int cid, int percentage){
	//g_callId = cid;

	printf(" * Transfer reached %d %%\n> ", percentage);
}


void transferPausedByPeer(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	printf(" * The user %s paused the transfer of the file %s (TYPE, %s)\n> ", username, short_filename, file_size);
}


void transferPaused(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	printf(" * You paused the transfer of the file %s (TYPE, %s) to/from %s\n> ", short_filename, file_size, username);
}


void transferResumedByPeer(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	printf(" * The user %s resumed the transfer of the file %s (TYPE, %s)\n> ", username, short_filename, file_size);
}


void transferResumed(int cid, char * username, char * short_filename, char * file_type, char * file_size){
	//g_callId = cid;
	set_callid_for_wxwengouser(username, cid);

	printf(" * You resumed the transfer of the file %s (TYPE, %s) to/from %s\n> ", short_filename, file_size, username);
}