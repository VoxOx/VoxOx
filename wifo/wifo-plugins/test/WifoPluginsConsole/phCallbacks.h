#ifndef _PH_CALLBACK_H_
#define _PH_CALLBACK_H_

extern "C" {
#include "phapi.h"
}

extern int g_callId;

extern char step[16];

void callProgress(int callId, const phCallStateInfo_t * info);

void transferProgress(int, const phTransferStateInfo_t *);

void conferenceProgress(int, const phConfStateInfo_t *);

void registerProgress(int registerId, int status);

void messageProgress(int messageId, const phMsgStateInfo_t * info);

void subscriptionProgress(int subscriptionId, const phSubscriptionStateInfo_t *info);

void onNotify(const char * event, const char * from, const char * content);

extern phCallbacks_t callbacks;

#endif // _PH_CALLBACK_H_