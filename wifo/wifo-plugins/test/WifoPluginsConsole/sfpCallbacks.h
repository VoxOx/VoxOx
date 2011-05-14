#ifndef __SFPCALLBACKS_H__
#define __SFPCALLBACKS_H__

extern "C" {
#include <sfp-plugin/sfp-plugin.h>
}

extern int g_callId;

extern char step[16];

extern sfp_callbacks_t sfp_cbks;

extern int users_to_callids[9];

void inviteToTransfer(int cid, char * uri, char * short_filename, char * file_type, char * file_size);
void newIncomingFile(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void waitingForAnswer(int cid, char * uri);
void transferCancelled(int cid, char * short_filename, char * file_type, char * file_size);
void transferCancelledByPeer(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void sendingFileBegin(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferClosedByPeer(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferClosed(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferFromPeerFinished(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferToPeerFinished(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferFromPeerFailed(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferToPeerFailed(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferFromPeerStopped(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferToPeerStopped(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferProgression(int cid, int percentage);
void transferPausedByPeer(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferPaused(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferResumedByPeer(int cid, char * username, char * short_filename, char * file_type, char * file_size);
void transferResumed(int cid, char * username, char * short_filename, char * file_type, char * file_size);

#endif /* __SFPCALLBACKS_H__ */