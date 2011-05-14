#ifndef _OWPL_API_EVENT_CALLBACK_HEADER_
#define _OWPL_API_EVENT_CALLBACK_HEADER_

#ifdef WIN32
#if defined(BUILD_PHAPI_DLL)
#define MY_DLLEXPORT __declspec(dllexport) 
#elif defined(PHAPI_DLL)
#define MY_DLLEXPORT __declspec(dllimport) 
#endif
#endif

#ifndef MY_DLLEXPORT
#define MY_DLLEXPORT 
#endif

#include <owpl.h>
#include <stdlib.h>
#include <phapi-old.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

// HACK forward declaration in order not to include <owpl_plugin.h> (and so not <phapi.h>)
struct owpl_plugin;

typedef enum CallingConventionEnum_T
{
	OWPL_CC_CDECL = 0,
	OWPL_CC_STDCALL = 1
} OWPL_CALLING_CONVENTION;

/*
 * This file is base on SipXtapi header file
 */

/**
 * Enum with all of the possible event types.
 *
 *
 */
typedef enum OWPL_EVENT_CATEGORY
{
    EVENT_CATEGORY_CALLSTATE,       /**< CALLSTATE events signify a change in state of a 
                                         call.  States range from the notification of a 
                                         new call to ringing to connection established to 
                                         changes in audio state (starting sending, stop 
                                         sending) to termination of a call. */
    EVENT_CATEGORY_LINESTATE,       /**< LINESTATE events indicate changes in the status 
                                         of a line appearance.  Lines identify inbound 
                                         and outbound identities and can be either 
                                         provisioned (hardcoded) or configured to 
                                         automatically register with a registrar.  
                                         Lines also encapsulate the authentication 
                                         criteria needed for dynamic registrations. */
    EVENT_CATEGORY_INFO_STATUS,     /**< INFO_STATUS events are sent when the application 
                                         requests to send an INFO message to 
                                         another user agent.  The status event includes 
                                         the response for the INFO method.  Application 
                                         developers should look at this event to determine 
                                         the outcome of the INFO message. */
    EVENT_CATEGORY_INFO,            /**< INFO events are sent to the application whenever 
                                         an INFO message is received by the PhAPI user 
                                         agent.  INFO messages are sent to a specific call.
                                         PhAPI will automatically acknowledges the INFO 
                                         message at the protocol layer. */
    EVENT_CATEGORY_SUB_STATUS,      /**< Subscription events: OK or expired, notify, etc.*/
    EVENT_CATEGORY_NOTIFY,          /**< Notification of subscribed-to events */
    EVENT_CATEGORY_CONFIG,           /**< CONFIG events signify changes in configuration.
                                         For example, when requesting STUN support, a 
                                         notification is sent with the STUN outcome (either
                                         SUCCESS or FAILURE) */
	EVENT_CATEGORY_ERROR,           /**< ERROR events signify error in the engine.
                                         For example, an error happen when trying to open audio 
										 device for a call*/
	EVENT_CATEGORY_MESSAGE			/**< Message events */
} OWPL_EVENT_CATEGORY;



/**
 * Major call state events identify significant changes in the state of a 
 * call.
 */
typedef enum OWPL_CALLSTATE_EVENT
{
    CALLSTATE_UNKNOWN         = 0,/**< An UNKNOWN event is generated when the state for a call 
                                 is no longer known.  This is generally an error 
                                 condition; see the minor event for specific causes. */
    CALLSTATE_NEWCALL         = 1000, /**< The NEWCALL event indicates that a new call has been 
                                 created automatically by the PhAPI.  This event is 
                                 most frequently generated in response to an inbound 
                                 call request.  */
	CALLSTATE_REMOTE_OFFERING = 2000, /**< or DIALING, The REMOTE_OFFERING event indicates that a call setup 
                                 invitation has been sent to the remote party.  The 
                                 invitation may or may not every receive a response.  If
                                 a response is not received in a timely manor, sipXtapi 
                                 will move the call into a disconnected state.  If 
                                 calling another sipXtapi user agent, the reciprocate 
                                 state is OFFER. */
	CALLSTATE_REMOTE_ALERTING = 3000, /**< The REMOTE_ALERTING event indicates that a call setup 
                                 invitation has been accepted and the end user is in the
                                 alerting state (ringing).  Depending on the SIP 
                                 configuration, end points, and proxy servers involved, 
                                 this event should only last for 3 minutes.  Afterwards,
                                 the state will automatically move to DISCONNECTED.  If 
                                 calling another phAPI user agent, the reciprocate 
                                 state is ALERTING. */
	CALLSTATE_CONNECTED       = 4000, /**< The CONNECTED state indicates that call has been setup 
                                 between the local and remote party.  Audio should be 
                                 flowing provided and the microphone and speakers should
                                 be engaged. */
	CALLSTATE_DISCONNECTED    = 5000, /**< The DISCONNECTED state indicates that a call was 
                                 disconnected or failed to connect.  A call may move 
                                 into the DISCONNECTED states from almost every other 
                                 state.  Please review the DISCONNECTED minor events to
                                 understand the cause. */
	CALLSTATE_OFFERING        = 6000, /**< An OFFERING state indicates that a new call invitation 
                                 has been extended this user agent.  Application 
                                 developers should invoke owplCallAccept(), 
                                 owplCallReject() or owplCallRedirect() in response.  
                                 Not responding will result in an implicit call 
                                 owplCallReject(). */

/** VIDEO: The CALLSTATE_INFO should contain supported remote audio and video codecs offered */

                                 
    CALLSTATE_ALERTING        = 7000, /**< An ALERTING state indicates that an inbound call has 
                                 been accepted and the application layer should alert 
                                 the end user.  The alerting state is limited to 3 
                                 minutes in most configurations; afterwards the call 
                                 will be canceled.  Applications will generally play 
                                 some sort of ringing tone in response to this event. */

/** VIDEO: The CALLSTATE_INFO should contain supported remote audio and video codecs offered */


    CALLSTATE_DESTROYED       = 8000, /**< The DESTORYED event indicates the underlying resources 
                                 have been removed for a call.  This is the last event 
                                 that the application will receive for any call.  The 
                                 call handle is invalid after this event is received. */
	CALLSTATE_AUDIO_EVENT      = 9000, /**< The  AUDIO_EVENT event indicates the Audio session has 
											either started or stopped, or a frame received */

	CALLSTATE_VIDEO_EVENT	   = 10000, /**< The  VIDEO_EVENT event indicates the Video session has 
											either started or stopped, or a frame received */
    CALLSTATE_TRANSFER         = 11000, /**< The transfer state indicates a state change in a 
                                 transfer attempt initiated by our side.  Please see the CALLSTATE_TRANSFER cause 
                                 codes for details on each state transition */
	CALLSTATE_REDIRECTED	   = 12000, /**< The transfer state indicates a state change in a 
                                 transfer attempt initiated by the other side.*/

	CALLSTATE_HOLD			   = 13000, /**< The transfer state indicates a state change in a 
                                 transfer attempt initiated by the other side.*/

    CALLSTATE_SECURITY_EVENT   = 14000, /** The SECURITY_EVENT is sent to the application 
                                           when S/MIME or SRTP events occur which the application
                                           should know about. */ 
    CALLSTATE_IDENTITY_CHANGE  = 15000 /** The identity of the remote party on this call has changed
                                           to the identity given  in szRemoteIdentity. */
} OWPL_CALLSTATE_EVENT;

/**
 * Callstate cuase events identify the reason for a Callstate event or 
 * provide more detail.
 */
typedef enum OWPL_CALLSTATE_CAUSE
{
	CALLSTATE_NEW_CALL_NORMAL		  = CALLSTATE_NEWCALL + 1,	        /**< See NEWCALL callstate event */
	CALLSTATE_NEW_CALL_TRANSFERRED	  = CALLSTATE_NEWCALL + 2,	        /**< Call created because a transfer has is
	                                                                         occurring */
	CALLSTATE_NEW_CALL_TRANSFER	      = CALLSTATE_NEWCALL + 3,	        /**< Call created because a transfer has 
                                                                             been initiated locally. */
	CALLSTATE_REMOTE_OFFERING_NORMAL  = CALLSTATE_REMOTE_OFFERING + 1,  /**< See REMOTE_OFFERING callstate event */
	CALLSTATE_REMOTE_ALERTING_NORMAL  = CALLSTATE_REMOTE_ALERTING + 1,  /**< Remote party is alerting, play ringback 
                                                                             locally */
	CALLSTATE_REMOTE_ALERTING_MEDIA_START  = CALLSTATE_REMOTE_ALERTING + 2,  

    CALLSTATE_REMOTE_ALERTING_MEDIA_STOP   = CALLSTATE_REMOTE_ALERTING + 3,

	CALLSTATE_CONNECTED_ACTIVE		  = CALLSTATE_CONNECTED + 1,        /**< Call is connected and active (playing 
                                                                             local media)*/
	CALLSTATE_CONNECTED_ACTIVE_HELD,                                    /**< Call is connected, held (not playing local
                                                                             media), and bridging media for a 
                                                                             conference */
    CALLSTATE_CONNECTED_INACTIVE,                                       /**< Call is held (not playing local media) and
                                                                             is not bridging any other calls */
	CALLSTATE_DISCONNECTED_BADADDRESS = CALLSTATE_DISCONNECTED + 1,     /**< Disconnected: Invalid or unreachable 
                                                                             address */
	CALLSTATE_DISCONNECTED_BUSY,                                        /**< Disconnected: Caller or Callee was busy*/
	CALLSTATE_DISCONNECTED_NORMAL,                                      /**< Disconnected: Normal call tear down (either 
                                                                             local or remote)*/
    CALLSTATE_DISCONNECTED_RESOURCES,                                   /**< Disconnected: Not enough resources 
                                                                             available to complete call*/
    CALLSTATE_DISCONNECTED_NETWORK,                                     /**< Disconnected: A network error cause call 
                                                                             to fail*/
	CALLSTATE_DISCONNECTED_REDIRECTED,                                  /**< Disconnected: Call was redirected a 
                                                                             different user agent */
	CALLSTATE_DISCONNECTED_NO_RESPONSE,                                 /**< Disconnected: No response was received */
    CALLSTATE_DISCONNECTED_AUTH,                                        /**< Disconnected: Unable to authenticate */
    CALLSTATE_DISCONNECTED_UNKNOWN,                                     /**< Disconnected: Unknown reason */

	CALLSTATE_OFFERING_ACTIVE		  = CALLSTATE_OFFERING + 1,         /**< See OFFERING callstate event */
	CALLSTATE_ALERTING_NORMAL		  = CALLSTATE_ALERTING + 1,         /**< See ALERTING callstate event */
	CALLSTATE_DESTROYED_NORMAL        = CALLSTATE_DESTROYED + 1,        /**< See DESTROYED callstate event */

    CALLSTATE_AUDIO_START             = CALLSTATE_AUDIO_EVENT + 1,      /**< RTP session started */
    CALLSTATE_AUDIO_STOP              = CALLSTATE_AUDIO_START + 1,      /**< RTP session stopped */
	CALLSTATE_AUDIO_DTMF			  = CALLSTATE_AUDIO_STOP + 1,

	CALLSTATE_VIDEO_START			  = CALLSTATE_VIDEO_EVENT + 1,
	CALLSTATE_VIDEO_STOP			  = CALLSTATE_VIDEO_START + 1,
	CALLSTATE_VIDEO_FRAME_RCV		  = CALLSTATE_VIDEO_STOP + 1,

    CALLSTATE_TRANSFER_INITIATED      = CALLSTATE_TRANSFER + 1,         /**< A transfer attempt has been initiated.  This event
                                                                             is sent when a user agent attempts either a blind
                                                                             or consultative transfer. */
    CALLSTATE_TRANSFER_ACCEPTED,                                        /**< A transfer attempt has been accepted by the remote
                                                                             transferee.  This event indicates that the 
                                                                             transferee supports transfers (REFER method).  The
                                                                             event is fired upon a 2xx class response to the SIP
                                                                             REFER request. */
    CALLSTATE_TRANSFER_TRYING,                                          /**< The transfer target is attempting the transfer.  
                                                                             This event is sent when transfer target (or proxy /
                                                                             B2BUA) receives the call invitation, but before the
                                                                             the tranfer target accepts is. */
    CALLSTATE_TRANSFER_RINGING,                                         /**< The transfer target is ringing.  This event is 
                                                                             generally only sent during blind transfer.  
                                                                             Consultative transfer should proceed directly to 
                                                                             TRANSFER_SUCCESS or TRANSFER_FAILURE. */
    CALLSTATE_TRANSFER_SUCCESS,                                         /**< The transfer was completed successfully.  The
                                                                             original call to transfer target will
                                                                             automatically disconnect.*/
    CALLSTATE_TRANSFER_FAILURE,                                         /**< The transfer failed.  After a transfer fails,
                                                                             the application layer is responsible for 
                                                                             recovering original call to the transferee. 
                                                                             That call is left on hold. */

	CALLSTATE_REDIRECTED_NORMAL		= CALLSTATE_REDIRECTED + 1,			/**< The remote side has asked us to call to another uri*/

	CALLSTATE_HOLD_STARTED			= CALLSTATE_HOLD + 1,
	CALLSTATE_HOLD_RESUMED			= CALLSTATE_HOLD + 2,

    CALLSTATE_SECURITY_SELF_SIGNED_CERT = CALLSTATE_SECURITY_EVENT + 1, /**< A self-signed certificate is being used for S/MIME. */
    CALLSTATE_SECURITY_SESSION_NOT_SECURED,                             /**< Fired if a secure session could not be made. */
    CALLSTATE_SECURITY_REMOTE_SMIME_UNSUPPORTED,                        /**< Fired if the remote party's user-agent does not
                                                                             support S/MIME. */
    CALLSTATE_IDENTITY_CHANGE_UNKNOWN = CALLSTATE_IDENTITY_CHANGE + 1,   /**< The P-Asserted-Identity changed for a unknown reason
                                                                             The identity may have changed because of a transfer or
                                                                             some other reason, but the signalling did not give any
                                                                             indication as to why it changed. */
    CALLSTATE_CAUSE_UNKNOWN,                                            /* used in applicaiton when not event based */

} OWPL_CALLSTATE_CAUSE;

/**
 * Callstate event information structure
 */
typedef struct
{
    // TODO: Add a bitmask that identified which structure items are valid.  For 
    //       example, codec and hAssociatedCall are only valid for certain event
    //       sequences.

    size_t    nSize;                /**< The size of this structure. */
    OWPL_CALL hCall;                /**< Call handle associated with the callstate event. */
    OWPL_LINE hLine;                /**< Line handle associated with the callstate event. */
    OWPL_CALLSTATE_EVENT event;     /**< Callstate event enum code.
                                         Identifies the callstate event. */
    OWPL_CALLSTATE_CAUSE cause;     /**< Callstate cause enum code. 
                                         Identifies the cause of the callstate event. */
    OWPL_CALL hAssociatedCall ;     /**< Call associated with this event.  For example, when
                                         a new call is created as part of a consultative 
                                         transfer, this handle contains the handle of the 
                                         original call. */
	int		nData;					/**< The number data related to the event (the DTMF for example)*/
	const void* pData;				/**< The custom data related to the event (e.g The message body for a plugin )*/
    const char* szRemoteIdentity;   /**< The identity of the remote party on this call. */

} OWPL_CALLSTATE_INFO; 

/**
 * Enumeration of possible linestate Events.
 */
 typedef enum OWPL_LINESTATE_EVENT
{
    LINESTATE_UNKNOWN  = -1,            /**< This is the initial Line event state. */
    LINESTATE_REGISTERING   = 20000,    /**< The REGISTERING event is fired when sipXtapi
                                             has successfully sent a REGISTER message,
                                             but has not yet received a success response from the
                                             registrar server */    
    LINESTATE_REGISTERED  = 21000,      /**< The REGISTERED event is fired after sipXtapi has received
                                             a response from the registrar server, indicating a successful
                                             registration. */
    LINESTATE_UNREGISTERING    = 22000, /**< The UNREGISTERING event is fired when sipXtapi
                                             has successfully sent a REGISTER message with an expires=0 parameter,
                                             but has not yet received a success response from the
                                             registrar server */
    LINESTATE_UNREGISTERED     = 23000, /**< The UNREGISTERED event is fired after sipXtapi has received
                                             a response from the registrar server, indicating a successful
                                             un-registration. */
    LINESTATE_REGISTER_FAILED  = 24000, /**< The REGISTER_FAILED event is fired to indicate a failure of REGISTRATION.
                                             It is fired in the following cases:  
                                             The client could not connect to the registrar server.
                                             The registrar server challenged the client for authentication credentials,
                                             and the client failed to supply valid credentials.
                                             The registrar server did not generate a success response (status code == 200)
                                             within a timeout period.  */
    LINESTATE_UNREGISTER_FAILED  = 25000,/**< The UNREGISTER_FAILED event is fired to indicate a failure of un-REGISTRATION.
                                             It is fired in the following cases:  
                                             The client could not connect to the registrar server.
                                             The registrar server challenged the client for authentication credentials,
                                             and the client failed to supply valid credentials.
                                             The registrar server did not generate a success response (status code == 200)
                                             within a timeout period.  */
    LINESTATE_PROVISIONED      = 26000, /**< The PROVISIONED event is fired when a owpl Line is added, and Registration is not 
                                             requested (i.e. - sipxLineAdd is called with a bRegister parameter of false. */ 
	LINESTATE_DELETING			= 27000, /**< The DELETING event is fired when a owpl Line is beging removed. An UNREGISTER
                                              must be sent if the line was connected */ 

} OWPL_LINESTATE_EVENT;

/**
 * Enumeration of possible linestate Event causes.
 */
typedef enum OWPL_LINESTATE_CAUSE
{
    LINESTATE_CAUSE_UNKNOWN = 0,                            /**< No cause specified. */

    LINESTATE_CAUSE_NORMAL,

    LINESTATE_CAUSE_COULD_NOT_CONNECT,

    LINESTATE_CAUSE_NOT_AUTHORIZED,

    LINESTATE_CAUSE_TIMEOUT,

    LINESTATE_CAUSE_NOT_FOUND

} OWPL_LINESTATE_CAUSE;

/**
 * Linestate event information structure
 */
typedef struct                      
{
    size_t    nSize ;               /**< The size of this structure. */
    OWPL_LINE hLine;                /**< Line handle associated with the linestate event. */ 
    OWPL_LINESTATE_EVENT event ;    /**< Callstate event enum code.
                                         Identifies the linestate event. */
    OWPL_LINESTATE_CAUSE cause ;    /**< Callstate cause enum code. 
                                         Identifies the cause of the linestate event. */
    const char *szBodyBytes;        /**< msg body to extract reg body */

} OWPL_LINESTATE_INFO ;


typedef enum OWPL_ERROR_EVENT 
{ 
	OWPL_ERROR = PH_ERROR,    /* generic error */
	OWPL_ERROR_NO_AUDIO_DEVICE = PH_NOAUDIODEVICE /* failed to open an audio device */
} OWPL_ERROR_EVENT;

/**
 * Error event information structure
 */
typedef struct                      
{
    OWPL_ERROR_EVENT event ;		/**< ERROR event enum code. */
    const void *Data;				/**< Additional data related to the error event */
} OWPL_ERROR_INFO;


/**
 * Enumeration of the possible subscription states visible to the client.
 */
typedef enum 
{
    OWPL_SUBSCRIPTION_PENDING,      /**< THe subscription is being set up, but not yet active. */
	OWPL_SUBSCRIPTION_CLOSING,
    OWPL_SUBSCRIPTION_ACTIVE ,      /**< The subscription is currently active. */
	OWPL_SUBSCRIPTION_CLOSED,
    OWPL_SUBSCRIPTION_FAILED ,      /**< The subscription is not active due to a failure.*/
	OWPL_SUBSCRIPTION_CLOSE_FAILED,
    OWPL_SUBSCRIPTION_EXPIRED ,     /**< The subscription's lifetime has expired. */

    OWPL_INSUBSCRIPTION_NEW ,         /**< New incoming subscription */
    OWPL_INSUBSCRIPTION_CLOSE         /**< Incoming subscription closed */
    // TBD
} OWPL_SUBSCRIPTION_STATE;

/**
 * Enumeration of cause codes for state subscription state changes.
 */
typedef enum
{
    SUBSCRIPTION_CAUSE_UNKNOWN = -1, /**< No cause specified. */
    SUBSCRIPTION_CAUSE_NORMAL     /**< Normal cause for state change. */
} OWPL_SUBSCRIPTION_CAUSE;

/**
 * An SUBSTATUS event informs that application layer of the status
 * of an outbound SUBSCRIPTION requests;
 */
typedef struct 
{
	int                 nSize ;     /**< the size of this structure in bytes */
	OWPL_SUB			hSub;		/**< a handle to the subscription to which
									this state change occurred. */
	OWPL_LINE hLine;                /**< Line handle associated with the subscription event. */
	OWPL_SUBSCRIPTION_STATE state;	/**< Enum state value indicating the current
									state of the subscription. */
	OWPL_SUBSCRIPTION_CAUSE cause;	/**< Enum cause for the state change in this
									event. */
	const char * szRemoteIdentity;	/**< The identity of the remote party of this subscription. */
        const char * szEvtType;         /**< sip event type */
} OWPL_SUBSTATUS_INFO;

typedef enum {
	NOTIFICATION_UNKNOWN = 0,		/**< Unknown notify event */
	NOTIFICATION_PRESENCE = 1000,	/**< Notification of peer presence */
	NOTIFICATION_MWI = 2000			/**< Presence watcher event */
} OWPL_NOTIFICATION_EVENT;

typedef enum {
	NOTIFICATION_CAUSE_UNKNOWN = NOTIFICATION_UNKNOWN +1 ,
	NOTIFICATION_PARSE_ERROR = NOTIFICATION_UNKNOWN + 2,
	NOTIFICATION_PRESENCE_ONLINE = NOTIFICATION_PRESENCE + 1,	/**< Peer is on line */
	NOTIFICATION_PRESENCE_OFFLINE = NOTIFICATION_PRESENCE + 2,	/**< Peer is off line */
	NOTIFICATION_PRESENCE_WATCHER = NOTIFICATION_PRESENCE + 3			/**< Presence watcher */
} OWPL_NOTIFICATION_CAUSE;

typedef struct {
	size_t    nSize ;               /**< The size of this structure. */
	const char * szStatusNote;		/**< The note specifying a more accurate status. */
	const char * szRemoteIdentity;	/**< The identity of the remote party of this notification. */
} OWPL_NOTIFICATION_STATUS_INFO;

typedef struct {
	size_t    nSize ;               /**< The size of this structure. */
} OWPL_NOTIFICATION_WATCHER_INFO;

typedef struct {
	size_t    nSize ;               /**< The size of this structure. */
	OWPL_LINE hLine;                /**< Line handle associated with the MWI event. */
	int nUnreadMessageCount;		/**< The number of unread message. */
	int nReadMessageCount;			/**< The number of read message. */
	int nImpUnreadMessageCount;		/**< The number of important unread message. */
	int nImpReadMessageCount;		/**< The number of important read message. */
	const char * szMessageAccount;	/**< The account that these previous infos provided from */
} OWPL_NOTIFICATION_MWI_INFO;

typedef struct {
	size_t    nSize ;               /**< The size of this structure. */
	OWPL_LINE hLine;                /**< Line handle associated with the notification event. */
	OWPL_NOTIFICATION_EVENT	event;	/**< Subscription event enum code.
                                         Identifies the subsciption event. */
	OWPL_NOTIFICATION_CAUSE cause;	/**< Notification cause enum code.
										 Identifies the cause of the notification event */	
	const char * szXmlContent;		/**< The notify XML content */

	union {
		OWPL_NOTIFICATION_STATUS_INFO * StatusInfo;
		OWPL_NOTIFICATION_WATCHER_INFO * WatcherInfo;
		OWPL_NOTIFICATION_MWI_INFO * MWI;
	} Data;
} OWPL_NOTIFICATION_INFO;


typedef enum {
	MESSAGE_UNKNOWN = 0,	/**< Unknown message event */
	MESSAGE_NEW = 1000,		/**< A new message has been received */
	MESSAGE_SUCCESS = 2000,	/**< The sent message reached its destination */
	MESSAGE_FAILURE = 3000	/**< The sent message could not be delivered */
} OWPL_MESSAGE_EVENT;

typedef enum {
	MESSAGE_NEW_NORMAL = MESSAGE_NEW+1,
	MESSAGE_NEW_BUDDY_ICON = MESSAGE_NEW+2,
	MESSAGE_NEW_TYPING = MESSAGE_NEW+3,
	MESSAGE_NEW_STOP_TYPING = MESSAGE_NEW+4,
	MESSAGE_NEW_NOT_TYPING = MESSAGE_NEW+5,

	MESSAGE_SUCCESS_NORMAL = MESSAGE_SUCCESS+1,

	MESSAGE_FAILURE_UNKNOWN = MESSAGE_FAILURE+1,
	MESSAGE_FAILURE_COULD_NOT_SEND = MESSAGE_FAILURE+2
} OWPL_MESSAGE_CAUSE;

typedef struct {
	size_t    nSize ;               /**< The size of this structure. */
	OWPL_MESSAGE_EVENT event;		/**< Message event enum code.
                                         Identifies the subsciption event. */
	OWPL_MESSAGE_CAUSE cause;		/**< Subscription cause enum code.
                                         Identifies the subsciption event. */
	int messageId;					/**< The message identifier */
	OWPL_LINE hLine;                /**< Line handle associated with the message event. */
	const char * szContent;			/**< The content of the message */
	const char * szLocalIdentity;	/**< The identity of the local party of this message. */
	const char * szRemoteIdentity;	/**< The identity of the remote party of this message. */
	const char * szContentType;		/**< The content type of the message (first part of the MIME type) */
	const char * szSubContentType;	/**< The sub content type of the message (second part of the MIME type) */
} OWPL_MESSAGE_INFO;

/**
 * Signature for event callback/observer.  Application developers should
 * not block the calling thread.
 *
 * @param category The category of the event (call, line, subscription, notify, etc.).
 * @param pInfo Pointer to the event info structure.
 * @param pUserData User data provided when listener was added
 */
typedef int (*OWPL_EVENT_CALLBACK_PROC)(OWPL_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData);


/**
 * Signature for event callback/observer that has the stdcall calling convention
 * Application developers should not block the calling thread.
 *
 * @param category The category of the event (call, line, subscription, notify, etc.).
 * @param pInfo Pointer to the event info structure.
 * @param pUserData User data provided when listener was added
 */
 
#ifdef WIN32
typedef int (__stdcall * OWPL_CALLBACK_PROC_STDCALL)(OWPL_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData);
#else
typedef int (* OWPL_CALLBACK_PROC_STDCALL)(OWPL_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData);
#endif


/* ============================ FUNCTIONS ================================= */

/**
 * Add a event callback listener
 *
 * @param	pCallbackProc	The pointer to the callback function
 * @param	pUserData		The data that will be passed back to the callback function
 * @return	an OWPL_RESULT
 */
MY_DLLEXPORT OWPL_RESULT owplEventListenerAdd(OWPL_EVENT_CALLBACK_PROC pCallbackProc,
                                             void *pUserData);

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
                                             void *pUserData);


/**
 * Remove a owpl event callback/observer.  Supply the same
 * pCallbackProc and pUserData values as sipxEventListenerAdd.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function used to receive sipx events
 * @param pUserData user data specified as part of sipxListenerAdd
 */
MY_DLLEXPORT OWPL_RESULT owplEventListenerRemove(OWPL_EVENT_CALLBACK_PROC pCallbackProc);

/****************  INTERNAL USAGE FUNCTIONS ****************************/

OWPL_RESULT
owplEventListenerRemoveAll();

/**
 * This function fires an event to all subscribed listeners
 *
 */

OWPL_RESULT
owplFireEvent(OWPL_EVENT_CATEGORY Category, 
                                            void* pInfo);


/**
 * owplFireCallEvent: creates a call event and send it to all subscribers
 *
 */
OWPL_RESULT
owplFireCallEvent(OWPL_CALL hCall, 
					OWPL_CALLSTATE_EVENT event, 
					OWPL_CALLSTATE_CAUSE cause,
					const char * szRemoteIdentity,
					OWPL_CALL hAssociatedCall);

/**
 * owplFireCallEvent2 does the same thing as owplFireCallEvent except that it accepts more parametters
 *
 */
OWPL_RESULT
owplFireCallEvent2(OWPL_CALL hCall, 
					OWPL_CALLSTATE_EVENT event, 
					OWPL_CALLSTATE_CAUSE cause,
					const char * szRemoteIdentity,
					int nData,
					const void *pData,
					OWPL_CALL hAssociatedCall);

/**
 * This function creates a line event and send it to all subscribers
 *
 */
OWPL_RESULT
owplFireLineEvent(OWPL_LINE hLine,
						OWPL_LINESTATE_EVENT event,
						OWPL_LINESTATE_CAUSE cause,
						const char *szBodyBytes);        /**< msg body to extract reg body */

/**
 * This function creates an error event and send it to all subscribers
 *
 */
OWPL_RESULT
owplFireErrorEvent(OWPL_ERROR_EVENT event,
						void * Data);
/**
 * Creates a subscription state structure and sends it to all subscribers
 *
 * @param	hSub	a subscription handle
 * @param	state	the subscription state
 * @param	cause	the cause of this event
 * @param	szRemoteIdentity	the identity of the remote party of this notification
 * @param	szEvtType	        sip event type for subscription event
 * @return	an OWPL_RESULT...
 */
OWPL_RESULT
owplFireSubscriptionEvent(OWPL_SUB hSub,
						  OWPL_SUBSCRIPTION_STATE state,
						  OWPL_SUBSCRIPTION_CAUSE cause,
			  const char* szRemoteIdentity,
						  const char* szEvtType);

/**
 * Creates a notification state structure and sends it to all subscribers
 *
 * @param	event	the notification event type
 * @param	szXmlContent	the notify XML content 
 * @param	szRemoteIdentity	the identity of the remote party of this notification
 * @return	an OWPL_RESULT...
 */
OWPL_RESULT
owplFireNotificationEvent(OWPL_NOTIFICATION_EVENT event,
						  OWPL_NOTIFICATION_CAUSE cause,
						  const char* szXmlContent,
						  const char* szRemoteIdentity);

OWPL_RESULT
owplNotificationPresenceGetIdentity(const char * notify, char * buffer, size_t size);

OWPL_RESULT
owplNotificationPresenceGetStatus(const char * notify, char * buffer, size_t size);

OWPL_RESULT
owplNotificationPresenceGetNote(const char * notify, char * buffer, size_t size);

OWPL_RESULT
owplNotificationMWIGetInfos(const char * szContent, 
							char * szMessAccBuff, size_t szMessAccBuffSize, 
							int * unreadMessageCount, int * readMessageCount,
							int * impUnreadMessageCount, int * impReadMessageCount);

/**
 * Creates a notification state structure and sends it to all subscribers
 *
 * @param	event	the notification event type
 * @param	cause	the cause of this event
 * @param	szMessage	the message string itself
 * @param	szLocalIdentity	the identity of the local party of this message
 * @param	szRemoteIdentity	the identity of the remote party of this message
 * @param	szBuddyIcon	used for the buddy icon message
 * @return	an OWPL_RESULT...
 */
OWPL_RESULT
owplFireMessageEvent(OWPL_MESSAGE_EVENT event,
					 OWPL_MESSAGE_CAUSE cause,
					 const int messageId,
					 const OWPL_LINE hLine,
					 const char * szContent,
					 const char * szLocalIdentity,
					 const char * szRemoteIdentity,
					 const char * szContentType,
					 const char * szSubContentType);


//typedef struct owpl_plugin;

OWPL_RESULT
owplFireCallEvent2Plugin(struct owpl_plugin * plugin,
					OWPL_CALL hCall, 
					OWPL_CALLSTATE_EVENT event, 
					OWPL_CALLSTATE_CAUSE cause,
					const char * szRemoteIdentity,
					int nData,
					const void *pData,
					OWPL_CALL hAssociatedCall);


/*=====================================================================================*
 *                                  INTERNAL FUNCTIONS                                 *
 *=====================================================================================*/

/*  eXosip dependent routines */ 
struct eXosip_event;

void owplFireExosipCallEvent(struct eXosip_event *je);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif

