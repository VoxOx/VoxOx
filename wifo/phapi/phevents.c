#include <assert.h>
#include <ctype.h>

#include <phapi-util/linkedlist.h>
#include <eXosip/eXosip.h>
#include "phevents.h"
#include "phcall.h"
#include "owpl_plugin.h"

typedef struct OWPL_EventSubscriber_t
{
	OWPL_EVENT_CALLBACK_PROC cbFunc;
	OWPL_CALLING_CONVENTION eCallingConvention;
	void *cbData;
} OWPL_EventSubscriber;


linkedlist_t * gEventSubscriber = 0;

/**
 * Add a event callback listener
 *
 * @param	pCallbackProc	The pointer to the callback function
 * @param	pUserData		The data that will be passed back to the callback function
 * @return	an OWPL_RESULT
 */
MY_DLLEXPORT OWPL_RESULT
owplEventListenerAdd(OWPL_EVENT_CALLBACK_PROC pCallbackProc,
                                             void *pUserData)
{
	OWPL_EventSubscriber * newListener;
	if ( ! gEventSubscriber)
	{
		gEventSubscriber = create_linkedlist();
	}

	newListener = malloc(sizeof(OWPL_EventSubscriber));
	newListener->cbData = pUserData;
	newListener->eCallingConvention = OWPL_CC_CDECL;
	newListener->cbFunc = pCallbackProc;
	
	linkedlist_add(gEventSubscriber, newListener, 0);

	return OWPL_RESULT_SUCCESS;
}

/**
 * Add a event callback listener with specified calling convention
 * (this function was intended to be used with C#)
 *
 * @param	pCallbackProc	The pointer to the callback function
 * @param	CallingConvention	The calling convention of the pCallbackProc
 * @param	pUserData		The data that will be passed back to the callback function
 * @return	an OWPL_RESULT
 */
MY_DLLEXPORT OWPL_RESULT
owplEventListenerAdd1(OWPL_EVENT_CALLBACK_PROC pCallbackProc,
											 OWPL_CALLING_CONVENTION CallingConvention,
                                             void *pUserData)
{
	OWPL_EventSubscriber * newListener;
	if ( ! gEventSubscriber)
	{
		gEventSubscriber = create_linkedlist();
	}

	newListener = malloc(sizeof(OWPL_EventSubscriber));
	newListener->cbData = pUserData;
	newListener->eCallingConvention = CallingConvention;
	newListener->cbFunc = pCallbackProc;
	
	linkedlist_add(gEventSubscriber, newListener, 0);

	return OWPL_RESULT_SUCCESS;
}

//=================================
//  Remove a listener
//=================================

MY_DLLEXPORT OWPL_RESULT
owplEventListenerRemove(OWPL_EVENT_CALLBACK_PROC pCallbackProc)
{
	OWPL_EventSubscriber * subscriber;
	linkedlist_move_first(gEventSubscriber);
	while (1)
	{
		subscriber = linkedlist_get(gEventSubscriber);
		if (subscriber && subscriber->cbFunc == pCallbackProc)
		{
			linkedlist_remove_element(gEventSubscriber, subscriber, 0);
			return OWPL_RESULT_SUCCESS;
		}
		if ( ! linkedlist_move_next(gEventSubscriber)) {
			break;
		}
	}
	return OWPL_RESULT_FAILURE;
}

/***************************  INTERNAL USAGE FUNCTIONS ****************************/

OWPL_RESULT
owplEventListenerRemoveAll()
{
	free_linkedlist(&gEventSubscriber);
	return OWPL_RESULT_SUCCESS;
}



/**
 * This function fires an event to all subscribed listeners
 *
 */

OWPL_RESULT
owplFireEvent(OWPL_EVENT_CATEGORY Category, 
                                            void* pInfo)
{
	OWPL_EventSubscriber * subscriber;

	if (!gEventSubscriber) {
		return OWPL_RESULT_FAILURE;	
	}
	linkedlist_move_first(gEventSubscriber);
	while (1)
	{
		subscriber = linkedlist_get(gEventSubscriber);
		if (subscriber && subscriber->cbFunc)
		{
			switch(subscriber->eCallingConvention)
			{
				case OWPL_CC_CDECL:
					subscriber->cbFunc(Category, pInfo, subscriber->cbData);
					break;
				case OWPL_CC_STDCALL:
					((OWPL_CALLBACK_PROC_STDCALL) subscriber->cbFunc) (Category, pInfo, subscriber->cbData);
					break;
				default:
					subscriber->cbFunc(Category, pInfo, subscriber->cbData);
					break;
			}
		}
		if (! linkedlist_move_next(gEventSubscriber)) {
			break;
		}
	}
	return OWPL_RESULT_SUCCESS;
}

/**
 * This function creates a call event and send it to all subscribers
 *
 */
OWPL_RESULT
owplFireCallEvent(OWPL_CALL hCall, 
					OWPL_CALLSTATE_EVENT event, 
					OWPL_CALLSTATE_CAUSE cause,
					const char * szRemoteIdentity,
					OWPL_CALL hAssociatedCall)
{
	phcall_t *call;

	OWPL_CALLSTATE_INFO csInfo;
	memset(&csInfo, 0, sizeof(OWPL_CALLSTATE_INFO));
	csInfo.nSize = sizeof(OWPL_CALLSTATE_INFO);
	csInfo.hCall = hCall;
	call = ph_locate_call_by_cid(hCall);
	if (call) {
		csInfo.hLine = call->vlid;
	}
	csInfo.event = event;
	csInfo.cause = cause;
	csInfo.szRemoteIdentity = szRemoteIdentity;
	csInfo.hAssociatedCall = hAssociatedCall;
	return owplFireEvent(EVENT_CATEGORY_CALLSTATE, &csInfo);
}


/**
 * This function also creates a call event (with more advanced params) 
 * and send it to all subscribers
 *
 */
OWPL_RESULT
owplFireCallEvent2(OWPL_CALL hCall, 
					OWPL_CALLSTATE_EVENT event, 
					OWPL_CALLSTATE_CAUSE cause,
					const char * szRemoteIdentity,
					int nData,
					const void *pData,
					OWPL_CALL hAssociatedCall)
{
	phcall_t *call;

	OWPL_CALLSTATE_INFO csInfo;
	memset(&csInfo, 0, sizeof(OWPL_CALLSTATE_INFO));
	csInfo.nSize = sizeof(OWPL_CALLSTATE_INFO);
	csInfo.hCall = hCall;
	call = ph_locate_call_by_cid(hCall);
	if (call) {
		csInfo.hLine = call->vlid;
	}
	csInfo.event = event;
	csInfo.cause = cause;
	csInfo.szRemoteIdentity = szRemoteIdentity;
	csInfo.hAssociatedCall = hAssociatedCall;
	csInfo.nData = nData;
	csInfo.pData = pData;
	return owplFireEvent(EVENT_CATEGORY_CALLSTATE, &csInfo);
}
						
/**
 * This function creates a line event and send it to all subscribers
 *
 */
OWPL_RESULT
owplFireLineEvent(OWPL_LINE hLine,
						OWPL_LINESTATE_EVENT event,
						OWPL_LINESTATE_CAUSE cause,
						const char *szBodyBytes)        /**< msg body to extract reg body */
{
	OWPL_LINESTATE_INFO lsInfo;
	memset(&lsInfo, 0, sizeof(OWPL_LINESTATE_INFO));
	lsInfo.nSize = sizeof(OWPL_LINESTATE_INFO);
	lsInfo.event = event;
	lsInfo.cause = cause;
	lsInfo.hLine = hLine;
	lsInfo.szBodyBytes = szBodyBytes;

	return owplFireEvent(EVENT_CATEGORY_LINESTATE, &lsInfo);;
}


/**
 * This function creates an error event and send it to all subscribers
 *
 */
OWPL_RESULT
owplFireErrorEvent(OWPL_ERROR_EVENT event,
						void * Data)
{
	OWPL_ERROR_INFO eInfo;
	memset(&eInfo, 0, sizeof(eInfo));
	eInfo.event = event;
	eInfo.Data = Data;
	return owplFireEvent(EVENT_CATEGORY_ERROR, &eInfo);
}

OWPL_RESULT
owplFireSubscriptionEvent(OWPL_SUB hSub,
						  OWPL_SUBSCRIPTION_STATE state,
						  OWPL_SUBSCRIPTION_CAUSE cause,
			  const char* szRemoteIdentity,
			                          const char* szEvtType)
{
	OWPL_SUBSTATUS_INFO sInfo;
	memset(&sInfo, 0, sizeof(OWPL_SUBSTATUS_INFO));
	sInfo.nSize = sizeof(OWPL_SUBSTATUS_INFO);
	sInfo.hSub = hSub;
	/* TODO: find hLine */
	/*sInfo.hLine =  ;*/
	sInfo.state = state;
	sInfo.cause = cause;
	sInfo.szRemoteIdentity = szRemoteIdentity;
	sInfo.szEvtType = szEvtType;
	return owplFireEvent(EVENT_CATEGORY_SUB_STATUS, &sInfo);
}

OWPL_RESULT
owplFireNotificationEvent(OWPL_NOTIFICATION_EVENT event,
						  OWPL_NOTIFICATION_CAUSE cause,
						  const char* szXmlContent,
						  const char* szRemoteIdentity)
{
	OWPL_NOTIFICATION_INFO nInfo;
	OWPL_NOTIFICATION_STATUS_INFO statusInfo;
	OWPL_NOTIFICATION_MWI_INFO mwiInfo;
	char szStatusNote[512];
	char szMessageAccount[256];


	memset(&nInfo, 0, sizeof(OWPL_NOTIFICATION_INFO));
	nInfo.nSize = sizeof(OWPL_NOTIFICATION_INFO);
	/* TODO: find hLine */
	/*nInfo.hLine =  ;*/
	nInfo.event = event;
	nInfo.cause = cause;
	nInfo.szXmlContent = szXmlContent;

	if (event == NOTIFICATION_PRESENCE) {
		memset(&statusInfo, 0, sizeof(OWPL_NOTIFICATION_STATUS_INFO));
		statusInfo.szRemoteIdentity = szRemoteIdentity;

		owplNotificationPresenceGetNote(szXmlContent, szStatusNote, sizeof(szStatusNote));
		statusInfo.nSize = sizeof(OWPL_NOTIFICATION_STATUS_INFO);
		statusInfo.szStatusNote = szStatusNote;

		nInfo.Data.StatusInfo = &statusInfo;
	}
	else if (event == NOTIFICATION_MWI) {
		memset(&mwiInfo, 0, sizeof(OWPL_NOTIFICATION_MWI_INFO));
		mwiInfo.nSize = sizeof(OWPL_NOTIFICATION_MWI_INFO);
		owplNotificationMWIGetInfos(szXmlContent, 
			szMessageAccount, sizeof(szMessageAccount), 
			&mwiInfo.nUnreadMessageCount, &mwiInfo.nReadMessageCount,
			&mwiInfo.nImpUnreadMessageCount, &mwiInfo.nImpReadMessageCount);
		mwiInfo.szMessageAccount = szMessageAccount;
		nInfo.Data.MWI = &mwiInfo;
	}

	return owplFireEvent(EVENT_CATEGORY_NOTIFY, &nInfo);
}

OWPL_RESULT
owplNotificationPresenceGetIdentity(const char * notify, char * buffer, size_t size) {
	char * entity_string;
	char * first_dbquote;
	char * second_dbquote;
	size_t length = 0;

	if(notify == NULL
		|| strlen(notify) == 0
		|| buffer == NULL
		|| size == 0)
	{
		return OWPL_RESULT_INVALID_ARGS;
	}

	memset(buffer, 0, size);

	if((entity_string = strstr(notify, "entity")) == NULL) {
		return OWPL_RESULT_FAILURE;
	}

	if((first_dbquote = strchr(entity_string, '"')) == NULL) {
		return OWPL_RESULT_FAILURE;
	}

	if((second_dbquote = strchr(++first_dbquote, '"')) == NULL) {
		return OWPL_RESULT_FAILURE;
	}

	if((length = strlen(first_dbquote) - strlen(second_dbquote)) == 0 || length >= size) {
		return OWPL_RESULT_FAILURE;
	}

	if(strlen(strncpy(buffer, first_dbquote, length)) == 0) {
		return OWPL_RESULT_FAILURE;
	}

	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplNotificationPresenceGetStatus(const char * notify, char * buffer, size_t size) {
	char * basic_start_tag;
	char * basic_end_tag;
	size_t length = 0;
	int found = 0;

	if(notify == NULL
		|| strlen(notify) == 0
		|| buffer == NULL
		|| size == 0)
	{
		return OWPL_RESULT_INVALID_ARGS;
	}

	memset(buffer, 0, size);

	if((basic_start_tag = strstr(notify, "<basic")) == NULL) {
		return OWPL_RESULT_FAILURE;
	}

	if((basic_end_tag = strstr(basic_start_tag, "</basic>")) == NULL) {
		return OWPL_RESULT_FAILURE;
	}

	while (basic_start_tag && basic_start_tag != basic_end_tag)
	{
		if (*basic_start_tag == '>')
		{
			found = 1;
			basic_start_tag++;
			break;
		}
		basic_start_tag++;
	}

	if (!found)
	{
		return OWPL_RESULT_FAILURE;
	}

	if((length = strlen(basic_start_tag) - strlen(basic_end_tag)) == 0 || length >= size) {
		return OWPL_RESULT_FAILURE;
	}

	if(strlen(strncpy(buffer, basic_start_tag, length)) == 0) {
		return OWPL_RESULT_FAILURE;
	}

	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplNotificationPresenceGetNote(const char * notify, char * buffer, size_t size) {
	char * note_start_tag;
	char * note_end_tag;
	size_t length = 0;

	if(notify == NULL
		|| strlen(notify) == 0
		|| buffer == NULL
		|| size == 0)
	{
		return OWPL_RESULT_INVALID_ARGS;
	}

	memset(buffer, 0, size);

	if((note_start_tag = strstr(notify, "<note>")) == NULL) {
		return OWPL_RESULT_FAILURE;
	}

	if((note_end_tag = strstr(note_start_tag, "</note>")) == NULL) {
		return OWPL_RESULT_FAILURE;
	}

	note_start_tag += 6;

	if((length = strlen(note_start_tag) - strlen(note_end_tag)) == 0 || length >= size) {
		return OWPL_RESULT_FAILURE;
	}

	if(strlen(strncpy(buffer, note_start_tag, length)) == 0) {
		return OWPL_RESULT_FAILURE;
	}

	return OWPL_RESULT_SUCCESS;
}

#define MSG_ACC_HDR		"message-account:"
#define VOICE_MSG_HDR	"voice-message:"

OWPL_RESULT
owplNotificationMWIGetInfos(const char * szContent, 
							char * szMessAccBuff, size_t szMessAccBuffSize, 
							int * unreadMessageCount, int * readMessageCount,
							int * impUnreadMessageCount, int * impReadMessageCount)
{
	char * hdr_string;
	char * value_begin;
	char * value_end;
	size_t length = 0;
	char * tmpContent, * beginContent;

	if (szContent == NULL
		|| strlen(szContent) == 0
		|| szMessAccBuff == 0
		|| szMessAccBuffSize == 0)
	{
		return OWPL_RESULT_INVALID_ARGS;
	}

	tmpContent = malloc(strlen(szContent) + 1);
	beginContent = tmpContent;
	while (szContent && *szContent) {
		*tmpContent++ = (char) tolower((int)*szContent++);
	}
	*tmpContent = 0;

	memset(szMessAccBuff, 0, szMessAccBuffSize);

	if ((hdr_string = strstr(beginContent, MSG_ACC_HDR)) == NULL) {
		free(beginContent);
		return OWPL_RESULT_FAILURE;
	}
	else {
		for (value_begin = (hdr_string + strlen(MSG_ACC_HDR));
			*value_begin && (*value_begin == ' ' ||  *value_begin == '\t');
			value_begin++);

		if ((value_end = strstr(value_begin, "\r\n")) == NULL) {
			free(beginContent);
			return OWPL_RESULT_FAILURE;
		}
	
		strncpy(szMessAccBuff, value_begin, value_end - value_begin);
	}

	if ((hdr_string = strstr(beginContent, VOICE_MSG_HDR)) == NULL) {
		free(beginContent);
		return OWPL_RESULT_FAILURE;
	}
	else {
		for (value_begin = (hdr_string + strlen(VOICE_MSG_HDR));
			*value_begin && (*value_begin == ' ' ||  *value_begin == '\t');
			value_begin++);
	
		sscanf(value_begin, "%d/%d", unreadMessageCount, readMessageCount);
		
		if ((value_begin = strchr(value_begin, '(')) == NULL) {
			free(beginContent);
			return OWPL_RESULT_FAILURE;
		}

		sscanf(value_begin, "(%d/%d)", impUnreadMessageCount, impReadMessageCount);
	}

	free(beginContent);
	return OWPL_RESULT_SUCCESS;
}

OWPL_RESULT
owplFireMessageEvent(OWPL_MESSAGE_EVENT event,
					 OWPL_MESSAGE_CAUSE cause,
					 const int messageId,
					 const OWPL_LINE hLine,
					 const char * szContent,
					 const char * szLocalIdentity,
					 const char * szRemoteIdentity,
					 const char * szContentType,
					 const char * szSubContentType)
{
	OWPL_MESSAGE_INFO mInfo;
	memset(&mInfo, 0, sizeof(OWPL_MESSAGE_INFO));
	mInfo.nSize = sizeof(OWPL_MESSAGE_INFO);
	mInfo.event = event;
	mInfo.cause = cause;
	mInfo.messageId = messageId;
	mInfo.hLine = hLine;
	mInfo.szContent = szContent;
	mInfo.szLocalIdentity = szLocalIdentity;
	mInfo.szRemoteIdentity = szRemoteIdentity;
	mInfo.szContentType = szContentType;
	mInfo.szSubContentType = szSubContentType;
	return owplFireEvent(EVENT_CATEGORY_MESSAGE, &mInfo);
}

/************************************************/
/*     PLUGIN handling routines                 */
/************************************************/


/**
 * This function fires an event to all subscribed plugins
 *
 */

OWPL_RESULT
owplFireEvent2Plugin(OWPL_PLUGIN *plugin, OWPL_EVENT_CATEGORY Category, 
                                            void* pInfo)
{
	if (plugin && 
			plugin->exports && 
			plugin->exports->ct_info &&
			plugin->exports->ct_info->EventHandlerProc)		
	{
			plugin->exports->ct_info->EventHandlerProc(Category, pInfo, 0);
	}	
	return OWPL_RESULT_SUCCESS;
}


/**
 * This function creates a call event 
 * and send it to the specified plugin
 *
 */
OWPL_RESULT
owplFireCallEvent2Plugin(OWPL_PLUGIN *plugin,
					OWPL_CALL hCall, 
					OWPL_CALLSTATE_EVENT event, 
					OWPL_CALLSTATE_CAUSE cause,
					const char * szRemoteIdentity,
					int nData,
					const void *pData,
					OWPL_CALL hAssociatedCall)
{
	phcall_t *call;

	OWPL_CALLSTATE_INFO csInfo;
	memset(&csInfo, 0, sizeof(OWPL_CALLSTATE_INFO));
	csInfo.nSize = sizeof(OWPL_CALLSTATE_INFO);
	csInfo.hCall = hCall;
	call = ph_locate_call_by_cid(hCall);
	if (call) {
		csInfo.hLine = call->vlid;
	}
	csInfo.event = event;
	csInfo.cause = cause;
	csInfo.szRemoteIdentity = szRemoteIdentity;
	csInfo.hAssociatedCall = hAssociatedCall;
	csInfo.nData = nData;
	csInfo.pData = pData;
	return owplFireEvent2Plugin(plugin, EVENT_CATEGORY_CALLSTATE, &csInfo);
}


/*  eXosip dependent routines */

void owplFireExosipCallEvent(struct eXosip_event *je)
{
	OWPL_PLUGIN *owplPlugin;
	char buf[101];

	// Find the call or create it if je->type is EXOSIP_CALL_NEW
	phcall_t * call = ph_locate_call(je, je->type == EXOSIP_CALL_NEW ? 1 : 0);

	if ( call && (call->owplPlugin == 0))
	{
		if (je->i_ctt && je->i_ctt->type)
		{
			strncpy(buf, je->i_ctt->type, sizeof(buf)-1);
			if (je->i_ctt->subtype) {
				strncat(buf, "/", sizeof(buf) - strlen(buf) -1);
				strncat(buf,je->i_ctt->subtype, sizeof(buf) - strlen(buf) - 1);
			}
			owplPlugin = owplGetPlugin4ContentType(buf);
			if (owplPlugin) {
				call->owplPlugin = owplPlugin;
			}
		}
		else {
			return;
		}
	}

	switch(je->type){
			case EXOSIP_CALL_NEW:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_OFFERING, 
													CALLSTATE_OFFERING_ACTIVE, 
													je->remote_contact, 
													0, 
													je->msg_body, 
													0);
				break;
			
			case EXOSIP_CALL_ANSWERED:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_CONNECTED, 
													CALLSTATE_CONNECTED_ACTIVE, 
													je->remote_contact, 
													0, 
													je->msg_body,
													0);
				break;

			case EXOSIP_CALL_PROCEEDING:
				// Trying SIP message is received from other peer. What to do with it???
				break;

			case EXOSIP_CALL_RINGING:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_REMOTE_ALERTING, 
													CALLSTATE_REMOTE_ALERTING_NORMAL,
													je->remote_contact, 
													0, 
													je->msg_body,
													0);
				break;

			case EXOSIP_CALL_REDIRECTED:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_REDIRECTED, 
													CALLSTATE_REDIRECTED_NORMAL,
													je->remote_contact, 
													0, 
													je->msg_body,
													0);
				break;

			case EXOSIP_CALL_REPLACES:
				// What is it???
				break;

			case EXOSIP_CALL_REQUESTFAILURE:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_DISCONNECTED, 
													CALLSTATE_DISCONNECTED_NETWORK,
													je->remote_contact, 
													0, 
													je->msg_body,
													0);
				break;

			case EXOSIP_CALL_SERVERFAILURE:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_DISCONNECTED, 
													CALLSTATE_DISCONNECTED_NETWORK,
													je->remote_contact, 
													0, 
													je->msg_body,
													0);
				break;

			case EXOSIP_CALL_GLOBALFAILURE:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_DISCONNECTED, 
													CALLSTATE_DISCONNECTED_NETWORK,
													je->remote_contact, 
													0, 
													je->msg_body,
													0);
				break;

			case EXOSIP_CALL_NOANSWER:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_DISCONNECTED, 
													CALLSTATE_DISCONNECTED_NO_RESPONSE,
													je->remote_contact, 
													0, 
													je->msg_body,
													0);
				break;

			case EXOSIP_CALL_CLOSED:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_DISCONNECTED, 
													CALLSTATE_DISCONNECTED_NORMAL,
													je->remote_contact, 
													0, 
													je->msg_body,
													0);
				break;

			case EXOSIP_CALL_HOLD:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_HOLD, 
													CALLSTATE_HOLD_STARTED,
													je->remote_contact, 
													0, 
													je->msg_body,
													0);
				break;

			case EXOSIP_CALL_OFFHOLD:
				owplFireCallEvent2Plugin(call->owplPlugin,call->cid, 
													CALLSTATE_HOLD, 
													CALLSTATE_HOLD_RESUMED,
													je->remote_contact, 
													0, 
													je->msg_body,
													0);
				break;			

			case EXOSIP_CALL_REFERED:
				// ???
				break;

			case EXOSIP_CALL_REFER_STATUS:
				///???
				break;

			case EXOSIP_CALL_REFER_FAILURE:
				///???
				break;			

			default:
				assert(0);
				break;
		}
}
